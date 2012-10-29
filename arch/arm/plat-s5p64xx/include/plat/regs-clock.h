/* arch/arm/plat-s5p64xx/include/plat/regs-clock.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * S5P64XX clock register definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_REGS_CLOCK_H
#define __PLAT_REGS_CLOCK_H __FILE__

#define S3C_CLKREG(x)		(S3C_VA_SYS + (x))

#define S3C_APLL_LOCK		S3C_CLKREG(0x00)
#define S3C_MPLL_LOCK		S3C_CLKREG(0x04)
#define S3C_EPLL_LOCK		S3C_CLKREG(0x08)
#define S3C_APLL_CON		S3C_CLKREG(0x0C)
#define S3C_MPLL_CON		S3C_CLKREG(0x10)
#define S3C_EPLL_CON		S3C_CLKREG(0x14)
#define S3C_EPLL_CON_K		S3C_CLKREG(0x18)
#define S3C_CLK_SRC0		S3C_CLKREG(0x1C)
#define S3C_CLK_DIV0		S3C_CLKREG(0x20)
#define S3C_CLK_DIV1		S3C_CLKREG(0x24)
#define S3C_CLK_DIV2		S3C_CLKREG(0x28)
#define S3C_CLK_OUT		S3C_CLKREG(0x2C)
#define S3C_CLK_GATE_HCLK0	S3C_CLKREG(0x30)
#define S3C_CLK_GATE_PCLK	S3C_CLKREG(0x34)
#define S3C_CLK_GATE_SCLK0	S3C_CLKREG(0x38)
#define S3C_CLK_GATE_MEM0	S3C_CLKREG(0x3C)
#define S3C_CLK_DIV3		S3C_CLKREG(0x40)
#define S3C_CLK_GATE_HCLK1	S3C_CLKREG(0x44)
#define S3C_CLK_GATE_SCLK1	S3C_CLKREG(0x48)
#define S3C_AHB_CON0           	S3C_CLKREG(0x100) 
#define S3C_CLK_SRC1           	S3C_CLKREG(0x10C)
#define S3C_SWRESET		S3C_CLKREG(0x114)
#define S3C_SYS_ID		S3C_CLKREG(0x118)
#define S3C_SYS_OTHERS		S3C_CLKREG(0x11C)
#define S3C_MEM_CFG_STAT	S3C_CLKREG(0x12C)
#define S3C_PWR_CFG		S3C_CLKREG(0x804)
#define S3C_EINT_WAKEUP_MASK	S3C_CLKREG(0x808)
#define S3C_NORMAL_CFG		S3C_CLKREG(0x810)
#define S3C_STOP_CFG		S3C_CLKREG(0x814)
#define S3C_SLEEP_CFG		S3C_CLKREG(0x818)
#define S3C_OSC_FREQ		S3C_CLKREG(0x820)
#define S3C_OSC_STABLE		S3C_CLKREG(0x824)
#define S3C_PWR_STABLE		S3C_CLKREG(0x828)
#define S3C_MTC_STABLE		S3C_CLKREG(0x830)
#define S3C_OTHERS		S3C_CLKREG(0x900)
#define S3C_RST_STAT		S3C_CLKREG(0x904)
#define S3C_WAKEUP_STAT		S3C_CLKREG(0x908)
#define S3C_INFORM0		S3C_CLKREG(0xA00)
#define S3C_INFORM1		S3C_CLKREG(0xA04)
#define S3C_INFORM2		S3C_CLKREG(0xA08)
#define S3C_INFORM3		S3C_CLKREG(0xA0C)

#define S3C_EPLL_CON_M_SHIFT	16
#define S3C_EPLL_CON_P_SHIFT	8
#define S3C_EPLL_CON_S_SHIFT	0
#define S3C_EPLL_CON_K_SHIFT	0

#define S3C_EPLL_CON_M_MASK	(0xff<<S3C_EPLL_CON_M_SHIFT)
#define S3C_EPLL_CON_P_MASK	(0x3f<<S3C_EPLL_CON_P_SHIFT)
#define S3C_EPLL_CON_S_MASK	(0x7<<S3C_EPLL_CON_S_SHIFT)
#define S3C_EPLL_CON_K_MASK	(0xffff<<S3C_EPLL_CON_K_SHIFT)

/* CLKDIV0 */
#define S3C_CLKDIV0_PCLK_MASK	(0xf << 12)
#define S3C_CLKDIV0_PCLK_SHIFT	(12)
#define S3C_CLKDIV0_HCLK_MASK	(0xf << 8)
#define S3C_CLKDIV0_HCLK_SHIFT	(8)
#define S3C_CLKDIV0_MPLL_MASK	(0x1 << 4)
#define S3C_CLKDIV0_MPLL_SHIFT	(4)
#define S3C_CLKDIV0_ARM_MASK	(0xf << 0)
#define S3C_CLKDIV0_ARM_SHIFT	(0)

