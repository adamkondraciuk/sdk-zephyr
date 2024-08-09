/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdlib.h>
#include <zephyr/drivers/i2s.h>
//#include <zephyr/drivers/clock_control/nrf_clock_control.h>
#include <zephyr/drivers/pinctrl.h>
#include <soc.h>
#include <hal/nrf_tdm.h>
#include <haly/nrfy_gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/irq.h>
LOG_MODULE_REGISTER(tdm_nrfx, CONFIG_I2S_LOG_LEVEL);

#define NRFX_TDM_STATUS_NEXT_BUFFERS_NEEDED (1UL << 0)
    /**< The application must provide buffers that are to be used in the next
     *   part of the transfer. A call to @ref nrfx_i2s_next_buffers_set must
     *   be done before the currently used buffers are completely processed
     *   (that is, the time remaining for supplying the next buffers depends on
     *   the used size of the buffers). */

#define NRFX_TDM_STATUS_TRANSFER_STOPPED    (1UL << 1)
    /**< The I2S peripheral has been stopped and all buffers that were passed
     *   to the driver have been released. */

#define NRFX_TDM130_INST_IDX 130

typedef struct
{
    uint32_t       * p_rx_buffer; ///< Pointer to the buffer for received data.
    uint32_t const * p_tx_buffer; ///< Pointer to the buffer with data to be sent.
    uint16_t         buffer_size; ///< Size of buffers.
} tdm_buffers_t;

typedef void (* nrfx_tdm_data_handler_t)(tdm_buffers_t const * p_released, uint32_t status);

#define NRFX_TDM_DEFAULT_CONFIG(_pin_sck, _pin_lrck, _pin_mck, _pin_sdout, _pin_sdin) \
{ \
    .nrf_tdm_config = { \
    }, \
    .sck_pin      = _pin_sck,                                                         \
    .lrck_pin     = _pin_lrck,                                                        \
    .mck_pin      = _pin_mck,                                                         \
    .sdout_pin    = _pin_sdout,                                                       \
    .sdin_pin     = _pin_sdin,                                                        \
    .irq_priority = 7,                             \
    NRFX_COND_CODE_1(NRF_TDM_HAS_CLKCONFIG,                                           \
                     (.clksrc = NRF_TDM_CLKSRC_PCLK32M,                               \
                      .enable_bypass = false,),                                       \
                     ())                                                              \
}

typedef struct
{
    nrfx_tdm_data_handler_t handler;
    nrfx_drv_state_t        state;

    bool use_rx         : 1;
    bool use_tx         : 1;
    bool rx_ready       : 1;
    bool tx_ready       : 1;
    bool buffers_needed : 1;
    bool buffers_reused : 1;
    bool skip_gpio_cfg  : 1;
    bool skip_psel_cfg  : 1;

#if !NRFX_API_VER_AT_LEAST(3, 3, 0)
    uint16_t            buffer_size;
#endif
    tdm_buffers_t  next_buffers;
    tdm_buffers_t  current_buffers;
} nrfx_tdm_cb_t;

static nrfx_tdm_cb_t m_cb[1];

/** @brief I2S driver instance structure. */
typedef struct
{
    NRF_TDM_Type  * p_reg;        ///< Pointer to a structure with I2S registers.
    uint8_t         drv_inst_idx; ///< Index of the driver instance. For internal use only.
} nrfx_tdm_t;

#define NRFX_TDM_INSTANCE(id)                               \
{                                                           \
    .p_reg        = NRF_TDM##id ,                           \
    .drv_inst_idx = NRFX_CONCAT_3(NRFX_TDM, id, _INST_IDX), \
}

typedef struct {
    nrf_tdm_config_t nrf_tdm_config;
    uint32_t           sck_pin;       ///< SCK pin number.
    uint32_t           lrck_pin;      ///< LRCK pin number.
    uint32_t           mck_pin;       ///< MCK pin number.
                                      /**< Optional. Use @ref NRF_TDM_PIN_NOT_CONNECTED
                                       *   if this signal is not needed. */
    uint32_t           sdout_pin;     ///< SDOUT pin number.
                                      /**< Optional. Use @ref NRF_TDM_PIN_NOT_CONNECTED
                                       *   if this signal is not needed. */
    uint32_t           sdin_pin;      ///< SDIN pin number.
                                      /**< Optional. Use @ref NRF_TDM_PIN_NOT_CONNECTED
                                       *   if this signal is not needed. */
    uint8_t            irq_priority;  ///< Interrupt priority.
    nrf_tdm_src_t      clksrc;        ///< Clock source selection.
	bool               skip_gpio_cfg;
	bool               skip_psel_cfg;
} nrfx_tdm_config_t;

struct stream_cfg {
	struct i2s_config cfg;
	nrfx_tdm_config_t nrfx_cfg;
};

struct tdm_buf {
	void *mem_block;
	size_t size;
};

struct tdm_nrfx_drv_data {
	//struct onoff_manager *clk_mgr;
	//struct onoff_client clk_cli;
	struct stream_cfg tx;
	struct k_msgq tx_queue;
	struct stream_cfg rx;
	struct k_msgq rx_queue;
	const nrfx_tdm_t *p_tdm;
	const uint32_t *last_tx_buffer;
	enum i2s_state state;
	enum i2s_dir active_dir;
	bool stop;       /* stop after the current (TX or RX) block */
	bool discard_rx; /* discard further RX blocks */
	volatile bool next_tx_buffer_needed;
	bool tx_configured : 1;
	bool rx_configured : 1;
	bool request_clock : 1;
};

struct tdm_nrfx_drv_cfg {
	nrfx_tdm_data_handler_t data_handler;
	nrfx_tdm_t tdm;
	nrfx_tdm_config_t nrfx_def_cfg;
	const struct pinctrl_dev_config *pcfg;
	enum clock_source {
		PCLK32M,
		ACLK
	} clk_src;
};

enum tdm_status {
	TDM_STATUS_NEXT_BUFFERS_NEEDED,
	TDM_STATUS_TRANSFER_STOPPED
};

