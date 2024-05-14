/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/drivers/timer/nrf_grtc_timer.h>

/* Between 1 and 255. */
#define CLKFAST_DIV 255

int main(void)
{
	z_nrf_grtc_clkout32_set(true);
	z_nrf_grtc_clkfast_set(true, CLKFAST_DIV);
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	return 0;
}

