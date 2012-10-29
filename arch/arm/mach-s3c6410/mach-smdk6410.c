/* linux/arch/arm/mach-s3c6410/mach-smdk6410.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/pwm_backlight.h>
#include <linux/spi/spi.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/regs-mem.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <plat/iic.h>
#include <plat/fimc.h>
#include <plat/fb.h>

#include <plat/regs-rtc.h>

#include <plat/nand.h>
#include <plat/partition.h>
#include <plat/s3c6410.h>
#include <plat/clock.h>
#include <plat/regs-clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/ts.h>
#include <plat/adc.h>
#include <plat/pm.h>
#include <plat/pll.h>
#include <plat/spi.h>

#include <mach/gpio.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-c.h>
#include <plat/gpio-bank-f.h>

#ifdef CONFIG_USB_SUPPORT
#include <plat/regs-otg.h>
#include <linux/usb/ch9.h>

/* S3C_USB_CLKSRC 0: EPLL 1: CLK_48M */
#define S3C_USB_CLKSRC	1

#ifdef USB_HOST_PORT2_EN
#define OTGH_PHY_CLK_VALUE      (0x60)  /* Serial Interface, otg_phy input clk 48Mhz Oscillator */
#else
#define OTGH_PHY_CLK_VALUE      (0x20)  /* UTMI Interface, otg_phy input clk 48Mhz Oscillator */
#endif
#endif

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

extern struct sys_timer s3c_timer;
extern void s3c64xx_reserve_bootmem(void);

#if defined(CONFIG_SPI_CNTRLR_0) || defined(CONFIG_SPI_CNTRLR_1)
static void cs_set(int pin, int lvl)
{
	unsigned int val;

	val = __raw_readl(S3C64XX_GPFDAT);
	val &= ~(1<<pin);

	if(lvl == CS_HIGH)
	   val |= (1<<pin);

	__raw_writel(val, S3C64XX_GPFDAT);
}

static void sam_cs_setF13(int pin, int lvl)
{
	cs_set(13, lvl);
}

static void sam_cs_setF14(int pin, int lvl)
{
	cs_set(14, lvl);
}

static void sam_cs_setF15(int pin, int lvl)
{
	cs_set(15, lvl);
}

static void cs_cfg(int pin, int pull)
{
	unsigned int val;

	val = __raw_readl(S3C64XX_GPFCON);
	val &= ~(3<<(pin*2));
	val |= (1<<(pin*2)); /* Output Mode */
	__raw_writel(val, S3C64XX_GPFCON);

	val = __raw_readl(S3C64XX_GPFPUD);
	val &= ~(3<<(pin*2));

	if(pull == CS_HIGH)
	   val |= (2<<(pin*2));	/* PullUp */
	else
	   val |= (1<<(pin*2)); /* PullDown */

	__raw_writel(val, S3C64XX_GPFPUD);
}

static void sam_cs_configF13(int pin, int mode, int pull)
{
	cs_cfg(13, pull);
}

static void sam_cs_configF14(int pin, int mode, int pull)
{
	cs_cfg(14, pull);
}

static void sam_cs_configF15(int pin, int mode, int pull)
{
	cs_cfg(15, pull);
}

static void sam_cs_set(int pin, int lvl)
{
	if(lvl == CS_HIGH)
	   s3c_gpio_setpin(pin, 1);
	else
	   s3c_gpio_setpin(pin, 0);
}

static void sam_cs_config(int pin, int mode, int pull)
{
	s3c_gpio_cfgpin(pin, mode);

	if(pull == CS_HIGH)
	   s3c_gpio_setpull(pin, S3C_GPIO_PULL_UP);
	else
	   s3c_gpio_setpull(pin, S3C_GPIO_PULL_DOWN);
}
#endif

#if defined(CONFIG_SPI_CNTRLR_0)
/* GPC-3 is ChipSelect(Active Low) of Slave-0 connected to Bus-0 */
DECLARE_SPIPDATA(sam_cs_set, sam_cs_config, CS_ACT_LOW, S3C64XX_GPC(3), S3C64XX_GPC_OUTPUT(3), 0, 0);
/* GPF-13(J1-pin3 on SMDK base b/d) is ChipSelect(Active Low) of Slave-1 connected to Bus-0 */
DECLARE_SPIPDATA(sam_cs_setF13, sam_cs_configF13, CS_ACT_LOW, S3C64XX_GPF(13), S3C64XX_GPF_OUTPUT(13), 0, 1);
#endif

