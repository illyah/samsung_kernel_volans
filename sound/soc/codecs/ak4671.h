/*
 * ak4671.h  --  AK4671 Soc Audio driver
 *
 * Copyright 2009 Samsung Electronics Co., Ltd.
 *
 * Based on ak4535.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _AK4671_H
#define _AK4671_H

#include <sound/soc.h>
#include <mach/volans.h>
#include "ak4671_gaintable.h"
#include "dprintk.h"

#undef CODEC_DEBUG
#undef DPRINTK
#ifdef CODEC_DEBUG
#define DPRINTK(ARGS...)  printk(KERN_INFO "<%s>: ",__FUNCTION__);printk(ARGS)
#define FN_IN printk(KERN_INFO "[%s]: start\n", __FUNCTION__)
#define FN_OUT(n) printk(KERN_INFO "[%s]: end(%u)\n",__FUNCTION__, n)
#else
#define DPRINTK( x... )
#define FN_IN
#define FN_OUT(n)
#endif				/* DEBUG */

#define AK4671_AD_DA_PM			0x00
#define AK4671_PLL_MODE0		0x01
#define AK4671_PLL_MODE1		0x02
#define AK4671_FORMAT			0x03
#define AK4671_MIC_SIG			0x04
#define AK4671_MIC_AMP_GAIN		0x05
#define AK4671_MIX_PM0			0x06
#define AK4671_MIX_PM1			0x07
#define AK4671_OUTPUT_VOLUME	0x08
#define AK4671_LOUT1_SIG		0x09
#define AK4671_ROUT1_SIG		0x0A
#define AK4671_LOUT2_SIG		0x0B
#define AK4671_ROUT2_SIG		0x0C
#define AK4671_LOUT3_SIG		0x0D
#define AK4671_ROUT3_SIG		0x0E
#define AK4671_LOUT1_PM			0x0F
#define AK4671_LOUT2_PM			0x10
#define AK4671_LOUT3_PM			0x11
#define AK4671_L_IN_VOLUME		0x12
#define AK4671_R_IN_VOLUME		0x13
#define AK4671_ALC_REF			0x14
#define AK4671_DIGITAL_MIXING_1	0x15
#define AK4671_ALC_TIMER		0x16
#define AK4671_ALC_MODE			0x17
#define AK4671_MODE1			0x18
#define AK4671_MODE2			0x19
#define AK4671_L_OUT_VOLUME		0x1A
#define AK4671_R_OUT_VOLUME		0x1B
#define AK4671_SIDE_TONE_A		0x1C
#define AK4671_DIGITAL_FILTER	0x1D
#define AK4671_FIL3_COEFFICIENT0 0x1E
#define AK4671_FIL3_COEFFICIENT1 0x1F
#define AK4671_FIL3_COEFFICIENT2 0x20
#define AK4671_FIL3_COEFFICIENT3 0x21
#define AK4671_EQ_COEFFICIENT0 	0x22
#define AK4671_EQ_COEFFICIENT1 	0x23
#define AK4671_EQ_COEFFICIENT2 	0x24
#define AK4671_EQ_COEFFICIENT3 	0x25
#define AK4671_EQ_COEFFICIENT4 	0x26
#define AK4671_EQ_COEFFICIENT5 	0x27
#define AK4671_FIL1_COEFFICIENT0 0x28
#define AK4671_FIL1_COEFFICIENT1 0x29
#define AK4671_FIL1_COEFFICIENT2 0x2A
#define AK4671_FIL1_COEFFICIENT3 0x2B
#define AK4671_FIL2_COEFFICIENT0 0x2C
#define AK4671_FIL2_COEFFICIENT1 0x2D
#define AK4671_FIL2_COEFFICIENT2 0x2E
#define AK4671_FIL2_COEFFICIENT3 0x2F
#define AK4671_DIGITAL_FILTER2	 0x30
// 0x31 is reserved
#define AK4671_E1_COEFFICIENT0 	0x32
#define AK4671_E1_COEFFICIENT1 	0x33
#define AK4671_E1_COEFFICIENT2 	0x34
#define AK4671_E1_COEFFICIENT3 	0x35
#define AK4671_E1_COEFFICIENT4 	0x36
#define AK4671_E1_COEFFICIENT5 	0x37
#define AK4671_E2_COEFFICIENT0 	0x38
#define AK4671_E2_COEFFICIENT1 	0x39
#define AK4671_E2_COEFFICIENT2 	0x3A
#define AK4671_E2_COEFFICIENT3 	0x3B
#define AK4671_E2_COEFFICIENT4 	0x3C
#define AK4671_E2_COEFFICIENT5 	0x3D
#define AK4671_E3_COEFFICIENT0 	0x3E
#define AK4671_E3_COEFFICIENT1 	0x3F
#define AK4671_E3_COEFFICIENT2 	0x40
#define AK4671_E3_COEFFICIENT3 	0x41
#define AK4671_E3_COEFFICIENT4 	0x42
#define AK4671_E3_COEFFICIENT5 	0x43
#define AK4671_E4_COEFFICIENT0 	0x44
#define AK4671_E4_COEFFICIENT1 	0x45
#define AK4671_E4_COEFFICIENT2 	0x46
#define AK4671_E4_COEFFICIENT3 	0x47
#define AK4671_E4_COEFFICIENT4 	0x48
#define AK4671_E4_COEFFICIENT5 	0x49
#define AK4671_E5_COEFFICIENT0 	0x4A
#define AK4671_E5_COEFFICIENT1 	0x4B
#define AK4671_E5_COEFFICIENT2 	0x4C
#define AK4671_E5_COEFFICIENT3 	0x4D
#define AK4671_E5_COEFFICIENT4 	0x4E
#define AK4671_E5_COEFFICIENT5 	0x4F
#define AK4671_EQ_CON_250_100HZ 0x50
#define AK4671_EQ_CON_35_1KHZ 	0x51
#define AK4671_EQ_CON_10KHZ 	0x52
#define AK4671_PCM_IF_CON0		0x53
#define AK4671_PCM_IF_CON1		0x54
#define AK4671_PCM_IF_CON2		0x55
#define AK4671_DIGITAL_VOL_B	0x56
#define AK4671_DIGITAL_VOL_C	0x57
#define AK4671_SIDE_TONE_VOL	0x58
#define AK4671_DIGITAL_MIXING_2	0x59
#define AK4671_SAR_ADC_CON		0x5A


