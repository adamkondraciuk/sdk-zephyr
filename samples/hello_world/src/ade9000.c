#include "nrfx_spim.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#if 1
#include <zephyr/sys/printk.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "ade9000_registers.h"

// SPI interface pins on nRF52840 development kit

//TODO: Get it from DeviceTree
#define ADE9000_CS_PIN   0x1F // 31
#define ADE9000_MOSI_PIN 0X20 // 32
#define ADE9000_MISO_PIN 0x18 // 24
#define ADE9000_SCK_PIN  0x13 // default 22

#define ADE9000_PM0_NODE   DT_ALIAS(ade9000pm0)
#define ADE9000_PM1_NODE   DT_ALIAS(ade9000pm1)
#define ADE9000_RESET_NODE DT_ALIAS(ade9000reset)
#define ADE9000_DRDY_NODE  DT_ALIAS(ade9000drdy)

static const struct gpio_dt_spec pm0   = GPIO_DT_SPEC_GET(ADE9000_PM0_NODE, gpios);
static const struct gpio_dt_spec pm1   = GPIO_DT_SPEC_GET(ADE9000_PM1_NODE, gpios);
static const struct gpio_dt_spec reset = GPIO_DT_SPEC_GET(ADE9000_RESET_NODE, gpios);
static const struct gpio_dt_spec drdy  = GPIO_DT_SPEC_GET(ADE9000_DRDY_NODE, gpios);

// SPI frequency (Hz)
#define SPI_FREQ         1000000

typedef enum {
    PM_Reduced = 0,
    PM_Normal  = 1,
    PM_Low     = 2,
    PM_Sleep   = 3
} ade9000_power_mode_t; 

#define TEST_REG 0x4FE
#define EXPECTED_TEST_RESULT 0x00FE

#define TEST_MES  0xCFE

// static K_SEM_DEFINE(transfer_finished, 0, 1);

static volatile uint8_t m_rx_buffer[] = {0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00,0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00,0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00};
static volatile uint8_t m_tx_buffer[] = {0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00,0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00};

static volatile bool spi_flag = true;

static void spim_handler(const nrfx_spim_evt_t *p_event, void *p_context)
{
	if (p_event->type == NRFX_SPIM_EVENT_DONE) {
        spi_flag = true;
        memset((void*)m_tx_buffer , 0 , sizeof(m_tx_buffer));
	}
}

static const nrfx_spim_t spi_inst = NRFX_SPIM_INSTANCE(1);

static nrfx_err_t RegWrite(uint16_t addr , uint8_t data[] , unsigned char lenght){
    while(!spi_flag);
    spi_flag = false;
    m_tx_buffer[0] = (uint8_t)(addr >> 8);
    m_tx_buffer[1] = (uint8_t)addr;
    memcpy((void*)(m_tx_buffer + 2) , data , sizeof(uint8_t) * lenght);
    m_tx_buffer[1] |= 0x08;
#if 1
    printk("W:");
    for (uint32_t i = 0; i < (2 + lenght); i++)
    {
        printk(" %02X", m_tx_buffer[i]);
    }
    printk("\n");
#endif
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TX(m_tx_buffer, 2 + lenght);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    if (err != NRFX_SUCCESS)
    {
        printk("Err code: %x\n", err);
    }
    return err;
}

static nrfx_err_t RegRead(uint16_t addr ,uint8_t rcv_data[] , unsigned char rcv_lenght){
    while(!spi_flag);
    m_tx_buffer[0] = (uint8_t)(addr >> 8);
    m_tx_buffer[1] = (uint8_t)addr;
#if 1
    printk("WR:");
    for (uint32_t i = 0; i < 2; i++)
    {
        printk(" %02X", m_tx_buffer[i]);
    }
#endif
    //memcpy(tmp_send+2 , data , sizeof(uint8_t) * lenght);
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TRX(m_tx_buffer, 2 , rcv_data , rcv_lenght + 2);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
#if 1
    if (err == NRFX_SUCCESS)
    {
        printk("<->");
        for (uint32_t i = 0; i < rcv_lenght; i++)
        {
            printk(" %02X", rcv_data[i]); 
        }
        printk("\n");
    }
    else
    {
        printk("Err code: %x\n", err);
    }
#endif
    return err;
}

