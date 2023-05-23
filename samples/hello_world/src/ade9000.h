/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#ifndef ADE9000_H
#define ADE9000_H

#include <stdbool.h>

bool ADE9000Init(void);

void ADE9000MeasParamsSet(void);

bool ADE9000ConversionStart(void);

void ADE9000StreamRead(void);

#endif // ADE9000_H