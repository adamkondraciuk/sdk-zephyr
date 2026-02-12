/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

int main(void)
{
	printk("Hello world from %s\n", CONFIG_BOARD_TARGET);

	// hibernate
while(1)
{
	k_msleep(1100);
	k_msleep(49);
}	

	// idle
	//k_msleep(5);



	//k_msleep(90);

	//arch_cpu_idle();
	return 0;
}