static bool SPIInit(void)
{
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG(ADE9000_SCK_PIN,
                                                             ADE9000_MOSI_PIN,
                                                             ADE9000_MISO_PIN,
                                                             ADE9000_CS_PIN);

    return nrfx_spim_init(&spi_inst, &spi_config, spim_handler, NULL) == NRFX_SUCCESS ?
           true : false;
}

static bool GPIOConfig(void)
{
    int ret;

    if (!device_is_ready(reset.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&reset, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return false;
	}
    gpio_pin_set_dt(&reset, 1);
    //TODO: Check those timings.
    k_msleep(100);

    if (!device_is_ready(pm0.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&pm0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return false;
	}

    if (!device_is_ready(pm1.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&pm1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return false;
	}

    if (!device_is_ready(drdy.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&drdy, GPIO_INPUT);
	if (ret < 0) {
		return false;
	}
    return true;
}

static bool PowerModeSet(ade9000_power_mode_t mode)
{
    switch(mode) {
        case PM_Normal:
            gpio_pin_set_dt(&pm0, 0);
            gpio_pin_set_dt(&pm1, 0);
            break;
        default:
            // Other modes not supported at the moment.
            return false;
  }
  
  return true;
}

bool ADE9000Init(void)  
{
    if (!GPIOConfig()) {
        return false;
    }
    if (!PowerModeSet(PM_Normal)) {
        return false;
    }
    if (!SPIInit()) {
        return false;
    }
    //TODO: Check those timings.
    k_msleep(10);
    gpio_pin_set_dt(&reset, 0);
    //TODO: Check those timings.
    k_msleep(100);
    return true;
}

void ADE9000MeasParamsSet(void)
{
    uint8_t i_gain = 1;
    uint8_t v_gain = 1;
    uint16_t u_g = v_gain - 1, i_g = i_gain - 1;
    ade9000_reg32_t reg;
    reg.v = 0;
    reg.S_PGA_GAIN.VA_GAIN = u_g;
    reg.S_PGA_GAIN.VB_GAIN = u_g;
    reg.S_PGA_GAIN.VC_GAIN = u_g;
    reg.S_PGA_GAIN.IA_GAIN = i_g;
    reg.S_PGA_GAIN.IB_GAIN = i_g;
    reg.S_PGA_GAIN.IC_GAIN = i_g;
    reg.S_PGA_GAIN.IN_GAIN = i_g;

    RegWrite(R_PGA_GAIN ,reg.v,2);

    reg.v = 0;
    reg.S_ACCMODE.SELFSREQ = 0;
    RegWrite(R_ACCMODE, reg.v,2); 
}


bool ADE9000ConversionStart(void)
{
  ade9000_reg32_t reg, r_reg;
  //ADCData.MeasInProgress = 1; 
  reg.v = 0;
  RegWrite(R_WFB_PG_IRQEN, 0x8080, 2);
  //konfiguracja przebiegow:
  RegWrite(R_WFB_CFG, reg.v,sizeof(reg.S_WFB_CFG));
  reg.S_WFB_CFG.WF_MODE = 3;
  reg.S_WFB_CFG.WF_CAP_SEL = 1;
  reg.S_WFB_CFG.WF_CAP_EN = 1;
  reg.S_WFB_CFG.WF_SRC = 3;  
  RegWrite(R_WFB_CFG, reg.v,sizeof(reg.S_WFB_CFG));  
  RegRead(R_WFB_CFG, &r_reg.v,sizeof(r_reg.S_WFB_CFG));  
  if (r_reg.v != reg.v) return false;

  RegRead(R_CONFIG1, &reg.v,sizeof(reg.S_CONFIG1));  
  reg.S_CONFIG1.BURST_EN = 1;
  reg.S_CONFIG1.CF4_CFG = 0x2;
  RegWrite(R_CONFIG1, reg.v, sizeof(reg.S_CONFIG1));
  
  reg.v = 0;
  reg.S_EVENT_MASK.DREADY = 1;
  RegWrite(R_EVENT_MASK, reg.v, sizeof(reg.S_EVENT_MASK));
  
  RegRead(R_RUN, &reg.v,sizeof(reg.S_RUN));  

  if (!reg.S_RUN.Start) {
    reg.S_RUN.Start = 1;
    RegWrite(R_RUN,reg.v,sizeof(reg.S_RUN));
  }

  return true;
}



void ADE9000StreamRead(void)
{
    RegRead(R_AV_PCF_1 , m_rx_buffer, 28);
    k_sleep(K_SECONDS(1));
}
