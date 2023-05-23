/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include <zephyr/sys/util.h>

#define NB_OF_PHASES			3
#define NB_OF_PM_INPUT_CHANNELS 	(NB_OF_PHASES * 2)

typedef struct SADE9000Calib {
   int32_t Coeffs[NB_OF_PM_INPUT_CHANNELS];
   int32_t Offsets[NB_OF_PM_INPUT_CHANNELS];
   int32_t ActPowerCoeffs[NB_OF_PHASES];
   int32_t ActPowerOffsets[NB_OF_PHASES];
   int32_t ReactPowerCoeffs[NB_OF_PHASES];
   int32_t ReactPowerOffsets[NB_OF_PHASES];
   int32_t AppPowerCoeffs[NB_OF_PHASES];
   int32_t AppPowerOffsets[NB_OF_PHASES];
   int32_t PhaseCoeffs[NB_OF_PHASES];
   uint16_t CRC;
} SEleFlashDataCal;

enum Eade9000_register{
 R_AIGAIN                          = 0x0000 ,
 R_AIGAIN0                         = 0x0010 ,
 R_AIGAIN1                         = 0x0020 ,
 R_AIGAIN2                         = 0x0030 ,
 R_AIGAIN3                         = 0x0040 ,
 R_AIGAIN4                         = 0x0050 ,
 R_APHCAL0                         = 0x0060 ,     
 R_APHCAL1                         = 0x0070 ,     
 R_APHCAL2                         = 0x0080 ,     
 R_APHCAL3                         = 0x0090 ,     
 R_APHCAL4                         = 0x00A0 ,
 R_AVGAIN                          = 0x00B0 ,     
 R_AIRMSOS                         = 0x00C0 ,
 R_AVRMSOS                         = 0x00D0 ,
 R_APGAIN                          = 0x00E0 ,
 R_AWATTOS                         = 0x00F0 ,
 R_AVAROS                          = 0x0100 ,
 R_AFWATTOS                        = 0x0110 ,
 R_AFVAROS                         = 0x0120 ,
 R_AIFRMSOS                        = 0x0130 ,
 R_AVFRMSOS                        = 0x0140 ,
 R_AVRMSONEOS                      = 0x0150 ,
 R_AIRMSONEOS                      = 0x0160 ,
 R_AVRMS1012OS                     = 0x0170 ,
 R_AIRMS1012OS                     = 0x0180 ,
 R_BIGAIN                          = 0x0200 ,
 R_BIGAIN0                         = 0x0210 ,
 R_BIGAIN1                         = 0x0220 ,
 R_BIGAIN2                         = 0x0230 ,
 R_BIGAIN3                         = 0x0240 ,
 R_BIGAIN4                         = 0x0250 ,
 R_BPHCAL0                         = 0x0260 ,
 R_BPHCAL1                         = 0x0270 ,
 R_BPHCAL2                         = 0x0280 ,
 R_BPHCAL3                         = 0x0290 ,
 R_BPHCAL4                         = 0x02A0 ,
 R_BVGAIN                          = 0x02B0 ,
 R_BIRMSOS                         = 0x02C0 ,
 R_BVRMSOS                         = 0x02D0 ,
 R_BPGAIN                          = 0x02E0 ,
 R_BWATTOS                         = 0x02F0 ,
 R_BVAROS                          = 0x0300 ,
 R_BFWATTOS                        = 0x0310 ,
 R_BFVAROS                         = 0x0320 ,
 R_BIFRMSOS                        = 0x0330 ,
 R_BVFRMSOS                        = 0x0340 ,
 R_BVRMSONEOS                      = 0x0350 ,
 R_BIRMSONEOS                      = 0x0360 ,
 R_BVRMS1012OS                     = 0x0370 ,
 R_BIRMS1012OS                     = 0x0380 ,
 R_CIGAIN                          = 0x0400 ,
 R_CIGAIN0                         = 0x0410 ,
 R_CIGAIN1                         = 0x0420 ,
 R_CIGAIN2                         = 0x0430 ,
 R_CIGAIN3                         = 0x0440 ,
 R_CIGAIN4                         = 0x0450 ,
 R_CPHCAL0                         = 0x0460 ,
 R_CPHCAL1                         = 0x0470 ,
 R_CPHCAL2                         = 0x0480 ,
 R_CPHCAL3                         = 0x0490 ,
 R_CPHCAL4                         = 0x04A0 ,
 R_CVGAIN                          = 0x04B0 ,
 R_CIRMSOS                         = 0x04C0 ,
 R_CVRMSOS                         = 0x04D0 ,
 R_CPGAIN                          = 0x04E0 ,
 R_CWATTOS                         = 0x04F0 ,
 R_CVAROS                          = 0x0500 ,
 R_CFWATTOS                        = 0x0510 ,
 R_CFVAROS                         = 0x0520 ,
 R_CIFRMSOS                        = 0x0530 ,
 R_CVFRMSOS                        = 0x0540 ,
 R_CVRMSONEOS                      = 0x0550 ,
 R_CIRMSONEOS                      = 0x0560 ,
 R_CVRMS1012OS                     = 0x0570 ,
 R_CIRMS1012OS                     = 0x0580 ,
 R_CONFIG0                         = 0x0600 ,
 R_MTTHR_L0                        = 0x0610 ,
 R_MTTHR_L1                        = 0x0620 ,
 R_MTTHR_L2                        = 0x0630 ,
 R_MTTHR_L3                        = 0x0640 ,
 R_MTTHR_L4                        = 0x0650 ,
 R_MTTHR_H0                        = 0x0660 ,
 R_MTTHR_H1                        = 0x0670 ,
 R_MTTHR_H2                        = 0x0680 ,
 R_MTTHR_H3                        = 0x0690 ,
 R_MTTHR_H4                        = 0x06A0 ,
 R_NIRMSOS                         = 0x06B0 ,
 R_ISUMRMSOS                       = 0x06C0 ,
 R_NIGAIN                          = 0x06D0 ,
 R_NPHCAL                          = 0x06E0 ,
 R_NIRMSONEOS                      = 0x06F0 ,
 R_NIRMS1012OS                     = 0x0700 ,
 R_VNOM                            = 0x0710 ,
 R_DICOEFF                         = 0x0720 ,
 R_ISUMLVL                         = 0x0730 ,
 R_AI_PCF                          = 0x20A0 ,
 R_AV_PCF                          = 0x20B0 ,
 R_AIRMS                           = 0x20C0 ,
 R_AVRMS                           = 0x20D0 ,
 R_AIFRMS                          = 0x20E0 ,
 R_AVFRMS                          = 0x20F0 ,
 R_AWATT                           = 0x2100 ,
 R_AVAR                            = 0x2110 ,
 R_AVA                             = 0x2120 ,
 R_AFWATT                          = 0x2130 ,
 R_AFVAR                           = 0x2140 ,
 R_AFVA                            = 0x2150 ,
 R_APF                             = 0x2160 ,
 R_AVTHD                           = 0x2170 ,
 R_AITHD                           = 0x2180 ,
 R_AIRMSONE                        = 0x2190 ,
 R_AVRMSONE                        = 0x21A0 ,
 R_AIRMS1012                       = 0x21B0 ,
 R_AVRMS1012                       = 0x21C0 ,
 R_AMTREGION                       = 0x21D0 ,
 R_BI_PCF                          = 0x22A0 ,
 R_BV_PCF                          = 0x22B0 ,
 R_BIRMS                           = 0x22C0 ,
 R_BVRMS                           = 0x22D0 ,
 R_BIFRMS                          = 0x22E0 ,
 R_BVFRMS                          = 0x22F0 ,
 R_BWATT                           = 0x2300 ,
 R_BVAR                            = 0x2310 ,
 R_BVA                             = 0x2320 ,
 R_BFWATT                          = 0x2330 ,
 R_BFVAR                           = 0x2340 ,
 R_BFVA                            = 0x2350 ,
 R_BPF                             = 0x2360 ,
 R_BVTHD                           = 0x2370 ,
 R_BITHD                           = 0x2380 ,
 R_BIRMSONE                        = 0x2390 ,
 R_BVRMSONE                        = 0x23A0 ,
 R_BIRMS1012                       = 0x23B0 ,
 R_BVRMS1012                       = 0x23C0 ,
 R_BMTREGION                       = 0x23D0 ,
 R_CI_PCF                          = 0x24A0 ,
 R_CV_PCF                          = 0x24B0 ,
 R_CIRMS                           = 0x24C0 ,
 R_CVRMS                           = 0x24D0 ,
 R_CIFRMS                          = 0x24E0 ,
 R_CVFRMS                          = 0x24F0 ,
 R_CWATT                           = 0x2500 ,
 R_CVAR                            = 0x2510 ,
 R_CVA                             = 0x2520 ,
 R_CFWATT                          = 0x2530 ,
 R_CFVAR                           = 0x2540 ,
 R_CFVA                            = 0x2550 ,
 R_CPF                             = 0x2560 ,
 R_CVTHD                           = 0x2570 ,
 R_CITHD                           = 0x2580 ,
 R_CIRMSONE                        = 0x2590 ,
 R_CVRMSONE                        = 0x25A0 ,
 R_CIRMS1012                       = 0x25B0 ,
 R_CVRMS1012                       = 0x25C0 ,
 R_CMTREGION                       = 0x25D0 ,
 R_NI_PCF                          = 0x2650 ,
 R_NIRMS                           = 0x2660 ,
 R_NIRMSONE                        = 0x2670 ,
 R_NIRMS1012                       = 0x2680 ,
 R_ISUMRMS                         = 0x2690 ,
 R_VERSION2                        = 0x26A0 ,
 R_AWATT_ACC                       = 0x2E50 ,
 R_AWATTHR_LO                      = 0x2E60 ,
 R_AWATTHR_HI                      = 0x2E70 ,
 R_AVAR_ACC                        = 0x2EF0 ,
 R_AVARHR_LO                       = 0x2F00 ,
 R_AVARHR_HI                       = 0x2F10 ,
 R_AVA_ACC                         = 0x2F90 ,
 R_AVAHR_LO                        = 0x2FA0 ,
 R_AVAHR_HI                        = 0x2FB0 ,
 R_AFWATT_ACC                      = 0x3030 ,
 R_AFWATTHR_LO                     = 0x3040 ,
 R_AFWATTHR_HI                     = 0x3050 ,
 R_AFVAR_ACC                       = 0x30D0 ,
 R_AFVARHR_LO                      = 0x30E0 ,
 R_AFVARHR_HI                      = 0x30F0 ,
 R_AFVA_ACC                        = 0x3170 ,
 R_AFVAHR_LO                       = 0x3180 ,
 R_AFVAHR_HI                       = 0x3190 ,
 R_BWATT_ACC                       = 0x3210 ,
 R_BWATTHR_LO                      = 0x3220 ,
 R_BWATTHR_HI                      = 0x3230 ,
 R_BVAR_ACC                        = 0x32B0 ,
 R_BVARHR_LO                       = 0x32C0 ,
 R_BVARHR_HI                       = 0x32D0 ,
 R_BVA_ACC                         = 0x3350 ,
 R_BVAHR_LO                        = 0x3360 ,
 R_BVAHR_HI                        = 0x3370 ,
 R_BFWATT_ACC                      = 0x33F0 ,
 R_BFWATTHR_LO                     = 0x3400 ,
 R_BFWATTHR_HI                     = 0x3410 ,
 R_BFVAR_ACC                       = 0x3490 ,
 R_BFVARHR_LO                      = 0x34A0 ,
 R_BFVARHR_HI                      = 0x34B0 ,
 R_BFVA_ACC                        = 0x3530 ,
 R_BFVAHR_LO                       = 0x3540 ,
 R_BFVAHR_HI                       = 0x3550 ,
 R_CWATT_ACC                       = 0x35D0 ,
 R_CWATTHR_LO                      = 0x35E0 ,
 R_CWATTHR_HI                      = 0x35F0 ,
 R_CVAR_ACC                        = 0x3670 ,
 R_CVARHR_LO                       = 0x3680 ,
 R_CVARHR_HI                       = 0x3690 ,
 R_CVA_ACC                         = 0x3710 ,
 R_CVAHR_LO                        = 0x3720 ,
 R_CVAHR_HI                        = 0x3730 ,
 R_CFWATT_ACC                      = 0x37B0 ,
 R_CFWATTHR_LO                     = 0x37C0 ,
 R_CFWATTHR_HI                     = 0x37D0 ,
 R_CFVAR_ACC                       = 0x3850 ,
 R_CFVARHR_LO                      = 0x3860 ,
 R_CFVARHR_HI                      = 0x3870 ,
 R_CFVA_ACC                        = 0x38F0 ,
 R_CFVAHR_LO                       = 0x3900 ,
 R_CFVAHR_HI                       = 0x3910 ,
 R_PWATT_ACC                       = 0x3970 ,
 R_NWATT_ACC                       = 0x39B0 ,
 R_PVAR_ACC                        = 0x39F0 ,
 R_NVAR_ACC                        = 0x3A30 ,
 R_IPEAK                           = 0x4000 ,
 R_VPEAK                           = 0x4010 ,
 R_STATUS0                         = 0x4020 ,
 R_STATUS1                         = 0x4030 ,
 R_EVENT_STATUS                    = 0x4040 ,
 R_MASK0                           = 0x4050 ,
 R_MASK1                           = 0x4060 ,
 R_EVENT_MASK                      = 0x4070 ,
 R_OILVL                           = 0x4090 ,
 R_OIA                             = 0x40A0 ,
 R_OIB                             = 0x40B0 ,
 R_OIC                             = 0x40C0 ,
 R_OIN                             = 0x40D0 ,
 R_USER_PERIOD                     = 0x40E0 ,
 R_VLEVEL                          = 0x40F0 ,
 R_DIP_LVL                         = 0x4100 ,
 R_DIPA                            = 0x4110 ,
 R_DIPB                            = 0x4120 ,
 R_DIPC                            = 0x4130 ,
 R_SWELL_LVL                       = 0x4140 ,
 R_SWELLA                          = 0x4150 ,
 R_SWELLB                          = 0x4160 ,
 R_SWELLC                          = 0x4170 ,
 R_APERIOD                         = 0x4180 ,
 R_BPERIOD                         = 0x4190 ,
 R_CPERIOD                         = 0x41A0 ,
 R_COM_PERIOD                      = 0x41B0 ,
 R_ACT_NL_LVL                      = 0x41C0 ,
 R_REACT_NL_LVL                    = 0x41D0 ,
 R_APP_NL_LVL                      = 0x41E0 ,
 R_PHNOLOAD                        = 0x41F0 ,
 R_WTHR                            = 0x4200 ,
 R_VARTHR                          = 0x4210 ,
 R_VATHR                           = 0x4220 ,
 R_LAST_DATA_32                    = 0x4230 ,
 R_ADC_REDIRECT                    = 0x4240 ,
 R_CF_LCFG                         = 0x4250 ,
 R_PART_ID                         = 0x4720 ,
 R_TEMP_TRIM                       = 0x4740 ,
 R_RUN                             = 0x4800 ,
 R_CONFIG1                         = 0x4810 ,
 R_ANGL_VA_VB                      = 0x4820 ,
 R_ANGL_VB_VC                      = 0x4830 ,
 R_ANGL_VA_VC                      = 0x4840 ,
 R_ANGL_VA_IA                      = 0x4850 ,
 R_ANGL_VB_IB                      = 0x4860 ,
 R_ANGL_VC_IC                      = 0x4870 ,
 R_ANGL_IA_IB                      = 0x4880 ,
 R_ANGL_IB_IC                      = 0x4890 ,
 R_ANGL_IA_IC                      = 0x48A0 ,
 R_DIP_CYC                         = 0x48B0 ,
 R_SWELL_CYC                       = 0x48C0 ,
 R_OISTATUS                        = 0x48F0 ,
 R_CFMODE                          = 0x4900 ,
 R_COMPMODE                        = 0x4910 ,
 R_ACCMODE                         = 0x4920 ,
 R_CONFIG3                         = 0x4930 ,
 R_CF1DEN                          = 0x4940 ,
 R_CF2DEN                          = 0x4950 ,
 R_CF3DEN                          = 0x4960 ,
 R_CF4DEN                          = 0x4970 ,
 R_ZXTOUT                          = 0x4980 ,
 R_ZXTHRSH                         = 0x4990 ,
 R_ZX_LP_SEL                       = 0x49A0 ,
 R_SEQ_CYC                         = 0x49C0 ,
 R_PHSIGN                          = 0x49D0 ,
 R_WFB_CFG                         = 0x4A00 ,
 R_WFB_PG_IRQEN                    = 0x4A10 ,
 R_WFB_TRG_CFG                     = 0x4A20 ,
 R_WFB_TRG_STAT                    = 0x4A30 ,
 R_CONFIG5                         = 0x4A40 ,
 R_CRC_RSLT                        = 0x4A80 ,
 R_CRC_SPI                         = 0x4A90 ,
 R_LAST_DATA_16                    = 0x4AC0 ,
 R_LAST_CMD                        = 0x4AE0 ,
 R_CONFIG2                         = 0x4AF0 ,
 R_EP_CFG                          = 0x4B00 ,
 R_PWR_TIME                        = 0x4B10 ,
 R_EGY_TIME                        = 0x4B20 ,
 R_CRC_FORCE                       = 0x4B40 ,
 R_CRC_OPTEN                       = 0x4B50 ,
 R_TEMP_CFG                        = 0x4B60 ,
 R_TEMP_RSLT                       = 0x4B70 ,
 R_PGA_GAIN                        = 0x4B90 ,
 R_CHNL_DIS                        = 0x4BA0 ,
 R_WR_LOCK                         = 0x4BF0 ,
 R_VAR_DIS                         = 0x4E00 ,
 R_RESERVED1                       = 0x4F00 ,
 R_Version                         = 0x4FE0 ,
 R_AI_SINC_DAT                     = 0x5000 ,
 R_AV_SINC_DAT                     = 0x5010 ,
 R_BI_SINC_DAT                     = 0x5020 ,
 R_BV_SINC_DAT                     = 0x5030 ,
 R_CI_SINC_DAT                     = 0x5040 ,
 R_CV_SINC_DAT                     = 0x5050 ,
 R_NI_SINC_DAT                     = 0x5060 ,
 R_AI_LPF_DAT                      = 0x5100 ,
 R_AV_LPF_DAT                      = 0x5110 ,
 R_BI_LPF_DAT                      = 0x5120 ,
 R_BV_LPF_DAT                      = 0x5130 ,
 R_CI_LPF_DAT                      = 0x5140 ,
 R_CV_LPF_DAT                      = 0x5150 ,
 R_NI_LPF_DAT                      = 0x5160 ,
 R_AV_PCF_1                        = 0x6000 ,
 R_BV_PCF_1                        = 0x6010 ,
 R_CV_PCF_1                        = 0x6020 ,
 R_NI_PCF_1                        = 0x6030 ,
 R_AI_PCF_1                        = 0x6040 ,
 R_BI_PCF_1                        = 0x6050 ,
 R_CI_PCF_1                        = 0x6060 ,
 R_AIRMS_1                         = 0x6070 ,
 R_BIRMS_1                         = 0x6080 ,
 R_CIRMS_1                         = 0x6090 ,
 R_AVRMS_1                         = 0x60A0 ,
 R_BVRMS_1                         = 0x60B0 ,
 R_CVRMS_1                         = 0x60C0 ,
 R_NIRMS_1                         = 0x60D0 ,
 R_AWATT_1                         = 0x60E0 ,
 R_BWATT_1                         = 0x60F0 ,
 R_CWATT_1                         = 0x6100 ,
 R_AVA_1                           = 0x6110 ,
 R_BVA_1                           = 0x6120 ,
 R_CVA_1                           = 0x6130 ,
 R_AVAR_1                          = 0x6140 ,
 R_BVAR_1                          = 0x6150 ,
 R_CVAR_1                          = 0x6160 ,
 R_AFVAR_1                         = 0x6170 ,
 R_BFVAR_1                         = 0x6180 ,
 R_CFVAR_1                         = 0x6190 ,
 R_APF_1                           = 0x61A0 ,
 R_BPF_1                           = 0x61B0 ,
 R_CPF_1                           = 0x61C0 ,
 R_AVTHD_1                         = 0x61D0 ,
 R_BVTHD_1                         = 0x61E0 ,
 R_CVTHD_1                         = 0x61F0 ,
 R_AITHD_1                         = 0x6200 ,
 R_BITHD_1                         = 0x6210 ,
 R_CITHD_1                         = 0x6220 ,
 R_AFWATT_1                        = 0x6230 ,
 R_BFWATT_1                        = 0x6240 ,
 R_CFWATT_1                        = 0x6250 ,
 R_AFVA_1                          = 0x6260 ,
 R_BFVA_1                          = 0x6270 ,
 R_CFVA_1                          = 0x6280 ,
 R_AFIRMS_1                        = 0x6290 ,
 R_BFIRMS_1                        = 0x62A0 ,
 R_CFIRMS_1                        = 0x62B0 ,
 R_AFVRMS_1                        = 0x62C0 ,
 R_BFVRMS_1                        = 0x62D0 ,
 R_CFVRMS_1                        = 0x62E0 ,
 R_AIRMSONE_1                      = 0x62F0 ,
 R_BIRMSONE_1                      = 0x6300 ,
 R_CIRMSONE_1                      = 0x6310 ,
 R_AVRMSONE_1                      = 0x6320 ,
 R_BVRMSONE_1                      = 0x6330 ,
 R_CVRMSONE_1                      = 0x6340 ,
 R_NIRMSONE_1                      = 0x6350 ,
 R_AIRMS1012_1                     = 0x6360 ,
 R_BIRMS1012_1                     = 0x6370 ,
 R_CIRMS1012_1                     = 0x6380 ,
 R_AVRMS1012_1                     = 0x6390 ,
 R_BVRMS1012_1                     = 0x63A0 ,
 R_CVRMS1012_1                     = 0x63B0 ,
 R_NIRMS1012_1                     = 0x63C0 ,
 R_AV_PCF_2                        = 0x6800 ,
 R_AI_PCF_2                        = 0x6810 ,
 R_AIRMS_2                         = 0x6820 ,
 R_AVRMS_2                         = 0x6830 ,
 R_AWATT_2                         = 0x6840 ,
 R_AVA_2                           = 0x6850 ,
 R_AVAR_2                          = 0x6860 ,
 R_AFVAR_2                         = 0x6870 ,
 R_APF_2                           = 0x6880 ,
 R_AVTHD_2                         = 0x6890 ,
 R_AITHD_2                         = 0x68A0 ,
 R_AFWATT_2                        = 0x68B0 ,
 R_AFVA_2                          = 0x68C0 ,
 R_AFIRMS_2                        = 0x68D0 ,
 R_AFVRMS_2                        = 0x68E0 ,
 R_AIRMSONE_2                      = 0x68F0 ,
 R_AVRMSONE_2                      = 0x6900 ,
 R_AIRMS1012_2                     = 0x6910 ,
 R_AVRMS1012_2                     = 0x6920 ,
 R_BV_PCF_2                        = 0x6930 ,
 R_BI_PCF_2                        = 0x6940 ,
 R_BIRMS_2                         = 0x6950 ,
 R_BVRMS_2                         = 0x6960 ,
 R_BWATT_2                         = 0x6970 ,
 R_BVA_2                           = 0x6980 ,
 R_BVAR_2                          = 0x6990 ,
 R_BFVAR_2                         = 0x69A0 ,
 R_BPF_2                           = 0x69B0 ,
 R_BVTHD_2                         = 0x69C0 ,
 R_BITHD_2                         = 0x69D0 ,
 R_BFWATT_2                        = 0x69E0 ,
 R_BFVA_2                          = 0x69F0 ,
 R_BFIRMS_2                        = 0x6A00 ,
 R_BFVRMS_2                        = 0x6A10 ,
 R_BIRMSONE_2                      = 0x6A20 ,
 R_BVRMSONE_2                      = 0x6A30 ,
 R_BIRMS1012_2                     = 0x6A40 ,
 R_BVRMS1012_2                     = 0x6A50 ,
 R_CV_PCF_2                        = 0x6A60 ,
 R_CI_PCF_2                        = 0x6A70 ,
 R_CIRMS_2                         = 0x6A80 ,
 R_CVRMS_2                         = 0x6A90 ,
 R_CWATT_2                         = 0x6AA0 ,
 R_CVA_2                           = 0x6AB0 ,
 R_CVAR_2                          = 0x6AC0 ,
 R_CFVAR_2                         = 0x6AD0 ,
 R_CPF_2                           = 0x6AE0 ,
 R_CVTHD_2                         = 0x6AF0 ,
 R_CITHD_2                         = 0x6B00 ,
 R_CFWATT_2                        = 0x6B10 ,
 R_CFVA_2                          = 0x6B20 ,
 R_CFIRMS_2                        = 0x6B30 ,
 R_CFVRMS_2                        = 0x6B40 ,
 R_CIRMSONE_2                      = 0x6B50 ,
 R_CVRMSONE_2                      = 0x6B60 ,
 R_CIRMS1012_2                     = 0x6B70 ,
 R_CVRMS1012_2                     = 0x6B80 ,
 R_NI_PCF_2                        = 0x6B90 ,
 R_NIRMS_2                         = 0x6BA0 ,
 R_NIRMSONE_2                      = 0x6BB0 ,
 R_NIRMS1012_2                     = 0x6BC0 ,
 R_WAVEFORM_BUFFER                 = 0x8000 
};

