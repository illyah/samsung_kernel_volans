














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
#include <linux/i2c/pmic.h>

#include "dprintk.h"
#include "s3cfb.h"
#include "lcd_spi_sequence.h"

#include <plat/power-domain.h>



#define LCD_STATUS_CABC         0x200
#define LCD_STATUS_ALC          0x100
#define LCD_BACKLIGHT_VALUE     0xFF


#define GPIO_LEVEL_LOW      0
#define GPIO_LEVEL_HIGH     1

#define S3C_FB_HFP	 10		
#define S3C_FB_HSW	 10		
#define S3C_FB_HBP	 20		

#define S3C_FB_VFP	 8	
#define S3C_FB_VSW	 4	
#define S3C_FB_VBP	 4	

#define S3C_FB_HRES				240 	
#define S3C_FB_VRES				400 	

#define S3C_FB_HRES_VIRTUAL		240 	
#define S3C_FB_VRES_VIRTUAL		800 	

#define S3C_FB_HRES_OSD			240		
#define S3C_FB_VRES_OSD			400 	

#define S3C_FB_VFRAME_FREQ  	70 

#define S3C_FB_PIXEL_CLOCK	(S3C_FB_VFRAME_FREQ * \
								(S3C_FB_HFP + S3C_FB_HSW + S3C_FB_HBP + S3C_FB_HRES) * \
								(S3C_FB_VFP + S3C_FB_VSW + S3C_FB_VBP + S3C_FB_VRES))


extern int aat1231_1_lcd_backlight_set_level(int level);
extern void aat1231_1_lcd_backlight_power_on_off(int onoff);
extern int aat1231_1_lcd_backlight_get_level(void);
extern void aat1231_1_lcd_backlight_gpio_init();
extern int aat1231_1_init(struct class *lcd_class);
extern int aat1231_1_exit(void);

#if !(CONFIG_FB_S3C_V2==1)

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

	
	s3cfb_fimd.vidosd2a 	= S3C_VIDOSDxA_OSD_LTX_F(0) |
							S3C_VIDOSDxA_OSD_LTY_F(0);
	s3cfb_fimd.vidosd2b 	= S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES_OSD - 1) |
							S3C_VIDOSDxB_OSD_RBY_F(S3C_FB_VRES_OSD - 1);

	
	s3cfb_fimd.vidosd3a 	= S3C_VIDOSDxA_OSD_LTX_F(0) |
							S3C_VIDOSDxA_OSD_LTY_F(0);
	s3cfb_fimd.vidosd3b 	= S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES_OSD - 1) |
							S3C_VIDOSDxB_OSD_RBY_F(S3C_FB_VRES_OSD - 1);

	
	s3cfb_fimd.vidosd4a 	= S3C_VIDOSDxA_OSD_LTX_F(0) |
							S3C_VIDOSDxA_OSD_LTY_F(0);
	s3cfb_fimd.vidosd4b 	= S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES_OSD - 1) |
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

