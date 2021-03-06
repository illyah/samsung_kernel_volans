/*
 * s3c6410-i2s.c  --  ALSA Soc Audio Layer
 *
 * (c) 2009 Samsung Electronics   - Jaswinder Singh Brar <jassi.brar@samsung.com>
 *  Derived from Ben Dooks' driver for s3c24xx
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <asm/dma.h>
#include <mach/map.h>

#include "s3c-pcm.h"
#include "s3c6410-i2s.h"

//#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define debug_msg(x...) printk(x)
#else
#define debug_msg(x...)
#endif

static struct s3c2410_dma_client s3c6410_dma_client_out = {
	.name = "I2S PCM Stereo out"
};

static struct s3c2410_dma_client s3c6410_dma_client_in = {
	.name = "I2S PCM Stereo in"
};

static struct s3c24xx_pcm_dma_params s3c6410_i2s_pcm_stereo_out = {
	.client		= &s3c6410_dma_client_out,
#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
	.channel	= DMACH_I2S_OUT,
	.dma_addr	= S3C64XX_PA_IIS + S3C64XX_IISTXD,
#else
	.channel	= DMACH_I2S_V40_OUT,
	.dma_addr	= S3C64XX_PA_IIS_V40 + S3C64XX_IISTXD,
#endif
	.dma_size	= 4,
};

static struct s3c24xx_pcm_dma_params s3c6410_i2s_pcm_stereo_in = {
	.client		= &s3c6410_dma_client_in,
#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
	.channel	= DMACH_I2S_IN,
	.dma_addr	= S3C64XX_PA_IIS + S3C64XX_IISRXD,
#else
	.channel	= DMACH_I2S_V40_IN,
	.dma_addr	= S3C64XX_PA_IIS_V40 + S3C64XX_IISRXD,
#endif
	.dma_size	= 4,
};

struct s3c6410_i2s_info {
	void __iomem	*regs;
	struct clk	*iis_clk;
	struct clk	*audio_bus;
	u32		iiscon;
	u32		iismod;
	u32		iisfic;
	u32		iispsr;
	u32		slave;
	u32		clk_rate;
};
static struct s3c6410_i2s_info s3c6410_i2s;

static void dump_i2s(void)
{
	debug_msg("S3C_IISCON : %x\n", readl(s3c6410_i2s.regs + S3C64XX_IISCON));
	debug_msg("S3C_IISMOD : %x\n", readl(s3c6410_i2s.regs + S3C64XX_IISMOD));
	debug_msg("S3C_IISFIC : %x\n", readl(s3c6410_i2s.regs + S3C64XX_IISFIC));
	debug_msg("S3C_IISPSR : %x\n", readl(s3c6410_i2s.regs + S3C64XX_IISPSR));
}

static void s3c6410_snd_txctrl(int on)
{
	u32 iiscon;
//	u32 iisfic;
//	u32 timeout;
	debug_msg("%s:%d \n",__func__,__LINE__);

	iiscon  = readl(s3c6410_i2s.regs + S3C64XX_IISCON);

	if(on){
		iiscon |= S3C64XX_IISCON_I2SACTIVE;
		iiscon  &= ~S3C64XX_IISCON_TXCHPAUSE;
		iiscon  &= ~S3C64XX_IISCON_TXDMAPAUSE;
		iiscon  |= S3C64XX_IISCON_TXDMACTIVE;
		writel(iiscon,  s3c6410_i2s.regs + S3C64XX_IISCON);

//		dump_i2s();
	}else{
		//if recording is running, do not disable IIS clock
		if ( (iiscon & S3C64XX_IISCON_RXDMACTIVE) != S3C64XX_IISCON_RXDMACTIVE)
		{
			iiscon &= ~S3C64XX_IISCON_I2SACTIVE;
		}
		
		iiscon  |= S3C64XX_IISCON_TXCHPAUSE;
		iiscon  |= S3C64XX_IISCON_TXDMAPAUSE;
		iiscon  &= ~S3C64XX_IISCON_TXDMACTIVE;
		writel(iiscon,  s3c6410_i2s.regs + S3C64XX_IISCON);
#if 0	
		/*Empty Tx FIFO*/
		timeout = 50;
	
		iisfic = readl(s3c6410_i2s.regs + S3C64XX_IISFIC);
		iisfic |= S3C64XX_IISFIC_TFLUSH;
		writel(iisfic, s3c6410_i2s.regs + S3C64XX_IISFIC);

		do{
			iiscon = readl(s3c6410_i2s.regs + S3C64XX_IISCON);
			if (!timeout--)
			{
				printk("Warning: Tx FIFO is not empty\n");
				return 0;
			}
		}while((iiscon & S3C64XX_IISCON_FTX0EMPT) != (S3C64XX_IISCON_FTX0EMPT));