void tdm_130_irq_handler(const struct device *dev)
{
	const struct tdm_nrfx_drv_cfg * drv_cfg = dev->config;
	NRF_TDM_Type * p_reg = drv_cfg->tdm.p_reg;
	nrfx_tdm_cb_t * p_cb = &m_cb[0];
	printk("HH\n");
	uint32_t event_mask = 0;
	if (nrf_tdm_event_check(p_reg, NRF_TDM_EVENT_MAXCNT)) {
		nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_MAXCNT);
	}
	if (nrf_tdm_event_check(p_reg, NRF_TDM_EVENT_RXPTRUPD)) {
		nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_RXPTRUPD);
		event_mask |= NRFY_EVENT_TO_INT_BITMASK(NRF_TDM_EVENT_RXPTRUPD);
	        p_cb->rx_ready = true;
		if (p_cb->use_rx && p_cb->buffers_needed)
		{
			printk("HR1\n");
			p_cb->buffers_reused = true;
		}

	}
	if (nrf_tdm_event_check(p_reg, NRF_TDM_EVENT_TXPTRUPD)) {
		nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_TXPTRUPD);
		event_mask |= NRFY_EVENT_TO_INT_BITMASK(NRF_TDM_EVENT_TXPTRUPD);
		p_cb->tx_ready = true;
		if (p_cb->use_tx && p_cb->buffers_needed)
		{
			printk("HR2\n");
			p_cb->buffers_reused = true;
		}
	}
	if (nrf_tdm_event_check(p_reg, NRF_TDM_EVENT_STOPPED)) {
		nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_STOPPED);
		event_mask |= NRFY_EVENT_TO_INT_BITMASK(NRF_TDM_EVENT_STOPPED);
		nrf_tdm_int_disable(p_reg, NRF_TDM_INT_STOPPED_MASK_MASK);
		nrf_tdm_disable(p_reg);
		// When stopped, release all buffers, including these scheduled for
		// the next part of the transfer, and signal that the transfer has
		// finished.
		printk("H1\n");
		p_cb->handler(&p_cb->current_buffers, 0);

		// Change the state of the driver before calling the handler with
		// the flag signaling that the transfer has finished, so that it is
		// possible to start a new transfer directly from the handler function.
		p_cb->state = NRFX_DRV_STATE_INITIALIZED;

		p_cb->handler(&p_cb->next_buffers, NRFX_TDM_STATUS_TRANSFER_STOPPED);
	}
	else
	{
		// Check if the requested transfer has been completed:
		// - full-duplex mode
		if ((p_cb->use_tx && p_cb->use_rx &&
		p_cb->tx_ready && p_cb->rx_ready) ||
		// - TX only mode
		(!p_cb->use_rx && p_cb->tx_ready) ||
		// - RX only mode
		(!p_cb->use_tx && p_cb->rx_ready))
		{
		p_cb->tx_ready = false;
		p_cb->rx_ready = false;

		// If the application did not supply the buffers for the next
		// part of the transfer until this moment, the current buffers
		// cannot be released, since the I2S peripheral already started
		// using them. Signal this situation to the application by
		// passing NULL instead of the structure with released buffers.
		if (p_cb->buffers_reused)
		{
			printk("H2\n");
			p_cb->buffers_reused = false;
			// This will most likely be set at this point. However, there is
			// a small time window between TXPTRUPD and RXPTRUPD events,
			// and it is theoretically possible that next buffers will be
			// set in this window, so to be sure this flag is set to true,
			// set it explicitly.
			p_cb->buffers_needed = true;
			p_cb->handler(NULL, NRFX_TDM_STATUS_NEXT_BUFFERS_NEEDED);
		}
		else
		{
			// Buffers that have been used by the I2S peripheral (current)
			// are now released and will be returned to the application,
			// and the ones scheduled to be used as next become the current
			// ones.
			printk("H3\n");
			tdm_buffers_t released_buffers = p_cb->current_buffers;
			p_cb->current_buffers = p_cb->next_buffers;
			p_cb->next_buffers.p_rx_buffer = NULL;
			p_cb->next_buffers.p_tx_buffer = NULL;
			p_cb->buffers_needed = true;
			p_cb->handler(&released_buffers, NRFX_TDM_STATUS_NEXT_BUFFERS_NEEDED);
		}
		}
	}
	printk("HX\n");
#warning co z tym NRFY_CACHE_INV
#warning FRAMESTART to MAXCNT
}



/* Finds the clock settings that give the frame clock frequency closest to
 * the one requested, taking into account the hardware limitations.
 */
static void find_suitable_clock(const struct tdm_nrfx_drv_cfg *drv_cfg,
				nrfx_tdm_config_t *config,
				const struct i2s_config *tdm_cfg)
{
	uint32_t sample_rate = tdm_cfg->frame_clk_freq;
	uint32_t requested_sck_freq = 0;
	uint32_t nb_of_channels = tdm_cfg->channels;
	uint32_t word_size = tdm_cfg->word_size;
	const uint32_t src_freq =
		(drv_cfg->clk_src == ACLK)
		/* The I2S_NRFX_DEVICE() macro contains build assertions that
		 * make sure that the ACLK clock source is only used when it is
		 * available and only with the "hfclkaudio-frequency" property
		 * defined, but the default value of 0 here needs to be used to
		 * prevent compilation errors when the property is not defined
		 * (this expression will be eventually optimized away then).
		 */
		? DT_PROP_OR(DT_NODELABEL(clock), hfclkaudio_frequency, 0)
		: 32*1000*1000UL;
	config->nrf_tdm_config.mck_setup = NRF_TDM_MCK_DIV_2;
	//config->ratio = ratios[best_r].ratio_enum;

}

static bool get_next_tx_buffer(struct tdm_nrfx_drv_data *drv_data,
			       tdm_buffers_t *buffers)
{
	struct tdm_buf buf;
	int ret = k_msgq_get(&drv_data->tx_queue,
			     &buf,
			     K_NO_WAIT);
	if (ret == 0) {
		buffers->p_tx_buffer = buf.mem_block;
		buffers->buffer_size = buf.size / sizeof(uint32_t);
	}
	return (ret == 0);
}

static bool get_next_rx_buffer(struct tdm_nrfx_drv_data *drv_data,
			       tdm_buffers_t *buffers)
{
	int ret = k_mem_slab_alloc(drv_data->rx.cfg.mem_slab,
				   (void **)&buffers->p_rx_buffer,
				   K_NO_WAIT);
	if (ret < 0) {
		printk("Failed to allocate next RX buffer: %d\n",
			ret);
		return false;
	}

	return true;
}

static void free_tx_buffer(struct tdm_nrfx_drv_data *drv_data,
			   const void *buffer)
{
	k_mem_slab_free(drv_data->tx.cfg.mem_slab, (void *)buffer);
	LOG_DBG("Freed TX %p", buffer);
}

