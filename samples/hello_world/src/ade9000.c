/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#if 1
#include <zephyr/sys/printk.h>
#endif

#include "ade9000_registers.h"
#include "ade9000Estimates.h"

#define ADE9000_PM0_NODE   DT_ALIAS(ade9000pm0)
#define ADE9000_PM1_NODE   DT_ALIAS(ade9000pm1)
#define ADE9000_RESET_NODE DT_ALIAS(ade9000reset)
#define ADE9000_DRDY_NODE  DT_ALIAS(ade9000drdy)
#define ADE9000_CF1_NODE  DT_ALIAS(ade9000cf1)
#define ADE9000_CF2_NODE  DT_ALIAS(ade9000cf2)
#define ADE9000_CF3_NODE  DT_ALIAS(ade9000cf3)
#define ADE9000_IRQ0_NODE  DT_ALIAS(ade9000irq0)
#define ADE9000_IRQ1_NODE  DT_ALIAS(ade9000irq1)

// TODO: Move out and rework when error handling will be finished.
#define CHECK_RESULT_BOOL(expression) \
        if (!expression) return false;

static const struct gpio_dt_spec pm0   = GPIO_DT_SPEC_GET(ADE9000_PM0_NODE, gpios);
static const struct gpio_dt_spec pm1   = GPIO_DT_SPEC_GET(ADE9000_PM1_NODE, gpios);
static const struct gpio_dt_spec reset = GPIO_DT_SPEC_GET(ADE9000_RESET_NODE, gpios);
static const struct gpio_dt_spec drdy  = GPIO_DT_SPEC_GET(ADE9000_DRDY_NODE, gpios);
static const struct gpio_dt_spec cf1   = GPIO_DT_SPEC_GET(ADE9000_CF1_NODE, gpios);
static const struct gpio_dt_spec cf2   = GPIO_DT_SPEC_GET(ADE9000_CF2_NODE, gpios);
static const struct gpio_dt_spec cf3   = GPIO_DT_SPEC_GET(ADE9000_CF3_NODE, gpios);
static const struct gpio_dt_spec irq0  = GPIO_DT_SPEC_GET(ADE9000_IRQ0_NODE, gpios);
static const struct gpio_dt_spec irq1  = GPIO_DT_SPEC_GET(ADE9000_IRQ1_NODE, gpios);

typedef enum {
    PM_Reduced = 0,
    PM_Normal  = 1,
    PM_Low     = 2,
    PM_Sleep   = 3
} ade9000_power_mode_t; 

#define TEST_REG 0x4FE
#define EXPECTED_TEST_RESULT 0x00FE

#define TEST_MES  0xCFE

