/*
 * Copyright (c) 2017, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include <xtensa-asm2.h>
#include <zephyr/kernel.h>
#include <ksched.h>
#include <zephyr/kernel_structs.h>
#include <kernel_internal.h>
#include <kswap.h>
#include <_soc_inthandlers.h>
#include <zephyr/toolchain.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

extern char xtensa_arch_except_epc[];

void *xtensa_init_stack(struct k_thread *thread, int *stack_top,
			void (*entry)(void *, void *, void *),
			void *arg1, void *arg2, void *arg3)
{
	/* Not-a-cpu ID Ensures that the first time this is run, the
	 * stack will be invalidated.  That covers the edge case of
	 * restarting a thread on a stack that had previously been run
	 * on one CPU, but then initialized on this one, and
	 * potentially run THERE and not HERE.
	 */
	thread->arch.last_cpu = -1;

	/* We cheat and shave 16 bytes off, the top four words are the
	 * A0-A3 spill area for the caller of the entry function,
	 * which doesn't exist.  It will never be touched, so we
	 * arrange to enter the function with a CALLINC of 1 and a
	 * stack pointer 16 bytes above the top, so its ENTRY at the
	 * start will decrement the stack pointer by 16.
	 */
	const int bsasz = BASE_SAVE_AREA_SIZE - 16;
	void *ret, **bsa = (void **) (((char *) stack_top) - bsasz);

	(void)memset(bsa, 0, bsasz);

	bsa[BSA_PC_OFF/4] = z_thread_entry;
	bsa[BSA_PS_OFF/4] = (void *)(PS_WOE | PS_UM | PS_CALLINC(1));

#if XCHAL_HAVE_THREADPTR && defined(CONFIG_THREAD_LOCAL_STORAGE)
	bsa[BSA_THREADPTR_OFF/4] = UINT_TO_POINTER(thread->tls);
#endif

	/* Arguments to z_thread_entry().  Remember these start at A6,
	 * which will be rotated into A2 by the ENTRY instruction that
	 * begins the C function.  And A4-A7 and A8-A11 are optional
	 * quads that live below the BSA!
	 */
	bsa[-1] = arg1;  /* a7 */
	bsa[-2] = entry; /* a6 */
	bsa[-3] = 0;     /* a5 */
	bsa[-4] = 0;     /* a4 */

	bsa[-5] = 0;     /* a11 */
	bsa[-6] = 0;     /* a10 */
	bsa[-7] = arg3;  /* a9 */
	bsa[-8] = arg2;  /* a8 */

	/* Finally push the BSA pointer and return the stack pointer
	 * as the handle
	 */
	bsa[-9] = bsa;
	ret = &bsa[-9];

	return ret;
}

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack,
		     char *stack_ptr, k_thread_entry_t entry,
		     void *p1, void *p2, void *p3)
{
	thread->switch_handle = xtensa_init_stack(thread,
						  (int *)stack_ptr, entry,
						  p1, p2, p3);
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT((((size_t)stack) % XCHAL_DCACHE_LINESIZE) == 0, "");
	__ASSERT((((size_t)stack_ptr) % XCHAL_DCACHE_LINESIZE) == 0, "");
	z_xtensa_cache_flush_inv(stack, (char *)stack_ptr - (char *)stack);
#endif
}

void z_irq_spurious(const void *arg)
{
	int irqs, ie;

	ARG_UNUSED(arg);

	__asm__ volatile("rsr.interrupt %0" : "=r"(irqs));
	__asm__ volatile("rsr.intenable %0" : "=r"(ie));
	LOG_ERR(" ** Spurious INTERRUPT(s) %p, INTENABLE = %p",
		(void *)irqs, (void *)ie);
	z_xtensa_fatal_error(K_ERR_SPURIOUS_IRQ, NULL);
}

