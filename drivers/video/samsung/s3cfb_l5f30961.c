














#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <plat/regs-gpio.h>
#include <plat/regs-lcd.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-clock.h>
#include <mach/hardware.h>

#include <asm/plat-s3c/regs-iic.h>

#include "s3cfb.h"

#define GPIO_LEVEL_LOW      0
#define GPIO_LEVEL_HIGH     1

#define S3C_FB_HFP			8	
#define S3C_FB_HSW			8	
#define S3C_FB_HBP			8	

#define S3C_FB_VFP		38	
#define S3C_FB_VSW		4	
#define S3C_FB_VBP		38	

#define S3C_FB_HRES				240 	
#define S3C_FB_VRES				400 	

#define S3C_FB_HRES_VIRTUAL		240 	
#define S3C_FB_VRES_VIRTUAL		800 	

#define S3C_FB_HRES_OSD			240		
#define S3C_FB_VRES_OSD			400 	

#define S3C_FB_VFRAME_FREQ  	60 

#define S3C_FB_PIXEL_CLOCK	(S3C_FB_VFRAME_FREQ * \
								(S3C_FB_HFP + S3C_FB_HSW + S3C_FB_HBP + S3C_FB_HRES) * \
								(S3C_FB_VFP + S3C_FB_VSW + S3C_FB_VBP + S3C_FB_VRES))


static void s3cfb_set_fimd_info(void)
{
	s3cfb_fimd.vidcon1	= S3C_VIDCON1_IHSYNC_INVERT |
							S3C_VIDCON1_IVSYNC_INVERT |
							S3C_VIDCON1_IVDEN_NORMAL;

	s3cfb_fimd.vidtcon0 	= S3C_VIDTCON0_VBPD(S3C_FB_VBP - 1) |
							S3C_VIDTCON0_VFPD(S3C_FB_VFP - 1) |
							S3C_VIDTCON0_VSPW(S3C_FB_VSW - 1);
	s3cfb_fimd.vidtcon1	= S3C_VIDTCON1_HBPD(S3C_FB_HBP - 1) |
							S3C_VIDTCON1_HFPD(S3C_FB_HFP - 1) |
							S3C_VIDTCON1_HSPW(S3C_FB_HSW - 1);
	s3cfb_fimd.vidtcon2	= S3C_VIDTCON2_LINEVAL(S3C_FB_VRES - 1) |
							S3C_VIDTCON2_HOZVAL(S3C_FB_HRES - 1);

	s3cfb_fimd.vidosd0a 	= S3C_VIDOSDxA_OSD_LTX_F(0) |
							S3C_VIDOSDxA_OSD_LTY_F(0);
	s3cfb_fimd.vidosd0b 	= S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES - 1) |
							S3C_VIDOSDxB_OSD_RBY_F(S3C_FB_VRES - 1);

	s3cfb_fimd.vidosd1a 	= S3C_VIDOSDxA_OSD_LTX_F(0) |
							S3C_VIDOSDxA_OSD_LTY_F(0);
	s3cfb_fimd.vidosd1b 	= S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES_OSD - 1) |
							S3C_VIDOSDxB_OSD_RBY_F(S3C_FB_VRES_OSD - 1);

	s3cfb_fimd.width		= S3C_FB_HRES;
	s3cfb_fimd.height 	= S3C_FB_VRES;
	s3cfb_fimd.xres 		= S3C_FB_HRES;
	s3cfb_fimd.yres 		= S3C_FB_VRES;

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
	s3cfb_fimd.xres_virtual = S3C_FB_HRES_VIRTUAL;
	s3cfb_fimd.yres_virtual = S3C_FB_VRES_VIRTUAL;
#else
	s3cfb_fimd.xres_virtual = S3C_FB_HRES;
	s3cfb_fimd.yres_virtual = S3C_FB_VRES;