#if defined(CONFIG_SPI_CNTRLR_1)
/* GPC-7 is ChipSelect(Active Low) of Slave-0 connected to Bus-1 */
DECLARE_SPIPDATA(sam_cs_set, sam_cs_config, CS_ACT_LOW, S3C64XX_GPC(7), S3C64XX_GPC_OUTPUT(7), 1, 0);
/* GPF-14(J1-pin2 on SMDK base b/d) is ChipSelect(Active Low) of Slave-1 connected to Bus-1 */
DECLARE_SPIPDATA(sam_cs_setF14, sam_cs_configF14, CS_ACT_LOW, S3C64XX_GPF(14), S3C64XX_GPF_OUTPUT(14), 1, 1);
/* GPF-15(J1-pin1 on SMDK base b/d) is ChipSelect(Active Low) of Slave-2 connected to Bus-1 */
DECLARE_SPIPDATA(sam_cs_setF15, sam_cs_configF15, CS_ACT_LOW, S3C64XX_GPF(15), S3C64XX_GPF_OUTPUT(15), 1, 2);
#endif

static struct sam_spi_mstr_info spi_mst_info[] = {
	[0] = {
#if defined(CONFIG_SPI_CNTRLR_0)
		.num_slaves = 2,	/* From above declarations, SPI0 has 2 Slaves attached to it */
#else
		.num_slaves = 0,	/* No Slave */
#endif
	},
	[1] = {
#if defined(CONFIG_SPI_CNTRLR_1)
		.num_slaves = 3,	/* From above declarations, SPI1 has 3 Slaves attached to it */
#else
		.num_slaves = 0,	/* No Slave */
#endif
	},
};

static struct spi_board_info sam_spi_devs[] = {
#if defined(CONFIG_SPI_CNTRLR_0)
	[0] = {
		.modalias	 = "spidev", /* Test Interface */
		.mode		 = SPI_MODE_0,	/* CPOL=0, CPHA=0 */
		.max_speed_hz    = 100000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num	 = 0,
		.irq		 = IRQ_SPI0,
		.chip_select	 = 0,
		.controller_data = (void *)&sam_slv_pdata_b0s0,
	},
	[1] = {
		.modalias	 = "spidev", /* Test Interface */
		.mode		 = SPI_MODE_0,	/* CPOL=0, CPHA=0 */
		.max_speed_hz    = 100000,
		/* Connected to SPI-0 as 2nd Slave */
		.bus_num	 = 0,
		.irq		 = IRQ_SPI0,
		.chip_select	 = 1,
		.controller_data = (void *)&sam_slv_pdata_b0s1,
	},
#endif
#if defined(CONFIG_SPI_CNTRLR_1)
	[2] = {
		.modalias	 = "spidev", /* Test Interface */
		.mode		 = SPI_MODE_0,	/* CPOL=0, CPHA=0 */
		.max_speed_hz    = 100000,
		/* Connected to SPI-1 as 1st Slave */
		.bus_num	 = 1,
		.irq		 = IRQ_SPI1,
		.chip_select	 = 0,
		.controller_data = (void *)&sam_slv_pdata_b1s0,
	},
	[3] = {
		.modalias	 = "spidev", /* Test Interface */
		.mode		 = SPI_MODE_0,	/* CPOL=0, CPHA=0 */
		.max_speed_hz    = 100000,
		/* Connected to SPI-1 as 2nd Slave */
		.bus_num	 = 1,
		.irq		 = IRQ_SPI1,
		.chip_select	 = 1,
		.controller_data = (void *)&sam_slv_pdata_b1s1,
	},
	[4] = {
		.modalias	 = "mmc_spi", /* MMC SPI */
		.mode		 = SPI_MODE_0,
		.max_speed_hz    = 100000,
		/* Connected to SPI-1 as 3rd Slave */
		.bus_num	= 1,
		.irq		= IRQ_SPI1,
		.chip_select	 = 2,
		.controller_data = (void *)&sam_slv_pdata_b1s2,
	},
#endif
};