#define AK4671_MCLK 			1
#define AK4671_SAMPLE_FREQ		2
#define AK4671_CLKSRC_MCLK		1
#define AK4671_CLKSRC_OSC		2

/* AK4671 register count */
#define AK4671_CACHEREGNUM 		0x5B	// 0 ~ 5A

/* Power control definitions. */
#define POWER_ON	0x01
#define POWER_OFF	0x00
/* Mic definitions for selection */
#define MAIN_MIC	0x01
#define SUB_MIC		0x00

/* true or false */
#define ON 		true 
#define OFF		false

#define SPEAKER 1
#define HEADPHONE 2
#define DUALMODE 	3

/*
 * Define channel 
 * 
 * INPUT_CH_1	:	Earjack(Headset) Mic		
 * INPUT_CH_2	:	Main Mic	
 * INPUT_CH_3	:	Sub Mic	
 * INPUT_CH_4	:	BT Handsfree	
 * INPUT_CH_5	:	Application Processor
 * INPUT_CH_6	:	Baseband(MSM)
 * INPUT_CH_7	:	FM Radio
 * INPUT_CH_8,9	:	Reserved
 *              
 * OUTPUT_CH_1	:	Earjack (Headset)
 * OUTPUT_CH_2	:	Left Speaker
 * OUTPUT_CH_3	:	Right Speaker
 * OUTPUT_CH_4	:	Receiver (Mono)
 * OUTPUT_CH_5	:	BT Handsfree
 * OUTPUT_CH_6	:	Baseband
 * OUTPUT_CH_7	:	Application Processor (Recording Only)
 * OUTPUT_CH_8,9:	Reserved
 */              

