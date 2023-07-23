/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include <zephyr/kernel.h>

#include "ade9000Estimates.h"
#include "ade9000_registers.h"


// TODO: Get from config
#define LOG_ADE9000ESTIMATES 0

#if defined(LOG_ADE9000ESTIMATES) && (LOG_ADE9000ESTIMATES== 1)
#include <zephyr/sys/printk.h>
#endif

typedef struct estimates_seq_t{
  enum Eade9000_register Addr;
  int32_t DstOffs;
  uint8_t Size;
} estimates_seq_t;

static const estimates_seq_t EstimatesReadSequenceTable[] = {
    {R_AIRMS, offsetof(ade_est_data_t, IAW), 4 },
    {R_AVRMS, offsetof(ade_est_data_t, VAW), 4 },
    {R_BIRMS, offsetof(ade_est_data_t, IBW), 4 },
    {R_BVRMS, offsetof(ade_est_data_t, VBW), 4 },
    {R_CIRMS, offsetof(ade_est_data_t, ICW), 4 },
    {R_CVRMS, offsetof(ade_est_data_t, VCW), 4 },

    {R_AWATT, offsetof(ade_est_data_t, AWATT), 4 },
    {R_BWATT, offsetof(ade_est_data_t, BWATT), 4 },
    {R_CWATT, offsetof(ade_est_data_t, CWATT), 4 },

    {R_AVAR, offsetof(ade_est_data_t, AVAR), 4 },
    {R_BVAR, offsetof(ade_est_data_t, BVAR), 4 },
    {R_CVAR, offsetof(ade_est_data_t, CVAR), 4 },

    {R_AVA, offsetof(ade_est_data_t, AVA), 4 },
    {R_BVA, offsetof(ade_est_data_t, BVA), 4 },
    {R_CVA, offsetof(ade_est_data_t, CVA), 4 },

    {R_AWATTHR_HI, offsetof(ade_est_data_t, AWATTHR), 4 },
    {R_BWATTHR_HI, offsetof(ade_est_data_t, BWATTHR), 4 },
    {R_CWATTHR_HI, offsetof(ade_est_data_t, CWATTHR), 4 },

    {R_AVARHR_HI, offsetof(ade_est_data_t, AVARHR), 4 },
    {R_BVARHR_HI, offsetof(ade_est_data_t, BVARHR), 4 },
    {R_CVARHR_HI, offsetof(ade_est_data_t, CVARHR), 4 },

    {R_ANGL_VA_IA, offsetof(ade_est_data_t, Angle0), 2 },
    {R_ANGL_VB_IB, offsetof(ade_est_data_t, Angle1), 2 },
    {R_ANGL_VC_IC, offsetof(ade_est_data_t, Angle2), 2 },
};

bool ADE9000EstimatesAllEstimates1sRead(void *p_est)
{
	//__ASSERT(p_est);
	uint32_t * read_data = (uint32_t *)p_est;
	for(uint32_t i = 0; i < ARRAY_SIZE(EstimatesReadSequenceTable); i ++) {
		if (!ADE9000InterfaceRegRead(EstimatesReadSequenceTable[i].Addr,
            				     &read_data[i],
            				     EstimatesReadSequenceTable[i].Size)) {
			return false;
		}
		while(!ADE9000InterfaceTransferDone()) {
			//TODO: use semaphores?
			k_usleep(10);
		}
	}
	return true;
}

bool ADE9000EstimatesEstimate1sRead(uint8_t idx, void *p_est)
{
    if (idx >= ARRAY_SIZE(EstimatesReadSequenceTable)) {
      return false;
    }

    ADE9000InterfaceRegRead(EstimatesReadSequenceTable[idx].Addr,
            (uint32_t *)p_est,
            EstimatesReadSequenceTable[idx].Size);
#if defined(LOG_ADE9000ESTIMATES) && (LOG_ADE9000ESTIMATES== 1)
    while(!ADE9000InterfaceTransferDone());
    printk("Estimate(%s) = %d\n", EstimStringArray[idx], (uint32_t)*p_est);
#endif
    return true;
}
