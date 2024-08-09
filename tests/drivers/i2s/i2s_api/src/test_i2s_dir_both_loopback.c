/*
 * Copyright (c) 2017 comsuisse AG
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/i2s.h>
#include "i2s_api_test.h"

/* The test cases here are copied from test_i2s_loopback.c and adapted for use
 * on devices that cannot independently start and stop the RX and TX streams
 * and require the use of the I2S_DIR_BOTH value for RX/TX transfers.
 */


#define TEST_I2S_TRANSFER_LONG_REPEAT_COUNT  100

/** @brief Long I2S transfer.
 *
 * - START trigger starts both the transmission and reception.
 * - Sending / receiving a long sequence of data returns success.
 * - DRAIN trigger empties the transmit queue and stops both streams.
 */
ZTEST_USER(i2s_dir_both_loopback, test_i2s_dir_both_transfer_long)
{
	extern void test_slab_init(void);
	test_slab_init();
	if (!dir_both_supported) {
		TC_PRINT("I2S_DIR_BOTH value is not supported.\n");
		ztest_test_skip();
		return;
	}

	int ret;

	//static volatile bool wait = true;
	//while(wait);
	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS);

	ret = i2s_trigger(dev_i2s, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX/TX START trigger failed\n");

	for (int i = 0; i < TEST_I2S_TRANSFER_LONG_REPEAT_COUNT; i++) {
		//printk("X%d\n", i);
		ret = tx_block_write(dev_i2s, 0, 0);
		zassert_equal(ret, TC_PASS);

		ret = rx_block_read(dev_i2s, 0);
		zassert_equal(ret, TC_PASS);
	}

	/* All data written, all but one data block read, flush TX queue
	 * and stop both streams.
	 */
	ret = i2s_trigger(dev_i2s, I2S_DIR_BOTH, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "RX/TX DRAIN trigger failed");

	ret = rx_block_read(dev_i2s, 0);
	zassert_equal(ret, TC_PASS);

	/* TODO: Verify the interface is in READY state when i2s_state_get
	 * function is available.
	 */
}