static void free_rx_buffer(struct tdm_nrfx_drv_data *drv_data, void *buffer)
{
	k_mem_slab_free(drv_data->rx.cfg.mem_slab, buffer);
	LOG_DBG("Freed RX %p", buffer);
}

static void tdm_start(NRF_TDM_Type * p_reg, tdm_buffers_t const * p_initial_buffers)
{
	nrfx_tdm_cb_t * p_cb = &m_cb[0];
	__ASSERT_NO_MSG(p_initial_buffers->p_rx_buffer != NULL || p_initial_buffers->p_tx_buffer != NULL);
	p_cb->use_rx         = (p_initial_buffers->p_rx_buffer != NULL);
	p_cb->use_tx         = (p_initial_buffers->p_tx_buffer != NULL);
	p_cb->rx_ready       = false;
	p_cb->tx_ready       = false;
	p_cb->buffers_needed = false;

	p_cb->next_buffers = *p_initial_buffers;
	p_cb->current_buffers.p_rx_buffer = NULL;
    	p_cb->current_buffers.p_tx_buffer = NULL;
	nrf_tdm_enable(p_reg);

	nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_RXPTRUPD);
	nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_TXPTRUPD);

	nrf_tdm_int_enable(p_reg,
				(p_initial_buffers->p_rx_buffer ? NRF_TDM_INT_RXPTRUPD_MASK_MASK : 0UL) |
				(p_initial_buffers->p_tx_buffer ? NRF_TDM_INT_TXPTRUPD_MASK_MASK : 0UL) |
				NRF_TDM_INT_STOPPED_MASK_MASK);

	nrf_tdm_tx_count_set(p_reg, p_initial_buffers->buffer_size);
	nrf_tdm_rx_count_set(p_reg, p_initial_buffers->buffer_size);
	nrf_tdm_rx_buffer_set(p_reg, p_initial_buffers->p_rx_buffer);
	nrf_tdm_tx_buffer_set(p_reg, p_initial_buffers->p_tx_buffer);
	nrf_tdm_task_trigger(p_reg, NRF_TDM_TASK_START);
}

static void tdm_stop(NRF_TDM_Type * p_reg)
{
    nrf_tdm_int_disable(p_reg, NRF_TDM_INT_RXPTRUPD_MASK_MASK |
                	       NRF_TDM_INT_TXPTRUPD_MASK_MASK);

    nrf_tdm_task_trigger(p_reg, NRF_TDM_TASK_STOP);
}

static void next_buffers_set(nrfx_tdm_t const *    p_instance,
                             tdm_buffers_t const * p_buffers)
{
	nrfx_tdm_cb_t * p_cb = &m_cb[0];
	__ASSERT_NO_MSG(p_buffers->p_rx_buffer != NULL || p_buffers->p_tx_buffer != NULL);

	if (!p_cb->buffers_needed) {
		printk("=========Err1\n");
	}

	nrf_tdm_tx_count_set(p_instance->p_reg, p_buffers->buffer_size);
	nrf_tdm_rx_count_set(p_instance->p_reg, p_buffers->buffer_size);
	nrf_tdm_rx_buffer_set(p_instance->p_reg, p_buffers->p_rx_buffer);
	nrf_tdm_tx_buffer_set(p_instance->p_reg, p_buffers->p_tx_buffer);

	nrf_tdm_rxtxen_t dir = NRF_TDM_RXTXEN_DUPLEX;
	if (p_buffers->p_rx_buffer == NULL) {
		dir = NRF_TDM_RXTXEN_TX;
	}
	else if (p_buffers->p_tx_buffer == NULL) {
		dir = NRF_TDM_RXTXEN_RX;
	}
	nrf_tdm_transfer_direction_set(p_instance->p_reg, dir);

	p_cb->next_buffers   = *p_buffers;
    	p_cb->buffers_needed = false;

}

static bool supply_next_buffers(struct tdm_nrfx_drv_data *drv_data,
				tdm_buffers_t *next)
{
	if (drv_data->active_dir != I2S_DIR_TX) { /* -> RX active */
		if (!get_next_rx_buffer(drv_data, next)) {
			drv_data->state = I2S_STATE_ERROR;
			tdm_stop(drv_data->p_tdm->p_reg);
			return false;
		}
		/* Set buffer size if there is no TX buffer (which effectively
		 * controls how many bytes will be received).
		 */
		if (drv_data->active_dir == I2S_DIR_RX) {
			next->buffer_size =
				drv_data->rx.cfg.block_size / sizeof(uint32_t);
		}
	}

	drv_data->last_tx_buffer = next->p_tx_buffer;

	LOG_DBG("Next buffers: %p/%p", next->p_tx_buffer, next->p_rx_buffer);
	next_buffers_set(drv_data->p_tdm, next);
	return true;
}

static void purge_queue(const struct device *dev, enum i2s_dir dir)
{
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	struct tdm_buf buf;

	if (dir == I2S_DIR_TX || dir == I2S_DIR_BOTH) {
		while (k_msgq_get(&drv_data->tx_queue,
				  &buf,
				  K_NO_WAIT) == 0) {
			free_tx_buffer(drv_data, buf.mem_block);
		}
	}

	if (dir == I2S_DIR_RX || dir == I2S_DIR_BOTH) {
		while (k_msgq_get(&drv_data->rx_queue,
				  &buf,
				  K_NO_WAIT) == 0) {
			free_rx_buffer(drv_data, buf.mem_block);
		}
	}
}