/* CLKDIV1 */
#define S3C_CLKDIV1_LCD_MASK	(0xf << 12)
#define S3C_CLKDIV1_LCD_SHIFT	(12)
#define S3C_CLKDIV1_MMC2_MASK	(0xf << 8)
#define S3C_CLKDIV1_MMC2_SHIFT	(8)
#define S3C_CLKDIV1_MMC1_MASK	(0xf << 4)
#define S3C_CLKDIV1_MMC1_SHIFT	(4)
#define S3C_CLKDIV1_MMC0_MASK	(0xf << 0)
#define S3C_CLKDIV1_MMC0_SHIFT	(0)

/* CLKDIV2 */
#define S3C_CLKDIV2_AUDIO2_MASK	(0xf << 24)
#define S3C_CLKDIV2_AUDIO2_SHIFT	(24)
#define S3C_CLKDIV2_UART_MASK	(0xf << 16)
#define S3C_CLKDIV2_UART_SHIFT	(16)
#define S3C_CLKDIV2_SPI1_MASK	(0xf << 4)
#define S3C_CLKDIV2_SPI1_SHIFT	(4)
#define S3C_CLKDIV2_SPI0_MASK	(0xf << 0)
#define S3C_CLKDIV2_SPI0_SHIFT	(0)

/* CLKDIV3 */
#define S3C_CLKDIV3_PCLK_LOW_MASK	(0xf << 12)
#define S3C_CLKDIV3_PCLK_LOW_SHIFT	(12)
#define S3C_CLKDIV3_HCLK_LOW_MASK	(0xf << 8)
#define S3C_CLKDIV3_HCLK_LOW_SHIFT	(8)
#define S3C_CLKDIV3_FIMGVG_MASK	(0xf << 4)
#define S3C_CLKDIV3_FIMGVG_SHIFT	(4)
#define S3C_CLKDIV3_DISPCON_MASK	(0xf << 0)
#define S3C_CLKDIV3_DISPCON_SHIFT	(0)

/* HCLK0 GATE Registers */
#define S3C_CLKCON_HCLK0_MEM0	(1<<21)
#define S3C_CLKCON_HCLK0_USB	(1<<20)
#define S3C_CLKCON_HCLK0_HSMMC2	(1<<19)
#define S3C_CLKCON_HCLK0_HSMMC1	(1<<18)
#define S3C_CLKCON_HCLK0_HSMMC0	(1<<17)
#define S3C_CLKCON_HCLK0_DMA0	(1<<12)
#define S3C_CLKCON_HCLK0_2D	(1<<8)
#define S3C_CLKCON_HCLK0_POST0	(1<<5)
#define S3C_CLKCON_HCLK0_TZIC	(1<<2)
#define S3C_CLKCON_HCLK0_INTC	(1<<1)

/* HCLK1 GATE Registers */
#define S3C_CLKCON_HCLK1_FIMGVG	(1<<2)
#define S3C_CLKCON_HCLK1_DISPCON	(1<<1)
#define S3C_CLKCON_HCLK1_TSI	(1<<0)

/* PCLK GATE Registers */
#define S3C_CLKCON_PCLK_FIMGVG		(1<<31)
#define S3C_CLKCON_PCLK_DMC0		(1<<30)
#define S3C_CLKCON_PCLK_ETM		(1<<29)
#define S3C_CLKCON_PCLK_DSIM		(1<<28)
#define S3C_CLKCON_PCLK_IIC1		(1<<27)
#define S3C_CLKCON_PCLK_IIS2		(1<<26)
#define S3C_CLKCON_PCLK_GPS		(1<<25)
#define S3C_CLKCON_PCLK_CHIPID		(1<<23)
#define S3C_CLKCON_PCLK_SPI1		(1<<22)
#define S3C_CLKCON_PCLK_SPI0		(1<<21)
#define S3C_CLKCON_PCLK_GPIO		(1<<18)
#define S3C_CLKCON_PCLK_IIC0		(1<<17)
#define S3C_CLKCON_PCLK_TSADC		(1<<12)
#define S3C_CLKCON_PCLK_PCM0		(1<<8)
#define S3C_CLKCON_PCLK_PWM		(1<<7)
#define S3C_CLKCON_PCLK_RTC		(1<<6)
#define S3C_CLKCON_PCLK_WDT		(1<<5)
#define S3C_CLKCON_PCLK_UART3		(1<<4)
#define S3C_CLKCON_PCLK_UART2		(1<<3)
#define S3C_CLKCON_PCLK_UART1		(1<<2)
#define S3C_CLKCON_PCLK_UART0		(1<<1)