typedef struct status0_t{
      uint32_t EGYRDY   : 1;
      uint32_t REVAPA   : 1;
      uint32_t REVAPB   : 1;
      uint32_t REVAPC   : 1;
      uint32_t REVRPA   : 1;
      uint32_t REVRPB   : 1;
      uint32_t REVRPC   : 1;
      uint32_t REVPSUM1 : 1;
      uint32_t REVPSUM2 : 1;
      uint32_t REVPSUM3 : 1;
      uint32_t REVPSUM4 : 1;
      uint32_t CF1      : 1;
      uint32_t CF2      : 1;
      uint32_t CF3      : 1;
      uint32_t CF4      : 1;
      uint32_t DREADY   : 1;
      uint32_t WFB_TRIG_IRQ : 1;
      uint32_t PAGE_FULL : 1;
      uint32_t PWRRDY : 1;
      uint32_t RMSONERDY : 1;
      uint32_t RMS1012RDY : 1;
      uint32_t THD_PF_RDY : 1;
      uint32_t WFB_TRIG : 1;
      uint32_t COH_WFB_FULL : 1;
      uint32_t MISMTCH : 1;
      uint32_t TEMP_RDY : 1;
      uint32_t RESERVED : 6;
    }status0_t;

typedef struct status1_t{
      uint32_t ANLOAD : 1;
      uint32_t RNLOAD : 1;
      uint32_t VANLOAD : 1;
      uint32_t AFNOLOAD : 1;
      uint32_t RFNOLOAD : 1;
      uint32_t VAFNOLOAD : 1;
      uint32_t ZXTOVA : 1;
      uint32_t ZXTOVB : 1;
      uint32_t ZXTOVC : 1;
      uint32_t ZXVA : 1;
      uint32_t ZXVB : 1;
      uint32_t ZXVC : 1;
      uint32_t ZXCOMB : 1;
      uint32_t ZXIA : 1;
      uint32_t ZXIB : 1;
      uint32_t ZXIC : 1;
      uint32_t RESERVED : 1;
      uint32_t OI : 1;
      uint32_t SEQERR : 1;
      uint32_t RESERVED1 : 1;
      uint32_t SWELLA : 1;
      uint32_t SWELLB : 1;
      uint32_t SWELLC : 1;
      uint32_t DIPA : 1;
      uint32_t DIPB : 1;
      uint32_t DIPC : 1;
      uint32_t CRC_CHG : 1;
      uint32_t CRC_DONE : 1;
      uint32_t ERROR0 : 1;
      uint32_t ERROR1 : 1;
      uint32_t ERROR2 : 1;
      uint32_t ERROR3 : 1;
}status1_t;

