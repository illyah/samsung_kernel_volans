/*
 * ak4671.c  --  AK4671 ALSA Soc Audio driver
 *
 * Copyright 2009 Samsung Electronics Co., Ltd.
 *
 * Based on ak4535.c 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

#include "ak4671.h"

#define AK4671_VERSION "0.1"

struct snd_soc_codec_device soc_codec_dev_ak4671;
static struct snd_soc_device *ak4671_socdev;

/* headset_status */
unsigned int headset_status = 0;
//EXPORT_SYMBOL(headset_status);

int gain_reg = 0;
int gain_val = 0;
int gaintable_num = 0;
int tuning_gaintable_num = 0;
unsigned int curr_sound_scenario = 0;
unsigned int prev_sound_scenario = 0;
unsigned int curr_tuning_scenario = 0;
unsigned int capture_mute_flag  = 0;
unsigned int playback_mute_flag = 0;
unsigned int read_reg_flag = 0;
unsigned int set_gain_table_flag = 0;
unsigned char audio_format = 0x03; // I2S format (SND_SOC_DAIFMT_I2S)
unsigned char pll_mode0 = 0xF8;    // Sample Freq : 44.1KHz, pll input clock : 19.2MHz
unsigned char pll_mode1 = 0x01;    // AP Master

unsigned int capture_path = 0;
unsigned int playback_path = 0;
/* codec private data */
struct ak4671_priv {
	unsigned int sysclk;
};

/*
 * ak4671 register cache
 */
const u16 ak4671_reg[AK4671_CACHEREGNUM] = {

/*	    0		1		2		3		4		5		6		7	
 *	    8		9 		A		B		C		D		E		F	*/
/* 00~0F */
	0x0000, 0x00F6,	0x0000, 0x0002, 0x0000, 0x0055,	0x0000, 0x0000,	
	0x00B5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	
/* 10~1F */
	0x0000, 0x0080,	0x0091, 0x0091, 0x00E1, 0x0000,	0x0000, 0x0000,	
	0x0002, 0x0001, 0x0018, 0x0018, 0x0000, 0x0002, 0x0000, 0x0000,	
/* 20~2F */
	0x0000, 0x0000,	0x0000, 0x0000, 0x0000, 0x0000,	0x0000, 0x0000,	
	0x00A9, 0x001F, 0x00AD, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000,	
/* 30~3F */
	0x0000, 0x0000,	0x0000, 0x0000, 0x0000, 0x0000,	0x0000, 0x0000,	
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	
/* 40~4F */
	0x0000, 0x0000,	0x0000, 0x0000, 0x0000, 0x0000,	0x0000, 0x0000,	
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	
/* 50~5A */
	0x0088, 0x0088,	0x0008, 0x0000, 0x0000, 0x0000,	0x0018, 0x0018,	
	0x0000, 0x0000, 0x0000, 

};

/* Amplifier funcpointer for mapping amp_i2c function */
/* export_symbol in ak4671.c */
int (*volans_amp_spk_mode_enable)(void) = NULL;
int (*volans_amp_headphone_mode_enable)(void) = NULL;
int (*volans_amp_receiver_mode_enable)(void) = NULL;
int (*volans_amp_spk_mode_disable)(void) = NULL;
int (*volans_amp_headphone_mode_disable)(void) = NULL;
int (*volans_amp_receiver_mode_disable)(void) = NULL;
int (*volans_amp_dualmode_enable)(void) = NULL;
int (*volans_amp_get_output_mode)(void) = NULL;
int (*amp_i2c_read)( unsigned char reg, unsigned char* value )= NULL;
int (*amp_i2c_write)( unsigned char reg, unsigned char value )= NULL;


static struct proc_dir_entry* 		printflag_entry = NULL;
static char ak4671reg[PAGE_SIZE-80] = {0, };

