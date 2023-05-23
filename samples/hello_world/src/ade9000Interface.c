/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include "ade9000Interface.h"

// TODO: Get from config
//#define LOG_ADE9000INTERFACE 1

#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
#include <zephyr/sys/printk.h>
#endif

//TODO: Get it from DeviceTree
#define ADE9000_CS_PIN   0x1F // 31
#define ADE9000_MOSI_PIN 0X20 // 32
#define ADE9000_MISO_PIN 0x18 // 24
#define ADE9000_SCK_PIN  0x13 // default 22

static volatile uint8_t m_rx_buffer[6];
static volatile uint8_t m_tx_buffer[4];

static volatile bool spi_flag = true;

static void WaitForTransferEnd(bool resetFlag)
{
    while(!spi_flag);
    if (resetFlag) {
        spi_flag = false;
    }
}

static void spim_handler(const nrfx_spim_evt_t *p_event, void *p_context)
{
	if (p_event->type == NRFX_SPIM_EVENT_DONE) {
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
        printk(" (IRQ)\n");
#endif
        spi_flag = true;
        memset((void*)m_tx_buffer , 0 , sizeof(m_tx_buffer));
	}
}

bool ADE9000InterfaceTransferDone(void)
{
    return spi_flag;
}

static const nrfx_spim_t spi_inst = NRFX_SPIM_INSTANCE(1);

static bool SPIInit(void)
{
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG(ADE9000_SCK_PIN,
                                                             ADE9000_MOSI_PIN,
                                                             ADE9000_MISO_PIN,
                                                             ADE9000_CS_PIN);

    return nrfx_spim_init(&spi_inst, &spi_config, spim_handler, NULL) == NRFX_SUCCESS ?
           true : false;
}

bool ADE9000InterfaceRegWrite(uint16_t addr , uint32_t data, unsigned char length) {
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    printk("Write\n  ");
#endif
    if (((length != 2) && (length != 4)) || (addr > 0xFFF0)) {
        while(1);
    }
    if (length == 2) {
        uint16_t *p16 = (uint16_t*)&data;
        *p16 = __REVSH(*p16);
    }
    else if (length == 4) {
        uint32_t *p32 = (uint32_t*)&data;
        *p32 = __REV(*p32);
    }
    WaitForTransferEnd(true);
    m_tx_buffer[0] = (uint8_t)(addr >> 8);
    m_tx_buffer[1] = (uint8_t)addr;
    memcpy((uint8_t*)&m_tx_buffer[2], &data, length);
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    printk("W:");
    for (uint32_t i = 0; i < (2 + length); i++) {
        printk(" %02X", m_tx_buffer[i]);
    }
    printk("\n");
#endif
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TX(m_tx_buffer, 2 + length);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    if (err != NRFX_SUCCESS) {
        printk("Err code: %x\n", err);
    }
#endif
    return err == NRFX_SUCCESS;
}

bool ADE9000InterfaceRegRead(uint16_t addr ,uint32_t * rcv_data, unsigned char rcv_lenght)
{
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    printk("Read\n  ");
#endif
    WaitForTransferEnd(true);
    addr |= 0x0008;
    m_tx_buffer[0] = (uint8_t)(addr >> 8);
    m_tx_buffer[1] = (uint8_t)addr;
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    printk("R:");
    for (uint32_t i = 0; i < 2; i++)
    {
        printk(" %02X", m_tx_buffer[i]);
    }
    printk("\n");
#endif
    //memcpy(tmp_send+2 , data , sizeof(uint8_t) * lenght);
    *rcv_data = 0;
    memset((uint8_t*)m_rx_buffer, 0, sizeof(m_rx_buffer));
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TRX(m_tx_buffer, 2 ,(uint8_t*)m_rx_buffer , rcv_lenght + 2);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    WaitForTransferEnd(false);
    memcpy(rcv_data, (uint32_t*)&m_rx_buffer[2], sizeof(uint32_t));
    if(rcv_lenght == 4) {
        uint32_t * p32 = (uint32_t*)rcv_data;
        *p32 = __REV(*p32);
    }
    else if(rcv_lenght == 2) {
        uint16_t * p16 = (uint16_t*)rcv_data;
        *p16 = __REVSH(*p16);
    }
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    if (err == NRFX_SUCCESS)
    {
        printk("  <->");
        for (uint32_t i = 2; i < rcv_lenght + 2; i++)
        {
            printk(" %02X", m_rx_buffer[i]); 
        }
        printk("\n");
    }
    else
    {
        printk("Err code: %x\n", err);
    }
#endif
    return err == NRFX_SUCCESS;
}

bool ADE9000InterfaceRegWriteAndCheck(uint16_t addr , uint32_t data , unsigned char length)
{
    uint32_t rcv_data;
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)    
    printk("WriteAndCheck\n  ");
#endif
    ADE9000InterfaceRegWrite(addr, data, length);
#if defined(LOG_ADE9000INTERFACE) && (LOG_ADE9000INTERFACE == 1)
    printk("  Verify:\n  ");
#endif
    ADE9000InterfaceRegRead(addr, &rcv_data, length);
    return data == rcv_data;
}

bool ADE9000InterfaceInit(void)
{
    if (!SPIInit()) {
        return false;
    }
    return true;
}