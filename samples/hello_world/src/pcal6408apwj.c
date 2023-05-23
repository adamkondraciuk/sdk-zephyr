/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include "pcal6408apwj.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/irq.h>
#include <nrfx_twi.h>
#include <zephyr/kernel.h>

// TODO: Get from config
//#define LOG_PCAl6408APWJ 1

#if defined(LOG_PCAl6408APWJ) && (LOG_PCAl6408APWJ == 1)
#include <zephyr/sys/printk.h>
#endif

#define PCAL6408APWJ_ADDR           0x40
#define PCAL6408APWJ_SCL_PIN        42
#define PCAL6408APWJ_SDA_PIN        45
#define I2C_NODE 					DT_NODELABEL(i2c0)

// PCAL6408APWJ register addresses
#define PCAL6408APWJ_REG_INPUT   0x00
#define PCAL6408APWJ_REG_OUTPUT  0x01
#define PCAL6408APWJ_REG_CONFIG  0x03



static nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(0);
nrfx_twi_config_t twi_config;


void twi_handler(nrfx_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRFX_TWI_EVT_DONE:
            printk("TWI transfer completed successfully\n");
            break;

        case NRFX_TWI_EVT_ADDRESS_NACK:
            printk("TWI slave did not acknowledge the address\n");
            break;

        case NRFX_TWI_EVT_DATA_NACK:
            printk("TWI slave did not acknowledge the data\n");
            break;

        default:
            printk("Unhandled TWI event: %d\n", p_event->type);
            break;
    }
}

void init_leds(void)
{
	nrfx_twi_uninit(&m_twi);
    twi_config.scl = PCAL6408APWJ_SCL_PIN;
	twi_config.sda = PCAL6408APWJ_SDA_PIN;
	twi_config.interrupt_priority = 7;
	twi_config.frequency = NRF_TWI_FREQ_100K;
	twi_config.hold_bus_uninit = false;
	/*IRQ_CONNECT(DT_IRQN(I2C_NODE),
		    	DT_IRQ(I2C_NODE, priority),
		   		nrfx_isr,
				nrfx_twi_0_irq_handler,
				0);*/
	nrfx_twi_init(&m_twi , &twi_config , NULL ,NULL);
}

int test_leds(void)
{
    int err_code = 0;
	uint8_t data[16];

	uint8_t data_return[16];

	data_return[0] = 0;
	data_return[1] = 0;

	nrfx_twi_enable(&m_twi);
	
	printk("init code %x SDA %x SCL %x \n",
		   err_code,
		   nrf_twi_sda_pin_get(m_twi.p_twi),
		   nrf_twi_scl_pin_get(m_twi.p_twi));

	data[1] = 0xFF;
	data[0] = 0x4F;

	nrfx_twi_xfer_desc_t transfer_data;

	transfer_data.type = NRFX_TWI_XFER_TX;
	transfer_data.address = 0x40;
	transfer_data.primary_length = 2;
	transfer_data.p_primary_buf = data;
	transfer_data.secondary_length = 0;
	transfer_data.p_secondary_buf = NULL;	

	k_sleep(K_SECONDS(1));
	nrfx_twi_xfer(&m_twi , &transfer_data , 0);
	// printk("transmit code %d \n" , err_code_twi_innit);
	data_return[0] = 0;
	data_return[0] = 0;

	data[1] = 0x00;
	data[0] = 0x03;

	transfer_data.type = NRFX_TWI_XFER_TX;
	transfer_data.address = 0x40;
	transfer_data.primary_length = 2;
	transfer_data.p_primary_buf = data;
	transfer_data.secondary_length = 0;
	transfer_data.p_secondary_buf = NULL;

	k_sleep(K_SECONDS(1));


	nrfx_twi_xfer(&m_twi , &transfer_data , 0);
	//printk("transmit code %d \n" , err_code_twi_innit);



	data[1] = 0x00;
	//data[0] = 0x4F;

	data[0] = 0x4F;

	transfer_data.type = NRFX_TWI_XFER_TXRX;
	transfer_data.address = 0x40;
	transfer_data.primary_length = 1;
	transfer_data.p_primary_buf = data;
	transfer_data.secondary_length = 1;
	transfer_data.p_secondary_buf = data_return;

	//transfer_data = NRFX_TWI_XFER_DESC_TX(0x41 , &data , 3);

	int err_code_twi_innit = nrfx_twi_xfer(&m_twi , &transfer_data , 0);
	printk("receive code %d %d %d \n" , err_code_twi_innit , data_return[0] , data_return[1]);
	data_return[0] = 0;
	data_return[0] = 0;

	data[1] = 0x00;
	//data[0] = 0x03;

	data[0] = 0x03;

	transfer_data.type = NRFX_TWI_XFER_TXRX;
	transfer_data.address = 0x40;
	transfer_data.primary_length = 1;
	transfer_data.p_primary_buf = data;
	transfer_data.secondary_length = 1;
	transfer_data.p_secondary_buf = data_return;

	k_sleep(K_SECONDS(1));

	err_code = nrfx_twi_xfer(&m_twi , &transfer_data , 0);
	printk("receive code %d %d \n" , data_return[0] , data_return[1]);
	return data_return[0];	
}