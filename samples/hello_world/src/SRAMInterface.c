/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include "SRAMInterface.h"
#include <zephyr/kernel.h>

// TODO: Get from config
//#define LOG_SRAMINTERFACE 1
#define SRAM_TX_BUFFER_SIZE 16
#define SRAM_RX_BUFFER_SIZE 16

#if defined(LOG_SRAMINTERFACE) && (LOG_SRAMINTERFACE == 1)
#include <zephyr/sys/printk.h>
#endif

//TODO: Get it from DeviceTree
#define SRAM_CS_PIN   29//0x1F // 31
#define SRAM_MOSI_PIN 17 // 32
//TODO: This pin is temporary and should be 15 (some DK/DTS problems?)
#define SRAM_MISO_PIN (32 + 11) // 15
#define SRAM_SCK_PIN  13 // default 22

static volatile uint8_t m_rx_buffer[16];
static volatile uint8_t m_tx_buffer[16];

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
#if defined(LOG_SRAMINTERFACE) && (LOG_SRAMINTERFACE == 1)
        printk(" (IRQ)\n");
#endif
        spi_flag = true;
        memset((void*)m_tx_buffer , 0 , sizeof(m_tx_buffer));
	}
}

#warning change instance
static const nrfx_spim_t spi_inst = NRFX_SPIM_INSTANCE(2);

static bool SPIInit(void)
{
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG(SRAM_SCK_PIN,
                                                             SRAM_MOSI_PIN,
                                                             SRAM_MISO_PIN,
                                                             SRAM_CS_PIN);

    return nrfx_spim_init(&spi_inst, &spi_config, spim_handler, NULL) == NRFX_SUCCESS ?
           true : false;
}

bool SRAMInit(void)
{
    /*if (!SRAMInterfaceInit()) {
        return false;
    }*/
    if (!SPIInit()) {
        return false;
    }
    return true;
}

bool SRAMWrite(uint32_t addr, uint8_t const * const data, size_t length)
{
    WaitForTransferEnd(true);
    addr &= 0x00FFFFFF;
    m_tx_buffer[0] = 0x02;
    m_tx_buffer[1] = (uint8_t)(addr >> 16);
    m_tx_buffer[2] = (uint8_t)(addr >> 8);
    m_tx_buffer[3] = (uint8_t)(addr >> 0);
    memcpy((uint8_t*)&m_tx_buffer[4], (uint8_t*)data, length);
    length += 4;
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TX(m_tx_buffer, 4 + length);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    return err == NRFX_SUCCESS;
}

uint8_t * SRAMRead(uint32_t addr, size_t length)
{
    WaitForTransferEnd(true);
    addr &= 0x00FFFFFF;
    m_tx_buffer[0] = 0x03;
    m_tx_buffer[1] = (uint8_t)(addr >> 16);
    m_tx_buffer[2] = (uint8_t)(addr >> 8);
    m_tx_buffer[3] = (uint8_t)(addr >> 0);

    memset((uint8_t*)m_rx_buffer, 0, sizeof(m_rx_buffer));
    
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TRX(m_tx_buffer, 4 ,(uint8_t*)m_rx_buffer , length + 4);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    WaitForTransferEnd(false);
    return err == NRFX_SUCCESS ? &m_rx_buffer[4] : NULL;
}

bool SRAMIDRead(uint8_t * id)
{
    WaitForTransferEnd(true);
    uint32_t addr = 0xFFFFFFFF;
    size_t length = 8;
    m_tx_buffer[0] = 0x9F;
    m_tx_buffer[1] = (uint8_t)(addr >> 16);
    m_tx_buffer[2] = (uint8_t)(addr >> 8);
    m_tx_buffer[3] = (uint8_t)(addr >> 0);

    memset((uint8_t*)m_rx_buffer, 0, sizeof(m_rx_buffer));
    
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TRX(m_tx_buffer, 4 ,(uint8_t*)m_rx_buffer , length + 4);
    nrfx_err_t err = nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    WaitForTransferEnd(false);
    id = (uint8_t*)m_rx_buffer;
    return err == NRFX_SUCCESS;
}

void SRAMReset(void)
{
    //0x66, 0x99
    WaitForTransferEnd(true);
    m_tx_buffer[0] = 0x66;
    
    size_t length = 1;
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TX(m_tx_buffer, length);
    (void)nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    WaitForTransferEnd(true);
    k_sleep(K_MSEC(10));
    m_tx_buffer[0] = 0x99;
    (void)nrfx_spim_xfer(&spi_inst, &spim_xfer_desc, 0);
    WaitForTransferEnd(false);
}
