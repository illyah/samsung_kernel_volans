/*
 * ak4671_gain.c  --  AK4671 Gain Table Setting.
 *
 */

#include "ak4671.h"

struct gain_info_t sound_scenario_table[GAIN_TABLE_NUM][GAIN_POINT_NUM] = {
	{ 	
		/*  Default Gain table 					(0) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 1,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 1,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 1,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 1,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 1,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 1,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_MP3_SPEAKER 					(1) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_MP3_HEADSET					 (2) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_VIDEO_SPEAKER				 	(3) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 28},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 28},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_VIDEO_HEADSET				 	(4) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_RINGTONE_SPEAKER				 (5) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 15},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 15},
	},
	{ 	
		/* NORMAL_RINGTONE_HEADSET				 (6) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 84},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 84},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_RINGTONE_BLUETOOTH		 	(7) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_KEYTONE_SPEAKER				 (8) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_KEYTONE_HEADSET				 (9) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 34},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 34},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_KEYTONE_BLUETOOTH			 	(10) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_CAMERA_SPEAKER				(11) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 12},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_CAMERA_HEADSET				(12) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_CAMERA_BLUETOOTH			 	(13) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_GAMETONE_SPEAKER				(14) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_GAMETONE_HEADSET				(15) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 54},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 54},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 9},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_GAMETONE_BLUETOOTH		 	(16) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_FMRADIO_SPEAKER				(17) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 1,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 161},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 161},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 1},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 26},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_FMRADIO_HEADSET				 (18) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 1,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 161},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 161},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 12},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_VOICEREC_MAINMIC				 (19) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 15},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 153},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_VOICEREC_HEADSETMIC			 (20) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 15},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 161},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_CAMCORDING_MAINMIC			 (21) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 15},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 153},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_CAMCORDING_HEADSETMIC			 (22) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 15},
		{.flag = 0,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 161},
		{.flag = 0,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* NORMAL_FMREC_FMINPUT					 (23) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 161},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 161},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 0,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* TALKING_VOICECALL_RECEIVER			 (24) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 1,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 1,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 13},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 2},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* TALKING_VOICECALL_SPEAKER				 (25) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 6},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 1,.reg = L1G_IN1_GAIN,.gain = 1},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 13},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 3},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* TALKING_VOICECALL_HEADSET			 (26) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 1,.reg = L3G_IN3_GAIN,.gain = 1},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 3},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 26},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 26},
	},
	{ 	
		/* TALKING_VOICECALL_BLUETOOTH			 (27) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 4},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 28},
		{.flag = 1,.reg = BVL_DATTB_GAIN,.gain = 54},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* TALKING_VIDEOCALL_RECEIVER			 (28) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 1,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 1,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 13},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 2},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* TALKING_VIDEOCALL_SPEAKER			 (29) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 6},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 1,.reg = L1G_IN1_GAIN,.gain = 1},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 13},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 3},
		{.flag = 1,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
	{ 	
		/* TALKING_VIDEOCALL_HEADSET			 (30) */
		{.flag = 1,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 5},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 1,.reg = L3G_IN3_GAIN,.gain = 1},
		{.flag = 1,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 1,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 0,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 0,.reg = OVR_DATT_RGAIN,.gain = 24},
		{.flag = 0,.reg = BVL_DATTB_GAIN,.gain = 24},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 1,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 3},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 1,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 1,.reg = HPLVOL_AMP_LGAIN,.gain = 26},
		{.flag = 1,.reg = HPRVOL_AMP_RGAIN,.gain = 26},
	},
	{ 	
		/* TALKING_VIDEOCALL_BLUETOOTH			 (31) */
		{.flag = 0,.reg = MGNL_MICAMP_LGAIN,.gain = 5},
		{.flag = 1,.reg = MGNR_MICAMP_RGAIN,.gain = 4},
		{.flag = 0,.reg = L1G_IN1_GAIN,.gain = 0},
		{.flag = 0,.reg = L2G_IN2_GAIN,.gain = 0},
		{.flag = 0,.reg = L3G_IN3_GAIN,.gain = 0},
		{.flag = 0,.reg = L4G_IN4_GAIN,.gain = 0},
		{.flag = 0,.reg = LPG_MICAMP_GAIN,.gain = 0},
		{.flag = 0,.reg = IVL_ALC_LGAIN,.gain = 145},
		{.flag = 1,.reg = IVR_ALC_RGAIN,.gain = 145},
		{.flag = 1,.reg = OVL_DATT_LGAIN,.gain = 24},
		{.flag = 1,.reg = OVR_DATT_RGAIN,.gain = 28},
		{.flag = 1,.reg = BVL_DATTB_GAIN,.gain = 54},
		{.flag = 0,.reg = L1VL_OUT1_RCV_GAIN,.gain = 4},
		{.flag = 0,.reg = HPG_OUT2_SPK_HP_GAIN,.gain = 11},
		{.flag = 1,.reg = L3VL_OUT3_MSMSPK_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINA_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = PGAINB_AMP_GAIN,.gain = 0},
		{.flag = 0,.reg = SVOL_AMP_GAIN,.gain = 31},
		{.flag = 0,.reg = HPLVOL_AMP_LGAIN,.gain = 31},
		{.flag = 0,.reg = HPRVOL_AMP_RGAIN,.gain = 31},
	},
};


