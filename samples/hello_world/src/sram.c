#include "sram.h"

//TODO: Get it from DeviceTree
#define SRAM_CS_PIN   29 // 31
#define SRAM_MOSI_PIN 0X20 // 32
#define SRAM_MISO_PIN 0x18 // 24
#define SRAM_SCK_PIN  22//0x13 // default 22

bool SRAMTest(void)
{
    uint8_t some_data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint8_t *rcv_data = NULL;
	if (!SRAMWrite(0, some_data, sizeof(some_data))) {
        return false;
    }
    rcv_data = SRAMRead(0, sizeof(some_data));
    if (rcv_data == NULL || memcmp(some_data, rcv_data, sizeof(some_data))) {
        return false;
    }
    return true;
}
