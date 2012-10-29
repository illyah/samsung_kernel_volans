/*
 * ak4671_hw.c  --  AK4671 Platform Hardware driver
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#include "ak4671.h"

int codec_hw_init(void)
{
	s3c_gpio_cfgpin(GPIO_AUDIO_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(GPIO_MICBIAS_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(GPIO_MIC_SEL, S3C_GPIO_OUTPUT);

	return 0;
}

int codec_power_ctrl(int mode)
{
	int power_lv = (mode == POWER_ON) ? 1 : 0;

	gpio_set_value(GPIO_AUDIO_EN, power_lv);

	udelay(10);

	if (gpio_get_value(GPIO_AUDIO_EN) != power_lv)
	{
		printk("%s:%d -- ..\n", __func__, __LINE__);

		return 0;
	}

	return 1;
}

int mic_select_ctrl(int mic_sel)
{
	int mic_sel_lv = (mic_sel == MAIN_MIC) ? 1 : 0;

	gpio_set_value(GPIO_MIC_SEL, mic_sel_lv);

	udelay(10);

	if(gpio_get_value(GPIO_MIC_SEL) != mic_sel_lv)
	{
		printk("%s:%d why...\n",__func__,__LINE__);

		return 0;
	}
	return 1;
}

int mic_power_ctrl(int mode)
{
	int power_lv = (mode == POWER_ON) ? 1 : 0;

	gpio_set_value(GPIO_MICBIAS_EN, power_lv);

	udelay(10);

	if (gpio_get_value(GPIO_MICBIAS_EN) != power_lv)
	{
		printk("%s:%d why...\n", __func__, __LINE__);

		return 0;
	}

	return 1;
}
EXPORT_SYMBOL(mic_power_ctrl);
