/* linux/arch/arm/mach-s3c6410/setup-sdhci.c
 *
 * Copyright 2008 Simtec Electronics
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * S3C6410 - Helper functions for settign up SDHCI device(s) (HSMMC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>

#include <mach/gpio.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include <plat/regs-sdhci.h>
#include <plat/sdhci.h>

/* clock sources for the mmc bus clock, order as for the ctrl2[5..4] */

char *s3c6410_hsmmc_clksrcs[4] = {
	[0] = "hsmmc",
	[1] = "hsmmc",
	[2] = "mmc_bus",
	/* [3] = "48m", - note not succesfully used yet */
};

void s3c6410_setup_sdhci0_cfg_gpio(struct platform_device *dev, int width)
{
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPG(2 + width);

	/* Set all the necessary GPG pins to special-function 0 */
	for (gpio = S3C64XX_GPG(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

#if defined(CONFIG_MACH_VOLANS)
	s3c_gpio_cfgpin(GPIO_TF_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TF_EN, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TF_EN, GPIO_LEVEL_HIGH);	
	
	s3c_gpio_cfgpin(GPIO_T_FLASH_DETECT, S3C_GPIO_SFN(GPIO_T_FLASH_DETECT_AF));
	s3c_gpio_setpull(GPIO_T_FLASH_DETECT, S3C_GPIO_PULL_UP);

	writel((readl(S3C64XX_EINT0CON0) & ~(0x7 << 12)) |
		     (S3C64XX_EXTINT_BOTHEDGE << 12), S3C64XX_EINT0CON0);
#else
	s3c_gpio_setpull(S3C64XX_GPG(6), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(S3C64XX_GPG(6), S3C_GPIO_SFN(2));
#endif
}

void s3c6410_setup_sdhci0_cfg_card(struct platform_device *dev,
				    void __iomem *r,
				    struct mmc_ios *ios,
				    struct mmc_card *card)
{
	u32 ctrl2, ctrl3 = 0;


	writel(S3C64XX_SDHCI_CONTROL4_DRIVE_9mA, r + S3C64XX_SDHCI_CONTROL4);

	/* finally, we don't need to add delay values in HS-MMC interface.
	 * all delay values are removed. by scsuh.
	 */
	ctrl2 = readl(r + S3C_SDHCI_CONTROL2);
	ctrl2 &= S3C_SDHCI_CTRL2_SELBASECLK_MASK;
	ctrl2 |= (S3C64XX_SDHCI_CTRL2_ENSTAASYNCCLR |
		  S3C64XX_SDHCI_CTRL2_ENCMDCNFMSK |
		  S3C_SDHCI_CTRL2_DFCNT_NONE |
		  S3C_SDHCI_CTRL2_ENCLKOUTHOLD);

	writel(ctrl2, r + S3C_SDHCI_CONTROL2);
	writel(ctrl3, r + S3C_SDHCI_CONTROL3);
}

void s3c6410_setup_sdhci1_cfg_gpio(struct platform_device *dev, int width)
{
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPH(2 + width);

	/* Set all the necessary GPG pins to special-function 0 */
	for (gpio = S3C64XX_GPH(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}

void s3c6410_setup_sdhci2_cfg_gpio(struct platform_device *dev, int width)
{
	unsigned int gpio;
	unsigned int end;

	/* CMD */
	s3c_gpio_cfgpin(S3C64XX_GPC(4), S3C_GPIO_SFN(3));
	s3c_gpio_setpull(S3C64XX_GPC(4), S3C_GPIO_PULL_NONE);

	/* CLK */
	s3c_gpio_cfgpin(S3C64XX_GPC(5), S3C_GPIO_SFN(3));
	s3c_gpio_setpull(S3C64XX_GPC(5), S3C_GPIO_PULL_NONE);

	end = S3C64XX_GPH(6+width);

	/* Set all the necessary GPG pins to special-function 0 */
	for (gpio = S3C64XX_GPH(6); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}