static int tdm_nrfx_configure(const struct device *dev, enum i2s_dir dir,
			      const struct i2s_config *tdm_cfg)
{
	printk("CC\n");
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	const struct tdm_nrfx_drv_cfg *drv_cfg = dev->config;
	nrfx_tdm_config_t nrfx_cfg;

	if (drv_data->state != I2S_STATE_READY) {
		printk("Cannot configure in state: %d\n", drv_data->state);
		return -EINVAL;
	}

	if (tdm_cfg->frame_clk_freq == 0) { /* -> reset state */
		purge_queue(dev, dir);
		if (dir == I2S_DIR_TX || dir == I2S_DIR_BOTH) {
			drv_data->tx_configured = false;
			memset(&drv_data->tx, 0, sizeof(drv_data->tx));
		}
		if (dir == I2S_DIR_RX || dir == I2S_DIR_BOTH) {
			drv_data->rx_configured = false;
			memset(&drv_data->rx, 0, sizeof(drv_data->rx));
		}
		return 0;
	}

	__ASSERT_NO_MSG(tdm_cfg->mem_slab != NULL &&
			tdm_cfg->block_size != 0);

	if ((tdm_cfg->block_size % sizeof(uint32_t)) != 0) {
		printk("This device can transfer only full 32-bit words\n");
		return -EINVAL;
	}

	nrfx_cfg = drv_cfg->nrfx_def_cfg;

	switch (tdm_cfg->word_size) {
	case 8:
		nrfx_cfg.nrf_tdm_config.sample_width = NRF_TDM_SWIDTH_8BIT;
		break;
	case 16:
		nrfx_cfg.nrf_tdm_config.sample_width = NRF_TDM_SWIDTH_16BIT;
		break;
	case 24:
		nrfx_cfg.nrf_tdm_config.sample_width = NRF_TDM_SWIDTH_24BIT;
		break;
	case 32:
		nrfx_cfg.nrf_tdm_config.sample_width = NRF_TDM_SWIDTH_32BIT;
		break;
	default:
		printk("Unsupported word size: %u\n", tdm_cfg->word_size);
		return -EINVAL;
	}

	switch (tdm_cfg->format & I2S_FMT_DATA_FORMAT_MASK) {
	case I2S_FMT_DATA_FORMAT_I2S:
		nrfx_cfg.nrf_tdm_config.alignment = NRF_TDM_ALIGN_LEFT;
		nrfx_cfg.nrf_tdm_config.fsync_polarity = NRF_TDM_POLARITY_NEGEDGE;
		nrfx_cfg.nrf_tdm_config.sck_polarity = NRF_TDM_POLARITY_POSEDGE;
		nrfx_cfg.nrf_tdm_config.fsync_duration = NRF_TDM_FSYNC_DURATION_CHANNEL;
		nrfx_cfg.nrf_tdm_config.channel_delay = NRF_TDM_CHANNEL_DELAY_1CK;
		break;
	case I2S_FMT_DATA_FORMAT_LEFT_JUSTIFIED:
		nrfx_cfg.nrf_tdm_config.alignment = NRF_TDM_ALIGN_LEFT;
		nrfx_cfg.nrf_tdm_config.fsync_polarity = NRF_TDM_POLARITY_POSEDGE;
		nrfx_cfg.nrf_tdm_config.sck_polarity = NRF_TDM_POLARITY_POSEDGE;
		nrfx_cfg.nrf_tdm_config.fsync_duration = NRF_TDM_FSYNC_DURATION_CHANNEL;
		nrfx_cfg.nrf_tdm_config.channel_delay = NRF_TDM_CHANNEL_DELAY_NONE;
		break;
	case I2S_FMT_DATA_FORMAT_RIGHT_JUSTIFIED:
		nrfx_cfg.nrf_tdm_config.alignment = NRF_TDM_ALIGN_RIGHT;
		nrfx_cfg.nrf_tdm_config.fsync_polarity = NRF_TDM_POLARITY_POSEDGE;
		nrfx_cfg.nrf_tdm_config.sck_polarity = NRF_TDM_POLARITY_POSEDGE;
		nrfx_cfg.nrf_tdm_config.fsync_duration = NRF_TDM_FSYNC_DURATION_CHANNEL;
		nrfx_cfg.nrf_tdm_config.channel_delay = NRF_TDM_CHANNEL_DELAY_NONE;
		break;
	default:
		printk("Unsupported data format: 0x%02x\n", tdm_cfg->format);
		return -EINVAL;
	}

	if ((tdm_cfg->format & I2S_FMT_DATA_ORDER_LSB) ||
	    (tdm_cfg->format & I2S_FMT_BIT_CLK_INV) ||
	    (tdm_cfg->format & I2S_FMT_FRAME_CLK_INV)) {
		printk("Unsupported stream format: 0x%02x\n", tdm_cfg->format);
		return -EINVAL;
	}

#warning extend me
	if (tdm_cfg->channels == 2) {
		nrfx_cfg.nrf_tdm_config.num_of_channels = NRF_TDM_CHANNELS_COUNT_2;
		nrfx_cfg.nrf_tdm_config.channels = 0x00030000;
	} else if (tdm_cfg->channels == 1) {
		nrfx_cfg.nrf_tdm_config.num_of_channels = NRF_TDM_CHANNELS_COUNT_1;
		nrfx_cfg.nrf_tdm_config.channels = 0x00010000;;
	} else {
		printk("Unsupported number of channels: %u\n",
			tdm_cfg->channels);
		return -EINVAL;
	}

	if ((tdm_cfg->options & I2S_OPT_BIT_CLK_SLAVE) &&
	    (tdm_cfg->options & I2S_OPT_FRAME_CLK_SLAVE)) {
		nrfx_cfg.nrf_tdm_config.mode = NRF_TDM_MODE_SLAVE;
	} else if (!(tdm_cfg->options & I2S_OPT_BIT_CLK_SLAVE) &&
		   !(tdm_cfg->options & I2S_OPT_FRAME_CLK_SLAVE)) {
		nrfx_cfg.nrf_tdm_config.mode = NRF_TDM_MODE_MASTER;
	} else {
		printk("Unsupported operation mode: 0x%02x\n", tdm_cfg->options);
		return -EINVAL;
	}
	nrfx_cfg.nrf_tdm_config.sck_setup = NRF_TDM_SCK_DIV_125;
	/* If the master clock generator is needed (i.e. in Master mode or when
	 * the MCK output is used), find a suitable clock configuration for it.
	 */
	if (nrfx_cfg.nrf_tdm_config.mode == NRF_TDM_MODE_MASTER ||
	    (nrf_tdm_mck_pin_get(drv_cfg->tdm.p_reg) & TDM_PSEL_MCK_CONNECT_Msk)
	    == TDM_PSEL_MCK_CONNECT_Connected << TDM_PSEL_MCK_CONNECT_Pos) {
		find_suitable_clock(drv_cfg, &nrfx_cfg, tdm_cfg);
		/* Unless the PCLK32M source is used with the HFINT oscillator
		 * (which is always available without any additional actions),
		 * it is required to request the proper clock to be running
		 * before starting the transfer itself.
		 */
		drv_data->request_clock = (drv_cfg->clk_src != PCLK32M);
		nrf_tdm_mck_set(drv_cfg->tdm.p_reg, true);
		nrfx_cfg.nrf_tdm_config.mck_setup = NRF_TDM_MCK_DIV_2;
	} else {
		nrf_tdm_mck_set(drv_cfg->tdm.p_reg, false);
		drv_data->request_clock = false;
	}

	if ((tdm_cfg->options & I2S_OPT_LOOPBACK) ||
	    (tdm_cfg->options & I2S_OPT_PINGPONG)) {
		printk("Unsupported options: 0x%02x\n", tdm_cfg->options);
		return -EINVAL;
	}

	if (dir == I2S_DIR_TX || dir == I2S_DIR_BOTH) {
		drv_data->tx.cfg = *tdm_cfg;
		drv_data->tx.nrfx_cfg = nrfx_cfg;
		drv_data->tx_configured = true;
	}

	if (dir == I2S_DIR_RX || dir == I2S_DIR_BOTH) {
		drv_data->rx.cfg = *tdm_cfg;
		drv_data->rx.nrfx_cfg = nrfx_cfg;
		drv_data->rx_configured = true;
	}
	printk("CX\n");
	return 0;
}