static int proc_read_ak4671_reg(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct snd_soc_codec *codec = ak4671_socdev->codec;
	int len;
	int i;
	unsigned char temp;
	
	
	len = sprintf(page,"***** AK4671 registers *****\n");
	for(i=0; i < 0x5B; i++)
	{
		if(0 == (i%8)) 
			len += sprintf(page+len,"\n");

		temp = codec->read(codec, i);
		len += sprintf(page+len, "%02x ", temp);
	}
	len += sprintf(page+len, "\n");
	len += sprintf(page+len, "\n***** MAX9877 registers *****\n");
	for(i=0; i<5; i++) {
		amp_reg_read(i, &temp);
		len += sprintf(page+len, "%02x ",temp);
	}
	len += sprintf(page+len, "\n");

	return len;
}

static int proc_write_ak4671_reg(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct snd_soc_codec *codec = ak4671_socdev->codec;
	char *realdata;
	int addr, value;

	realdata = (char *)data;

	if (copy_from_user(realdata, buffer, count))
		return -EFAULT;

	realdata[count] = '\0';
	sscanf(realdata, "%d %d", &addr, &value);

	if (addr >= 0 && addr <= 255)
		codec->write(codec, addr, value);

	return count;

#if 0

	int len; 
//	unsigned long long ret;
	unsigned char value[20];

	if(count > 19)
		len = 19;
	else
		len = count;

	if(copy_from_user(value, buffer, len)) {
		return -EFAULT;
	}
	value[len] = '\0';
/*
	if(strncmp(value,"0x", 2) == 0)
		ret = MyStrToULL(value+2, 16);
	else
		ret = MyStrToULL(value, 10);

	ak4671reg = ret;
*/
	return len;
#endif
}

static __inline__ void proc_print_create(void) {
	printflag_entry = create_proc_entry("ak4671reg",0644,NULL);
	if(printflag_entry == NULL) {
		remove_proc_entry("ak4671reg", NULL);
	}
	else {
		printflag_entry->data = &ak4671reg;
		printflag_entry->read_proc = proc_read_ak4671_reg;
		printflag_entry->write_proc = proc_write_ak4671_reg;
		printflag_entry->owner = THIS_MODULE;
	}
}

static __inline__ void proc_print_remove(void) {
	remove_proc_entry("ak4671reg",NULL);
}

/*
 * read ak4671 register cache
 */
static inline unsigned int ak4671_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	if (reg >= AK4671_CACHEREGNUM)
		return -1;
	return cache[reg];
}

/*
 * read ak4671 register space by i2c
 */
static inline unsigned int ak4671_read(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u8 data;
	data = reg;

	if (codec->hw_write(codec->control_data, &data, 1) != 1)
		return -EIO;

	if (codec->hw_read(codec->control_data, &data, 1) != 1)
		return -EIO;

	return data;
};

/*
 * write ak4671 register cache
 */
static inline void ak4671_write_reg_cache(struct snd_soc_codec *codec,
	u16 reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;
	if (reg >= AK4671_CACHEREGNUM)
		return;
	cache[reg] = value;
}

/*
 * write to the AK4671 register space by i2c
 */
static int ak4671_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	u8 data[2];

	/* data is
	 *   D15..D8 AK4671 register offset
	 *   D7...D0 register data
	 */
	data[0] = reg & 0xff;
	data[1] = value & 0xff;
#if 0
	DPRINTK("%s:%d -- reg: 0x%02x, data: 0x%02x\n",
			__func__, __LINE__, data[0], data[1]);
#endif
	ak4671_write_reg_cache(codec, reg, value);
	if (codec->hw_write(codec->control_data, data, 2) == 2)
		return 0;
	else
		return -EIO;
}

static int ak4671_sync(struct snd_soc_codec *codec)
{
	u16 *cache = codec->reg_cache;
	int i, r = 0;

	for (i = 0; i < AK4671_CACHEREGNUM; i++)
		r |= ak4671_write(codec, i, cache[i]);

	return r;
}

static int headset_status_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	
	return 0;
}

static int headset_status_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	ucontrol->value.integer.value[0]=headset_status;
//	DPRINTK("Current headset status : %d \n", headset_status);
	return 0;
}

static int headset_status_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	if(ucontrol->value.integer.value[0] == 0) {
                headset_status = 0;
//		DPRINTK("headset plugged out! \n");
	} else {
        headset_status = 1;
//		DPRINTK("headset plugged in! \n");
	}  
	return 0;
}

