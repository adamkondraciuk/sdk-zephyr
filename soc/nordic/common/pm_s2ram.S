/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 /**
 * @file
 * @brief Nordic suspend-to-RAM code (S2RAM)
 */

#include <zephyr/toolchain.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/common/pm_s2ram.h>

//#define RESETINFO_ADDRESS 0x5201E4A4
#define RESETREAS_OFFSET    0x4A4
#define RESTOREVALID_OFFSET 0x4C0

GDATA(nrf_resetinfo_base)

GTEXT(pm_s2ram_mark_set)
SECTION_FUNC(TEXT, pm_s2ram_mark_set)
	mov r15, r1

GTEXT(pm_s2ram_mark_check_and_clear)
SECTION_FUNC(TEXT, pm_s2ram_mark_check_and_clear)
	mov	r0, #0
	/* Load RESETREAS.LOCAL address into R2. */
	ldr	r3, =nrf_resetinfo_base
	/* Check RESETREAS. */
	ldr 	r2, [r3]
	ldr 	r4, =RESETREAS_OFFSET
	add	r4, r2
	ldr 	r3, [r4]
	cmp	r3, #16 //NRF_RESETINFO_RESETREAS_LOCAL_UNRETAINED_MASK
	bne	exit

	/* Clear RESETREAS */
	str	r0, [r4]

	/* Check and clear RESETVALID. */
	ldr 	r4, =RESTOREVALID_OFFSET
	add	r4, r2
	/* Load RESTOREVALID value into R3. */
	ldr 	r3, [r4]
	// nrf_resetinfo_restore_valid_set(NRF_RESETINFO, false);
	str	r0, [r4]
	cmp	r3, #1 //RESTOREVALID bit pos
	bne	exit
exit_and_resume:
	/* move return address to PC. */
	mov r15, r1
exit:
	bx	lr