void z_xtensa_dump_stack(const z_arch_esf_t *stack)
{
	int *bsa = *(int **)stack;

	LOG_ERR(" **  A0 %p  SP %p  A2 %p  A3 %p",
		(void *)bsa[BSA_A0_OFF/4],
		((char *)bsa) + BASE_SAVE_AREA_SIZE,
		(void *)bsa[BSA_A2_OFF/4], (void *)bsa[BSA_A3_OFF/4]);

	if (bsa - stack > 4) {
		LOG_ERR(" **  A4 %p  A5 %p  A6 %p  A7 %p",
			(void *)bsa[-4], (void *)bsa[-3],
			(void *)bsa[-2], (void *)bsa[-1]);
	}

	if (bsa - stack > 8) {
		LOG_ERR(" **  A8 %p  A9 %p A10 %p A11 %p",
			(void *)bsa[-8], (void *)bsa[-7],
			(void *)bsa[-6], (void *)bsa[-5]);
	}

	if (bsa - stack > 12) {
		LOG_ERR(" ** A12 %p A13 %p A14 %p A15 %p",
			(void *)bsa[-12], (void *)bsa[-11],
			(void *)bsa[-10], (void *)bsa[-9]);
	}

#if XCHAL_HAVE_LOOPS
	LOG_ERR(" ** LBEG %p LEND %p LCOUNT %p",
		(void *)bsa[BSA_LBEG_OFF/4],
		(void *)bsa[BSA_LEND_OFF/4],
		(void *)bsa[BSA_LCOUNT_OFF/4]);
#endif

	LOG_ERR(" ** SAR %p", (void *)bsa[BSA_SAR_OFF/4]);
}

static inline unsigned int get_bits(int offset, int num_bits, unsigned int val)
{
	int mask;

	mask = BIT(num_bits) - 1;
	val = val >> offset;
	return val & mask;
}

static ALWAYS_INLINE void usage_stop(void)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
#endif
}

static inline void *return_to(void *interrupted)
{
	return _current_cpu->nested <= 1 ?
		z_get_next_switch_handle(interrupted) : interrupted;
}

/* The wrapper code lives here instead of in the python script that
 * generates _xtensa_handle_one_int*().  Seems cleaner, still kind of
 * ugly.
 *
 * This may be unused depending on number of interrupt levels
 * supported by the SoC.
 */