#endif

	}
//	printk("iiscon 0x%08x\n", iiscon);
}

static void s3c6410_snd_rxctrl(int on)
{
	u32 iiscon;
	debug_msg("%s:%d \n",__func__,__LINE__);

	iiscon  = readl(s3c6410_i2s.regs + S3C64XX_IISCON);

	if(on){
		iiscon |= S3C64XX_IISCON_I2SACTIVE;
		iiscon  &= ~S3C64XX_IISCON_RXCHPAUSE;
		iiscon  &= ~S3C64XX_IISCON_RXDMAPAUSE;
		iiscon  |= S3C64XX_IISCON_RXDMACTIVE;
		writel(iiscon,  s3c6410_i2s.regs + S3C64XX_IISCON);
	}else{
		//if playback is running, do not disable IIS clock
		if ( (iiscon & S3C64XX_IISCON_TXDMACTIVE)  != S3C64XX_IISCON_TXDMACTIVE)
		{
			iiscon &= ~S3C64XX_IISCON_I2SACTIVE;
		}
		iiscon  |= S3C64XX_IISCON_RXCHPAUSE;
		iiscon  |= S3C64XX_IISCON_RXDMAPAUSE;
		iiscon  &= ~S3C64XX_IISCON_RXDMACTIVE;
		writel(iiscon,  s3c6410_i2s.regs + S3C64XX_IISCON);
	}

}

/*
 * Wait for the LR signal to allow synchronisation to the L/R clock
 * from the codec. May only be needed for slave mode.
 */
static int s3c6410_snd_lrsync(void)
{
	u32 iiscon;
	int timeout = 50; /* 5ms */
	debug_msg("%s:%d \n",__func__,__LINE__);

	while (1) {
		iiscon = readl(s3c6410_i2s.regs + S3C64XX_IISCON);
//		printk("s3c6410_snd_lrsync, iiscon: 0x%08d\n",iiscon);
		if (iiscon & S3C64XX_IISCON_LRI)
			break;

		if (!timeout--)
			return -ETIMEDOUT;
		udelay(100);
	}

	return 0;
}

/*
 * Set s3c64xx I2S DAI format
 */
static int s3c6410_i2s_set_fmt(struct snd_soc_dai *cpu_dai,
		unsigned int fmt)
{
	u32 iismod;
	debug_msg("%s:%d \n",__func__,__LINE__);

	iismod = readl(s3c6410_i2s.regs + S3C64XX_IISMOD);
	iismod &= ~S3C64XX_IISMOD_SDFMASK;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		s3c6410_i2s.slave = 1;
		debug_msg("AP Slave\n");
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		s3c6410_i2s.slave = 0;
		debug_msg("AP Master\n");
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iismod &= ~S3C64XX_IISMOD_MSB;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iismod |= S3C64XX_IISMOD_MSB;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		iismod |= S3C64XX_IISMOD_LSB;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		iismod &= ~S3C64XX_IISMOD_LRP;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iismod |= S3C64XX_IISMOD_LRP;
		break;
	case SND_SOC_DAIFMT_IB_IF:
	case SND_SOC_DAIFMT_IB_NF:
	default:
		printk("Inv-combo(%d) not supported!\n", fmt & SND_SOC_DAIFMT_FORMAT_MASK);
		return -EINVAL;
	}

	writel(iismod, s3c6410_i2s.regs + S3C64XX_IISMOD);
	return 0;
}