static const struct i2s_config *tdm_nrfx_config_get(const struct device *dev,
						    enum i2s_dir dir)
{
	struct tdm_nrfx_drv_data *drv_data = dev->data;

	if (dir == I2S_DIR_TX && drv_data->tx_configured) {
		return &drv_data->tx.cfg;
	}
	if (dir == I2S_DIR_RX && drv_data->rx_configured) {
		return &drv_data->rx.cfg;
	}

	return NULL;
}

static int tdm_nrfx_read(const struct device *dev,
			 void **mem_block, size_t *size)
{
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	struct tdm_buf buf;
	int ret;
	printk("RR\n");
	if (!drv_data->rx_configured) {
		printk("Device is not configured\n");
		return -EIO;
	}

	ret = k_msgq_get(&drv_data->rx_queue,
			 &buf,
			 (drv_data->state == I2S_STATE_ERROR)
				? K_NO_WAIT
				: SYS_TIMEOUT_MS(drv_data->rx.cfg.timeout));
	if (ret == -ENOMSG) {
		return -EIO;
	}

	LOG_DBG("Released RX %p", buf.mem_block);

	if (ret == 0) {
		*mem_block = buf.mem_block;
		*size = buf.size;
	}
	printk("RX\n");
	return ret;
}

static int tdm_nrfx_write(const struct device *dev,
			  void *mem_block, size_t size)
{
	printk("WW\n");
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	struct tdm_buf buf = { .mem_block = mem_block, .size = size };
	int ret;

	if (!drv_data->tx_configured) {
		printk("Device is not configured\n");
		return -EIO;
	}

	if (drv_data->state != I2S_STATE_RUNNING &&
	    drv_data->state != I2S_STATE_READY) {
		printk("Cannot write in state: %d\n", drv_data->state);
		return -EIO;
	}

	if (size > drv_data->tx.cfg.block_size || size < sizeof(uint32_t)) {
		printk("This device can only write blocks up to %u bytes\n",
			drv_data->tx.cfg.block_size);
		return -EIO;
	}

	ret = k_msgq_put(&drv_data->tx_queue,
			 &buf,
			 SYS_TIMEOUT_MS(drv_data->tx.cfg.timeout));
	if (ret < 0) {
		return ret;
	}

	printk("Queued TX %p\n", mem_block);

	/* Check if interrupt wanted to get next TX buffer before current buffer
	 * was queued. Do not move this check before queuing because doing so
	 * opens the possibility for a race condition between this function and
	 * data_handler() that is called in interrupt context.
	 */
	if (drv_data->state == I2S_STATE_RUNNING &&
	    drv_data->next_tx_buffer_needed) {
		tdm_buffers_t next = { 0 };
		printk("W1\n");
		if (!get_next_tx_buffer(drv_data, &next)) {
			/* Log error because this is definitely unexpected.
			 * Do not return error because the caller is no longer
			 * responsible for releasing the buffer.
			 */
			printk("Cannot reacquire queued buffer\n");
			return 0;
		}

		drv_data->next_tx_buffer_needed = false;

		LOG_DBG("Next TX %p", next.p_tx_buffer);

		if (!supply_next_buffers(drv_data, &next)) {
			printk("W2\n");
			return -EIO;
		}

	}
	printk("WX\n");
	return 0;
}

static int start_transfer(struct tdm_nrfx_drv_data *drv_data)
{
	printk("SS\n");
	tdm_buffers_t initial_buffers = { 0 };

	if (drv_data->active_dir != I2S_DIR_RX && /* -> TX to be started */
	    !get_next_tx_buffer(drv_data, &initial_buffers)) {
		printk("No TX buffer available\n");
		return -ENOMEM;
	} else if (drv_data->active_dir != I2S_DIR_TX && /* -> RX to be started */
		   !get_next_rx_buffer(drv_data, &initial_buffers)) {
		/* Failed to allocate next RX buffer */
		return -ENOMEM;
	} else {
		nrfx_err_t err;

		/* It is necessary to set buffer size here only for I2S_DIR_RX,
		 * because only then the get_next_tx_buffer() call in the if
		 * condition above gets short-circuited.
		 */
		if (drv_data->active_dir == I2S_DIR_RX) {
			initial_buffers.buffer_size =
				drv_data->rx.cfg.block_size / sizeof(uint32_t);
		}

		drv_data->last_tx_buffer = initial_buffers.p_tx_buffer;

		tdm_start(drv_data->p_tdm->p_reg, &initial_buffers);
	}
	printk("SX\n");
	return 0;
}