#define DEF_INT_C_HANDLER(l)				\
__unused void *xtensa_int##l##_c(void *interrupted_stack)	\
{							   \
	uint32_t irqs, intenable, m;			   \
	usage_stop();					   \
	__asm__ volatile("rsr.interrupt %0" : "=r"(irqs)); \
	__asm__ volatile("rsr.intenable %0" : "=r"(intenable)); \
	irqs &= intenable;					\
	while ((m = _xtensa_handle_one_int##l(irqs))) {		\
		irqs ^= m;					\
		__asm__ volatile("wsr.intclear %0" : : "r"(m)); \
	}							\
	return return_to(interrupted_stack);		\
}

#if XCHAL_NMILEVEL >= 2
DEF_INT_C_HANDLER(2)
#endif

#if XCHAL_NMILEVEL >= 3
DEF_INT_C_HANDLER(3)
#endif

#if XCHAL_NMILEVEL >= 4
DEF_INT_C_HANDLER(4)
#endif

#if XCHAL_NMILEVEL >= 5
DEF_INT_C_HANDLER(5)
#endif

#if XCHAL_NMILEVEL >= 6
DEF_INT_C_HANDLER(6)
#endif

#if XCHAL_NMILEVEL >= 7
DEF_INT_C_HANDLER(7)
#endif

static inline DEF_INT_C_HANDLER(1)

/* C handler for level 1 exceptions/interrupts.  Hooked from the
 * DEF_EXCINT 1 vector declaration in assembly code.  This one looks
 * different because exceptions and interrupts land at the same
 * vector; other interrupt levels have their own vectors.
 */
void *xtensa_excint1_c(int *interrupted_stack)
{
	int cause, vaddr, *bsa = *(int **)interrupted_stack;
	bool is_fatal_error = false;

	__asm__ volatile("rsr.exccause %0" : "=r"(cause));

	if (cause == EXCCAUSE_LEVEL1_INTERRUPT) {
		return xtensa_int1_c(interrupted_stack);
	} else if (cause == EXCCAUSE_SYSCALL) {
		/* Just report it to the console for now */
		LOG_ERR(" ** SYSCALL PS %p PC %p",
			(void *)bsa[BSA_PS_OFF/4], (void *)bsa[BSA_PC_OFF/4]);
		z_xtensa_dump_stack(interrupted_stack);

		/* Xtensa exceptions don't automatically advance PC,
		 * have to skip the SYSCALL instruction manually or
		 * else it will just loop forever
		 */
		bsa[BSA_PC_OFF/4] += 3;
	} else {
		uint32_t ps = bsa[BSA_PS_OFF/4];
		void *pc = (void *)bsa[BSA_PC_OFF/4];

		__asm__ volatile("rsr.excvaddr %0" : "=r"(vaddr));

		/* Default for exception */
		int reason = K_ERR_CPU_EXCEPTION;
		is_fatal_error = true;

		/* We need to distinguish between an ill in xtensa_arch_except,
		 * e.g for k_panic, and any other ill. For exceptions caused by
		 * xtensa_arch_except calls, we also need to pass the reason_p
		 * to z_xtensa_fatal_error. Since the ARCH_EXCEPT frame is in the
		 * BSA, the first arg reason_p is stored at the A2 offset.
		 * We assign EXCCAUSE the unused, reserved code 63; this may be
		 * problematic if the app or new boards also decide to repurpose
		 * this code.
		 */
		if ((pc ==  (void *) &xtensa_arch_except_epc) && (cause == 0)) {
			cause = 63;
			__asm__ volatile("wsr.exccause %0" : : "r"(cause));
			reason = bsa[BSA_A2_OFF/4];
		}

		LOG_ERR(" ** FATAL EXCEPTION");
		LOG_ERR(" ** CPU %d EXCCAUSE %d (%s)",
			arch_curr_cpu()->id, cause,
			z_xtensa_exccause(cause));
		LOG_ERR(" **  PC %p VADDR %p",
			pc, (void *)vaddr);
		LOG_ERR(" **  PS %p", (void *)bsa[BSA_PS_OFF/4]);
		LOG_ERR(" **    (INTLEVEL:%d EXCM: %d UM:%d RING:%d WOE:%d OWB:%d CALLINC:%d)",
			get_bits(0, 4, ps), get_bits(4, 1, ps),
			get_bits(5, 1, ps), get_bits(6, 2, ps),
			get_bits(18, 1, ps),
			get_bits(8, 4, ps), get_bits(16, 2, ps));

		/* FIXME: legacy xtensa port reported "HW" exception
		 * for all unhandled exceptions, which seems incorrect
		 * as these are software errors.  Should clean this
		 * up.
		 */
		z_xtensa_fatal_error(reason,
				     (void *)interrupted_stack);
	}

	if (is_fatal_error) {
		uint32_t ignore;

		/* We are going to manipulate _current_cpu->nested manually.
		 * Since the error is fatal, for recoverable errors, code
		 * execution must not return back to the current thread as
		 * it is being terminated (via above z_xtensa_fatal_error()).
		 * So we need to prevent more interrupts coming in which
		 * will affect the nested value as we are going outside of
		 * normal interrupt handling procedure.
		 *
		 * Setting nested to 1 has two effects:
		 * 1. Force return_to() to choose a new thread.
		 *    Since the current thread is being terminated, it will
		 *    not be chosen again.
		 * 2. When context switches to the newly chosen thread,
		 *    nested must be zero for normal code execution,
		 *    as that is not in interrupt context at all.
		 *    After returning from this function, the rest of
		 *    interrupt handling code will decrement nested,
		 *    resulting it being zero before switching to another
		 *    thread.
		 */
		__asm__ volatile("rsil %0, " STRINGIFY(XCHAL_NMILEVEL)
				: "=r" (ignore) : : );

		_current_cpu->nested = 1;
	}

	return return_to(interrupted_stack);
}

#if defined(CONFIG_GDBSTUB)
void *xtensa_debugint_c(int *interrupted_stack)
{
	extern void z_gdb_isr(z_arch_esf_t *esf);

	z_gdb_isr((void *)interrupted_stack);

	return return_to(interrupted_stack);
}
#endif

int z_xtensa_irq_is_enabled(unsigned int irq)
{
	uint32_t ie;

	__asm__ volatile("rsr.intenable %0" : "=r"(ie));

	return (ie & (1 << irq)) != 0U;
}
