





















#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>

#include <mach/gpio.h>
#include <plat/gpio-bank-o.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/volans.h>

#include <linux/delay.h>
#include "dprintk.h"
#include "message.h"

#define LCD_BL_DEFAULT_LEVEL	6	
#define BL_HIGH_MODE		1
#define BL_LOW_MODE		0

static int g_backlight_level = LCD_BL_DEFAULT_LEVEL;
static int g_backlight_control_value = 1; 
static int g_backlight_mode = BL_HIGH_MODE;

void  aat1231_1_lcd_backlight_gpio_init()
{
	s3c_gpio_cfgpin(GPIO_LED_DRV_EN, S3C_GPIO_SFN(GPIO_LED_DRV_EN_AF));
	gpio_set_value(GPIO_LED_DRV_EN, GPIO_LEVEL_LOW);

	s3c_gpio_cfgpin(GPIO_LED_DRV_SEL, S3C_GPIO_SFN(GPIO_LED_DRV_SEL_AF));
	gpio_set_value(GPIO_LED_DRV_SEL, GPIO_LEVEL_LOW);

	s3c_gpio_cfgpin(GPIO_LCD_ID, S3C_GPIO_SFN(GPIO_LCD_ID_AF));
	udelay(10);
}

static int aat1231_1_lcd_backlight_set(int value)
{
	int i;
	if((value < 0) || (value > 16 ))
	{
		dprintk(LCD_DBG, "[BL][ERR] Invalid backlight control value.(valid : 0~16)\n");
		return 0;
	}

	if(value == 0) 
	{
		gpio_set_value(GPIO_LED_DRV_SEL, GPIO_LEVEL_LOW);
		udelay(10);
		gpio_set_value(GPIO_LED_DRV_EN, GPIO_LEVEL_LOW);
	}
	else 
	{
		if (g_backlight_mode == 0)
			gpio_set_value(GPIO_LED_DRV_SEL, GPIO_LEVEL_LOW);
		else 
			gpio_set_value(GPIO_LED_DRV_SEL, GPIO_LEVEL_HIGH);
			
		udelay(10);

		for(i=0; i<value; i++)
		{
			gpio_set_value(GPIO_LED_DRV_EN, GPIO_LEVEL_LOW);
			udelay(10);
			gpio_set_value(GPIO_LED_DRV_EN, GPIO_LEVEL_HIGH);
			udelay(10);
		}
	}	

	msleep(5);
}

int aat1231_1_lcd_backlight_set_level(int level)
{
	int value = 0;
	
	if((level != 0) && (level != 1))
		g_backlight_level = level;

	value = gpio_get_value(GPIO_LCD_ID);

	dprintk(LCD_DBG, "[BL] aat1231_1_lcd_backlight_set_level(level : %d) %s led !!\n", level, value?"new":"old");

	if(value==0)
	{
		switch(level)
		{
			case 0:	
				value = 0;
				break;
			case 1: 
				g_backlight_mode = BL_LOW_MODE;
				value = 16;
				break;
			case 2:
				g_backlight_mode = BL_LOW_MODE;
				value = 14;
				break;
			case 3:
				g_backlight_mode = BL_LOW_MODE;
				value = 13;
				break;
			case 4:
				g_backlight_mode = BL_LOW_MODE;
				value = 12;
				break;
			case 5:
				g_backlight_mode = BL_LOW_MODE;
				value = 11;
				break;
			case 6:
				g_backlight_mode = BL_LOW_MODE;
				value = 10;
				break;
			case 7:
				g_backlight_mode = BL_LOW_MODE;
				value = 8;
				break;
			case 8:
				g_backlight_mode = BL_HIGH_MODE;
				value = 16;
				break;
			case 9:
				g_backlight_mode = BL_HIGH_MODE;
				value = 14;
				break;
			case 10:
				g_backlight_mode = BL_HIGH_MODE;
				value = 11;
				break;
			case 11:
				g_backlight_mode = BL_HIGH_MODE;
				value = 8;
				break;
			default:
				printk("[BL][ERR] Invalid backlight level. (valid : 0~11)\n");
				g_backlight_mode = BL_LOW_MODE;
				value = 8;
				break;
		}
	}else 
	{
		switch(level)
		{
			case 0:	
				value = 0;
				break;
			case 1: 
				g_backlight_mode = BL_LOW_MODE;
				value = 16;
				break;
			case 2:
				g_backlight_mode = BL_LOW_MODE;
				value = 10;
				break;
			case 3:
				g_backlight_mode = BL_LOW_MODE;
				value = 8;
				break;
			case 4:
				g_backlight_mode = BL_HIGH_MODE;
				value = 16;
				break;
			case 5:
				g_backlight_mode = BL_HIGH_MODE;
				value = 14;
				break;
			case 6:
				g_backlight_mode = BL_HIGH_MODE;
				value = 12;
				break;
			case 7:
				g_backlight_mode = BL_HIGH_MODE;
				value = 10;
				break;
			case 8:
				g_backlight_mode = BL_HIGH_MODE;
				value = 8;
				break;
			case 9:
				g_backlight_mode = BL_HIGH_MODE;
				value = 6;
				break;
			case 10:
				g_backlight_mode = BL_HIGH_MODE;
				value = 4;
				break;
			case 11:
				g_backlight_mode = BL_HIGH_MODE;
				value = 1;
				break;
			default:
				printk("[BL][ERR] Invalid backlight level. (valid : 0~11)\n");
				g_backlight_mode = BL_HIGH_MODE;
				value = 16;
				break;
		}

	}
	aat1231_1_lcd_backlight_set(value);
	return value;
}