#endif 
static void lcd_gpio_init(void)
{

	msleep(50);

	printk( "throughc : %s\n", __FUNCTION__ );
	s3c_gpio_cfgpin(GPIO_LCD_D0, S3C_GPIO_SFN(GPIO_LCD_D0_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D1, S3C_GPIO_SFN(GPIO_LCD_D1_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D2, S3C_GPIO_SFN(GPIO_LCD_D2_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D3, S3C_GPIO_SFN(GPIO_LCD_D3_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D4, S3C_GPIO_SFN(GPIO_LCD_D4_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D5, S3C_GPIO_SFN(GPIO_LCD_D5_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D6, S3C_GPIO_SFN(GPIO_LCD_D6_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D7, S3C_GPIO_SFN(GPIO_LCD_D7_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D8, S3C_GPIO_SFN(GPIO_LCD_D8_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D9, S3C_GPIO_SFN(GPIO_LCD_D9_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D10, S3C_GPIO_SFN(GPIO_LCD_D10_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D11, S3C_GPIO_SFN(GPIO_LCD_D11_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D12, S3C_GPIO_SFN(GPIO_LCD_D12_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D13, S3C_GPIO_SFN(GPIO_LCD_D13_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D14, S3C_GPIO_SFN(GPIO_LCD_D14_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D15, S3C_GPIO_SFN(GPIO_LCD_D15_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D16, S3C_GPIO_SFN(GPIO_LCD_D16_AF));
	s3c_gpio_cfgpin(GPIO_LCD_D17, S3C_GPIO_SFN(GPIO_LCD_D17_AF));
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

	msleep(1);

	
	gpio_set_value(GPIO_LCD_EN, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_EN,S3C_GPIO_SFN(GPIO_LCD_EN_AF));
	s3c_gpio_setpull(GPIO_LCD_EN, S3C_GPIO_PULL_NONE);

	msleep(50);

	
	s3c_gpio_cfgpin(GPIO_LCD_CLK, S3C_GPIO_SFN(GPIO_LCD_CLK_AF));
	s3c_gpio_setpull(GPIO_LCD_CLK, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_LCD_CLK, GPIO_LEVEL_LOW);

	
	gpio_set_value(GPIO_LCD_nCS, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_nCS, S3C_GPIO_SFN(GPIO_LCD_nCS_AF));
	s3c_gpio_setpull(GPIO_LCD_nCS, S3C_GPIO_PULL_NONE);

	
	gpio_set_value(GPIO_LCD_SI, GPIO_LEVEL_HIGH);
	s3c_gpio_cfgpin(GPIO_LCD_SI, S3C_GPIO_SFN(GPIO_LCD_SI_AF));
	s3c_gpio_setpull(GPIO_LCD_SI, S3C_GPIO_PULL_NONE);
	
	
	aat1231_1_lcd_backlight_gpio_init();

}







#define LCD_CS_HIGH      gpio_set_value(GPIO_LCD_nCS, GPIO_LEVEL_HIGH);
#define LCD_CS_LOW       gpio_set_value(GPIO_LCD_nCS, GPIO_LEVEL_LOW);

#define LCD_CLK_HIGH     gpio_set_value(GPIO_LCD_CLK, GPIO_LEVEL_HIGH);
#define LCD_CLK_LOW      gpio_set_value(GPIO_LCD_CLK, GPIO_LEVEL_LOW);

#define LCD_SI_HIGH      gpio_set_value(GPIO_LCD_SI, GPIO_LEVEL_HIGH);
#define LCD_SI_LOW       gpio_set_value(GPIO_LCD_SI, GPIO_LEVEL_LOW);

#define SPI_DELAY        5 


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


extern void s3cfb_pre_suspend(void);
extern void s3cfb_post_resume(void);

/*extern int s3c_rp_control_suspend();
extern int s3c_rp_control_resume();
*/

void lcd_power_on(void)
{
	int i, ret;

	dprintk(LCD_DBG, "[LCD]%s() \n", __FUNCTION__);

	
	//s3c_rp_control_resume();

	
	
	s3cfb_post_resume();
	
	pmic_power_control(POWER_LCD, PMIC_POWER_ON);
	gpio_set_value(GPIO_LCD_EN, GPIO_LEVEL_HIGH);

	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_LOW);

	msleep(1);
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_HIGH);
	msleep(2);




	for( i=0; ; i++ )
	{
		if( display_on_seq[i] == CMD_END )
		{
			break;
		}
		else if( display_on_seq[i] == TIME_DELAY )
		{

			msleep( display_on_seq[++i] );
			
		}
		else
		{
			
			spi_write( display_on_seq[i] );
		}
	}


}
	
void lcd_power_off(void)
{
	int i, ret;

	dprintk(LCD_DBG, "[LCD] %s() \n", __FUNCTION__);


	for( i=0; ; i++ )
	{
		if( display_off_seq[i] == CMD_END )
		{
			break;
		}
		else if( display_off_seq[i] == TIME_DELAY )
		{

			msleep( display_off_seq[++i] );
			
		}
		else
		{
			
			spi_write( display_off_seq[i] );
		}
	}
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_LOW);

	
	gpio_set_value(GPIO_LCD_EN, GPIO_LEVEL_LOW);
	pmic_power_control(POWER_LCD, PMIC_POWER_OFF);
	

	msleep(1);

	
	//s3c_rp_control_suspend();
	
	s3cfb_pre_suspend();	
}

void lcd_sleep_in(void)
{
	int i;

	dprintk(LCD_DBG, "[LCD][%s] \n", __FUNCTION__);
	
	for( i=0; ; i++ )
	{
		if( sleep_in_seq[i] == CMD_END )
		{
			break;
		}
		else if( sleep_in_seq[i] == TIME_DELAY )
		{

			msleep( sleep_in_seq[++i] );
			
		}
		else
		{
			
			spi_write( sleep_in_seq[i] );
		}
	}
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_LOW);
	msleep(1);

}

void lcd_sleep_out(void) 
{
	int i;

	dprintk(LCD_DBG, "[LCD][%s] \n", __FUNCTION__);
	
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_LOW);

	msleep(1);
	gpio_set_value(GPIO_LCD_nRST, GPIO_LEVEL_HIGH);

	msleep(10);

	for( i=0; ; i++ )
	{
		if( sleep_out_seq[i] == CMD_END )
		{
			break;
		}
		else if( sleep_out_seq[i] == TIME_DELAY )
		{

			msleep( sleep_out_seq[++i] );
			
		}
		else
		{
			
			spi_write( sleep_out_seq[i] );
		}
	}

}
	



void s3cfb_init_hw(void)
{
	
#if !(CONFIG_FB_S3C_V2==1)
	s3cfb_set_fimd_info();

	s3cfb_set_gpio();
#endif
}

static int g_lcd_onoff_state = 1;

static ssize_t sysfs_set_lcd_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	sscanf(buf, "%d", &g_lcd_onoff_state);

	if(g_lcd_onoff_state == 0) {
		lcd_power_off();

		dprintk(LCD_DBG, "[LCD] sysfs_set_lcd_status(off) !!\n");
	}
	else if(g_lcd_onoff_state > 0) {
		lcd_power_on();

		dprintk(LCD_DBG, "[LCD] sysfs_set_lcd_status(on) !!\n");
	}
	else {

		dprintk(LCD_DBG, "[LCD][ERR] sysfs_set_lcd_status(%d) !!\n", g_lcd_onoff_state);
	}

	return count;
}

static ssize_t sysfs_get_lcd_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", g_lcd_onoff_state);
}

#define LCD_DEV_MAJOR	233
static struct class *s6d04d1x21_class;
static struct device *s6d04d1x21_dev_t;

static DEVICE_ATTR(lcd_onoff, S_IRUGO | S_IWUSR, sysfs_get_lcd_status, sysfs_set_lcd_status);

static int sysfs_s6d04d1x21_create_file(struct device *dev) {
 
        int result = 0;

 
        result = device_create_file(dev, &dev_attr_lcd_onoff);
        if (result)
                return result;
 
        return 0;
}

 static void sysfs_s6d04d1x21_remove_file(struct device *dev) {
        device_remove_file(dev, &dev_attr_lcd_onoff);
}

static int __init s6d04d1x21_init(void)
{
	int err;
	
	printk("[LCD] s6d04d1x21_init() !!!\n");
	
	s6d04d1x21_class = class_create( THIS_MODULE, "display_class" );

	if( IS_ERR( s6d04d1x21_class ) )
	{
		printk( "[LCD][ERR] s6d04d1x21 class_create err\n" );
		return -1;
	}

	s6d04d1x21_dev_t = device_create( s6d04d1x21_class, NULL, MKDEV(LCD_DEV_MAJOR, 0), NULL, "lcd" );

	if( IS_ERR( s6d04d1x21_dev_t ) )
	{
		printk( "[LCD][ERR] s6d04d1x21 device_create err\n" );
		return -1;
	}
	
	err = sysfs_s6d04d1x21_create_file(s6d04d1x21_dev_t);
	if (err) {
		printk("[LCD][ERR] s6d04d1x21_init() - sysfs_create_file error\n");
		return -1;
	}

	return aat1231_1_init(s6d04d1x21_class);

}

static int __init s6d04d1x21_exit(void)
{
	sysfs_s6d04d1x21_remove_file(s6d04d1x21_dev_t);

	aat1231_1_exit();

	return 0;
}

#if (CONFIG_FB_S3C_V2 == 1)
#include "s3cfb2.h"


static struct s3cfb_lcd lcd_s6d04d1x21 =  {
        .width = S3C_FB_HRES,
        .height = S3C_FB_VRES,
        .bpp = 32,
        .freq = S3C_FB_VFRAME_FREQ,

        .timing = {
                .h_fp = S3C_FB_HFP,
                .h_bp = S3C_FB_HBP,
                .h_sw = S3C_FB_HSW,
                .v_fp = S3C_FB_VFP,
                .v_fpe = 1,				
                .v_bp = S3C_FB_VBP,
                .v_bpe = 1,				
                .v_sw = S3C_FB_VSW,
        },

        .polarity = {
                .rise_vclk = 1,
                .inv_hsync = 1,
                .inv_vsync = 1,
                .inv_vden = 0,
        },
};



void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
        lcd_s6d04d1x21.init_ldi = NULL;
        ctrl->lcd = &lcd_s6d04d1x21;
}
EXPORT_SYMBOL(s3cfb_set_lcd_info);
#endif	


EXPORT_SYMBOL(lcd_power_on);
EXPORT_SYMBOL(lcd_power_off);
EXPORT_SYMBOL(lcd_sleep_in);
EXPORT_SYMBOL(lcd_sleep_out);

module_init(s6d04d1x21_init);
module_exit(s6d04d1x21_exit);

MODULE_LICENSE("GPL");