#define IN		0
#define OUT		16

#define INPUT_CH_OFF	( 1 << (0+ IN) )
#define INPUT_CH_1		( 1 << (1+ IN) )
#define INPUT_CH_2		( 1 << (2+ IN) )
#define INPUT_CH_3		( 1 << (3+ IN) )
#define INPUT_CH_4		( 1 << (4+ IN) )
#define INPUT_CH_5		( 1 << (5+ IN) )
#define INPUT_CH_6		( 1 << (6+ IN) )
#define INPUT_CH_7		( 1 << (7+ IN) )
#define INPUT_CH_8		( 1 << (8+ IN) )
#define INPUT_CH_9		( 1 << (9+ IN) )

#define OUTPUT_CH_OFF	( 1 << (0+ OUT) )
#define OUTPUT_CH_1		( 1 << (1+ OUT) )
#define OUTPUT_CH_2		( 1 << (2+ OUT) )
#define OUTPUT_CH_3		( 1 << (3+ OUT) )
#define OUTPUT_CH_4		( 1 << (4+ OUT) )
#define OUTPUT_CH_5		( 1 << (5+ OUT) )
#define OUTPUT_CH_6		( 1 << (6+ OUT) )
#define OUTPUT_CH_7		( 1 << (7+ OUT) )
#define OUTPUT_CH_8		( 1 << (8+ OUT) )
#define OUTPUT_CH_9		( 1 << (9+ OUT) )


#define INPUT_MASK		(0x03FF << (IN))
#define OUTPUT_MASK		(0x03FF << (OUT))
#define PATH_MASK		(INPUT_MASK | OUTPUT_MASK)

/* local define for path */
#define IN_OFF			INPUT_CH_OFF
#define IN1_MAIN_MIC	INPUT_CH_2
#define IN1_SUB_MIC		INPUT_CH_3
#define IN2_FMRADIO		INPUT_CH_7
#define IN3_EAR_MIC		INPUT_CH_1
#define IN4_MSM_SPK		INPUT_CH_6
#define IN_BT			INPUT_CH_4
#define IN_AP			INPUT_CH_5

#define OUT_OFF			OUTPUT_CH_OFF
#define OUT1_RCV		OUTPUT_CH_4
#define OUT2_SPK_L		OUTPUT_CH_2
#define OUT2_SPK_R		OUTPUT_CH_3
#define OUT2_HP			OUTPUT_CH_1
#define OUT3_MSM_MIC	OUTPUT_CH_6
#define OUT_BT			OUTPUT_CH_5
#define OUT_AP			OUTPUT_CH_7
#define HEADSET_DELETE  OUTPUT_CH_8
#define TOGGLE_SPK_EAR  OUTPUT_CH_9
/* Path control definitions. */

/* DAC TO OUTPUT */
#define DAC_MUTE	0x00
#define DAC_TO_RCV	0x01
#define DAC_TO_SPK	0x02
#define DAC_TO_HP	0x04
#define BT_TO_MSMMIC	0x08
#define BT_TO_RCV		0x10
#define BT_TO_SPK		0x20
#define BT_TO_HP		0x40

/* ADC PATH */
#define MIC_TO_AP		0x01
#define MSMSPK_TO_AP	0x02
#define EARMIC_TO_AP	0x04
#define FMRADIO_TO_AP	0x08
#define MSMSPK_TO_BT	0x10
#define FMRADIO_TO_BT	0x20