void aat1231_1_lcd_backlight_power_on_off(int onoff)
{
	dprintk(LCD_DBG, "[BL] aat1231_1_lcd_backlight_power_on_off(%d) !!\n", onoff);
	
	if(onoff) 
	{
		aat1231_1_lcd_backlight_set_level(g_backlight_level);
	}
	else { 
		aat1231_1_lcd_backlight_set_level(0);
	}
}

int aat1231_1_lcd_backlight_get_level(void)
{
	return g_backlight_level;
}

static ssize_t sysfs_set_lcd_backlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int backlight_level = 0;
	sscanf(buf, "%d", &backlight_level);

	if(backlight_level == 0)
	{
		aat1231_1_lcd_backlight_power_on_off(0);
		dprintk(LCD_DBG, "[BL] sysfs_set_lcd_backlight(off) !!\n");
	}
	else if( 0 < backlight_level && backlight_level < 12)
	{
		aat1231_1_lcd_backlight_set_level(backlight_level);
		dprintk(LCD_DBG, "[BL] sysfs_set_lcd_backlight(%d) !!\n", backlight_level);
	}
	else
	{
		dprintk(LCD_DBG, "[BL][ERR] sysfs_set_lcd_backlight(%d) !!\n", backlight_level);
	}

	return count;
}

static ssize_t sysfs_get_lcd_backlight(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", g_backlight_level);
}

#define LCD_DEV_MAJOR	234
static struct device *aat1231_1_dev_t;

static DEVICE_ATTR(backlight_level, S_IRUGO | S_IWUSR, sysfs_get_lcd_backlight, sysfs_set_lcd_backlight);

static ssize_t sysfs_set_backlight_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int flag;

	sscanf(buf, "%d", &flag);

	if( (flag >= 0) && (flag < 17) )
	{
		g_backlight_control_value = flag;
		aat1231_1_lcd_backlight_set(g_backlight_control_value);
		printk("lcd backlight level : %d\n", g_backlight_control_value);
	}
	else if (flag == 17)
	{
		g_backlight_mode = BL_LOW_MODE;
		printk("[BL] backlight mode changed to LOW_MODE(%d)\n", g_backlight_mode);
	}
	else if (flag == 18 )
	{
		g_backlight_mode = BL_HIGH_MODE;
		printk("[BL] backlight mode changed to HIGH_MODE(%d)\n", g_backlight_mode);
	}
	else
	{
		printk("[BL][ERR] sysfs_set_backlight_control (%d) !!\n", flag);
		printk("[BL][ERR] 0~16 : backlight level change.\n");
		printk("[BL][ERR] 17 : LOW mode, 18 : HIGH mode\n");
	}
	return count;
}

static ssize_t sysfs_show_backlight_control(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("[BL] current mode : %d, level : %d\n", g_backlight_mode, g_backlight_control_value);
	return 0;
}

static DEVICE_ATTR(backlight_control, S_IRUGO | S_IWUSR, sysfs_show_backlight_control, sysfs_set_backlight_control);

static int sysfs_aat1231_1_create_file(struct device *dev) {
 
        int result = 0;

 
        result = device_create_file(dev, &dev_attr_backlight_level);
        if (result)
                return result;
 
        return 0;
}

 static void sysfs_aat1231_1_remove_file(struct device *dev) {
        device_remove_file(dev, &dev_attr_backlight_level);
}

int aat1231_1_init(struct class *lcd_class)
{
	int err;
	
	printk("[BL] aat1231_1_init()\n");
	
	aat1231_1_dev_t = device_create( lcd_class, NULL, MKDEV(LCD_DEV_MAJOR, 0), NULL, "backlight" );
	

	if( IS_ERR( aat1231_1_dev_t ) )
	{
		printk( "[BL][ERR] aat1231_1 device_create err\n" );
		return -1;
	}
	
	err = sysfs_aat1231_1_create_file(aat1231_1_dev_t);
	if (err) {
		printk("[BL][ERR] aat1231_1_init() - sysfs_create_file error\n");
		return -1;
	}

	err = device_create_file(aat1231_1_dev_t, &dev_attr_backlight_control);

	return 0;
}

int aat1231_1_exit(void)
{
	sysfs_aat1231_1_remove_file(aat1231_1_dev_t);
	return 0;
}


EXPORT_SYMBOL(aat1231_1_lcd_backlight_set_level);
EXPORT_SYMBOL(aat1231_1_lcd_backlight_power_on_off);
EXPORT_SYMBOL(aat1231_1_lcd_backlight_get_level);
EXPORT_SYMBOL(aat1231_1_lcd_backlight_gpio_init);
EXPORT_SYMBOL(aat1231_1_init);
EXPORT_SYMBOL(aat1231_1_exit);



MODULE_AUTHOR("GUN");
MODULE_DESCRIPTION("LCD Backlight Device Driver");
MODULE_LICENSE("GPL");
