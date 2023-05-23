/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#ifndef ADE9000ESTIMATES_H
#define ADE9000ESTIMATES_H

#include "ade9000Interface.h"

typedef struct ade_est_data_t {
   int32_t IA;
   int32_t VAB;
   int32_t IB;
   int32_t VBC;
   int32_t IC;
   int32_t VAC;
   int32_t ModuleTemperature;
   int32_t Imean;
   int32_t Umean;
   int32_t AWATT;
   int32_t BWATT;
   int32_t CWATT;
   int32_t AVAR;
   int32_t BVAR;
   int32_t CVAR;
   int32_t AVA;
   int32_t BVA;
   int32_t CVA;
   int32_t Angle0;
   int32_t Angle1;
   int32_t Angle2;
   int32_t AWATTHR;
   int32_t BWATTHR;
   int32_t CWATTHR;
   int32_t AVARHR;
   int32_t BVARHR;
   int32_t CVARHR;
   int32_t IAW;
   int32_t VAW;
   int32_t IBW;
   int32_t VBW;
   int32_t ICW;
   int32_t VCW;
   int32_t IN;
   int32_t VN;
   int32_t P;
   int32_t Q;
   int32_t S;
   uint32_t RPM;
   uint32_t PMS;
   uint32_t FrameCounter;
   uint64_t Timestamp;
   uint32_t Checksum;
} ade_est_data_t;

bool ADE9000EstimatesEstimate1sRead(uint32_t idx, uint32_t *p_est);

bool ADE9000EstimatesEstimate1sStart(void);

#endif // ADE9000ESTIMATES_H