static struct s3c2410_uartcfg smdk6410_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = S3C64XX_UCON_DEFAULT,
		.ulcon	     = S3C64XX_ULCON_DEFAULT,
		.ufcon	     = S3C64XX_UFCON_DEFAULT,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = S3C64XX_UCON_DEFAULT,
		.ulcon	     = S3C64XX_ULCON_DEFAULT,
		.ufcon	     = S3C64XX_UFCON_DEFAULT,
	},
};

struct map_desc smdk6410_iodesc[] = {};

static struct platform_device *smdk6410_devices[] __initdata = {
#ifdef CONFIG_FB_S3C_V2
	&s3c_device_fb,
#endif
#ifdef CONFIG_SMDK6410_SD_CH0
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_SMDK6410_SD_CH1
	&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_SMDK6410_SD_CH2
	&s3c_device_hsmmc2,
#endif
	&s3c_device_wdt,
	&s3c_device_rtc,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
#if defined(CONFIG_SPI_CNTRLR_0)
	&s3c_device_spi0,
#endif
#if defined(CONFIG_SPI_CNTRLR_1)
	&s3c_device_spi1,
#endif
	&s3c_device_keypad,
	&s3c_device_ts,
	&s3c_device_smc911x,
	&s3c_device_lcd,
	&s3c_device_vpp,
	&s3c_device_mfc,
	&s3c_device_tvenc,
	&s3c_device_tvscaler,
	&s3c_device_rotator,
	&s3c_device_jpeg,
	&s3c_device_nand,
	&s3c_device_onenand,
	&s3c_device_usb,
	&s3c_device_usbgadget,
	&s3c_device_usb_otghcd,
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_g2d,	
	&s3c_device_g3d,	
	&s3c_device_rp,

#ifdef CONFIG_S3C64XX_ADC
	&s3c_device_adc,
#endif

#ifdef CONFIG_HAVE_PWM
	&s3c_device_timer[0],
	&s3c_device_timer[1],
#endif
};

static struct i2c_board_info i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },
/*	{ I2C_BOARD_INFO("WM8580", 0x1b), },	*/
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{ I2C_BOARD_INFO("24c128", 0x57), },	/* Samsung S524AD0XD1 */
	{ I2C_BOARD_INFO("WM8580", 0x1b), },
};

static struct s3c_ts_mach_info s3c_ts_platform __initdata = {
	.delay 			= 10000,
	.presc 			= 49,
	.oversampling_shift	= 2,
	.resol_bit 		= 12,
	.s3c_adc_con		= ADC_TYPE_2,
};

static struct s3c_adc_mach_info s3c_adc_platform = {
	/* s3c6410 support 12-bit resolution */
	.delay	= 	10000,
	.presc 	= 	49,
	.resolution = 	12,
};

#if defined(CONFIG_HAVE_PWM)
static struct platform_pwm_backlight_data smdk_backlight_data = {
	.pwm_id		= 1,
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 78770,
};

static struct platform_device smdk_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent = &s3c_device_timer[1].dev,
		.platform_data = &smdk_backlight_data,
	},
};

static void __init smdk_backlight_register(void)
{
	int ret = platform_device_register(&smdk_backlight_device);
	if (ret)
		printk(KERN_ERR "smdk: failed to register backlight device: %d\n", ret);
}
#else
#define smdk_backlight_register()	do { } while (0)
#endif

static void __init smdk6410_map_io(void)
{
	s3c_device_nand.name = "s3c6410-nand";

	s3c64xx_init_io(smdk6410_iodesc, ARRAY_SIZE(smdk6410_iodesc));
	s3c24xx_init_clocks(XTAL_FREQ);
	s3c24xx_init_uarts(smdk6410_uartcfgs, ARRAY_SIZE(smdk6410_uartcfgs));
	s3c64xx_reserve_bootmem();
}