#endif

	s3cfb_fimd.osd_width 	= S3C_FB_HRES_OSD;
	s3cfb_fimd.osd_height = S3C_FB_VRES_OSD;
	s3cfb_fimd.osd_xres 	= S3C_FB_HRES_OSD;
	s3cfb_fimd.osd_yres 	= S3C_FB_VRES_OSD;

	s3cfb_fimd.osd_xres_virtual = S3C_FB_HRES_OSD;
	s3cfb_fimd.osd_yres_virtual = S3C_FB_VRES_OSD;

	s3cfb_fimd.pixclock		= S3C_FB_PIXEL_CLOCK;

	s3cfb_fimd.hsync_len 		= S3C_FB_HSW;
	s3cfb_fimd.vsync_len 		= S3C_FB_VSW;
	s3cfb_fimd.left_margin 	= S3C_FB_HFP;
	s3cfb_fimd.upper_margin 	= S3C_FB_VFP;
	s3cfb_fimd.right_margin 	= S3C_FB_HBP;
	s3cfb_fimd.lower_margin 	= S3C_FB_VBP;
}
static void lcd_gpio_init(void)
{
	mdelay(50);

	printk( "throughc : %s\n", __FUNCTION__ );
	s3c_gpio_cfgpin(GPIO_LCD_D2, S3C_GPIO_SFN(GPIO_LCD_D2_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D3, S3C_GPIO_SFN(GPIO_LCD_D3_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D4, S3C_GPIO_SFN(GPIO_LCD_D4_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D5, S3C_GPIO_SFN(GPIO_LCD_D5_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D6, S3C_GPIO_SFN(GPIO_LCD_D6_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D7, S3C_GPIO_SFN(GPIO_LCD_D7_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D10, S3C_GPIO_SFN(GPIO_LCD_D10_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D11, S3C_GPIO_SFN(GPIO_LCD_D11_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D12, S3C_GPIO_SFN(GPIO_LCD_D12_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D13, S3C_GPIO_SFN(GPIO_LCD_D13_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D14, S3C_GPIO_SFN(GPIO_LCD_D14_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D15, S3C_GPIO_SFN(GPIO_LCD_D15_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D18, S3C_GPIO_SFN(GPIO_LCD_D18_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D19, S3C_GPIO_SFN(GPIO_LCD_D19_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D20, S3C_GPIO_SFN(GPIO_LCD_D20_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D21, S3C_GPIO_SFN(GPIO_LCD_D21_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D22, S3C_GPIO_SFN(GPIO_LCD_D22_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D23, S3C_GPIO_SFN(GPIO_LCD_D23_AF));

	

	s3c_gpio_cfgpin(GPIO_LCD_HSYNC, S3C_GPIO_SFN(GPIO_LCD_HSYNC_AF));
	
	s3c_gpio_cfgpin(GPIO_LCD_VSYNC, S3C_GPIO_SFN(GPIO_LCD_VSYNC_AF));
	
	s3c_gpio_cfgpin(GPIO_LCD_DE, S3C_GPIO_SFN(GPIO_LCD_DE_AF));
	
	s3c_gpio_cfgpin(GPIO_LCD_MCLK, S3C_GPIO_SFN(GPIO_LCD_MCLK_AF));

	
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_nRST, S3C_GPIO_SFN(GPIO_LCD_nRST_AF));
	s3c_gpio_setpull(GPIO_LCD_nRST, S3C_GPIO_PULL_NONE);
	mdelay(1);

	
	gpio_set_value(GPIO_LCD_EN, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_EN,S3C_GPIO_SFN(GPIO_LCD_EN_AF));
	s3c_gpio_setpull(GPIO_LCD_EN, S3C_GPIO_PULL_NONE);
	mdelay(50);

	
	s3c_gpio_cfgpin(GPIO_LCD_CLK, S3C_GPIO_SFN(GPIO_LCD_CLK_AF));
	s3c_gpio_setpull(GPIO_LCD_CLK, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_LCD_CLK, GPIO_LEVEL_LOW);

	
	gpio_set_value(GPIO_LCD_nCS, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_nCS, S3C_GPIO_SFN(GPIO_LCD_nCS_AF));
	s3c_gpio_setpull(GPIO_LCD_nCS, S3C_GPIO_PULL_NONE);

	
	gpio_set_value(GPIO_LCD_SI, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_SI, S3C_GPIO_SFN(GPIO_LCD_SI_AF));
	s3c_gpio_setpull(GPIO_LCD_SI, S3C_GPIO_PULL_NONE);
}







#define LCD_CS_HIGH      gpio_set_value(GPIO_LCD_nCS, GPIO_LEVEL_HIGH);
#define LCD_CS_LOW       gpio_set_value(GPIO_LCD_nCS, GPIO_LEVEL_LOW);

#define LCD_CLK_HIGH     gpio_set_value(GPIO_LCD_CLK, GPIO_LEVEL_HIGH);
#define LCD_CLK_LOW      gpio_set_value(GPIO_LCD_CLK, GPIO_LEVEL_LOW);

#define LCD_SI_HIGH      gpio_set_value(GPIO_LCD_SI, GPIO_LEVEL_HIGH);
#define LCD_SI_LOW       gpio_set_value(GPIO_LCD_SI, GPIO_LEVEL_LOW);

#define SPI_DELAY        100


static void DelayLoop_spi(int delay)
{
	volatile int j;
	for(j = 0; j < SPI_DELAY*delay; j++)  ;
}

static void spi_write(unsigned short data)
{
	int j;

	LCD_CS_HIGH;
	LCD_CLK_LOW;


	DelayLoop_spi(1);
	
	LCD_CS_LOW;
	DelayLoop_spi(1);	
	
	for (j = 15; j >= 0; j--)
	{
		LCD_CLK_LOW;

		if((0x01<<j) & (data<<7))
		{
		
			LCD_SI_HIGH;
		}
		else
		{
		
			LCD_SI_LOW;
		}
		DelayLoop_spi(1);
		LCD_CLK_HIGH;
		DelayLoop_spi(1);
	}	
	
	LCD_CLK_LOW;
	DelayLoop_spi(10);	

	LCD_CS_HIGH;	
	DelayLoop_spi(1);
}
		




void lcd_power_ctrl(void)
{
	printk( "throughc : %s  : %d\n", __FUNCTION__,SPI_DELAY);	
	
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_LOW);
	udelay(200);
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_HIGH);
	mdelay(500);
	
	spi_write( 0x00F0 );
	spi_write(0x015A);
	
	spi_write( 0x00F1 );
	spi_write(0x015A);

	spi_write( 0x00B0 );
	spi_write(0x0123);
	spi_write(0x0123);
	spi_write(0x0123);
	spi_write(0x0123);
	spi_write(0x012A);
	spi_write(0x012A);
	spi_write(0x012A);
	spi_write(0x012A);
	spi_write(0x0126);
	spi_write(0x0126);
	spi_write(0x0126);
	spi_write(0x0126);
	spi_write(0x01B2);
	spi_write(0x01BF);
	spi_write(0x0155);
	spi_write(0x0107);
	spi_write(0x010F);
	spi_write(0x010C);
	spi_write(0x01CC);
	spi_write(0x01CC);
	

	spi_write( 0x00B1 ); 	
	spi_write(0x0142); 	
	spi_write(0x0132); 	
	spi_write(0x0145); 	
	spi_write(0x0132); 	
	spi_write(0x010E); 	
	spi_write(0x0102); 	
	spi_write(0x01CD); 	
	spi_write(0x010F); 	
	spi_write(0x0100); 	
	spi_write(0x0102); 	
	spi_write(0x010B); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	
	spi_write(0x0102); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	
	spi_write(0x0102); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	

	spi_write( 0x00CC ); 	
	spi_write(0x0100); 	
	
	spi_write( 0x00D0 ); 	
	spi_write(0x0102); 	

	spi_write( 0x00B2 ); 	
	spi_write(0x0191); 	
	spi_write(0x0100); 	
	spi_write(0x012A); 	
	spi_write(0x0126); 	
	spi_write(0x0110); 	
	spi_write(0x0108);

	spi_write( 0x003A ); 	
	spi_write(0x0106); 	
	
	spi_write( 0x0026 ); 	
	spi_write(0x0101); 	
	
	spi_write( 0x00C0 ); 	
	spi_write(0x0103); 	
	spi_write(0x0103); 	
	spi_write(0x0103); 	
	spi_write(0x011C); 	
	spi_write(0x0127); 	
	spi_write(0x012D); 	
	spi_write(0x012C); 	
	spi_write(0x0131); 	
	spi_write(0x0137); 	
	spi_write(0x013A); 	
	spi_write(0x0141); 	
	spi_write(0x014B); 	
	spi_write(0x0136);


	spi_write( 0x00C1 ); 	
	spi_write(0x0103); 	
	spi_write(0x0103); 	
	spi_write(0x0103); 	
	spi_write(0x011C); 	
	spi_write(0x0127); 	
	spi_write(0x012D); 	
	spi_write(0x012C); 	
	spi_write(0x0131); 	
	spi_write(0x0137); 	
	spi_write(0x013A); 	
	spi_write(0x0141); 	
	spi_write(0x014B); 	
	spi_write(0x0136); 	

	spi_write( 0x008A ); 	
	spi_write(0x0101); 	

	spi_write( 0x00E9 ); 	
	spi_write(0x0103); 	
	spi_write(0x0180); 	
	spi_write(0x0180); 	
	spi_write(0x0149); 	
	
	
	spi_write( 0x0059 ); 	
	spi_write(0x0126); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	
	spi_write(0x0101); 	
	spi_write(0x018F); 	

	
	spi_write( 0x00EA ); 	
	spi_write(0x0102); 	
	spi_write(0x0100); 	
	spi_write(0x0142); 	






	spi_write( 0x005B ); 	
	spi_write(0x01CC); 	
	
	spi_write( 0x0070 ); 	
	spi_write(0x0100); 	
	
	spi_write( 0x00CE ); 	
	spi_write(0x01FF);

	spi_write( 0x0036 ); 	
	spi_write(0x0148); 	
	
	spi_write( 0x00B8 ); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	
	spi_write(0x0120);

	spi_write( 0x002A ); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	
	spi_write(0x0100); 	
	spi_write(0x01EF); 	
	
	spi_write( 0x002B ); 	
	spi_write(0x0100); 	
	spi_write(0x0101); 	
	spi_write(0x018F); 	
	

	spi_write( 0x002C ); 	
	
	spi_write( 0x0011 );
	mdelay(250);

	spi_write( 0x00CD ); 	
	spi_write(0x0103); 	

	spi_write( 0x0029 );
}

void s3cfb_init_hw(void)
{
	
	s3cfb_set_fimd_info();

	s3cfb_set_gpio();

	lcd_gpio_init();
	mdelay(100); 
	lcd_power_ctrl();
}