static int s3c6410_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	u32 iismod;
	debug_msg("%s:%d \n",__func__,__LINE__);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		rtd->dai->cpu_dai->dma_data = &s3c6410_i2s_pcm_stereo_out;
	else
		rtd->dai->cpu_dai->dma_data = &s3c6410_i2s_pcm_stereo_in;

	/* Working copies of register */
	iismod = readl(s3c6410_i2s.regs + S3C64XX_IISMOD);
	iismod &= ~S3C64XX_IISMOD_BLCMASK;

	/* TODO */
	switch(params_channels(params)) {
	case 1:
		break;
	case 2:
		break;
	case 4:
		break;
	case 6:
		break;
	default:
		break;
	}

	/* RFS & BFS are set by dai_link(machine specific) code via set_clkdiv */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		iismod |= S3C64XX_IISMOD_8BIT;
 		break;
 	case SNDRV_PCM_FORMAT_S16_LE:
 		iismod |= S3C64XX_IISMOD_16BIT;
 		break;
 	case SNDRV_PCM_FORMAT_S24_LE:
 		iismod |= S3C64XX_IISMOD_24BIT;
 		break;
	default:
		return -EINVAL;
 	}
 
	writel(iismod, s3c6410_i2s.regs + S3C64XX_IISMOD);

	return 0;
}


static int s3c6410_i2s_startup(struct snd_pcm_substream *substream)

{

	u32 iiscon, iisfic;
	int timeout;

	debug_msg("%s\n", __FUNCTION__);

	/*Empty Tx FIFO*/
	timeout = 50;
	
	if (substream->stream  == SNDRV_PCM_STREAM_PLAYBACK){
		
		iisfic = readl(s3c6410_i2s.regs + S3C64XX_IISFIC);
		iisfic |= S3C64XX_IISFIC_TFLUSH;
		writel(iisfic, s3c6410_i2s.regs + S3C64XX_IISFIC);

		do{
			iiscon = readl(s3c6410_i2s.regs + S3C64XX_IISCON);
			if (!timeout--)
			{
				printk("Warning: Tx FIFO is not empty\n");
				return 0;
			}
		}while((iiscon & S3C64XX_IISCON_FTX0EMPT) != (S3C64XX_IISCON_FTX0EMPT));
	}

	/*Empty Rx FIFO*/
	timeout = 50;

	if (substream->stream  == SNDRV_PCM_STREAM_CAPTURE){

		iisfic = readl(s3c6410_i2s.regs + S3C64XX_IISFIC);
		iisfic |= S3C64XX_IISFIC_RFLUSH;
		writel(iisfic, s3c6410_i2s.regs + S3C64XX_IISFIC);

		do{
			iiscon = readl(s3c6410_i2s.regs + S3C64XX_IISCON);
			if (!timeout--)
			{
				printk("Warning: Rx FIFO is not empty\n");
				return 0;
			}
		}while((iiscon & S3C64XX_IISCON_FRXEMPT) != (S3C64XX_IISCON_FRXEMPT));

	}

	iisfic = readl(s3c6410_i2s.regs + S3C64XX_IISFIC);
	iisfic &= ~(S3C64XX_IISFIC_TFLUSH | S3C64XX_IISFIC_RFLUSH);
	writel(iisfic, s3c6410_i2s.regs + S3C64XX_IISFIC);

	return 0;
}