static void __init smdk6410_smc911x_set(void)
{
	unsigned int tmp;

	tmp = __raw_readl(S3C64XX_SROM_BW);
	tmp &= ~(S3C64XX_SROM_BW_WAIT_ENABLE1_MASK | S3C64XX_SROM_BW_WAIT_ENABLE1_MASK |
		S3C64XX_SROM_BW_DATA_WIDTH1_MASK);
	tmp |= S3C64XX_SROM_BW_BYTE_ENABLE1_ENABLE | S3C64XX_SROM_BW_WAIT_ENABLE1_ENABLE |
		S3C64XX_SROM_BW_DATA_WIDTH1_16BIT;

	__raw_writel(tmp, S3C64XX_SROM_BW);

	__raw_writel(S3C64XX_SROM_BCn_TACS(0) | S3C64XX_SROM_BCn_TCOS(4) |
			S3C64XX_SROM_BCn_TACC(13) | S3C64XX_SROM_BCn_TCOH(1) |
			S3C64XX_SROM_BCn_TCAH(4) | S3C64XX_SROM_BCn_TACP(6) |
			S3C64XX_SROM_BCn_PMC_NORMAL, S3C64XX_SROM_BC1);
}

static void __init smdk6410_machine_init(void)
{
	s3c_device_nand.dev.platform_data = &s3c_nand_mtd_part_info;
	s3c_device_onenand.dev.platform_data = &s3c_onenand_data;

	smdk6410_smc911x_set();

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);

	s3c_ts_set_platdata(&s3c_ts_platform);
	s3c_adc_set_platdata(&s3c_adc_platform);

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

#if defined(CONFIG_SPI_CNTRLR_0)
	samspi_set_info(0, &spi_mst_info[0]);
#endif
#if defined(CONFIG_SPI_CNTRLR_1)
	samspi_set_info(1, &spi_mst_info[1]);
#endif
	spi_register_board_info(sam_spi_devs, ARRAY_SIZE(sam_spi_devs));

	s3c_fimc0_set_platdata(NULL);
	s3c_fimc1_set_platdata(NULL);

/* fb */
#ifdef CONFIG_FB_S3C_V2
	s3cfb_set_platdata(NULL);
#endif

#ifdef CONFIG_VIDEO_FIMC
	s3c_fimc_reset_camera();
#endif

	platform_add_devices(smdk6410_devices, ARRAY_SIZE(smdk6410_devices));
	s3c6410_pm_init();

	smdk_backlight_register();
	
}

MACHINE_START(SMDK6410, "SMDK6410")
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C_PA_UART & 0xfff00000,
	.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C64XX_PA_SDRAM + 0x100,

	.init_irq	= s3c6410_init_irq,
	.map_io		= smdk6410_map_io,
	.init_machine	= smdk6410_machine_init,
	.timer		= &s3c64xx_timer,
MACHINE_END

#ifdef CONFIG_USB_SUPPORT
/* Initializes OTG Phy. */
void otg_phy_init(void) {

	writel(readl(S3C_OTHERS)|S3C_OTHERS_USB_SIG_MASK, S3C_OTHERS);
	writel(0x0, S3C_USBOTG_PHYPWR);		/* Power up */
        writel(OTGH_PHY_CLK_VALUE, S3C_USBOTG_PHYCLK);
	writel(0x1, S3C_USBOTG_RSTCON);

	udelay(50);
	writel(0x0, S3C_USBOTG_RSTCON);
	udelay(50);
}
EXPORT_SYMBOL(otg_phy_init);

/* USB Control request data struct must be located here for DMA transfer */
struct usb_ctrlrequest usb_ctrl __attribute__((aligned(8)));
EXPORT_SYMBOL(usb_ctrl);

/* OTG PHY Power Off */
void otg_phy_off(void) {
	writel(readl(S3C_USBOTG_PHYPWR)|(0x1F<<1), S3C_USBOTG_PHYPWR);
	writel(readl(S3C_OTHERS)&~S3C_OTHERS_USB_SIG_MASK, S3C_OTHERS);
}
EXPORT_SYMBOL(otg_phy_off);

