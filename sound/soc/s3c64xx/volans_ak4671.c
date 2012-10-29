/*
 * smdk6400_ak4671.c
 *
 * Copyright 2007, 2008 Wolfson Microelectronics PLC.
 *
 * Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>

#include <plat/regs-iis.h>
#include <plat/map-base.h>
#include <asm/gpio.h> 
#include <plat/gpio-cfg.h> 
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-h.h>
#include <plat/gpio-bank-c.h>

#include <mach/hardware.h>
#include <mach/audio.h>
#include <asm/io.h>
#include <plat/regs-clock.h>

#include "../codecs/ak4671.h"
#include "s3c-pcm.h"
#include "s3c6410-i2s.h"

#define SRC_CLK	s3c6410_i2s_get_clockrate()

//#define CONFIG_SND_DEBUG

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

/* XXX BLC(bits-per-channel) --> BFS(bit clock shud be >= FS*(Bit-per-channel)*2) XXX */
/* XXX BFS --> RFS(must be a multiple of BFS)                                 XXX */
/* XXX RFS & SRC_CLK --> Prescalar Value(SRC_CLK / RFS_VAL / fs - 1)          XXX */
static int smdk6410_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
        struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
        int bfs, rfs, psr, ret;
	
	s3cdbg("%s:%d \n",__func__,__LINE__);
        /* Choose BFS and RFS values combination that is supported by
	 * both the S5M8751 codec as well as the S5P6410 AP
	 *
	 * S5M8751 codec supports only S16_LE, S18_3LE, S20_3LE & S24_LE.
	 * S5P6410 AP supports only S8, S16_LE & S24_LE.
	 * We implement all for completeness but only S16_LE & S24_LE bit-lengths 
	 * are possible for this AP-Codec combination.
	 */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		bfs = 16;
		rfs = 256;		/* Can take any RFS value for AP */
 		break;
 	case SNDRV_PCM_FORMAT_S16_LE:
		bfs = 32;
		rfs = 256;		/* Can take any RFS value for AP */
 		break;
	case SNDRV_PCM_FORMAT_S18_3LE:
	case SNDRV_PCM_FORMAT_S20_3LE:
 	case SNDRV_PCM_FORMAT_S24_LE:
		bfs = 48;
		rfs = 384;		/* Can take only 384fs or 768fs RFS value for AP */
 		break;
	default:
		return -EINVAL;
 	}

        /* Select the AP Sysclk */
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C6410_CDCLKSRC_INT, params_rate(params), SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

#ifdef USE_CLKAUDIO

#if USE_AP_MASTER
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C6410_CLKSRC_CLKAUDIO, params_rate(params), SND_SOC_CLOCK_OUT);
#else
	DPRINTK("enter slave sysclk\n");
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C6410_CLKSRC_SLVPCLK, 0, SND_SOC_CLOCK_OUT); 
#endif

#else
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C6410_CLKSRC_PCLK, 0, SND_SOC_CLOCK_OUT);
#endif
	if (ret < 0)
		return ret;

	/* Set the AP DAI configuration */
#if USE_AP_MASTER
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
#else
	DPRINTK("enter slave set_fmt\n");
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
#endif
	if (ret < 0)
		return ret;

	/* Set the AP RFS */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C64XX_DIV_MCLK, rfs);
	if (ret < 0)
		return ret;

	/* Set the AP BFS */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C64XX_DIV_BCLK, bfs);
	if (ret < 0)
		return ret;

	switch (params_rate(params)) {
	case 8000:
	case 11025:
	case 16000:
	case 22050:
	case 32000:
	case 44100: 
	case 48000:
	case 64000:
	case 88200:
	case 96000:
		psr = SRC_CLK / rfs / params_rate(params);
		ret = SRC_CLK / rfs - psr * params_rate(params);
		if(ret >= params_rate(params)/2)	// round off
		   psr += 1;
		psr -= 1;
		break;
	default:
		return -EINVAL;
	}

	DPRINTK("SRC_CLK=%d PSR=%d RFS=%d BFS=%d\n", SRC_CLK, psr, rfs, bfs);

	/* Set the AP Prescalar */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C64XX_DIV_PRESCALER, psr);
	if (ret < 0)
		return ret;

	/* Set the Codec DAI configuration */