void ak4671_set_gain(struct snd_soc_codec *codec)
{
	u8 temp = 0;
	
	switch(gain_reg){
		case MGNL_MICAMP_LGAIN :
			temp = codec->read(codec, AK4671_MIC_AMP_GAIN);
			temp &= 0xF0;
			temp |= (gain_val & 0x0F);
			codec->write(codec, AK4671_MIC_AMP_GAIN, temp);
			DPRINTK("MGNL_MICAMP_LGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case MGNR_MICAMP_RGAIN :
			temp = codec->read(codec, AK4671_MIC_AMP_GAIN);
			temp &= 0x0F;
			temp |= ((gain_val << 4) & 0xF0);
			codec->write(codec, AK4671_MIC_AMP_GAIN, temp);
			DPRINTK("MGNR_MICAMP_RGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case L1G_IN1_GAIN :
			temp = codec->read(codec, AK4671_LOUT1_SIG);
			temp &= 0x3F;
			temp |= ((gain_val << 6) & 0xC0);
			codec->write(codec,  AK4671_LOUT1_SIG, temp);
			DPRINTK("L1G_IN1_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case L2G_IN2_GAIN :
			temp = codec->read(codec, AK4671_ROUT1_SIG);
			temp &= 0x3F;
			temp |= ((gain_val << 6) & 0xC0);
			codec->write(codec,  AK4671_ROUT1_SIG, temp);
			DPRINTK("L2G_IN2_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;

		case L3G_IN3_GAIN :
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			temp &= 0x3F;
			temp |= ((gain_val << 6) & 0xC0);
			codec->write(codec,  AK4671_LOUT2_SIG, temp);
			DPRINTK("L3G_IN3_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case L4G_IN4_GAIN :
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			temp &= 0x3F;
			temp |= ((gain_val << 6) & 0xC0);
			codec->write(codec,  AK4671_ROUT2_SIG, temp);
			DPRINTK("L4G_IN4_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case LPG_MICAMP_GAIN :
			temp = codec->read(codec, AK4671_LOUT3_SIG);
			temp &= 0x3F;
			temp |= ((gain_val << 6) & 0xC0);
			codec->write(codec,  AK4671_LOUT3_SIG, temp);
			DPRINTK("LPG_MICAMP_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case IVL_ALC_LGAIN :
			temp = codec->read(codec, AK4671_MODE1);
			temp &= 0xFD;
			codec->write(codec, AK4671_MODE1, temp);
			
			if(!capture_mute_flag) {
			temp = codec->read(codec, AK4671_L_IN_VOLUME);
			temp &= 0x00;
			temp |= (gain_val & 0xFF);
			codec->write(codec,  AK4671_L_IN_VOLUME, temp);
			}
			DPRINTK("IVL_ALC_LGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case IVR_ALC_RGAIN :
			temp = codec->read(codec, AK4671_MODE1);
			temp &= 0xFD;
			codec->write(codec, AK4671_MODE1, temp);
			
			if(!capture_mute_flag) {
			temp = codec->read(codec, AK4671_R_IN_VOLUME);
			temp &= 0x00;
			temp |= (gain_val & 0xFF);
			codec->write(codec,  AK4671_R_IN_VOLUME, temp);
			}
			DPRINTK("IVR_ALC_RGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case OVL_DATT_LGAIN : 
			temp = codec->read(codec, AK4671_L_OUT_VOLUME);
			temp &= 0x00;
			temp |= (gain_val & 0xFF);
			codec->write(codec,  AK4671_L_OUT_VOLUME, temp);
			DPRINTK("OVL_DATT_LGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case OVR_DATT_RGAIN :
			temp = codec->read(codec, AK4671_R_OUT_VOLUME);
			temp &= 0x00;
			temp |= (gain_val & 0xFF);
			codec->write(codec,  AK4671_R_OUT_VOLUME, temp);
			DPRINTK("OVR_DATT_RGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case BVL_DATTB_GAIN :
			if(!capture_mute_flag) {
			temp = codec->read(codec, AK4671_DIGITAL_VOL_B);
			temp &= 0x00;
			temp |= (gain_val & 0xFF);
			codec->write(codec, AK4671_DIGITAL_VOL_B, temp);
			}
			DPRINTK("BVL_DATTB_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case L1VL_OUT1_RCV_GAIN :
			temp = codec->read(codec, AK4671_OUTPUT_VOLUME);
			temp &= 0xF8;
			temp |= (gain_val & 0x07);
			codec->write(codec, AK4671_OUTPUT_VOLUME, temp);
			DPRINTK("L1VL_OUT1_RCV_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case HPG_OUT2_SPK_HP_GAIN :
			temp = codec->read(codec, AK4671_OUTPUT_VOLUME);
			temp &= 0x0F;
			temp |= ((gain_val << 4) & 0xF0);
			codec->write(codec, AK4671_OUTPUT_VOLUME, temp);
			DPRINTK("HPG_OUT2_SPK_HP_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case L3VL_OUT3_MSMSPK_GAIN :
			temp = codec->read(codec, AK4671_LOUT3_PM);
			temp &= 0x3F;
			temp |= ((gain_val << 6) & 0xC0);
			codec->write(codec,  AK4671_LOUT3_PM, temp);
			DPRINTK("L3VL_OUT3_MSMSPK_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case PGAINA_AMP_GAIN :
			amp_reg_read(0x00, &temp);
			temp &= 0xF3;
			temp |= (gain_val << 2) & 0x0C;
			amp_reg_write(0x00, temp);
			DPRINTK("PGAINA_AMP_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;
		
		case PGAINB_AMP_GAIN :
			amp_reg_read(0x00, &temp);
			temp &= 0xFC;
			temp |= gain_val & 0x03;
			amp_reg_write(0x00, temp);
			DPRINTK("PGAINA_AMP_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			break;

		case SVOL_AMP_GAIN :
			if(playback_mute_flag == false){
			temp = gain_val & 0x1F;
			amp_reg_write(0x01, temp);
			DPRINTK("SVOL_AMP_GAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			}
			break;
		
		case HPLVOL_AMP_LGAIN :
			if(playback_mute_flag == false){
			temp = gain_val & 0X1F;
			amp_reg_write(0x02, temp);
			DPRINTK("HPLVOL_AMP_LGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			}
			break;
		
		case HPRVOL_AMP_RGAIN :
			if(playback_mute_flag == false){
			temp = gain_val & 0X1F;
			amp_reg_write(0x03, temp);
			DPRINTK("HPLVOL_AMP_RGAIN : %d(0x%x), reg(0x%x)\n", gain_val, gain_val, temp);
			}
			break;

		default :
			break;
	}

}

/** 
 * @brief ak4671_set_all_sources_volume - set volume for all sources according
 *  to the values saved previously (stored in twl4030_local).  
 * 
 * @return  0 if successful
 */
static int ak4671_set_all_sources_volume(struct snd_soc_codec *codec)
{
	int ret = 0;
	int i = 0;
	
	FN_IN;

	for(i = 0; i < GAIN_POINT_NUM; i++){
		if(sound_scenario_table[gaintable_num][i].flag) {
			gain_reg = sound_scenario_table[gaintable_num][i].reg;
			gain_val = sound_scenario_table[gaintable_num][i].gain;

			ak4671_set_gain(codec);
		}
	}
	
	FN_OUT(ret);
	return ret;
}

void ak4671_select_gain_table(struct snd_soc_codec *codec)
{
  
  if(curr_sound_scenario < TALKING_STATE) {
	switch(curr_sound_scenario) {
	case DEFAULT_GAIN	:
		gaintable_num = 0;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF, "[sound] >>> sound scenario change to DEFAULT_GAIN.\n");
		break;
	
	case NORMAL_MP3_SPEAKER	:
		gaintable_num = 1;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_MP3_SPEAKER.\n");
		break;

	case NORMAL_MP3_HEADSET	:
		gaintable_num = 2;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_MP3_HEADSET.\n");
		break;
	
	case NORMAL_VIDEO_SPEAKER	:
		gaintable_num = 3;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_VIDEO_SPEAKER.\n");
		break;

	case NORMAL_VIDEO_HEADSET	:
		gaintable_num = 4;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_VIDEO_HEADSET.\n");
		break;
	
	case NORMAL_RINGTONE_SPEAKER	:
		gaintable_num = 5;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_RINGTONE_SPEAKER.\n");
		break;
	
	case NORMAL_RINGTONE_HEADSET	:
		gaintable_num = 6;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_RINGTONE_HEADSET.\n");
		break;
	
	case NORMAL_RINGTONE_BLUETOOTH	:
		gaintable_num = 7;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_RINGTONE_BLUETOOTH.\n");
		break;
	
	case NORMAL_KEYTONE_SPEAKER	:
		gaintable_num = 8;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_KEYTONE_SPEAKER.\n");
		break;
	
	case NORMAL_KEYTONE_HEADSET	:
		gaintable_num = 9;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_KEYTONE_HEADSET.\n");
		break;
	
	case NORMAL_KEYTONE_BLUETOOTH :
		gaintable_num = 10;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_KEYTONE_BLUETOOTH.\n");
		break;

	case NORMAL_CAMERA_SPEAKER :
		gaintable_num = 11;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_CAMERA_SPEAKER.\n");
		break;

	case NORMAL_CAMERA_HEADSET :
		gaintable_num = 12;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_CAMERA_HEADSET.\n");
		break;
	
	case NORMAL_CAMERA_BLUETOOTH :
		gaintable_num = 13;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_CAMERA_BLUETOOTH.\n");
		break;

	case NORMAL_GAMETONE_SPEAKER :
		gaintable_num = 14;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_GAMETONE_SPEAKER.\n");
		break;
	
	case NORMAL_GAMETONE_HEADSET :
		gaintable_num = 15;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_GAMETONE_HEADSET.\n");
		break;
	
	case NORMAL_GAMETONE_BLUETOOTH :
		gaintable_num = 16;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_GAMETONE_BLUETOOTH.\n");
		break;
	
	case NORMAL_FMRADIO_SPEAKER	:
		gaintable_num = 17;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_FMRADIO_SPEAKER.\n");
		break;
	
	case NORMAL_FMRADIO_HEADSET	:
		gaintable_num = 18;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_FMRADIO_HEADSET.\n");
		break;
	
	default:
		printk("[sound] >>> [warning] There no NORMAL scenario table to change .\n");
		break;
	}//switch end			
  }

  else if(curr_sound_scenario & RECORDING_STATE) {
	switch(curr_sound_scenario) {
	
	case NORMAL_VOICEREC_MAINMIC :
		gaintable_num = 19;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_VOICEREC_MAINMIC.\n");
		break;

	case NORMAL_VOICEREC_HEADSETMIC :
		gaintable_num = 20;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_VOICEREC_HEADSETMIC.\n");
		break;
	
	case NORMAL_CAMCORDING_MAINMIC :
		gaintable_num = 21;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_CAMCORDING_MAINMIC.\n");
		break;

	case NORMAL_CAMCORDING_HEADSETMIC :
		gaintable_num = 22;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_CAMCORDING_HEADSETMIC.\n");
		break;
	
	case NORMAL_FMREC_FMINPUT :
		gaintable_num = 23;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to NORMAL_FMREC_FMINPUT.\n");
		break;
	
	default:
		printk("[sound] >>> [warning] There no REC scenario table to change .\n");
		break;
	}//switch end			

  } 

  else if(curr_sound_scenario & TALKING_STATE) {
	switch(curr_sound_scenario) {
	case TALKING_VOICECALL_RECEIVER :
		gaintable_num = 24;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VOICECALL_RECEIVER.\n");
		break;

	case TALKING_VOICECALL_SPEAKER	:
		gaintable_num = 25;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VOICECALL_SPEAKER.\n");
		break;

	
	case TALKING_VOICECALL_HEADSET	:
		gaintable_num = 26;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VOICECALL_HEADSET.\n");
		break;

	case TALKING_VOICECALL_BLUETOOTH :
		gaintable_num = 27;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VOICECALL_BLUETOOTH.\n");
		break;
	
	case TALKING_VIDEOCALL_RECEIVER :
		gaintable_num = 28;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VIDEOCALL_RECEIVER.\n");
		break;
	
	case TALKING_VIDEOCALL_SPEAKER :
		gaintable_num = 29;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VIDEOCALL_SPEAKER.\n");
		break;
	
	case TALKING_VIDEOCALL_HEADSET	:
		gaintable_num = 30;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VIDEOCALL_HEADSET.\n");
		break;
	
	case TALKING_VIDEOCALL_BLUETOOTH :
		gaintable_num = 31;
		ak4671_set_all_sources_volume(codec);
		dprintk(SND_INF,"[sound] >>> sound scenario change to TALKING_VIDEOCALL_BLUETOOTH.\n");
		break;

	default:
		printk("[sound] >>> [warning] There no TALK scenario table to change .\n");
		break;
	}//switch end			

  } 

}

void ak4671_select_tuning_gain_table(struct snd_soc_codec *codec)
{
  if(curr_tuning_scenario < TALKING_STATE) {
	switch(curr_tuning_scenario) {
	case DEFAULT_GAIN	:
		tuning_gaintable_num = 0;
		DPRINTK("[sound] >>> tuning sound scenario for DEFAULT_GAIN.\n");
		break;
	
	case NORMAL_MP3_SPEAKER	:
		tuning_gaintable_num = 1;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_MP3_SPEAKER.\n");
		break;

	case NORMAL_MP3_HEADSET	:
		tuning_gaintable_num = 2;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_MP3_HEADSET.\n");
		break;
	
	case NORMAL_VIDEO_SPEAKER :
		tuning_gaintable_num = 3;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_VIDEO_SPEAKER.\n");
		break;

	case NORMAL_VIDEO_HEADSET :
		tuning_gaintable_num = 4;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_VIDEO_HEADSET.\n");
		break;
	
	case NORMAL_RINGTONE_SPEAKER :
		tuning_gaintable_num = 5;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_RINGTONE_SPEAKER.\n");
		break;
	
	case NORMAL_RINGTONE_HEADSET :
		tuning_gaintable_num = 6;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_RINGTONE_HEADSET.\n");
		break;
	
	case NORMAL_RINGTONE_BLUETOOTH :
		tuning_gaintable_num = 7;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_RINGTONE_BLUETOOTH.\n");
		break;
	
	case NORMAL_KEYTONE_SPEAKER	:
		tuning_gaintable_num = 8;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_KEYTONE_SPEAKER.\n");
		break;
	
	case NORMAL_KEYTONE_HEADSET	:
		tuning_gaintable_num = 9;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_KEYTONE_HEADSET.\n");
		break;
	
	case NORMAL_KEYTONE_BLUETOOTH :
		tuning_gaintable_num = 10;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_KEYTONE_BLUETOOTH.\n");
		break;

	case NORMAL_CAMERA_SPEAKER :
		tuning_gaintable_num = 11;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_CAMERA_SPEAKER.\n");
		break;

	case NORMAL_CAMERA_HEADSET :
		tuning_gaintable_num = 12;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_CAMERA_HEADSET.\n");
		break;
	
	case NORMAL_CAMERA_BLUETOOTH :
		tuning_gaintable_num = 13;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_CAMERA_BLUETOOTH.\n");
		break;

	case NORMAL_GAMETONE_SPEAKER :
		tuning_gaintable_num = 14;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_GAMETONE_SPEAKER.\n");
		break;
	
	case NORMAL_GAMETONE_HEADSET :
		tuning_gaintable_num = 15;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_GAMETONE_HEADSET.\n");
		break;
	
	case NORMAL_GAMETONE_BLUETOOTH :
		tuning_gaintable_num = 16;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_GAMETONE_BLUETOOTH.\n");
		break;
	
	case NORMAL_FMRADIO_SPEAKER	:
		tuning_gaintable_num = 17;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_FMRADIO_SPEAKER.\n");
		break;
	
	case NORMAL_FMRADIO_HEADSET	:
		tuning_gaintable_num = 18;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_FMRADIO_HEADSET.\n");
		break;
	
	default:
		printk("[sound] >>> [warning] There are no NORMAL scenario for tuning .\n");
		break;
	}//switch end			
  } 

  else if(curr_tuning_scenario & RECORDING_STATE) {
	switch(curr_tuning_scenario) {
	
	case NORMAL_VOICEREC_MAINMIC :
		tuning_gaintable_num = 19;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_VOICEREC_MAINMIC.\n");
		break;

	case NORMAL_VOICEREC_HEADSETMIC :
		tuning_gaintable_num = 20;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_VOICEREC_HEADSETMIC.\n");
		break;
	
	case NORMAL_CAMCORDING_MAINMIC :
		tuning_gaintable_num = 21;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_CAMCORDING_MAINMIC.\n");
		break;

	case NORMAL_CAMCORDING_HEADSETMIC :
		tuning_gaintable_num = 22;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_CAMCORDING_HEADSETMIC.\n");
		break;
	
	case NORMAL_FMREC_FMINPUT :
		tuning_gaintable_num = 23;
		DPRINTK("[sound] >>> tuning sound scenario for NORMAL_FMREC_FMINPUT.\n");
		break;
	
	default:
		printk("[sound] >>> [warning] There are no REC scenario for tuning.\n");
		break;
	}//switch end			
  } 

  else if(curr_tuning_scenario & TALKING_STATE) {
	switch(curr_tuning_scenario) {
	case TALKING_VOICECALL_RECEIVER :
		tuning_gaintable_num = 24;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VOICECALL_RECEIVER.\n");
		break;

	case TALKING_VOICECALL_SPEAKER :
		tuning_gaintable_num = 25;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VOICECALL_SPEAKER.\n");
		break;

	
	case TALKING_VOICECALL_HEADSET :
		tuning_gaintable_num = 26;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VOICECALL_HEADSET.\n");
		break;

	case TALKING_VOICECALL_BLUETOOTH :
		tuning_gaintable_num = 27;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VOICECALL_BLUETOOTH.\n");
		break;
	
	case TALKING_VIDEOCALL_RECEIVER :
		tuning_gaintable_num = 28;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VIDEOCALL_RECEIVER.\n");
		break;
	
	case TALKING_VIDEOCALL_SPEAKER :
		tuning_gaintable_num = 29;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VIDEOCALL_SPEAKER.\n");
		break;
	
	case TALKING_VIDEOCALL_HEADSET	:
		tuning_gaintable_num = 30;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VIDEOCALL_HEADSET.\n");
		break;
	
	case TALKING_VIDEOCALL_BLUETOOTH :
		tuning_gaintable_num = 31;
		DPRINTK("[sound] >>> tuning sound scenario for TALKING_VIDEOCALL_BLUETOOTH.\n");
		break;

	default:
		printk("[sound] >>> [warning] There are no TALK scenario for tuning.\n");
		break;
	}//switch end			
  } 

}

//=============================================================================
void set_default_gain(struct snd_soc_codec *codec)
{
	if (curr_sound_scenario == 0)
		ak4671_select_gain_table(codec);
}

//=============================================================================
int set_capture_mute(struct snd_soc_codec *codec, int mute)
{
	unsigned char temp = 0;
	DPRINTK("\n called %s\n",__func__);
	
	if (mute) {
		dprintk(SND_INF, "[SOUND] Capture mute on \n");
	   	
		/* IVL, IVR set mute */
		temp = codec->read(codec, AK4671_L_IN_VOLUME);
		temp = 0x00;
		codec->write(codec,  AK4671_L_IN_VOLUME, temp);
		temp = codec->read(codec, AK4671_R_IN_VOLUME);
		temp = 0x00;
		codec->write(codec,  AK4671_R_IN_VOLUME, temp);

		/* DATTB set mute */
		temp = codec->read(codec, AK4671_DIGITAL_VOL_B);
		temp = 0xFF;
		codec->write(codec, AK4671_DIGITAL_VOL_B, temp);


	} else {
		dprintk(SND_INF,"[SOUND] Capture mute off \n");
		ak4671_select_gain_table(codec);
	}

	return 0;
}

//=============================================================================
int set_playback_mute(struct snd_soc_codec *codec, int mute)
{
	unsigned char temp = 0;
	DPRINTK("\ncalled %s\n",__func__);
	
	if (mute) {
		/* DAC output soft mute(SMUTE = 1) */	
		temp = codec->read(codec, AK4671_MODE2);
		temp |= 0x4;
		codec->write(codec, AK4671_MODE2, temp);
		dprintk(SND_INF,"[SOUND] Playback mute on \n");
	
		/* MAX9877 amp mute */
		amp_reg_write(0x01, 0x0);
		amp_reg_write(0x02, 0x0);
		amp_reg_write(0x03, 0x0);
	
	} else {
		/* nomal_operation */
		ak4671_select_gain_table(codec);
		
		temp = codec->read(codec, AK4671_MODE2);
		temp &= (~0x4);
		codec->write(codec, AK4671_MODE2, temp);
		dprintk(SND_INF,"[SOUND] Playback mute off \n");
	
	}

	return 0;
}


