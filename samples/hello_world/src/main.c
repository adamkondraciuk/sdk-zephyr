#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/drivers/spi.h>

#define SPI_NODE DT_NODELABEL(spi00)
static const struct device *spi_dev = DEVICE_DT_GET(SPI_NODE);

struct spi_config spis_cfg = {
          .frequency = 0, /* Frequency is controlled entirely by the Master */
        .operation = SPI_WORD_SET(8) | SPI_OP_MODE_SLAVE,
};



int main(void)
{
	int err;
	int ret;
	int ret_val;
	uint8_t rx_buffer[32] = {
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89,
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89,
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89,
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89
	                        };
	uint8_t tx_buffer[32] = {
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89,
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89,
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89,
	                  0xab,0xcd,0xef,0xee,0xaa,0xff,0x12,0x89
	                        };


    if (!device_is_ready(spi_dev)) {
		printk("UART device not found!");
		return 0;
	}

	while (true) {
			printk("waiting for request\r\n");
		
    struct spi_buf tx_buf = { .buf = tx_buffer, .len = sizeof(tx_buffer) };
    struct spi_buf_set tx_bufs = { .buffers = &tx_buf, .count = 1 };

    struct spi_buf rx_buf = { .buf = rx_buffer, .len = sizeof(rx_buffer) };
    struct spi_buf_set rx_bufs = { .buffers = &rx_buf, .count = 1 };
  		ret = spi_transceive(spi_dev, &spis_cfg, &tx_bufs, &rx_bufs);
        printk("received %d bytes:", ret);
        for (int i = 0; i < ret && i < sizeof(rx_buffer); i++) {
                printk(" %02x", rx_buffer[i]);
        }
        printk("\n");

  		printk("spi work done\n");
				
	}

	/* return to idle thread */
	return 0;
}