static int s3c6410_i2s_prepare(struct snd_pcm_substream *substream)
{
	u32 iismod;
	debug_msg("%s:%d \n",__func__,__LINE__);
	
	iismod = readl(s3c6410_i2s.regs + S3C64XX_IISMOD);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		if((iismod & S3C64XX_IISMOD_TXRMASK) == S3C64XX_IISMOD_RX){
			iismod &= ~S3C64XX_IISMOD_TXRMASK;
			iismod |= S3C64XX_IISMOD_TXRX;
		}
	}else{
		if((iismod & S3C64XX_IISMOD_TXRMASK) == S3C64XX_IISMOD_TX){
			iismod &= ~S3C64XX_IISMOD_TXRMASK;
			iismod |= S3C64XX_IISMOD_TXRX;
		}
	}

	writel(iismod, s3c6410_i2s.regs + S3C64XX_IISMOD);

	return 0;
}

static int s3c6410_i2s_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	debug_msg("%s:%d \n",__func__,__LINE__);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (s3c6410_i2s.slave) {
			ret = s3c6410_snd_lrsync();
			if (ret)
				goto exit_err;
		}

		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			s3c6410_snd_rxctrl(1);
		else
			s3c6410_snd_txctrl(1);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			s3c6410_snd_rxctrl(0);
		else
			s3c6410_snd_txctrl(0);
		break;
	default:
		ret = -EINVAL;
		break;
	}

exit_err:
	return ret;
}

/*
 * Set s3c64xx Clock source
 * Since, we set frequencies using PreScaler and BFS, RFS, we select input clock source to the IIS here.
 */
static int s3c6410_i2s_set_sysclk(struct snd_soc_dai *cpu_dai,
	int clk_id, unsigned int freq, int dir)
{
	struct clk *clk;
	u32 iismod = readl(s3c6410_i2s.regs + S3C64XX_IISMOD);
	debug_msg("%s:%d \n",__func__,__LINE__);

	switch (clk_id) {
	case S3C6410_CLKSRC_PCLK:
		if(s3c6410_i2s.slave)
			return -EINVAL;
		iismod &= ~S3C64XX_IISMOD_IMSMASK;
		iismod |= clk_id;
		s3c6410_i2s.clk_rate = clk_get_rate(s3c6410_i2s.iis_clk);
		break;

#ifdef USE_CLKAUDIO
	case S3C6410_CLKSRC_CLKAUDIO:
		if(s3c6410_i2s.slave)
			return -EINVAL;
		iismod &= ~S3C64XX_IISMOD_IMSMASK;
		iismod |= clk_id;
/*
8000 x 256 = 2048000
         49152000 mod 2048000 = 0
         32768000 mod 2048000 = 0 
         73728000 mod 2048000 = 0

11025 x 256 = 2822400
         67738000 mod 2822400 = 400

16000 x 256 = 4096000
         49152000 mod 4096000 = 0
         32768000 mod 4096000 = 0 
         73728000 mod 4096000 = 0

22050 x 256 = 5644800
         67738000 mod 5644800 = 400

32000 x 256 = 8192000
         49152000 mod 8192000 = 0
         32768000 mod 8192000 = 0
         73728000 mod 8192000 = 0

44100 x 256 = 11289600
         67738000 mod 11289600 = 400

48000 x 256 = 12288000
         49152000 mod 12288000 = 0
         73728000 mod 12288000 = 0

64000 x 256 = 16384000
         49152000 mod 16384000 = 0
         32768000 mod 16384000 = 0

88200 x 256 = 22579200
         67738000 mod 22579200 = 400

96000 x 256 = 24576000
         49152000 mod 24576000 = 0
         73728000 mod 24576000 = 0

	From the table above, we find that 49152000 gives least(0) residue 
	for most sample rates, followed by 67738000.
*/
		clk = clk_get(NULL, "fout_epll");
		if (IS_ERR(clk)) {
			printk("failed to get FOUTepll\n");
			return -EBUSY;
		}
		clk_disable(clk);
		switch (freq) {
		case 8000:
		case 16000:
		case 32000:
		case 48000:
		case 64000:
		case 96000:
			clk_set_rate(clk, 49152000);
			break;
		case 11025:
		case 22050:
		case 44100:
		case 88200:
		default:
			clk_set_rate(clk, 67738000);
			break;
		}
		clk_enable(clk);
		s3c6410_i2s.clk_rate = clk_get_rate(s3c6410_i2s.audio_bus);
		clk_put(clk);
		break;
#endif

	case S3C6410_CLKSRC_SLVPCLK:
	case S3C6410_CLKSRC_I2SEXT:
#if USE_AP_MASTER	
		if(!s3c6410_i2s.slave)
			return -EINVAL;
#endif
		iismod &= ~S3C64XX_IISMOD_IMSMASK;
		iismod |= clk_id;
		break;

	/* Not sure about these two! */
	case S3C6410_CDCLKSRC_INT:
		iismod &= ~S3C64XX_IISMOD_CDCLKCON;
		break;

	case S3C6410_CDCLKSRC_EXT:
		iismod |= S3C64XX_IISMOD_CDCLKCON;
		break;

	default:
		return -EINVAL;
	}

	writel(iismod, s3c6410_i2s.regs + S3C64XX_IISMOD);
	return 0;
}

