/* linux/arch/arm/mach-s3c6400/include/mach/system.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * S3C6400 - system implementation
 */

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H __FILE__

#include <linux/io.h>
#include <mach/map.h>
#include <plat/regs-watchdog.h>

void (*s3c64xx_idle)(void);
void (*s3c64xx_reset_hook)(void);

void s3c64xx_default_idle(void)
{
	/* nothing here yet */
}
	
static void arch_idle(void)
{
	if (s3c64xx_idle != NULL)
		(s3c64xx_idle)();
	else
		s3c64xx_default_idle();
}

static void arch_reset(char mode)
{
	void __iomem	*wdt_reg;
#ifdef CONFIG_SAMSUNG_WATCHDOG_RESET_BOOT	

#define WTD_NO_LOCKUP_MAGIC_CODE_MASK		0x00123456

	static unsigned int * watch_dog_boot_flag;
	extern void __iomem * log_buf_base;
	
	// clear rb_dump index to prohibit watch dog reset boot
	if(log_buf_base != NULL)
	{
		watch_dog_boot_flag = (unsigned int *)log_buf_base;
		(*watch_dog_boot_flag) = WTD_NO_LOCKUP_MAGIC_CODE_MASK;
	}else
		printk("[WTD][ERROR] %s() log_buf_base is NULL\n", __FUNCTION__);
#endif


	wdt_reg = ioremap(S3C64XX_PA_WATCHDOG,S3C64XX_SZ_WATCHDOG);

	/* nothing here yet */

	writel(S3C2410_WTCNT_CNT, wdt_reg + S3C2410_WTCNT_OFFSET);	/* Watchdog Count Register*/
	writel(S3C2410_WTCNT_CON, wdt_reg + S3C2410_WTCON_OFFSET);	/* Watchdog Controller Register*/
	writel(S3C2410_WTCNT_DAT, wdt_reg + S3C2410_WTDAT_OFFSET);	/* Watchdog Data Register*/
	
}

#endif /* __ASM_ARCH_IRQ_H */