void usb_host_clk_en(void) {
	struct clk *otg_clk;

        switch (S3C_USB_CLKSRC) {
	case 0: /* epll clk */
		writel((readl(S3C_CLK_SRC)& ~S3C6400_CLKSRC_UHOST_MASK)
			|S3C_CLKSRC_EPLL_CLKSEL|S3C_CLKSRC_UHOST_EPLL,
			S3C_CLK_SRC);

		/* USB host colock divider ratio is 2 */
		writel((readl(S3C_CLK_DIV1)& ~S3C6400_CLKDIV1_UHOST_MASK)
			|(1<<20), S3C_CLK_DIV1);
		break;
	case 1: /* oscillator 48M clk */
		otg_clk = clk_get(NULL, "otg");
		clk_enable(otg_clk);
		writel(readl(S3C_CLK_SRC)& ~S3C6400_CLKSRC_UHOST_MASK, S3C_CLK_SRC);
		otg_phy_init();

		/* USB host colock divider ratio is 1 */
		writel(readl(S3C_CLK_DIV1)& ~S3C6400_CLKDIV1_UHOST_MASK, S3C_CLK_DIV1);
		break;
	default:
		printk(KERN_INFO "Unknown USB Host Clock Source\n");
		BUG();
		break;
	}

	writel(readl(S3C_HCLK_GATE)|S3C_CLKCON_HCLK_UHOST|S3C_CLKCON_HCLK_SECUR,
		S3C_HCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_UHOST, S3C_SCLK_GATE);

}

EXPORT_SYMBOL(usb_host_clk_en);
#endif

#if defined(CONFIG_RTC_DRV_S3C)
/* RTC common Function for samsung APs*/
unsigned int s3c_rtc_set_bit_byte(void __iomem *base, uint offset, uint val)
{
	writeb(val, base + offset);

	return 0;
}

unsigned int s3c_rtc_read_alarm_status(void __iomem *base)
{
	return 1;
}

void s3c_rtc_set_pie(void __iomem *base, uint to)
{
	unsigned int tmp;

	tmp = readw(base + S3C2410_RTCCON) & ~S3C_RTCCON_TICEN;

        if (to)
                tmp |= S3C_RTCCON_TICEN;

        writew(tmp, base + S3C2410_RTCCON);
}

void s3c_rtc_set_freq_regs(void __iomem *base, uint freq, uint s3c_freq)
{
	unsigned int tmp;

        tmp = readw(base + S3C2410_RTCCON) & (S3C_RTCCON_TICEN | S3C2410_RTCCON_RTCEN );
        writew(tmp, base + S3C2410_RTCCON);
        s3c_freq = freq;
        tmp = (32768 / freq)-1;
        writel(tmp, base + S3C2410_TICNT);
}

void s3c_rtc_enable_set(struct platform_device *pdev,void __iomem *base, int en)
{
	unsigned int tmp;

	if (!en) {
		tmp = readw(base + S3C2410_RTCCON);
		writew(tmp & ~ (S3C2410_RTCCON_RTCEN | S3C_RTCCON_TICEN), base + S3C2410_RTCCON);
	} else {
		/* re-enable the device, and check it is ok */
		if ((readw(base+S3C2410_RTCCON) & S3C2410_RTCCON_RTCEN) == 0){
			dev_info(&pdev->dev, "rtc disabled, re-enabling\n");

			tmp = readw(base + S3C2410_RTCCON);
			writew(tmp|S3C2410_RTCCON_RTCEN, base+S3C2410_RTCCON);
		}

		if ((readw(base + S3C2410_RTCCON) & S3C2410_RTCCON_CNTSEL)){
			dev_info(&pdev->dev, "removing RTCCON_CNTSEL\n");

			tmp = readw(base + S3C2410_RTCCON);
			writew(tmp& ~S3C2410_RTCCON_CNTSEL, base+S3C2410_RTCCON);
		}

		if ((readw(base + S3C2410_RTCCON) & S3C2410_RTCCON_CLKRST)){
			dev_info(&pdev->dev, "removing RTCCON_CLKRST\n");

			tmp = readw(base + S3C2410_RTCCON);
			writew(tmp & ~S3C2410_RTCCON_CLKRST, base+S3C2410_RTCCON);
		}
	}
}
#endif

#if defined(CONFIG_KEYPAD_S3C) || defined (CONFIG_KEYPAD_S3C_MODULE)
void s3c_setup_keypad_cfg_gpio(int rows, int columns)
{
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPK(8 + rows);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S3C64XX_GPK(8); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	end = S3C64XX_GPL(0 + columns);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S3C64XX_GPL(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}

EXPORT_SYMBOL(s3c_setup_keypad_cfg_gpio);
#endif

#ifdef CONFIG_MMC_SDHCI_S3C
void s3c_setup_hsmmc_clock(void)
{
	struct clk *clk;

	clk = clk_get(NULL, "mmc_bus");
}
EXPORT_SYMBOL(s3c_setup_hsmmc_clock);
#endif