#if USE_AP_MASTER
	DPRINTK("enter codec master set_fmt\n");
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
#else 
	DPRINTK("enter codec slave set_fmt\n");
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
#endif
	if (ret < 0)
		return ret;

	// pll and sample rate setting		
	ret = snd_soc_dai_set_clkdiv(codec_dai, AK4671_SAMPLE_FREQ, params_rate(params));

#if USE_AP_MASTER
	DPRINTK("enter codec master clkdiv\n");
	ret = snd_soc_dai_set_clkdiv(codec_dai, AK4671_MCLK, AK4671_CLKSRC_MCLK);
#else 
	DPRINTK("enter codec slave clkdiv\n");
	ret = snd_soc_dai_set_clkdiv(codec_dai, AK4671_MCLK, AK4671_CLKSRC_OSC);
#endif
	if (ret < 0)
		return ret;
	return 0;
}

static int smdk6410_hw_free(struct snd_pcm_substream *substream)
{
	DPRINTK("%s:%d\n", __func__, __LINE__);
	return 0;
}

/*
 * AK4671 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk6410_ops = {
	.hw_params = smdk6410_hw_params,
        .hw_free = smdk6410_hw_free,
};

static int smdk6410_ak4671_init(struct snd_soc_codec *codec)
{
	return 0;
}

static struct snd_soc_dai_link smdk6410_dai[] = {
{
	.name = "AK4671",
	.stream_name = "AK4671 Playback",
	.cpu_dai = &s3c6410_i2s_v32_dai,
	.codec_dai = &ak4671_dai,
	.init = smdk6410_ak4671_init,
	.ops = &smdk6410_ops,
},
};

static struct snd_soc_machine smdk6410 = {
	.name = "smdk6410",
	.dai_link = smdk6410_dai,
	.num_links = ARRAY_SIZE(smdk6410_dai),
};

static struct ak4671_setup_data smdk6410_ak4671_setup = {
	.i2c_bus = 3,	// id = 3, GPIO_FM_SCL, GPIO_FM_SDA platform device.
	.i2c_address = (0x24 >> 1),
};

static struct snd_soc_device smdk6410_snd_devdata = {
	.machine = &smdk6410,
	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_ak4671,
	.codec_data = &smdk6410_ak4671_setup,
};

static struct platform_device *smdk6410_snd_device;

static int __init smdk6410_init(void)
{
	int ret;

	s3cdbg("%s:%d \n",__func__,__LINE__);
      
        /* Configure the I2S pins in correct mode */
        s3c_gpio_cfgpin(GPIO_I2S_SCLK, S3C_GPIO_SFN(GPIO_I2S_SCLK_AF)); 
	s3c_gpio_cfgpin(GPIO_I2S_SYNC, S3C_GPIO_SFN(GPIO_I2S_SYNC_AF)); 
	s3c_gpio_cfgpin(GPIO_I2S_SDI, S3C_GPIO_SFN(GPIO_I2S_SDI_AF));   
	s3c_gpio_cfgpin(GPIO_I2S_SDO, S3C_GPIO_SFN(GPIO_I2S_SDO_AF));

	/* pull-up-enable, pull-down-disable*/
	s3c_gpio_setpull(GPIO_I2S_SCLK, S3C_GPIO_PULL_UP); 
	s3c_gpio_setpull(GPIO_I2S_SYNC, S3C_GPIO_PULL_UP); 
	s3c_gpio_setpull(GPIO_I2S_SDI, S3C_GPIO_PULL_UP); 
	s3c_gpio_setpull(GPIO_I2S_SDO, S3C_GPIO_PULL_UP);


	smdk6410_snd_device = platform_device_alloc("soc-audio", 0);
	if (!smdk6410_snd_device)
		return -ENOMEM;

	platform_set_drvdata(smdk6410_snd_device, &smdk6410_snd_devdata);
	smdk6410_snd_devdata.dev = &smdk6410_snd_device->dev;
	ret = platform_device_add(smdk6410_snd_device);

	if (ret)
		platform_device_put(smdk6410_snd_device);
	
	return ret;
}

static void __exit smdk6410_exit(void)
{
	s3cdbg("%s:%d \n",__func__,__LINE__);
	platform_device_unregister(smdk6410_snd_device);
}

module_init(smdk6410_init);
module_exit(smdk6410_exit);

/* Module information */
MODULE_AUTHOR("Mark Brown");
MODULE_DESCRIPTION("ALSA SoC SMDK6410 AK4671");
MODULE_LICENSE("GPL");
