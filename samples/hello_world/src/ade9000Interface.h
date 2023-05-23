/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#ifndef ADE9000INTERFACE_H
#define ADE9000INTERFACE_H

#include <nrfx_spim.h>
#include <stdlib.h>
#include <string.h>

bool ADE9000InterfaceInit(void);

bool ADE9000InterfaceRegWrite(uint16_t addr , uint32_t data, unsigned char length);

bool ADE9000InterfaceRegRead(uint16_t addr , uint32_t *rcv_data, unsigned char rcv_lenght);

bool ADE9000InterfaceRegWriteAndCheck(uint16_t addr , uint32_t data , unsigned char length);

bool ADE9000InterfaceTransferDone(void);

#endif // ADE9000INTERFACE_H