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

#include "adc.h"
#include "build_in_leds.h"
#include "ade9000.h"
#include "ade9000Estimates.h"
#include "pcal6408apwj.h"
#include "ade9000Interface.h"
//TODO: temporary
#include "sram.h"

// TODO:
#define NB_OF_ESIMATES_TO_GET 16

#define ERROR_REPORT(str)   \
	printk(str);            \
	while(1) {              \
		k_sleep(K_FOREVER); \
	}


LOG_MODULE_REGISTER(logging_blog, LOG_LEVEL_DBG);

static adc_t ADE9000 = ADC_INITIALIZE_STRUCT(
							ADE9000Init,
							ADE9000MeasParamsSet,
							ADE9000ConversionStart,
							NULL,
							ADE9000EstimatesEstimate1sRead);

void main(void)
{

	// TODO: error handling
	init_leds();
	(void)test_leds();

    if (!SRAMInit()) {
		ERROR_REPORT("Error while initializing SRAM\n");
	}

	SRAMReset();
	k_sleep(K_MSEC(100));
	if (!SRAMTest()) {
		ERROR_REPORT("Error while testing SRAM\n");
	}

	if (!ADE9000.initialize(NULL)) {
		ERROR_REPORT("Error while initializing ADE9000\n");
	}
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
		uint32_t estimates[NB_OF_ESIMATES_TO_GET];
		for (uint8_t i = 0; i < NB_OF_ESIMATES_TO_GET; i ++) {
			if (!ADE9000EstimatesEstimate1sRead(i, &estimates[i])) {
				printk("Error reading estimates\n");
				continue;
			}
			//printk("E(%u) = %d\n", i, estimate);
		}
	}
}