static void configure_pins(nrfx_tdm_config_t const * p_config)
{
    if (!p_config->skip_gpio_cfg)
    {
        // Configure pins used by the peripheral:

        // - SCK and LRCK (required) - depending on the mode of operation these
        //   pins are configured as outputs (in Master mode) or inputs (in Slave
        //   mode).
        if (p_config->nrf_tdm_config.mode == NRF_TDM_MODE_MASTER)
        {
            nrfy_gpio_cfg_output(p_config->sck_pin);
            nrfy_gpio_cfg_output(p_config->lrck_pin);
#if NRF_GPIO_HAS_CLOCKPIN && defined(NRF_TDM_CLOCKPIN_SCK_NEEDED)
            nrfy_gpio_pin_clock_set(p_config->sck_pin, true);
#endif
#if NRF_GPIO_HAS_CLOCKPIN && defined(NRF_TDM_CLOCKPIN_FSYNC_NEEDED)
            nrfy_gpio_pin_clock_set(p_config->lrck_pin, true);
#endif
        }
        else
        {
            nrfy_gpio_cfg_input(p_config->sck_pin,  NRF_GPIO_PIN_NOPULL);
            nrfy_gpio_cfg_input(p_config->lrck_pin, NRF_GPIO_PIN_NOPULL);
        }
        // - MCK (optional) - always output,
        if (p_config->mck_pin != NRF_TDM_PIN_NOT_CONNECTED)
        {
            nrfy_gpio_cfg_output(p_config->mck_pin);
#if NRF_GPIO_HAS_CLOCKPIN && defined(NRF_TDM_CLOCKPIN_MCK_NEEDED)
            nrfy_gpio_pin_clock_set(p_config->mck_pin, true);
#endif
        }
        // - SDOUT (optional) - always output,
        if (p_config->sdout_pin != NRF_TDM_PIN_NOT_CONNECTED)
        {
            nrfy_gpio_cfg_output(p_config->sdout_pin);
        }
        // - SDIN (optional) - always input.
        if (p_config->sdin_pin != NRF_TDM_PIN_NOT_CONNECTED)
        {
            nrfy_gpio_cfg_input(p_config->sdin_pin, NRF_GPIO_PIN_NOPULL);
        }
    }
}

static void deconfigure_pins(NRF_TDM_Type * p_reg)
{
	nrf_tdm_pins_t pins = {
		.sck_pin   = nrf_tdm_sck_pin_get(p_reg),
		.fsync_pin = nrf_tdm_fsync_pin_get(p_reg),
		.mck_pin   = nrf_tdm_mck_pin_get(p_reg),
		.sdout_pin = nrf_tdm_sdout_pin_get(p_reg),
		.sdin_pin  = nrf_tdm_sdin_pin_get(p_reg),
	};
	uint32_t pin_mask = NRF_TDM_PSEL_SCK_PIN_MASK;

	nrfy_gpio_cfg_default(pins.sck_pin & pin_mask);
	nrfy_gpio_cfg_default(pins.fsync_pin & pin_mask);

	if (pins.mck_pin != NRF_TDM_PIN_NOT_CONNECTED) {
		nrfy_gpio_cfg_default(pins.mck_pin & pin_mask);
	}

	if (pins.sdout_pin != NRF_TDM_PIN_NOT_CONNECTED) {
		nrfy_gpio_cfg_default(pins.sdout_pin & pin_mask);
	}

	if (pins.sdin_pin != NRF_TDM_PIN_NOT_CONNECTED) {
		nrfy_gpio_cfg_default(pins.sdin_pin & pin_mask);
	}
}

void tdm_int_init(NRF_TDM_Type * p_reg,
		  uint32_t       mask,
                  uint8_t        irq_priority,
                  bool           enable)
{
	nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_RXPTRUPD);
	nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_TXPTRUPD);
	nrf_tdm_event_clear(p_reg, NRF_TDM_EVENT_STOPPED);

	NRFX_IRQ_PRIORITY_SET(nrfx_get_irq_number(p_reg), irq_priority);
    	NRFX_IRQ_ENABLE(nrfx_get_irq_number(p_reg));
	if (enable)
	{
		nrf_tdm_int_enable(p_reg, mask);
	}
}

void tdm_int_uninit(NRF_TDM_Type * p_reg)
{
	NRFX_IRQ_DISABLE(nrfx_get_irq_number(p_reg));
}

void tdm_periph_configure(NRF_TDM_Type *            p_reg,
                          nrfx_tdm_config_t const * p_config)
{
	nrf_tdm_pins_t pins = {
		.sck_pin   = p_config->sck_pin,
		.fsync_pin = p_config->lrck_pin,
		.mck_pin   = p_config->mck_pin,
		.sdout_pin = p_config->sdout_pin,
		.sdin_pin  = p_config->sdin_pin,
	};

	nrf_tdm_configure(p_reg, &p_config->nrf_tdm_config);
	if (!p_config->skip_psel_cfg) {
		nrf_tdm_pins_set(p_reg, &pins);
	}
}

void tdm_init(struct tdm_nrfx_drv_data * drv_data,
              nrfx_tdm_config_t const  * p_config,
              nrfx_tdm_data_handler_t    handler)
{
	//volatile bool wait = true;
	//while(wait);
	printk("II\n");
	nrfx_tdm_cb_t * p_cb = &m_cb[0];
	configure_pins(p_config);

	tdm_periph_configure(drv_data->p_tdm->p_reg, p_config);

	p_cb->handler = handler;
	p_cb->skip_gpio_cfg = p_config->skip_gpio_cfg;
	p_cb->skip_psel_cfg = p_config->skip_psel_cfg;

	tdm_int_init(drv_data->p_tdm->p_reg,
		     NRF_TDM_INT_RXPTRUPD_MASK_MASK |
                     NRF_TDM_INT_TXPTRUPD_MASK_MASK |
                     NRF_TDM_INT_STOPPED_MASK_MASK,
                     p_config->irq_priority,
                     false);

	printk("IX\n");
}