static bool GPIOConfig(void)
{
    int ret;
    // TODO: Looks bad. Array of pins to be initialized/configured
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

    if (!device_is_ready(cf1.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&cf1, GPIO_INPUT);
	if (ret < 0) {
		return false;
	}

    if (!device_is_ready(cf2.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&cf2, GPIO_INPUT);
	if (ret < 0) {
		return false;
	}

    if (!device_is_ready(cf3.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&cf3, GPIO_INPUT);
	if (ret < 0) {
		return false;
	}

    if (!device_is_ready(irq0.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&irq0, GPIO_INPUT);
	if (ret < 0) {
		return false;
	}

    if (!device_is_ready(irq1.port)) {
		return false;
	}
	ret = gpio_pin_configure_dt(&irq1, GPIO_INPUT);
	if (ret < 0) {
		return false;
	}

    gpio_pin_set_dt(&cf1, 1);
    gpio_pin_set_dt(&cf2, 1);
    gpio_pin_set_dt(&cf3, 1);
    gpio_pin_set_dt(&irq0, 1);
    gpio_pin_set_dt(&irq1, 1);
    
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
static void StoreCoeffs(SEleFlashDataCal *fd)
{
  uint32_t default_coeff = 1;
  if (fd != NULL) {
    // TODO: Currently not supported
    while(1);
  }

  ADE9000InterfaceRegWriteAndCheck(R_APHCAL0, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BPHCAL0, default_coeff, 4);  
  ADE9000InterfaceRegWriteAndCheck(R_CPHCAL0, default_coeff, 4);  
    
  default_coeff = 0;
  ADE9000InterfaceRegWriteAndCheck(R_AIGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_AVGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BIGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BVGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CIGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CVGAIN, default_coeff, 4);
  default_coeff = 0;
  ADE9000InterfaceRegWriteAndCheck(R_AIRMSOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_AVRMSOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BIRMSOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BVRMSOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CIRMSOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CVRMSOS, default_coeff, 4);
  
  default_coeff = 1;
  ADE9000InterfaceRegWriteAndCheck(R_AWATTOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BWATTOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CWATTOS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_AVAROS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BVAROS, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CVAROS, default_coeff, 4);
  
  ADE9000InterfaceRegWriteAndCheck(R_APGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_BPGAIN, default_coeff, 4);
  ADE9000InterfaceRegWriteAndCheck(R_CPGAIN, default_coeff, 4);
}

void EnergyAccConfigure(void)
{
    //aktywujemy i konfigurujemy akumulator energii
    //reg.S_EP_CFG.RD_RST_EN = 1;
    ade9000_reg32_t reg;
    uint32_t no_load_thr = 2;
    reg.v = no_load_thr * 64;
    ADE9000InterfaceRegWrite(R_ACT_NL_LVL, reg.v, sizeof(reg.S_ACT_NL_LVL));
    ADE9000InterfaceRegWrite(R_REACT_NL_LVL, reg.v, sizeof(reg.S_REACT_NL_LVL));                  
    ADE9000InterfaceRegWrite(R_APP_NL_LVL, reg.v, sizeof(reg.S_APP_NL_LVL)); 

    reg.v = 0;
    reg.S_EP_CFG.EGY_PWR_EN = 1;//Set this bit to enable the energy and power accumulator
    reg.S_EP_CFG.RD_RST_EN = 1;//Every read causes reset energy accumulator
    ADE9000InterfaceRegWrite(R_EP_CFG, reg.v, sizeof(reg.S_EP_CFG));      
    
    reg.v = 8000;//Energy accumulation update time configuration.
    ADE9000InterfaceRegWrite(R_EGY_TIME, reg.v, sizeof(reg.S_EGY_TIME));
    
    reg.v = 0;
    reg.S_ACCMODE.WATTACC = 2;//Positive Accumuation Mode.
    reg.S_ACCMODE.VARACC = 2;//Positive Accumuation Mode.
    ADE9000InterfaceRegWrite(R_ACCMODE, reg.v, sizeof(reg.S_ACCMODE));
}

bool ADE9000Init(void)  
{
    if (!GPIOConfig()) {
        return false;
    }
    if (!PowerModeSet(PM_Normal)) {
        return false;
    }
    if (!ADE9000InterfaceInit()) {
        return false;
    }
    //TODO: Check those timings.
    k_msleep(10);
    gpio_pin_set_dt(&reset, 0);
    //TODO: Check those timings.
    k_msleep(100);

    StoreCoeffs(NULL);
    EnergyAccConfigure();
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

    ADE9000InterfaceRegWriteAndCheck(R_PGA_GAIN, reg.v, 2);

    reg.v = 0;
    reg.S_ACCMODE.SELFSREQ = 0;
    ADE9000InterfaceRegWriteAndCheck(R_ACCMODE, reg.v,2); 
}


bool ADE9000ConversionStart(void)
{
  ade9000_reg32_t reg, r_reg;
  //ADCData.MeasInProgress = 1; 
  reg.v = 0;
  CHECK_RESULT_BOOL(ADE9000InterfaceRegWriteAndCheck(R_WFB_PG_IRQEN, 0x8080, 2));
  //konfiguracja przebiegow:
  CHECK_RESULT_BOOL(ADE9000InterfaceRegWriteAndCheck(R_WFB_CFG, reg.v,sizeof(reg.S_WFB_CFG)));
  reg.S_WFB_CFG.WF_MODE = 3;
  reg.S_WFB_CFG.WF_CAP_SEL = 1;
  reg.S_WFB_CFG.WF_CAP_EN = 1;
  reg.S_WFB_CFG.WF_SRC = 3;  
  CHECK_RESULT_BOOL(ADE9000InterfaceRegWriteAndCheck(R_WFB_CFG, reg.v, sizeof(reg.S_WFB_CFG)));  
  ADE9000InterfaceRegRead(R_WFB_CFG, &r_reg.v,sizeof(r_reg.S_WFB_CFG));  
  if (r_reg.v != reg.v) return false;

  ADE9000InterfaceRegRead(R_CONFIG1, &reg.v, sizeof(reg.S_CONFIG1));  
  reg.S_CONFIG1.BURST_EN = 1;
  reg.S_CONFIG1.CF4_CFG = 0x2;
  CHECK_RESULT_BOOL(ADE9000InterfaceRegWriteAndCheck(R_CONFIG1, reg.v, sizeof(reg.S_CONFIG1)));
  
  reg.v = 0;
  reg.S_EVENT_MASK.DREADY = 1;
  CHECK_RESULT_BOOL(ADE9000InterfaceRegWriteAndCheck(R_EVENT_MASK, reg.v, sizeof(reg.S_EVENT_MASK)));
  
  ADE9000InterfaceRegRead(R_RUN, &reg.v, sizeof(reg.S_RUN));  

  if (!reg.S_RUN.Start) {
    reg.S_RUN.Start = 1;
    CHECK_RESULT_BOOL(ADE9000InterfaceRegWriteAndCheck(R_RUN,reg.v, sizeof(reg.S_RUN)));
  }

  return true;
}



void ADE9000StreamRead(void)
{
    //ADE9000InterfaceRegRead(R_AV_PCF_1 , (uint8_t*)m_rx_buffer, 28);
    k_sleep(K_SECONDS(1));
}
