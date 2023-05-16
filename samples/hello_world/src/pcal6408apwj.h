#define PCAL6408APWJ_ADDR           0x40
#define PCAL6408APWJ_SCL_PIN        42
#define PCAL6408APWJ_SDA_PIN        45

// PCAL6408APWJ register addresses
#define PCAL6408APWJ_REG_INPUT   0x00
#define PCAL6408APWJ_REG_OUTPUT  0x01
#define PCAL6408APWJ_REG_CONFIG  0x03

#include <nrfx_twi.h>

static nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(0);
nrfx_twi_config_t twi_config;


void twi_handler(nrfx_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRFX_TWI_EVT_DONE:
            printf("TWI transfer completed successfully\n");
            break;

        case NRFX_TWI_EVT_ADDRESS_NACK:
            printf("TWI slave did not acknowledge the address\n");
            break;

        case NRFX_TWI_EVT_DATA_NACK:
            printf("TWI slave did not acknowledge the data\n");
            break;

        default:
            printf("Unhandled TWI event: %d\n", p_event->type);
            break;
    }
}

void init_leds(){
	nrfx_twi_uninit(&m_twi);
    twi_config.scl = PCAL6408APWJ_SCL_PIN;
	twi_config.scl = PCAL6408APWJ_SDA_PIN;
	twi_config.interrupt_priority = 7;
	twi_config.frequency = NRF_TWI_FREQ_100K;
	twi_config.hold_bus_uninit = false;
	nrfx_twi_init(&m_twi , &twi_config , NULL ,NULL);
}

int test_leds(){
    int err_code = 0;
	uint8_t data[16];

	uint8_t data_return[16];

	data_return[0] = 0;
	data_return[1] = 0;


	// nrfx_twi_uninit(&m_twi);
	printk("status code %d \n" , nrfx_twi_get_state(&m_twi));
	
	err_code = nrfx_twi_init(&m_twi , &twi_config , NULL ,NULL);
	
	nrfx_twi_enable(&m_twi);
	printk("status code %d \n" , nrfx_twi_get_state(&m_twi));
	
	printk("innit code %d SDA %d SCL %d \n" , err_code , nrf_twi_sda_pin_get(&m_twi) , nrf_twi_scl_pin_get(&m_twi));
	// nrfx_twi_enable(&m_twi);

	data[1] = 0xFF;
	data[0] = 0x4F;

	nrfx_twi_xfer_desc_t transfer_data;
	

	//transfer_data = NRFX_TWI_XFER_DESC_TX(0x41 , &data , 3);

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