/* SCLK0 GATE Registers */
#define S3C_CLKCON_SCLK0_MMC2_48	(1<<29)
#define S3C_CLKCON_SCLK0_MMC1_48	(1<<28)
#define S3C_CLKCON_SCLK0_MMC0_48	(1<<27)
#define S3C_CLKCON_SCLK0_MMC2		(1<<26)
#define S3C_CLKCON_SCLK0_MMC1		(1<<25)
#define S3C_CLKCON_SCLK0_MMC0		(1<<24)
#define S3C_CLKCON_SCLK0_SPI1_48 	(1<<23)
#define S3C_CLKCON_SCLK0_SPI0_48 	(1<<22)
#define S3C_CLKCON_SCLK0_SPI1		(1<<21)
#define S3C_CLKCON_SCLK0_SPI0		(1<<20)
#define S3C_CLKCON_SCLK0_AUDIO2		(1<<11)
#define S3C_CLKCON_SCLK0_POST0		(1<<10)
#define S3C_CLKCON_SCLK0_UART		(1<<5)

/* SCLK1 GATE Registers */
#define S3C_CLKCON_SCLK1_FIMGVG		(1<<2)
#define S3C_CLKCON_SCLK1_DISPCON	(1<<1)

/* MEM0 GATE Registers */
#define S3C_CLKCON_MEM0_HCLK_NFCON	(1<<2)
#define S3C_CLKCON_MEM0_HCLK_SROM	(1<<1)
#define S3C_CLKCON_MEM0_HCLK_DMC0	(1<<0)

/*OTHERS Resgister */
#define S3C_OTHERS_USB_SIG_MASK		(1<<16)
#define S3C_OTHERS_HCLK_LOW_SEL_MPLL	(1<<6)

/* CLKSRC */
#define S3C_CLKSRC_APLL_MOUT	(1 << 0)
#define S3C_CLKSRC_MPLL_MOUT	(1 << 1)
#define S3C_CLKSRC_EPLL_MOUT	(1 << 2)
#define S3C_CLKSRC_APLL_MOUT_SHIFT	(0)
#define S3C_CLKSRC_MPLL_MOUT_SHIFT	(1)
#define S3C_CLKSRC_EPLL_MOUT_SHIFT	(2)

#define S3C_CLKSRC_POST0_MASK	(0x3 << 26)
#define S3C_CLKSRC_POST0_SHIFT	(26)
#define S3C_CLKSRC_MMC2_MASK	(0x3 << 22)
#define S3C_CLKSRC_MMC2_SHIFT	(22)
#define S3C_CLKSRC_MMC1_MASK	(0x3 << 20)
#define S3C_CLKSRC_MMC1_SHIFT	(20)
#define S3C_CLKSRC_MMC0_MASK	(0x3 << 18)
#define S3C_CLKSRC_MMC0_SHIFT	(18)
#define S3C_CLKSRC_SPI1_MASK	(0x3 << 16)
#define S3C_CLKSRC_SPI1_SHIFT	(16)
#define S3C_CLKSRC_SPI0_MASK	(0x3 << 14)
#define S3C_CLKSRC_SPI0_SHIFT	(14)
#define S3C_CLKSRC_UART_MASK	(0x1 << 13)
#define S3C_CLKSRC_UART_SHIFT	(13)

#define S3C_CLKSRC1_AUDIO2_MASK		(0x7 << 0)
#define S3C_CLKSRC1_AUDIO2_SHIFT	(0)

/*CLK SRC BITS*/
#define S3C_CLKSRC_APLL_CLKSEL          (1<<0)                                           
#define S3C_CLKSRC_MPLL_CLKSEL          (1<<1)                                           
#define S3C_CLKSRC_EPLL_CLKSEL          (1<<2)                                           


#endif /* _PLAT_REGS_CLOCK_H */
