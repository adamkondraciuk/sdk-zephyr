/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#ifndef SRAMINTERFACE_H
#define SRAMINTERFACE_H

#include <nrfx_spim.h>
#include <stdlib.h>
#include <string.h>

bool SRAMInit(void);

bool SRAMWrite(uint32_t addr, uint8_t const * const data, size_t length);

uint8_t * SRAMRead(uint32_t addr, size_t length);

bool SRAMIDRead(uint8_t * id);

void SRAMReset(void);

#endif // SRAMINTERFACE_H