static int capture_route_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x02ff02ff;
	
	return 0;
}

static int capture_route_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	DPRINTK("capture route: 0x%x \n",capture_path);
	ucontrol->value.integer.value[0] = get_sound_path();
	
	return 0;
}

static int capture_route_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	capture_path = ucontrol->value.integer.value[0];
	
	DPRINTK("capture route: 0x%x \n", capture_path);
	if(capture_path & (IN_OFF | OUT_OFF))
		set_capture_path(ak4671_socdev->codec, capture_path);
	else
		set_capture_path(ak4671_socdev->codec, capture_path | OUT_AP);
		
	return 0;
}

static int playback_route_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x03ff03ff;
	
	return 0;
}

static int playback_route_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	ucontrol->value.integer.value[0] = get_sound_path();
	return 0;
}

static int playback_route_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	playback_path = ucontrol->value.integer.value[0];
	DPRINTK("playback route: 0x%x \n", playback_path);
	set_sound_path(ak4671_socdev->codec, playback_path);
		
	return 0;
}

static int get_gain_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0xff;
	
	return 0;
}

static int get_gain_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = sound_scenario_table[tuning_gaintable_num][gain_reg].gain;
	DPRINTK("[Get_Gain]scenario: %d, point:%d, gain:%d\n", tuning_gaintable_num, gain_reg, gain_val);
	return 0;
}

static int get_gain_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int changed = 0;
	
	DPRINTK("called %s! \n",__func__);
		
	gain_reg = ucontrol->value.integer.value[0];

	changed = 1;
	
	return changed;

}

static int set_gain_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0xff;
	return 0;
}

static int set_gain_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = gain_reg; 
	
	ucontrol->value.integer.value[1] = gain_val;
	
	return 0;
}

static int set_gain_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int changed = 0;
		
	DPRINTK("called %s! \n",__func__);

	gain_reg = ucontrol->value.integer.value[0];
	gain_val = ucontrol->value.integer.value[1];

	sound_scenario_table[tuning_gaintable_num][gain_reg].gain = gain_val;
	DPRINTK("[Set_Gain]scenario: %d, point:%d, gain:%d\n", tuning_gaintable_num, gain_reg, gain_val);
	if(curr_sound_scenario == curr_tuning_scenario )
	{
	ak4671_set_gain(ak4671_socdev->codec);
	}

	changed = 1;
	
	return changed;
}

static int set_gain_table_values_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0xffff;
	
	return 0;
}

static int set_gain_table_values_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = curr_tuning_scenario;
	
	return 0;
}

static int set_gain_table_values_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	curr_tuning_scenario = ucontrol->value.integer.value[0];
 	ak4671_select_tuning_gain_table(ak4671_socdev->codec);
	
	return 0;
}

static int set_gain_table_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0xffff;
	
	return 0;
}

static int set_gain_table_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = curr_sound_scenario;
	
	return 0;
}

static int set_gain_table_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
		
	curr_sound_scenario = ucontrol->value.integer.value[0];
	DPRINTK("%x",curr_sound_scenario);
	ak4671_select_gain_table(ak4671_socdev->codec);
	prev_sound_scenario = curr_sound_scenario;
//	set_gain_table_flag = 1;
	
	return 0;
}

static int capture_mute_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	
	return 0;
}

static int capture_mute_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = capture_mute_flag;
	
	return 0;
}

static int capture_mute_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);

	if(capture_mute_flag != ucontrol->value.integer.value[0]) 
	{
	capture_mute_flag = ucontrol->value.integer.value[0];
		set_capture_mute(ak4671_socdev->codec, capture_mute_flag);
	}
	else 
	{
		if(	capture_mute_flag > 0 ) 
		{
			printk("[SOUND WARNING] Capture unmuted already..\n");
		}
		else
		{
			printk("[SOUND WARNING] Capture muted already..\n");
		}
			

	}
		
	
	return 0;
}

static int playback_mute_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
//	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	
	return 0;
}