static int trigger_start(const struct device *dev)
{
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	const struct tdm_nrfx_drv_cfg *drv_cfg = dev->config;
	nrfx_err_t err;
	int ret;
	const nrfx_tdm_config_t *nrfx_cfg = (drv_data->active_dir == I2S_DIR_TX)
					    ? &drv_data->tx.nrfx_cfg
					    : &drv_data->rx.nrfx_cfg;

	tdm_init(drv_data, nrfx_cfg, drv_cfg->data_handler);

	drv_data->state = I2S_STATE_RUNNING;

#if NRF_TDM_HAS_CLKCONFIG
	nrf_tdm_clk_configure(drv_cfg->i2s.p_reg,
			      drv_cfg->clk_src == ACLK ? NRF_TDM_CLKSRC_ACLK
						       : NRF_TDM_CLKSRC_PCLK32M,
			      false);
#endif

	ret = start_transfer(drv_data);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int tdm_nrfx_trigger(const struct device *dev,
			    enum i2s_dir dir, enum i2s_trigger_cmd cmd)
{
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	bool configured = false;
	bool cmd_allowed;
	printk("TT\n");
	/* This driver does not use the I2S_STATE_NOT_READY value.
	 * Instead, if a given stream is not configured, the respective
	 * flag (tx_configured or rx_configured) is cleared.
	 */
	if (dir == I2S_DIR_BOTH) {
		configured = drv_data->tx_configured && drv_data->rx_configured;
	} else if (dir == I2S_DIR_TX) {
		configured = drv_data->tx_configured;
	} else if (dir == I2S_DIR_RX) {
		configured = drv_data->rx_configured;
	}

	if (!configured) {
		printk("Device is not configured\n");
		return -EIO;
	}

	if (dir == I2S_DIR_BOTH &&
	    (memcmp(&drv_data->tx.nrfx_cfg,
		    &drv_data->rx.nrfx_cfg,
		    sizeof(drv_data->rx.nrfx_cfg)) != 0
	     ||
	     (drv_data->tx.cfg.block_size != drv_data->rx.cfg.block_size))) {
		printk("TX and RX configurations are different\n");
		return -EIO;
	}

	switch (cmd) {
	case I2S_TRIGGER_START:
		cmd_allowed = (drv_data->state == I2S_STATE_READY);
		break;
	case I2S_TRIGGER_STOP:
	case I2S_TRIGGER_DRAIN:
		cmd_allowed = (drv_data->state == I2S_STATE_RUNNING);
		break;
	case I2S_TRIGGER_DROP:
		cmd_allowed = configured;
		break;
	case I2S_TRIGGER_PREPARE:
		cmd_allowed = (drv_data->state == I2S_STATE_ERROR);
		break;
	default:
		printk("Invalid trigger: %d\n", cmd);
		return -EINVAL;
	}

	if (!cmd_allowed) {
		printk("Not allowed\n");
		return -EIO;
	}

	/* For triggers applicable to the RUNNING state (i.e. STOP, DRAIN,
	 * and DROP), ensure that the command is applied to the streams
	 * that are currently active (this device cannot e.g. stop only TX
	 * without stopping RX).
	 */
	if (drv_data->state == I2S_STATE_RUNNING &&
	    drv_data->active_dir != dir) {
		printk("Inappropriate trigger (%d/%d), active stream(s): %d\n",
			cmd, dir, drv_data->active_dir);
		return -EINVAL;
	}

	switch (cmd) {
	case I2S_TRIGGER_START:
		printk("T1\n");
		drv_data->stop = false;
		drv_data->discard_rx = false;
		drv_data->active_dir = dir;
		drv_data->next_tx_buffer_needed = false;
		return trigger_start(dev);

	case I2S_TRIGGER_STOP:
		printk("T2\n");
		drv_data->state = I2S_STATE_STOPPING;
		drv_data->stop = true;
		return 0;

	case I2S_TRIGGER_DRAIN:
		printk("T3\n");
		drv_data->state = I2S_STATE_STOPPING;
		/* If only RX is active, DRAIN is equivalent to STOP. */
		drv_data->stop = (drv_data->active_dir == I2S_DIR_RX);
		return 0;

	case I2S_TRIGGER_DROP:
		printk("T4\n");
		if (drv_data->state != I2S_STATE_READY) {
			drv_data->discard_rx = true;
			tdm_stop(drv_data->p_tdm->p_reg);
		}
		purge_queue(dev, dir);
		drv_data->state = I2S_STATE_READY;
		return 0;

	case I2S_TRIGGER_PREPARE:
		printk("T5\n");
		purge_queue(dev, dir);
		drv_data->state = I2S_STATE_READY;
		return 0;

	default:
		printk("T6\n");
		printk("Invalid trigger: %d\n", cmd);
		return -EINVAL;
	}
	printk("TX\n");
}

static void init_clock_manager(const struct device *dev)
{

}

static void tdm_uninit(NRF_TDM_Type * p_reg)
{
	nrfx_tdm_cb_t * p_cb = &m_cb[0];
	tdm_stop(p_reg);
	tdm_int_uninit(p_reg);

	if (!p_cb->skip_gpio_cfg)
	{
		deconfigure_pins(p_reg);
	}
}

static void data_handler(const struct device *dev,
			 const tdm_buffers_t *released,
			 uint32_t status)
{
	printk("DD\n");
	struct tdm_nrfx_drv_data *drv_data = dev->data;
	bool stop_transfer = false;

	if (status & NRFX_TDM_STATUS_TRANSFER_STOPPED) {
		if (drv_data->state == I2S_STATE_STOPPING) {
			drv_data->state = I2S_STATE_READY;
			printk("D1\n");
		}
		if (drv_data->last_tx_buffer) {
			/* Usually, these pointers are equal, i.e. the last TX
			 * buffer that were to be transferred is released by the
			 * driver after it stops. The last TX buffer pointer is
			 * then set to NULL here so that the buffer can be freed
			 * below, just as any other TX buffer released by the
			 * driver. However, it may happen that the buffer is not
			 * released this way, for example, when the transfer
			 * ends with an error because an RX buffer allocation
			 * fails. In such case, the last TX buffer needs to be
			 * freed here.
			 */
			printk("D2\n");
			if (drv_data->last_tx_buffer != released->p_tx_buffer) {
				printk("D3\n");
				free_tx_buffer(drv_data,
					       drv_data->last_tx_buffer);
			}
			drv_data->last_tx_buffer = NULL;
		}
		tdm_uninit(drv_data->p_tdm->p_reg);
		//xxxif (drv_data->request_clock) {
		//xxx	(void)onoff_release(drv_data->clk_mgr);
		//xxx}
	}

	if (released == NULL) {
		printk("D4\n");
		/* This means that buffers for the next part of the transfer
		 * were not supplied and the previous ones cannot be released
		 * yet, as pointers to them were latched in the I2S registers.
		 * It is not an error when the transfer is to be stopped (those
		 * buffers will be released after the transfer actually stops).
		 */
		if (drv_data->state != I2S_STATE_STOPPING) {
			printk("Next buffers not supplied on time\n");
			drv_data->state = I2S_STATE_ERROR;
		}
		tdm_stop(drv_data->p_tdm->p_reg);
		return;
	}

	if (released->p_rx_buffer) {
		printk("D5\n");
		if (drv_data->discard_rx) {
			free_rx_buffer(drv_data, released->p_rx_buffer);
		} else {
			struct tdm_buf buf = {
				.mem_block = released->p_rx_buffer,
				.size = released->buffer_size * sizeof(uint32_t)
			};
			int ret = k_msgq_put(&drv_data->rx_queue,
					     &buf,
					     K_NO_WAIT);
			if (ret < 0) {
				printk("No room in RX queue\n");
				drv_data->state = I2S_STATE_ERROR;
				stop_transfer = true;

				free_rx_buffer(drv_data, released->p_rx_buffer);
			} else {
				LOG_DBG("Queued RX %p", released->p_rx_buffer);

				/* If the TX direction is not active and
				 * the transfer should be stopped after
				 * the current block, stop the reception.
				 */
				if (drv_data->active_dir == I2S_DIR_RX &&
				    drv_data->stop) {
					drv_data->discard_rx = true;
					stop_transfer = true;
				}
			}
		}
	}

	if (released->p_tx_buffer) {
		printk("D6\n");
		/* If the last buffer that was to be transferred has just been
		 * released, it is time to stop the transfer.
		 */
		if (released->p_tx_buffer == drv_data->last_tx_buffer) {
			drv_data->discard_rx = true;
			stop_transfer = true;
		} else {
			free_tx_buffer(drv_data, released->p_tx_buffer);
		}
	}

	if (stop_transfer) {
		printk("D7\n");
		tdm_stop(drv_data->p_tdm->p_reg);
	} else if (status & NRFX_TDM_STATUS_NEXT_BUFFERS_NEEDED) {
		tdm_buffers_t next = { 0 };
		printk("D8\n");
		if (drv_data->active_dir != I2S_DIR_RX) { /* -> TX active */
			if (drv_data->stop) {
				printk("D9\n");
				/* If the stream is to be stopped, don't get
				 * the next TX buffer from the queue, instead
				 * supply the one used last time (it won't be
				 * transferred, the stream will stop right
				 * before this buffer would be started again).
				 */
				next.p_tx_buffer = drv_data->last_tx_buffer;
				next.buffer_size = 1;
			} else if (get_next_tx_buffer(drv_data, &next)) {
				printk("DA\n");
				/* Next TX buffer successfully retrieved from
				 * the queue, nothing more to do here.
				 */
			} else if (drv_data->state == I2S_STATE_STOPPING) {
				printk("DB\n");
				/* If there are no more TX blocks queued and
				 * the current state is STOPPING (so the DRAIN
				 * command was triggered) it is time to finish
				 * the transfer.
				 */
				drv_data->stop = true;
				/* Supply the same buffer as last time; it will
				 * not be transferred anyway, as the transfer
				 * will be stopped earlier.
				 */
				next.p_tx_buffer = drv_data->last_tx_buffer;
				next.buffer_size = 1;
			} else {
				printk("DC\n");
				/* Next TX buffer cannot be supplied now.
				 * Defer it to when the user writes more data.
				 */
				drv_data->next_tx_buffer_needed = true;
				return;
			}
		}

		(void)supply_next_buffers(drv_data, &next);

	}
	printk("DX\n");
}

static const struct i2s_driver_api tdm_nrf_drv_api = {
	.configure = tdm_nrfx_configure,
	.config_get = tdm_nrfx_config_get,
	.read = tdm_nrfx_read,
	.write = tdm_nrfx_write,
	.trigger = tdm_nrfx_trigger,
};

#define TDM(idx) DT_NODELABEL(tdm##idx)
#define TDM_CLK_SRC(idx) DT_STRING_TOKEN(TDM(idx), clock_source)

#define TDM_NRFX_DEVICE(idx)						     \
	static struct tdm_buf tx_msgs##idx[CONFIG_TDM_NRFX_TX_BLOCK_COUNT];  \
	static struct tdm_buf rx_msgs##idx[CONFIG_TDM_NRFX_RX_BLOCK_COUNT];  \
	static void data_handler##idx(tdm_buffers_t const *p_released, \
				      uint32_t status)			     \
	{								     \
		data_handler(DEVICE_DT_GET(TDM(idx)), p_released, status);   \
	}								     \
	PINCTRL_DT_DEFINE(TDM(idx));					     \
	static const struct tdm_nrfx_drv_cfg tdm_nrfx_cfg##idx = {	     \
		.data_handler = data_handler##idx,			     \
		.tdm = NRFX_TDM_INSTANCE(idx),				     \
		.nrfx_def_cfg = NRFX_TDM_DEFAULT_CONFIG(		     \
			NRF_TDM_PIN_NOT_CONNECTED,			     \
			NRF_TDM_PIN_NOT_CONNECTED,			     \
			NRF_TDM_PIN_NOT_CONNECTED,			     \
			NRF_TDM_PIN_NOT_CONNECTED,			     \
			NRF_TDM_PIN_NOT_CONNECTED),			     \
		.nrfx_def_cfg.skip_gpio_cfg = true,			     \
		.nrfx_def_cfg.skip_psel_cfg = true,			     \
		.pcfg = PINCTRL_DT_DEV_CONFIG_GET(TDM(idx)),		     \
		.clk_src = TDM_CLK_SRC(idx),				     \
	};								     \
	static struct tdm_nrfx_drv_data tdm_nrfx_data##idx = {		     \
		.state = I2S_STATE_READY,				     \
		.p_tdm = &tdm_nrfx_cfg##idx.tdm				     \
	};								     \
	static int tdm_nrfx_init##idx(const struct device *dev)		     \
	{								     \
		IRQ_CONNECT(DT_IRQN(TDM(idx)), DT_IRQ(TDM(idx), priority),   \
			    tdm_##idx##_irq_handler, DEVICE_DT_GET(TDM(idx)),0);	     \
		const struct tdm_nrfx_drv_cfg *drv_cfg = dev->config;	     \
		int err = pinctrl_apply_state(drv_cfg->pcfg,		     \
					      PINCTRL_STATE_DEFAULT);	     \
		if (err < 0) {						     \
			return err;					     \
		}							     \
		k_msgq_init(&tdm_nrfx_data##idx.tx_queue,		     \
			    (char *)tx_msgs##idx, sizeof(struct tdm_buf),    \
			    ARRAY_SIZE(tx_msgs##idx));			     \
		k_msgq_init(&tdm_nrfx_data##idx.rx_queue,		     \
			    (char *)rx_msgs##idx, sizeof(struct tdm_buf),    \
			    ARRAY_SIZE(rx_msgs##idx));			     \
		init_clock_manager(dev);				     \
		return 0;						     \
	}								     \
									     \
	DEVICE_DT_DEFINE(TDM(idx), tdm_nrfx_init##idx, NULL,		     \
			 &tdm_nrfx_data##idx, &tdm_nrfx_cfg##idx,	     \
			 POST_KERNEL, CONFIG_I2S_INIT_PRIORITY,		     \
			 &tdm_nrf_drv_api);

#ifdef CONFIG_HAS_HW_NRF_TDM130
TDM_NRFX_DEVICE(130);
#endif
