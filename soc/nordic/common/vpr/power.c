/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/toolchain.h>
#include <zephyr/pm/policy.h>
#include <hal/nrf_memconf.h>

#define VPR_POWER_IDX 1
#define VPR_RET_BIT   MEMCONF_POWER_RET_MEM0_Pos
#define VPR_SLEEP_SUBSTATE_WAIT 0
#define VPR_SLEEP_SUBSTATE_SLEEP 0
#define VPR_SLEEP_SUBSTATE_HIBERNATE 1
#define VPR_SLEEP_SUBSTATE_DEEPSLEEP 2


static inline void pm_go_to_wait(void)
{
	csr_write(VPRCSR_NORDIC_VPRNORDICSLEEPCTRL,
		  VPRCSR_NORDIC_VPRNORDICSLEEPCTRL_SLEEPSTATE_WAIT);
	nrf_barrier_w();
	arch_cpu_idle();
}

static inline void pm_go_to_sleep(void)
{
	//while(1);
	csr_write(VPRCSR_NORDIC_VPRNORDICSLEEPCTRL,
		  VPRCSR_NORDIC_VPRNORDICSLEEPCTRL_SLEEPSTATE_SLEEP);
	nrf_barrier_w();
	arch_cpu_idle();
}

static inline void pm_go_to_hibernate(void)
{
	nrf_memconf_ramblock_ret_enable_set(NRF_MEMCONF, VPR_POWER_IDX, VPR_RET_BIT, true);
	csr_write(VPRCSR_NORDIC_VPRNORDICSLEEPCTRL,
		  VPRCSR_NORDIC_VPRNORDICSLEEPCTRL_SLEEPSTATE_HIBERNATE);
	nrf_barrier_w();
	arch_cpu_idle();
}

void pm_state_set(enum pm_state state, uint8_t substate_id)
{
	if (state == PM_STATE_SUSPEND_TO_RAM) {
		pm_go_to_hibernate();
	} else if (state == PM_STATE_SUSPEND_TO_IDLE) {
		pm_go_to_sleep();
	} else {
		k_cpu_idle();
	}
}

void pm_state_exit_post_ops(enum pm_state state, uint8_t substate_id)
{
	ARG_UNUSED(substate_id);

	csr_write(VPRCSR_NORDIC_VPRNORDICSLEEPCTRL,
		  VPRCSR_NORDIC_VPRNORDICSLEEPCTRL_SLEEPSTATE_SLEEP);
	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
		nrf_memconf_ramblock_ret_enable_set(NRF_MEMCONF, VPR_POWER_IDX, VPR_RET_BIT, false);
		break;
	case PM_STATE_STANDBY:
	case PM_STATE_SUSPEND_TO_IDLE:
	case PM_STATE_SUSPEND_TO_DISK:
	case PM_STATE_SOFT_OFF:
	case PM_STATE_COUNT:
	case PM_STATE_ACTIVE:
	case PM_STATE_RUNTIME_IDLE:
		break;
	}

	/* unlock interrupts after sleep */
	irq_unlock(MSTATUS_IEN);
}
