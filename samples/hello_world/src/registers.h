/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#define R_AIGAIN		0x4380
#define R_AVGAIN		0x4381
#define R_BIGAIN		0x4382
#define R_BVGAIN		0x4383
#define R_CIGAIN		0x4384
#define R_CVGAIN		0x4385
#define R_NIGAIN		0x4386
#define R_AIRMSOS		0x4387
#define R_AVRMSOS		0x4388
#define R_BIRMSOS		0x4389
#define R_BVRMSOS		0x438A
#define R_CIRMSOS		0x438B
#define R_CVRMSOS		0x438C
#define R_NIRMSOS		0x438D

#define R_AVAGAIN		0x438E
#define R_BVAGAIN		0x438F
#define R_CVAGAIN		0x4390

#define R_AWGAIN		0x4391
#define R_AWATTOS		0x4392
#define R_BWGAIN		0x4393
#define R_BWATTOS		0x4394
#define R_CWGAIN		0x4395
#define R_CWATTOS		0x4396

#define R_AVARGAIN	    0x4397
#define R_AVAROS		0x4398
#define R_BVARGAIN	    0x4399
#define R_BVAROS		0x439A
#define R_CVARGAIN	    0x439B
#define R_CVAROS		0x439C

#define R_AFWGAIN		0x439D
#define R_AFWATTOS	    0x439E
#define R_BFWGAIN		0x439F
#define R_BFWATTOS	    0x43A0
#define R_CFWGAIN		0x43A1
#define R_CFWATTOS	    0x43A2

#define R_AFVARGAIN	    0x43A3
#define R_AFVAROS		0x43A4
#define R_BFVARGAIN	    0x43A5
#define R_BFVAROS		0x43A6
#define R_CFVARGAIN	    0x43A7
#define R_CFVAROS		0x43A8

#define R_VATHR1		0x43A9
#define R_VATHR0		0x43AA
#define R_WTHR1		    0x43AB
#define R_WTHR0		    0x43AC

#define R_VARTHR1		0x43AD
#define R_VARTHR0		0x43AE

#define R_VANOLOAD	    0x43B0
#define R_APNOLOAD	    0x43B1
#define R_VARNOLOAD	    0x43B2
#define R_VLEVEL		0x43B3
#define R_DICOEFF		0x43B5
#define R_HPFDIS		0x43B6
#define R_ISUM		    0x43BF
#define R_AIRMS		    0x43C0
#define R_AVRMS		    0x43C1
#define R_BIRMS		    0x43C2
#define R_BVRMS		    0x43C3
#define R_CIRMS		    0x43C4
#define R_CVRMS		    0x43C5
#define R_NIRMS		    0x43C6
#define R_RUN		    0xE228
#define R_AWATTHR		0xE400
#define R_BWATTHR		0xE401
#define R_CWATTHR		0xE402
#define R_AFWATTHR	    0xE403
#define R_BFWATTHR	    0xE404
#define R_CFWATTHR	    0xE405
#define R_AVARHR		0xE406
#define R_BVARHR		0xE407
#define R_CVARHR		0xE408
#define R_AFVARHR		0xE409
#define R_BFVARHR		0xE40A
#define R_CFVARHR		0xE40B
#define R_AVAHR		    0xE40C
#define R_BVAHR		    0xE40D
#define R_CVAHR		    0xE40E
#define R_IPEAK		    0xE500
#define R_VPEAK		    0xE501
#define R_STATUS0		0xE502
#define R_STATUS1		0xE503
#define R_AIMAV		    0xE504
#define R_BIMAV		    0xE505
#define R_CIMAV		    0xE506
#define R_OILVL		    0xE507
#define R_OVLVL         0xE508
#define R_SAGLVL		0xE509
#define R_MASK0		        0xE50A
#define R_MASK1		        0xE50B
#define R_IAWV		        0xE50C
#define R_IBWV		        0xE50D
#define R_ICWV		        0xE50E
#define R_INWV		        0xE50F
#define R_VAWV		        0xE510
#define R_VBWV		        0xE511
#define R_VCWV		        0xE512
#define R_AWATT		        0xE513
#define R_BWATT		        0xE514
#define R_CWATT		        0xE515
#define R_AVAR		        0xE516
#define R_BVAR		        0xE517
#define R_CVAR		        0xE518
#define R_AVA		        0xE519
#define R_BVA		        0xE51A
#define R_CVA		        0xE51B
#define R_CHECKSUM	        0xE51F
#define R_VNOM		        0xE520
#define IARMS_LRIP              0xE530
#define VARMS_LRIP              0xE531
#define IBRMS_LRIP              0xE532
#define VBRMS_LRIP              0xE533
#define ICRMS_LRIP              0xE534
#define VCRMS_LRIP              0xE535
#define INRMS_LRIP              0xE536


