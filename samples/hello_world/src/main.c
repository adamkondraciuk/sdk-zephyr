/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
// #include <zephyr.h>
#define NRFX_TWIM_ENABLED 1
#define NRFX_TWI_ENABLED 1
#define TWI0_ENABLED 1
// #define TWI0_USE_EASY_DMA 1

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
// #include <zephyr/drivers/uart.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/util.h>

#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>


#include <nrfx_twi.h>
#include <nrfx_twim.h>
// #include <nrfx_log.h>
//#define HAS_BUILDIN_PWN_DIDOES
#include "build_in_leds.h"

#include <zephyr/logging/log.h>

#include "ade9000.h"
#include "pcal6408apwj.h"



// LOG_MODULE_REGISTER(logging_blog);
LOG_MODULE_REGISTER(logging_blog, LOG_LEVEL_DBG);

struct device *dev;

int use_my_variable(int _var){
	_var += 1;
	_var -= 1;
	return _var;
}


void main(void)
{
	if (!device_is_ready(led.port)) {
		return;
	}
	int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}
	gpio_pin_toggle_dt(&led);

	// const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

	// #if IS_ENABLED(CONFIG_USB_DEVICE_STACK_NEXT)
	// 	if (enable_usb_device_next()) {
	// 		return;
	// 	}
	// #else
	// 	if (usb_enable(NULL)) {
	// 		return;
	// 	}
	// #endif

	
	k_sleep(K_SECONDS(1));

	// init_leds();
	// unsigned unit8_t = test_leds();


    
	if (!ADE9000Init()) {
		printk("Error while initializing ADE9000\n");
	}
	ADE9000MeasParamsSet();
	ADE9000ConversionStart();
	//read_register(R_AIRMS, m_rx_buffer, 4);
	
	//ADE9000StreamRead();
	// printk("status code %d \n" , err_code);
	k_sleep(K_SECONDS(5));
	//for(int i = 0; i < 10;i++){
	
	// while(true){
	// 	uint8_t m_rx_buffer[] = {0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00,0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00};
	// 	uint8_t m_rx_buffer_bis[] = {0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00,0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00};
	// 	err_code = test(m_rx_buffer , m_rx_buffer_bis);
	// 	int succes_code = err_code - NRFX_ERROR_BASE_NUM;
	// 	use_my_variable(succes_code);
	// 	// printk("receive code %d %d %d %d %d \n" , err_code , m_rx_buffer[0] , m_rx_buffer[1] , m_rx_buffer[2] , m_rx_buffer[3]);
	// 	// printk("receive code  		 %d %d %d %d \n" , m_rx_buffer[4] , m_rx_buffer[5] , m_rx_buffer[6] , m_rx_buffer[7]);
	// 	// printk("receive code  		 %d %d %d %d \n" , m_rx_buffer[8] , m_rx_buffer[9] , m_rx_buffer[10] , m_rx_buffer[11]);
	// 	// printk("receive code  		 %d %d %d %d \n" , m_rx_buffer[12] , m_rx_buffer[13] , m_rx_buffer[14] , m_rx_buffer[15]);
	// 	k_sleep(K_SECONDS(1));
	// }
	

	while (1) {
		k_sleep(K_SECONDS(1));
		gpio_pin_toggle_dt(&led);
		#ifdef HAS_BUILDIN_PWN_DIDOES
		pwm_set_pulse_dt(&red_pwm_led, 20000000);
		#endif		
		//read_register(R_AIRMS, m_rx_buffer, 4);
		//printk("M: %x\n", *((uint32_t*)m_rx_buffer));
	}
}