static int playback_mute_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = playback_mute_flag;
	
	return 0;
}

static int playback_mute_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);

	if(playback_mute_flag != ucontrol->value.integer.value[0]) {
	playback_mute_flag = ucontrol->value.integer.value[0];
		set_playback_mute(ak4671_socdev->codec, playback_mute_flag);
	}
	
	else 
	{
		if(	playback_mute_flag > 0 ) 
		{
			printk("[SOUND WARNING] Playback unmuted already..\n");
		}
		else
		{
			printk("[SOUND WARNING] Playback muted already..\n");
		}
		

	}
	
	return 0;
}

static int set_audio_info_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 10;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x02ff02ff;
	
	return 0;
}

static int set_audio_info_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = curr_audio_info.gain_info;
	ucontrol->value.integer.value[1] = curr_audio_info.path[0];
	ucontrol->value.integer.value[2] = curr_audio_info.path[1];
	ucontrol->value.integer.value[3] = curr_audio_info.path[2];
	ucontrol->value.integer.value[4] = curr_audio_info.path[3];
	ucontrol->value.integer.value[5] = curr_audio_info.path[4];
	ucontrol->value.integer.value[6] = curr_audio_info.path[5];
	ucontrol->value.integer.value[7] = curr_audio_info.path[6];
	ucontrol->value.integer.value[8] = curr_audio_info.path[7];
	ucontrol->value.integer.value[9] = curr_audio_info.path[8];
	
	return 0;
}

static int set_audio_info_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int i;

	DPRINTK("called %s! \n",__func__);
	
	curr_audio_info.gain_info = ucontrol->value.integer.value[0];	
	curr_audio_info.path[0]	= ucontrol->value.integer.value[1];	
	curr_audio_info.path[1]	= ucontrol->value.integer.value[2];	
	curr_audio_info.path[2]	= ucontrol->value.integer.value[3];	
	curr_audio_info.path[3]	= ucontrol->value.integer.value[4];	
	curr_audio_info.path[4]	= ucontrol->value.integer.value[5];	
	curr_audio_info.path[5]	= ucontrol->value.integer.value[6];	
	curr_audio_info.path[6]	= ucontrol->value.integer.value[7];	
	curr_audio_info.path[7]	= ucontrol->value.integer.value[8];	
	curr_audio_info.path[8]	= ucontrol->value.integer.value[9];	
	
	set_audio_info(ak4671_socdev->codec);	

	prev_audio_info = curr_audio_info;
#if 1	
	printk("prev gain : %d\n",prev_audio_info.gain_info);
	for(i = 0; i<MAX_PATH_COUNT; i++) {
		printk("prev path[%d] : %x\n", i, prev_audio_info.path[i]);
	}
#endif

	return 0;
}

static int read_sound_reg_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	DPRINTK("called %s! \n",__func__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	
	return 0;
}

static int read_sound_reg_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	
	ucontrol->value.integer.value[0] = read_reg_flag;
	
	return 0;
}

static int read_sound_reg_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	DPRINTK("called %s! \n",__func__);
	read_reg_flag = ucontrol->value.integer.value[0];
	if(read_reg_flag){
		ak4671_reg_print(ak4671_socdev->codec);
		amp_reg_print();
	} else {
		printk("give value 1 if you want check register's valuses\n");
	}
		
	return 0;
}


static const struct soc_enum ak4671_enum[] = {
	/* Nothing.. */
};

