#include <linux/string.h>
#include <linux/i2c.h>

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


extern int bd6086gu_i2c_write(u8 reg, u8 data);
extern int bd6086gu_i2c_init(struct class *lcd_class);
extern void bd6086gu_i2c_exit(void);

static struct i2c_driver bd6086gu_i2c_driver;
static struct i2c_client *bd6086gu_i2c_client = NULL;

static unsigned short ignore[] = {  1, 0x76, 3, 0x76, I2C_CLIENT_END };
static unsigned short normal_addr[] = {0x76, I2C_CLIENT_END };
static unsigned short backlight_probe[] = { I2C_CLIENT_END };
static DECLARE_WAIT_QUEUE_HEAD(bd6086gu_wait);

static struct i2c_client_address_data addr_data = {
	.normal_i2c		= normal_addr,
	.probe			= backlight_probe,
	.ignore			= ignore,
};

static int g_backlight_level = 6;

int bd6086gu_i2c_write(u8 reg, u8 data)
{
	char buf[2] = {reg, data};
	struct i2c_msg msg = { bd6086gu_i2c_client->addr, 0, 2, buf };

	return i2c_transfer(bd6086gu_i2c_client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

void bd6086gu_lcd_on(void)
{
	bd6086gu_i2c_write(0x00, 0x01);
	bd6086gu_i2c_write(0x01, 0x07);
	bd6086gu_i2c_write(0x07, 0xC7);
	bd6086gu_i2c_write(0x0B, 0x01);

	bd6086gu_i2c_write(0x0E, 0x13);
	bd6086gu_i2c_write(0x0F, 0x15);
	bd6086gu_i2c_write(0x10, 0x17);
	bd6086gu_i2c_write(0x11, 0x1A);
	bd6086gu_i2c_write(0x12, 0x1D);
	bd6086gu_i2c_write(0x13, 0x22);
	bd6086gu_i2c_write(0x14, 0x27);
	bd6086gu_i2c_write(0x15, 0x2D);
	bd6086gu_i2c_write(0x16, 0x38);
	bd6086gu_i2c_write(0x17, 0x42);
	bd6086gu_i2c_write(0x18, 0x4D);
	bd6086gu_i2c_write(0x19, 0x56);
	bd6086gu_i2c_write(0x1A, 0x5C);
	bd6086gu_i2c_write(0x1B, 0x60);
	bd6086gu_i2c_write(0x1C, 0x62);
	bd6086gu_i2c_write(0x1D, 0x63);

	bd6086gu_i2c_write(0x02, 0x10);
}




























void bd6086gu_sleep_in(void)
{
	bd6086gu_i2c_write(0x0B, 0x00);
	bd6086gu_i2c_write(0x02, 0x00);
}

void bd6086gu_sleep_out(void)
{
	bd6086gu_i2c_write(0x0B, 0x01);
	bd6086gu_i2c_write(0x07, 0xC7);
	bd6086gu_i2c_write(0x01, 0x07);
	bd6086gu_i2c_write(0x02, 0x10);
}


void bd6086gu_backlight_set(unsigned int backlight_level)
{
	bd6086gu_i2c_write(0x07, 0x00);
	bd6086gu_i2c_write(0x03, backlight_level);
	bd6086gu_i2c_write(0x01, 0x06);

	bd6086gu_i2c_write(0x02, 0x10);
}


static int bd6086gu_detect(struct i2c_adapter* adapter, int address, int kind)
{
	struct i2c_client *new_client;
	int err = 0;
	
	printk( "throughc : %s! nR=%d\n", __FUNCTION__, adapter->nr );




	if (!(new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	strlcpy(new_client->name, "bd6086gu", I2C_NAME_SIZE);
	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &bd6086gu_i2c_driver;

	
	if ((err = i2c_attach_client(new_client))) {
		printk("i2c attach client error!\n");
		kfree(new_client);
	}

	bd6086gu_i2c_client = new_client;

	return 0;

exit:
	printk("I2C Detect Error!!!\n");
	
	return err;
}

static int bd6086gu_i2c_attach(struct i2c_adapter *adap)
{
	if( adap->nr != 0 )
	{
		return 0;
	}
	
	return i2c_probe(adap, &addr_data, bd6086gu_detect);
}



static int bd6086gu_i2c_detach(struct i2c_client *client) 
{
	int rc;

	if ((rc = i2c_detach_client(client)) == 0) {
		kfree(client);
	}

	return 0;
}


static int bd6086gu_i2c_command(struct i2c_client *client, unsigned int cmd, void *arg) 
{
	printk("call : [%s:%d]\n", __FUNCTION__, __LINE__);
	return -EINVAL;
}


static struct i2c_driver bd6086gu_i2c_driver = {
	.attach_adapter	= bd6086gu_i2c_attach,
	.detach_client	= bd6086gu_i2c_detach,
	.command		= bd6086gu_i2c_command,
	.driver			= {
		.owner          = THIS_MODULE,
		.name			= "charge pump",
	},
};

static ssize_t sysfs_set_lcd_backlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int bl_level = 0;

	sscanf(buf, "%d", &g_backlight_level);

	if(g_backlight_level == 0)
	{
		bd6086gu_sleep_in();
		dprintk(DCM_INP, "[LCD] sysfs_set_lcd_backlight(off) !!\n");
	}
	else if(g_backlight_level == 9)
	{
		bd6086gu_sleep_out();
		dprintk(DCM_INP, "[LCD] sysfs_set_lcd_backlight(on) !!\n");
	}
	else if( 0 < g_backlight_level && g_backlight_level < 7)
	{
		switch(g_backlight_level)
		{
			case 1 :
				bl_level = 0x0A;
				break;
			case 2 :
				bl_level = 0x1F;
				break;
			case 3 :
				bl_level = 0x1F;
				break;
			case 4 :
				bl_level = 0x2B;
				break;
			case 5 :
				bl_level = 0x2B;
				break;
			case 6 :
				bl_level = 0x37;
				break;
			case 7 :
				bl_level = 0x37;
				break;
			case 8 :
				bl_level = 0x43;
				break;
			case 9 :
				bl_level = 0x43;
				break;
			case 10 :
				bl_level = 0x4F;
				break;
			case 11 :
				bl_level = 0x4F;
				break;
			default :
				bl_level = 0x4F;
				break;
		}
		bd6086gu_backlight_set(bl_level);
		dprintk(DCM_INP, "[LCD] sysfs_set_lcd_backlight(%d) !!\n", g_backlight_level);
	}
	else
	{
		dprintk(DCM_INP, "[LCD][ERR] sysfs_set_lcd_backlight(%d) !!\n", g_backlight_level);
	}
	return count;
}

static ssize_t sysfs_get_lcd_backlight(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", g_backlight_level);
}

#define LCD_DEV_MAJOR	234
static struct device *bd6086_dev_t;

static DEVICE_ATTR(backlight_level, S_IRUGO | S_IWUSR, sysfs_get_lcd_backlight, sysfs_set_lcd_backlight);

static int sysfs_bd6086_create_file(struct device *dev) {
 
        int result = 0;

        result = device_create_file(dev, &dev_attr_backlight_level);
        if (result)
                return result;
 
        return 0;
}

 static void sysfs_bd6086_remove_file(struct device *dev) {
        device_remove_file(dev, &dev_attr_backlight_level);
}

int bd6086gu_i2c_init(struct class *lcd_class) 
{
	int ret;
	
	bd6086gu_i2c_client = NULL;
	
	ret = i2c_add_driver(&bd6086gu_i2c_driver);
	if (ret)
		printk("i2c_add_driver() failed.\n");

	bd6086_dev_t = device_create( lcd_class, NULL, MKDEV(LCD_DEV_MAJOR, 0), NULL, "backlight" );

	ret = sysfs_bd6086_create_file(bd6086_dev_t);
	if (ret) {
		printk("[LCD][ERR] aat1231_1_init() - sysfs_create_file error\n");
		return -1;
	}

	return ret;
}

void bd6086gu_i2c_exit(void) 
{
	sysfs_bd6086_remove_file(bd6086_dev_t);
	i2c_del_driver(&bd6086gu_i2c_driver);
}