typedef struct event_status_t{
      uint32_t DIPA             : 1;               
      uint32_t DIPB             : 1;               
      uint32_t DIPC             : 1;               
      uint32_t SWELLA           : 1;               
      uint32_t SWELLB           : 1;               
      uint32_t SWELLC           : 1;               
      uint32_t REVPSUM1         : 1;               
      uint32_t REVPSUM2         : 1;               
      uint32_t REVPSUM3         : 1;               
      uint32_t REVPSUM4         : 1;               
      uint32_t ANLOAD           : 1;               
      uint32_t RNLOAD           : 1;               
      uint32_t VANLOAD          : 1;               
      uint32_t AFNLOAD          : 1;               
      uint32_t RFNLOAD          : 1;               
      uint32_t VAFNLOAD         : 1;               
      uint32_t DREADY           : 1;               
      uint32_t RESERVED         : 15;       
    }event_status_t;

typedef struct ade9000_irq_status_t{
  volatile bool Pending[2];
  status0_t Status0;
  status1_t Status1;
}ade9000_irq_status_t;

typedef union ade9000_reg32_t{
  uint32_t v;
  struct {
      uint32_t ISUM_CFG        : 2;
      uint32_t RESERVED1       : 1;
      uint32_t HPFDIS          : 1;
      uint32_t MTEN            : 1;
      uint32_t INTEN           : 1;
      uint32_t ZX_SRC_SEL      : 1;
      uint32_t RMS_SRC_SEL     : 1;
      uint32_t VNOMA_EN        : 1;
      uint32_t VNOMB_EN        : 1;
      uint32_t VNOMC_EN        : 1;
      uint32_t ININTEN         : 1;
      uint32_t DISAPLPF        : 1;
      uint32_t DISRPLPF        : 1;    
      uint32_t RESERVED        : 18;
    }S_CONFIG0;
    struct {
      uint16_t SWRST           :1;
      uint16_t CF3_CFG         :1;
      uint16_t CF4_CFG         :2;
      uint16_t RESERVED1       :1;
      uint16_t CF_ACC_CLR      :1;
      uint16_t RESERVED2       :2;
      uint16_t PWR_SETTLE      :2;
      uint16_t RESERVED3       :1;
      uint16_t BURST_EN        :1;
      uint16_t IRQ0_ON_IRQ1    :1;
      uint16_t RESERVED4       :2;
      uint16_t EXT_REF         :1;
    }S_CONFIG1;
    struct {
      uint32_t AREGION           : 4;
      uint32_t RESERVED          : 28;
    }S_AMTREGION;
    struct {
      uint16_t Start : 1;
      uint16_t RESERVED : 15;
    }S_RUN;
    struct {
      uint16_t IA_GAIN  : 2;
      uint16_t IB_GAIN  : 2;
      uint16_t IC_GAIN  : 2;
      uint16_t IN_GAIN  : 2;
      uint16_t VA_GAIN  : 2;
      uint16_t VB_GAIN  : 2;
      uint16_t VC_GAIN  : 2;
      uint16_t RESERVED : 2;
    }S_PGA_GAIN;
    struct {
      uint16_t RESERVED : 9;
      uint16_t HPF_CRN  : 3;
      uint16_t UPERIOD_SEL : 1;
      uint16_t RESERVED1 : 3;
    }S_CONFIG2;
    struct {
      uint16_t BURST_CHAN : 4;
      uint16_t WF_CAP_EN : 1;
      uint16_t WF_CAP_SEL : 1;
      uint16_t WF_MODE : 2;
      uint16_t WF_SRC : 2;
      uint16_t RESERVED : 2;
      uint16_t WF_IN_EN : 1;
      uint16_t RESERVED1 : 3;
    }S_WFB_CFG;
    struct {
      uint16_t DIP : 1;
      uint16_t SWELL : 1;
      uint16_t OI : 1;
      uint16_t ZXIA : 1;
      uint16_t ZXIB : 1;
      uint16_t ZXIC : 1;
      uint16_t ZXVA : 1;
      uint16_t ZXVB : 1;
      uint16_t ZXVC : 1;
      uint16_t ZXCOMB : 1;
      uint16_t TRIG_FORCE : 1;
      uint16_t RESERVED : 5;     
    }S_WFB_TRG_CFG;
    struct {
      uint16_t WFB_TRIG_ADDR : 11;
      uint16_t RESERVED : 1;
      uint16_t WFB_LAST_PAGE : 4;      
    }S_WFB_TRG_STAT;
    struct {
      uint16_t EGY_PWR_EN : 1;
      uint16_t EGY_TMR_MODE : 1;
      uint16_t RESERVED : 2;
      uint16_t EGY_LD_ACCUM : 1;
      uint16_t RD_RST_EN : 1;
      uint16_t PWR_SIGN_SEL_0 : 1;
      uint16_t PWR_SIGN_SEL_1 : 1;
      uint16_t RESERVED1 : 5;
      uint16_t NOLOAD_TMR : 3;
    }S_EP_CFG;
    struct {
      uint32_t RESERVED1 : 16;
      uint32_t ADE9004_ID : 1;
      uint32_t RESERVED2 : 3;
      uint32_t ADE9000_ID : 1;
      uint32_t ADE73370_ID : 1;
      uint32_t RESERVED3 : 10;
    }S_PART_ID;
    struct {
      uint16_t WATTACC : 2;
      uint16_t VARACC : 2;
      uint16_t VCONEL : 3;
      uint16_t ICONSEL : 1;
      uint16_t SELFSREQ : 1;
      uint16_t RESERVED : 7;
    }S_ACCMODE;
    uint16_t S_EGY_TIME;
    uint32_t S_ACT_NL_LVL;
    uint32_t S_REACT_NL_LVL;
    uint32_t S_APP_NL_LVL;
    event_status_t S_EVENT_STATUS;
    event_status_t S_EVENT_MASK;
    status0_t S_MASK0;
    status1_t S_MASK1;
    status0_t S_STATUS0;
    status1_t S_STATUS1;
} ade9000_reg32_t;