static const struct snd_kcontrol_new ak4671_snd_controls[] = {
	{
		.name  = "Headset Switch",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = headset_status_info, 
		.get   = headset_status_get,
		.put   = headset_status_put,
	},
	{
		.name  = "Capture Route",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = capture_route_info, 
		.get   = capture_route_get,
		.put   = capture_route_put,
	},
	{
		.name  = "Playback Route",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = playback_route_info, 
		.get   = playback_route_get,
		.put   = playback_route_put,
	},
	{
		.name  = "Get Gain",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = get_gain_info, 
		.get   = get_gain_get,
		.put   = get_gain_put,
	},
	{
		.name  = "Set Gain",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = set_gain_info, 
		.get   = set_gain_get,
		.put   = set_gain_put,
	},
	{
		.name  = "Set sound gain table",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = set_gain_table_info, 
		.get   = set_gain_table_get,
		.put   = set_gain_table_put,
	},
	{
		.name  = "Set Sound Gain Table Values",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = set_gain_table_values_info, 
		.get   = set_gain_table_values_get,
		.put   = set_gain_table_values_put,
	},
	{
		.name  = "Set Capture Mute",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = capture_mute_info, 
		.get   = capture_mute_get,
		.put   = capture_mute_put,
	},
	{
		.name  = "Set Playback Mute",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = playback_mute_info, 
		.get   = playback_mute_get,
		.put   = playback_mute_put,
	},
	{
		.name  = "Set Audio Info",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = set_audio_info_info, 
		.get   = set_audio_info_get,
		.put   = set_audio_info_put,
	},
	{
		.name  = "Read Sound Register For Debugging",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.index = 0,      
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.info  = read_sound_reg_info, 
		.get   = read_sound_reg_get,
		.put   = read_sound_reg_put,
	},

};

/* add non dapm controls */
static int ak4671_add_controls(struct snd_soc_codec *codec)
{
	int err, i;

	for (i = 0; i < ARRAY_SIZE(ak4671_snd_controls); i++) {
		err = snd_ctl_add(codec->card,
			snd_soc_cnew(&ak4671_snd_controls[i], codec, NULL));
		if (err < 0)
			return err;
	}

	return 0;
}

/* ak4671 dapm widgets */
static const struct snd_soc_dapm_widget ak4671_dapm_widgets[] = {
	/* Nothing.. */
};

static const struct snd_soc_dapm_route audio_map[] = {
	/* Nothing.. */
};

static int ak4671_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, ak4671_dapm_widgets,
				  ARRAY_SIZE(ak4671_dapm_widgets));

	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	snd_soc_dapm_new_widgets(codec);
	return 0;
}

static int ak4671_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
#if 0
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->codec;
	struct ak4671_priv *ak4671 = codec->private_data;
	u8 mode2 = ak4671_read_reg_cache(codec, AK4671_MODE2) & ~(0x3 << 5);
	int rate = params_rate(params), fs = 256;

	if (rate)
		fs = ak4671->sysclk / rate;

	/* set fs */
	switch (fs) {
	case 1024:
		mode2 |= (0x2 << 5);
		break;
	case 512:
		mode2 |= (0x1 << 5);
		break;
	case 256:
		break;
	}

	/* set rate */
	ak4671_write(codec, AK4671_MODE2, mode2);
#endif
	return 0;
}

static int ak4671_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		audio_format = 0x03;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		audio_format = 0x01;
		break;
	default:
		return -EINVAL;
	}

	DPRINTK("[sound format]: 0x%02x\n", audio_format);
	ak4671_write(codec, AK4671_FORMAT, audio_format);
	return 0;
}

static int ak4671_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 value = 0;	
	DPRINTK("ak4671_mute called, mute: %d",mute);

	value = codec->read(codec, AK4671_MODE2);
	
	if (!mute) {
		/* Normal operation */
		value &= ~0x04;
	} else {
		/* Mute */
		value |= 0x04;
	}

	codec->write(codec, AK4671_MODE2, value);

	return 0;
}

static int ak4671_set_dai_clkdiv(struct snd_soc_dai *codec_dai,
		int dev_id, int div)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u8 mode0 = 0;
	switch(dev_id) {		
	case AK4671_MCLK:
		switch(div) {
		case AK4671_CLKSRC_MCLK:
			/* SELECT EXT CLK */
			pll_mode1 = 0x01;
			ak4671_write(codec, AK4671_PLL_MODE1, pll_mode1 );
			break;
		case AK4671_CLKSRC_OSC:
			/* AP slave mode */
			pll_mode1 = 0x03;
			ak4671_write(codec, AK4671_PLL_MODE1, pll_mode1 );
			break;
		default:
			break;
		}
		break;
	
	case AK4671_SAMPLE_FREQ:
		switch(div) {
		case 8000:
			mode0 = 0x00; 
			break;
		case 11025:
			mode0 = 0x50; 
			break;
		case 12000:
			mode0 = 0x10; 
			break;
		case 16000:
			mode0 = 0x20; 
			break;
		case 22050:
			mode0 = 0x70; 
			break;
		case 32000:
			mode0 = 0xA0; 
			break;
		case 44100:
			mode0 = 0xF0; 
			break;
		case 48000:
			mode0 = 0xB0; 
			break;
		default:
			break;
		}
		pll_mode0 = mode0 | 0x08;	
		ak4671_write(codec, AK4671_PLL_MODE0, pll_mode0 );
		
		break;
	
	}