#define R_PHSTATUS	0xE600
#define R_ANGLE0		0xE601
#define R_ANGLE1		0xE602
#define R_ANGLE2		0xE603
#define R_PERIOD		0xE607
#define R_PHNOLOAD	0xE608
#define R_LINECYC		0xE60C
#define R_ZXTOUT		0xE60D
#define R_COMPMODE	0xE60E
#define R_GAIN		0xE60F
#define R_CFMODE		0xE610
#define R_CF1DEN		0xE611
#define R_CF2DEN		0xE612
#define R_CF3DEN		0xE613
#define R_APHCAL		0xE614
#define R_BPHCAL		0xE615
#define R_CPHCAL		0xE616
#define R_PHSIGN		0xE617
#define R_CONFIG		0xE618
#define R_MMODE		0xE700
#define R_ACCMODE		0xE701
#define R_LCYCMODE	0xE702
#define R_PEAKCYC		0xE703
#define R_SAGCYC		0xE704
#define R_CFCYC		0xE705
#define R_HSDC_CFG	0xE706
#define R_VERSION		0xE707
#define R_CONFIG_A              0xE740
#define R_LPOILVL		0xEC00
#define R_CONFIG2		0xEC01

//Mask1/Status1 bits
#define NLOAD 	0	// When this bit is set to 1, it enables an interrupt when at least one phase enters no load condition based on total active and reactive powers. 	
#define FNLOAD 	1	// When this bit is set to 1, it enables an interrupt when at least one phase enters no load condition based on fundamental active and reactive powers. Setting this bit to 1 does not have any consequence for ADE7854, ADE7858, and ADE7868. 	
#define VANLOAD 2	// When this bit is set to 1, it enables an interrupt when at least one phase enters no load condition based on apparent power. 	
#define ZXTOVA 	3	// When this bit is set to 1, it enables an interrupt when a zero crossing on Phase A voltage is missing. 	
#define ZXTOVB 	4	// When this bit is set to 1, it enables an interrupt when a zero crossing on Phase B voltage is missing. 	
#define ZXTOVC 	5	// When this bit is set to 1, it enables an interrupt when a zero crossing on Phase C voltage is missing. 	
#define ZXTOIA 	6	// When this bit is set to 1, it enables an interrupt when a zero crossing on Phase A current is missing. 	
#define ZXTOIB 	7	// When this bit is set to 1, it enables an interrupt when a zero crossing on Phase B current is missing. 	
#define ZXTOIC 	8	// When this bit is set to 1, it enables an interrupt when a zero crossing on Phase C current is missing. 	
#define ZXVA 	9	// When this bit is set to 1, it enables an interrupt when a zero crossing is detected on Phase A voltage. 	
#define ZXVB 	10	// When this bit is set to 1, it enables an interrupt when a zero crossing is detected on Phase B voltage. 	
#define ZXVC 	11	// When this bit is set to 1, it enables an interrupt when a zero crossing is detected on Phase C voltage. 	
#define ZXIA 	12	// When this bit is set to 1, it enables an interrupt when a zero crossing is detected on Phase A current. 	
#define ZXIB 	13	// When this bit is set to 1, it enables an interrupt when a zero crossing is detected on Phase B current. 	
#define ZXIC 	14	// When this bit is set to 1, it enables an interrupt when a zero crossing is detected on Phase C current. 	
#define RSTDONE 15	// Because the RSTDONE interrupt cannot be disabled, this bit does not have any functionality attached. It can be set to 1 or cleared to 0 without having any effect. 	
#define SAG 	16	// When this bit is set to 1, it enables an interrupt when a SAG event occurs on one of the phases indicated by Bits[14:12] (VSPHASE[x]) in the PHSTATUS register (see Table 41). 	
#define OI 	17	// When this bit is set to 1, it enables an interrupt when an overcurrent event occurs on one of the phases indicated by Bits[5:3] (OIPHASE[x]) in the PHSTATUS register (see Table 41). 	
#define OV 	18	// When this bit is set to 1, it enables an interrupt when an overvoltage event occurs on one of the phases indicated by Bits[11:9] (OVPHASE[x]) in the PHSTATUS register (see Table 41). 	
#define SEQERR 	19	// When this bit is set to 1, it enables an interrupt when a negative-to-positive zero crossing on Phase A voltage is not followed by a negative-to-positive zero crossing on Phase B voltage, but by a negative-to-positive zero crossing on Phase C voltage. 	
#define MISMTCH 20	// When this bit is set to 1, it enables an interrupt whenis greater than the value indicated in ISUMLVL register. Setting this bit to1 does not have any consequence for ADE7854 and ADE7858. ISUMLVLINWVISUM>- 
#define PKI 	23	// When this bit is set to 1, it enables an interrupt when the period used to detect the peak value in the current channel has ended. 	
#define PKV 	24	// When this bit is set to 1, it enables an interrupt when the period used to detect the peak value in the voltage channel has ended. 	