/* DIRECT PATH */
#define MIC_TO_MSMMIC		0x01
#define EARMIC_TO_MSMMIC	0x02
#define MSMSPK_TO_RCV		0x04
#define MSMSPK_TO_SPK		0x08
#define MSMSPK_TO_HP		0x10
#define FMRADIO_TO_SPK		0x20
#define FMRADIO_TO_HP		0x40
#define AP_TO_BT			0x80
#define BT_TO_AP			0x100
#define MIC_TO_RCV			0x200
#define MIC_TO_SPK			0x400
//temporary code 
#define MIC_CALL			0x11
#define EARMIC_CALL			0x12

#define CH_OFF  0x0

struct ak4671_setup_data {
	int            i2c_bus;
	unsigned short i2c_address;
};

/* ak4671.c */
extern struct snd_soc_dai ak4671_dai;
extern struct snd_soc_codec_device soc_codec_dev_ak4671;
extern const u16 ak4671_reg[];
extern unsigned int headset_status;
extern int gain_reg;
extern int gain_val;
extern int gaintable_num;
extern int tuning_gaintable_num;
extern unsigned int curr_sound_scenario;
extern unsigned int prev_sound_scenario;
extern unsigned int curr_tuning_scenario;
extern unsigned int capture_mute_flag;
extern unsigned int playback_mute_flag;
extern unsigned int read_reg_flag;
extern unsigned int set_gain_table_flag;
extern unsigned char audio_format;
extern unsigned char pll_mode0;
extern unsigned char pll_mode1;

extern int (*volans_amp_spk_mode_enable)(void);
extern int (*volans_amp_headphone_mode_enable)(void);
extern int (*volans_amp_receiver_mode_enable)(void);
extern int (*volans_amp_spk_mode_disable)(void);
extern int (*volans_amp_headphone_mode_disable)(void);
extern int (*volans_amp_receiver_mode_disable)(void);
extern int (*volans_amp_dualmode_enable)(void);
extern int (*volans_amp_get_output_mode)(void);
extern int (*amp_i2c_read)( unsigned char reg, unsigned char* value );
extern int (*amp_i2c_write)( unsigned char reg, unsigned char value );

extern int codec_hw_init(void);
extern int codec_power_ctrl(int mode);
extern int mic_select_ctrl(int mic_sel);
extern int mic_power_ctrl(int mode);

extern void ak4671_reg_print(struct snd_soc_codec *codec);
extern void amp_reg_write(unsigned char addr, unsigned char value);
extern void amp_reg_read(unsigned char addr, unsigned char *value);
extern void amp_reg_print(void);
extern int set_amp(int power, int direction);
extern void set_sound_path(struct snd_soc_codec *codec,	unsigned int direction);
extern int get_sound_path(void);
extern void set_capture_path(struct snd_soc_codec *codec, unsigned int direction);
extern void set_audio_info(struct snd_soc_codec *codec);

#define MAX_PATH_COUNT 9

struct ak4671_path_data {
	unsigned int path;
	unsigned int path_count;
	unsigned int prev_path[MAX_PATH_COUNT];
};
extern struct ak4671_path_data path_data;


struct audio_info_data {
	unsigned int gain_info;
	unsigned int path[MAX_PATH_COUNT];
};
extern struct audio_info_data curr_audio_info;
extern struct audio_info_data prev_audio_info;

extern void ak4671_set_gain(struct snd_soc_codec *codec);
extern void ak4671_select_gain_table(struct snd_soc_codec *codec);
extern void ak4671_select_tuning_gain_table(struct snd_soc_codec *codec);
extern void set_default_gain(struct snd_soc_codec *codec);
extern int set_capture_mute(struct snd_soc_codec *codec, int mute);
extern int set_playback_mute(struct snd_soc_codec *codec, int mute);

extern void earphone_gpio_check(void);
extern int earphone_detect_init(void);
extern void earphone_detect_exit(void);
extern void	earphone_detect_scan_svc(void);

extern int amp_init(void);
extern void amp_exit(void);
#endif