return 0;
}


static int ak4671_set_bias_level(struct snd_soc_codec *codec,
	enum snd_soc_bias_level level)
{

	switch (level) {
	case SND_SOC_BIAS_ON:
		DPRINTK("[%s] BIAS_ON\n",__func__);
//		ak4671_mute(codec->dai, 0);
		break;
	case SND_SOC_BIAS_PREPARE:
		DPRINTK("[%s] BIAS_PREPARE\n",__func__);
//		ak4671_mute(codec->dai, 1);
		break;
	case SND_SOC_BIAS_STANDBY:
		DPRINTK("[%s] BIAS_STANDBY\n",__func__);
#if 0
		if (!codec_power_ctrl(POWER_ON))
		{
			printk("%s:%d -- codec power on failed!\n", __func__, __LINE__);
		}
#endif
		/* TODO: ak4671 register PM setting. */
		break;
	case SND_SOC_BIAS_OFF:
		DPRINTK("[%s] BIAS_OFF\n",__func__);
#if 0 
		codec_power_ctrl(POWER_OFF);
#endif
		break;
	}
	codec->bias_level = level;
	return 0;
}

#define AK4671_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |\
		SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000)

struct snd_soc_dai ak4671_dai = {
	.name = "AK4671",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AK4671_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AK4671_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = {
		.hw_params = ak4671_hw_params,
	},
	.dai_ops = {
		.set_fmt = ak4671_set_dai_fmt,
		.set_clkdiv = ak4671_set_dai_clkdiv,
//		.digital_mute = ak4671_mute,
//		.set_sysclk = ak4671_set_dai_sysclk,
	},
};
EXPORT_SYMBOL_GPL(ak4671_dai);

static int ak4671_suspend(struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("[%s]\n",__func__);
#if 0
	if(path_data.path != 0x0) {
		ak4671_reg_print(ak4671_socdev->codec);
		amp_reg_print();
	}
#endif
	return 0;
}

static int ak4671_resume(struct platform_device *pdev)
{
	DPRINTK("[%s]\n",__func__);

	earphone_gpio_check();	

	return 0;
}

/*
 * initialise the AK4671 driver
 * register the mixer and dsp interfaces with the kernel
 */
static int ak4671_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->codec;
	int ret = 0;

	codec->name = "AK4671";
	codec->owner = THIS_MODULE;
	codec->read = ak4671_read;
	codec->write = ak4671_write;
	codec->set_bias_level = ak4671_set_bias_level;
	codec->dai = &ak4671_dai;
	codec->num_dai = 1;
	codec->reg_cache_size = ARRAY_SIZE(ak4671_reg);
	codec->reg_cache = kmemdup(ak4671_reg, sizeof(ak4671_reg), GFP_KERNEL);

	if (codec->reg_cache == NULL)
		return -ENOMEM;

	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "ak4671: failed to create pcms\n");
		goto pcm_err;
	}


	//temporary code
	set_sound_path(codec, (IN_OFF | OUT_OFF));	
	proc_print_create();
	//temporary code end

	ak4671_add_controls(codec);
	ak4671_add_widgets(codec);
	ret = snd_soc_register_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "ak4671: failed to register card\n");
		goto card_err;
	}

	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
pcm_err:
	kfree(codec->reg_cache);

	return ret;
}


#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)