/*
 * Set s3c64xx Clock dividers
 * NOTE: NOT all combinations of RFS, BFS and BCL are supported! XXX
 * Machine specific(dai-link) code must consider that while setting MCLK and BCLK in this function. XXX
 */
/* XXX BLC(bits-per-channel) --> BFS(bit clock shud be >= FS*(Bit-per-channel)*2) XXX */
/* XXX BFS --> RFS_VAL(must be a multiple of BFS)                                 XXX */
/* XXX RFS_VAL & SRC_CLK --> Prescalar Value(SRC_CLK / RFS_VAL / fs - 1)          XXX */
static int s3c6410_i2s_set_clkdiv(struct snd_soc_dai *cpu_dai,
	int div_id, int div)
{
	u32 reg;
	debug_msg("%s:%d \n",__func__,__LINE__);

	switch (div_id) {
	case S3C64XX_DIV_MCLK:
		reg = readl(s3c6410_i2s.regs + S3C64XX_IISMOD) & ~S3C64XX_IISMOD_RFSMASK;
		switch(div) {
		case 256: div = S3C64XX_IISMOD_256FS; break;
		case 512: div = S3C64XX_IISMOD_512FS; break;
		case 384: div = S3C64XX_IISMOD_384FS; break;
		case 768: div = S3C64XX_IISMOD_768FS; break;
		default: return -EINVAL;
		}
		writel(reg | div, s3c6410_i2s.regs + S3C64XX_IISMOD);
		break;
	case S3C64XX_DIV_BCLK:
		reg = readl(s3c6410_i2s.regs + S3C64XX_IISMOD) & ~S3C64XX_IISMOD_BFSMASK;
		switch(div) {
		case 16: div = S3C64XX_IISMOD_16FS; break;
		case 24: div = S3C64XX_IISMOD_24FS; break;
		case 32: div = S3C64XX_IISMOD_32FS; break;
		case 48: div = S3C64XX_IISMOD_48FS; break;
		default: return -EINVAL;
		}
		writel(reg | div, s3c6410_i2s.regs + S3C64XX_IISMOD);
		break;
	case S3C64XX_DIV_PRESCALER:
		reg = readl(s3c6410_i2s.regs + S3C64XX_IISPSR) & ~S3C64XX_IISPSR_PSRAEN;
		writel(reg, s3c6410_i2s.regs + S3C64XX_IISPSR);
		reg = readl(s3c6410_i2s.regs + S3C64XX_IISPSR) & ~S3C64XX_IISPSR_PSVALA;
		div &= 0x3f;
		writel(reg | (div<<8) | S3C64XX_IISPSR_PSRAEN, s3c6410_i2s.regs + S3C64XX_IISPSR);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * To avoid duplicating clock code, allow machine driver to
 * get the clockrate from here.
 */
u32 s3c6410_i2s_get_clockrate(void)
{
	debug_msg("%s:%d \n",__func__,__LINE__);
	
	return s3c6410_i2s.clk_rate;
}
EXPORT_SYMBOL_GPL(s3c6410_i2s_get_clockrate);

static irqreturn_t s3c_iis_irq(int irqno, void *dev_id)
{
	u32 iiscon;
	debug_msg("%s:%d \n",__func__,__LINE__);

	iiscon  = readl(s3c6410_i2s.regs + S3C64XX_IISCON);
	if(S3C64XX_IISCON_FTXURSTATUS & iiscon) {
		iiscon &= ~S3C64XX_IISCON_FTXURINTEN;
		iiscon |= S3C64XX_IISCON_FTXURSTATUS;
		writel(iiscon, s3c6410_i2s.regs + S3C64XX_IISCON);
		printk("underrun interrupt IISCON = 0x%08x\n", readl(s3c6410_i2s.regs + S3C64XX_IISCON));
	}

	return IRQ_HANDLED;
}

static int s3c6410_i2s_probe(struct platform_device *pdev,
			     struct snd_soc_dai *dai)
{
	int ret = 0;
	struct clk *cm, *cf;
	debug_msg("%s:%d \n",__func__,__LINE__);

#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
	s3c6410_i2s.regs = ioremap(S3C64XX_PA_IIS, 0x100);
#else
	s3c6410_i2s.regs = ioremap(S3C64XX_PA_IIS_V40, 0x100);
#endif
	if (s3c6410_i2s.regs == NULL)
		return -ENXIO;

#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
	ret = request_irq(IRQ_S3C6410_IIS, s3c_iis_irq, 0, "s3c-i2s-v32", pdev);
#else
	ret = request_irq(IRQ_S3C6410_IIS, s3c_iis_irq, 0, "s3c-i2s-v40", pdev);
#endif
	if (ret < 0) {
		printk("fail to claim i2s irq , ret = %d\n", ret);
		iounmap(s3c6410_i2s.regs);
		return -ENODEV;
	}

#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
	s3c6410_i2s.iis_clk = clk_get(&pdev->dev, "iis");
#else
	s3c6410_i2s.iis_clk = clk_get(&pdev->dev, "iis_v40");
#endif
	if (IS_ERR(s3c6410_i2s.iis_clk)) {
		printk("failed to get iis_clock\n");
		goto lb5;
	}
	clk_enable(s3c6410_i2s.iis_clk);
	s3c6410_i2s.clk_rate = clk_get_rate(s3c6410_i2s.iis_clk);

#ifdef USE_CLKAUDIO
#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
	s3c6410_i2s.audio_bus = clk_get(NULL, "audio-bus0");
#else
	s3c6410_i2s.audio_bus = clk_get(NULL, "audio-bus2");
#endif
	if (IS_ERR(s3c6410_i2s.audio_bus)) {
		printk("failed to get audio_bus\n");
		goto lb4;
	}

	cm = clk_get(NULL, "mout_epll");
	if (IS_ERR(cm)) {
		printk("failed to get MOUTepll\n");
		goto lb3;
	}
	if(clk_set_parent(s3c6410_i2s.audio_bus, cm)){
		printk("failed to set MOUTepll as parent of CLKAUDIO0\n");
		goto lb2;
	}

	cf = clk_get(NULL, "fout_epll");
	if (IS_ERR(cf)) {
		printk("failed to get FOUTepll\n");
		goto lb2;
	}
	clk_enable(cf);
	if(clk_set_parent(cm, cf)){
		printk("failed to set FOUTepll as parent of MOUTepll\n");
		goto lb1;
	}
	s3c6410_i2s.clk_rate = clk_get_rate(s3c6410_i2s.audio_bus);
	clk_put(cf);
	clk_put(cm);
#endif

	writel(S3C64XX_IISCON_I2SACTIVE, s3c6410_i2s.regs + S3C64XX_IISCON);

	s3c6410_snd_txctrl(0);
	s3c6410_snd_rxctrl(0);

	return 0;

#ifdef USE_CLKAUDIO
lb1:
	clk_put(cf);
lb2:
	clk_put(cm);
lb3:
	clk_put(s3c6410_i2s.audio_bus);
lb4:
	clk_disable(s3c6410_i2s.iis_clk);
	clk_put(s3c6410_i2s.iis_clk);
#endif
lb5:
	free_irq(IRQ_S3C6410_IIS, pdev);
	iounmap(s3c6410_i2s.regs);
	
	return -ENODEV;
}

#ifdef CONFIG_PM
static int s3c6410_i2s_suspend(struct platform_device *pdev,
		struct snd_soc_dai *cpu_dai)
{

	debug_msg("%s:%d \n",__func__,__LINE__);
	printk("%s:%d \n",__func__,__LINE__);
	
	s3c6410_i2s.iiscon = readl(s3c6410_i2s.regs + S3C64XX_IISCON);
	s3c6410_i2s.iismod = readl(s3c6410_i2s.regs + S3C64XX_IISMOD);
	s3c6410_i2s.iisfic = readl(s3c6410_i2s.regs + S3C64XX_IISFIC);
	s3c6410_i2s.iispsr = readl(s3c6410_i2s.regs + S3C64XX_IISPSR);

	dump_i2s();
	
	clk_disable(s3c6410_i2s.iis_clk);

	return 0;
}

static int s3c6410_i2s_resume(struct platform_device *pdev,
		struct snd_soc_dai *cpu_dai)
{
	debug_msg("%s:%d \n",__func__,__LINE__);
	printk("%s:%d \n",__func__,__LINE__);
	
	clk_enable(s3c6410_i2s.iis_clk);

	writel(s3c6410_i2s.iiscon, s3c6410_i2s.regs + S3C64XX_IISCON);
	writel(s3c6410_i2s.iismod, s3c6410_i2s.regs + S3C64XX_IISMOD);
	writel(s3c6410_i2s.iisfic, s3c6410_i2s.regs + S3C64XX_IISFIC);
	writel(s3c6410_i2s.iispsr, s3c6410_i2s.regs + S3C64XX_IISPSR);

	dump_i2s();
	
	return 0;
}
#else
#define s3c6410_i2s_suspend NULL
#define s3c6410_i2s_resume NULL
#endif


#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
struct snd_soc_dai s3c6410_i2s_v32_dai = {
#else
struct snd_soc_dai s3c6410_i2s_v40_dai = {
#endif
	.name = "s3c6410-i2s",
	.id = 0,
	.type = SND_SOC_DAI_I2S,
	.probe = s3c6410_i2s_probe,
	.suspend = s3c6410_i2s_suspend,
	.resume = s3c6410_i2s_resume,
	.playback = {
		.channels_min = 2,
#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
		.channels_max = 2,
#else
		.channels_max = 6,
#endif
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.capture = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.ops = {
		.hw_params = s3c6410_i2s_hw_params,
		.prepare = s3c6410_i2s_prepare,
		.startup = s3c6410_i2s_startup,
		.trigger = s3c6410_i2s_trigger,
	},
	.dai_ops = {
		.set_fmt = s3c6410_i2s_set_fmt,
		.set_clkdiv = s3c6410_i2s_set_clkdiv,
		.set_sysclk = s3c6410_i2s_set_sysclk,
	},
};
#ifdef CONFIG_SND_S3C6410_SOC_I2S_V32
EXPORT_SYMBOL_GPL(s3c6410_i2s_v32_dai);
#else
EXPORT_SYMBOL_GPL(s3c6410_i2s_v40_dai);
#endif

/* Module information */
MODULE_AUTHOR("Jaswinder Singh <jassi.brar@samsung.com>");
MODULE_DESCRIPTION("s3c6410 I2S SoC Interface");
MODULE_LICENSE("GPL");
