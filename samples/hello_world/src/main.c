/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/drivers/timer/nrf_grtc_timer.h>
#include <zephyr/kernel.h>


int main(void)
{
	while(1) {
		printf("Hello World! %s %llu\n", CONFIG_BOARD_TARGET, z_nrf_grtc_timer_read());
		k_msleep(1000);
	}
	return 0;
}