static int ak4671_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct snd_soc_device *socdev = ak4671_socdev;
	struct snd_soc_codec *codec = socdev->codec;
	int ret;

	i2c_set_clientdata(i2c, codec);
	codec->control_data = i2c;

	//add earphone module
	earphone_detect_init();

	//add amp module
	amp_init();	

	ret = ak4671_init(socdev);
	if (ret < 0)
		printk(KERN_ERR "failed to initialise AK4671\n");

	earphone_detect_scan_svc();
	
	return ret;
}

static int ak4671_i2c_remove(struct i2c_client *client)
{
	struct snd_soc_codec *codec = i2c_get_clientdata(client);
	kfree(codec->reg_cache);

	earphone_detect_exit();
	amp_exit();

	return 0;
}

static const struct i2c_device_id ak4671_i2c_id[] = {
	{ "ak4671", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ak4671_i2c_id);

static struct i2c_driver ak4671_i2c_driver = {
	.driver = {
		.name = "AK4671 I2C Codec",
		.owner = THIS_MODULE,
	},
	.probe =    ak4671_i2c_probe,
	.remove =   ak4671_i2c_remove,
	.id_table = ak4671_i2c_id,
};

static int ak4671_add_i2c_device(struct platform_device *pdev,
				 const struct ak4671_setup_data *setup)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int ret;

	ret = i2c_add_driver(&ak4671_i2c_driver);
	if (ret != 0) {
		dev_err(&pdev->dev, "can't add i2c driver\n");
		return ret;
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = setup->i2c_address;
	strlcpy(info.type, "ak4671", I2C_NAME_SIZE);

	adapter = i2c_get_adapter(setup->i2c_bus);
	if (!adapter) {
		dev_err(&pdev->dev, "can't get i2c adapter %d\n",
			setup->i2c_bus);
		goto err_driver;
	}

	client = i2c_new_device(adapter, &info);
	i2c_put_adapter(adapter);
	if (!client) {
		dev_err(&pdev->dev, "can't add i2c device at 0x%x\n",
			(unsigned int)info.addr);
		goto err_driver;
	}

	return 0;

err_driver:
	i2c_del_driver(&ak4671_i2c_driver);
	return -ENODEV;
}
#endif

static int ak4671_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct ak4671_setup_data *setup;
	struct snd_soc_codec *codec;
	struct ak4671_priv *ak4671;
	int ret;

//	DPRINTK(KERN_INFO "AK4671 Audio Codec %s", AK4671_VERSION);

	setup = socdev->codec_data;
	codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

	ak4671 = kzalloc(sizeof(struct ak4671_priv), GFP_KERNEL);
	if (ak4671 == NULL) {
		kfree(codec);
		return -ENOMEM;
	}

	codec->private_data = ak4671;
	socdev->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	ak4671_socdev = socdev;
	ret = -ENODEV;

	/* hw init. */
	ret = codec_hw_init();

	if (ret != 0) {
		kfree(codec->private_data);
		kfree(codec);
	}

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	if (setup->i2c_address) {
		codec->hw_write = (hw_write_t)i2c_master_send;
		codec->hw_read = (hw_read_t)i2c_master_recv;
		ret = ak4671_add_i2c_device(pdev, setup);
	}
#endif

	if (ret != 0) {
		kfree(codec->private_data);
		kfree(codec);
	}
	return ret;
}

/* power down chip */
static int ak4671_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;

	if (codec->control_data)
		ak4671_set_bias_level(codec, SND_SOC_BIAS_OFF);

	set_sound_path(ak4671_socdev->codec, (IN_OFF | OUT_OFF) );

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	i2c_unregister_device(codec->control_data);
	i2c_del_driver(&ak4671_i2c_driver);
#endif
	kfree(codec->private_data);
	kfree(codec);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_ak4671 = {
	.probe = 	ak4671_probe,
	.remove = 	ak4671_remove,
	.suspend = 	ak4671_suspend,
	.resume =	ak4671_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_ak4671);

MODULE_DESCRIPTION("Soc AK4671 driver");
MODULE_AUTHOR("Richard Purdie");
MODULE_LICENSE("GPL");
