/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
//TODO to be removed when bt transfer is implemented
#include <zephyr/random/rand32.h>
#include <stdio.h>

#include "adc.h"
#include "build_in_leds.h"
#include "ade9000.h"
#include "ade9000Estimates.h"
#include "pcal6408apwj.h"
#include "ade9000Interface.h"

//BLE
#include "ble_gatt_service.h"

/* size of stack area used by each thread */
#define STACKSIZE 1024
#define MEAS_PRIORITY 0
#define BLUETOOTH_PRIORITY 1

typedef struct {
	uint32_t counter;
	ade_est_data_t read_estimates;
} bt_data_queue_t;

static K_SEM_DEFINE(data_ready, 0, CONFIG_ESTIMATES_QUEUE_SIZE);

K_MEM_SLAB_DEFINE_STATIC(bt_mem_slab, sizeof(bt_data_queue_t), CONFIG_ESTIMATES_QUEUE_SIZE, 4);

#define ERROR_REPORT(str)   \
	printk(str);            \
	while(1) {              \
		k_sleep(K_FOREVER); \
	}


LOG_MODULE_REGISTER(logging_blog, LOG_LEVEL_DBG);

static adc_t ADE9000 = ADC_INITIALIZE_STRUCT(ADE9000Init,
					     ADE9000MeasParamsSet,
					     ADE9000ConversionStart,
					     NULL,
					     ADE9000EstimatesEstimate1sRead,
					     ADE9000EstimatesAllEstimates1sRead);


static bt_data_queue_t *bt_data_queue[CONFIG_ESTIMATES_QUEUE_SIZE];

static void channel_names_assign(void)
{
#if IS_ENABLED(CONFIG_ADC_CHANNEL_NAMES_USE)
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 0, "I1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 1, "U1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 2, "I2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 3, "U2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 4, "I3");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 5, "U3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 6, "P1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 7, "P2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 8, "P3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 9, "Q1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 10, "Q2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 11, "Q3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 12, "S1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 13, "S2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 14, "S3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 15, "EA1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 16, "EA2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 17, "EA3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 18, "ER1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 19, "ER2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 20, "ER3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 21, "A1");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 22, "A2");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 23, "A3");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 24, "U12");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 25, "U23");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 26, "U31");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 27, "Imean");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 28, "Umean");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 29, "Psum");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 30, "Qsum");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 31, "Ssum");

	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 32, "EAsum");
	ADC_CHANNEL_NAME_ASSIGN(ADE9000, 33, "ERsum");

#endif // IS_ENABLED(CONFIG_ADC_CHANNEL_NAMES_USE)
}

void estimates_meas(void)
{
	uint32_t cnt = 0;
	int ret;
	uint8_t queue_pos = 0;
	uint64_t timestamp = 0;
	// TODO: error handling
	init_leds();
	(void)test_leds();

	if (!ADE9000.initialize(NULL)) {
		ERROR_REPORT("Error while initializing ADE9000\n");
	}
	channel_names_assign();
	(void)ADE9000.configure(NULL);
	if (!ADE9000.start(NULL))
	{
		ERROR_REPORT("ADE9000MeasParamsSet() Error\n");
	}

	while (1) {
		k_sleep(K_SECONDS(1));
#ifdef HAS_BUILDIN_PWN_DIDOES
		pwm_set_pulse_dt(&red_pwm_led, 20000000);
#endif
		/* Allocate memory slab for estimates data. */
		ret = k_mem_slab_alloc(&bt_mem_slab,
				       (void **)&bt_data_queue[queue_pos],
				       K_MSEC(1000));
		if(ret != 0) {
			ERROR_REPORT("Error while allocating slab\n");
		}
		timestamp = k_uptime_get();
		printk("----%u Reading estimates at to %u to %u frame\n", (uint32_t)timestamp, queue_pos, cnt);
		if (!ADE9000.all_channels_read(&bt_data_queue[queue_pos]->read_estimates)) {
			ERROR_REPORT("Error reading estimates\n");
		}
		bt_data_queue[queue_pos]->counter = cnt ++;
		bt_data_queue[queue_pos]->read_estimates.Timestamp = timestamp;
		k_sem_give(&data_ready);
		queue_pos  = (queue_pos + 1) % CONFIG_ESTIMATES_QUEUE_SIZE;

	}
}

static bool my_bt_send_function(void * data, size_t num_of_bytes)
{
	static uint32_t frame_counter_check = 0;
	bt_data_queue_t * bt_data = (bt_data_queue_t *)data;
	// int32_t *p_estimate = (int32_t *)&bt_data->read_estimates;
	printk("Frame %u\n", bt_data->counter);
	estimates_notify(data, num_of_bytes);
	// for (uint8_t i = 0; i < CONFIG_ADC_MAX_NUMBER_OF_CHANNELS; i++) {
	// 	printk("%s = %d\n", ADC_CHANNEL_NAME_GET(ADE9000, i), p_estimate[i]);
	// }
	printk("Frame check %u / %u\n", bt_data->counter, frame_counter_check);
	if (frame_counter_check != bt_data->counter) {
		k_sleep(K_FOREVER);
	}
	frame_counter_check ++;
	return true;
}

void bluetooth_comm(void)
{
	bt_innit();
	// if(!bt_innit()){
	// 	printk("Bluetooth initialized\n");
	// } else {
	// 	ERROR_REPORT("Bluetooth  failed\n");
	// }
	//TODO to be removed when bt transfer is added
	const uint32_t max_bt_delay_ms = 100;
	uint32_t transfer_delay_sim;

	uint8_t queue_pos = 0;

	while(1) {
		k_sem_take(&data_ready, K_FOREVER);
		if (bt_data_queue[queue_pos] == NULL) {
			ERROR_REPORT("Error: mem_block is NULL");
		}
		printk("-----------S: %u / %u\n", k_sem_count_get(&data_ready), CONFIG_ESTIMATES_QUEUE_SIZE);
		transfer_delay_sim = sys_rand32_get() % max_bt_delay_ms;
		printk("-----------transfer delay: %u ms. \n", transfer_delay_sim);
		k_msleep(transfer_delay_sim);
		my_bt_send_function((void*)bt_data_queue[queue_pos], sizeof(bt_data_queue_t));
		k_mem_slab_free(&bt_mem_slab, (void **)&bt_data_queue[queue_pos]);
		queue_pos  = (queue_pos + 1) % CONFIG_ESTIMATES_QUEUE_SIZE;
	}
}

K_THREAD_DEFINE(meas_id, STACKSIZE, estimates_meas, NULL, NULL, NULL,
		MEAS_PRIORITY, 0, 0);
K_THREAD_DEFINE(bt_id, STACKSIZE, bluetooth_comm, NULL, NULL, NULL,
		BLUETOOTH_PRIORITY, 0, 0);

