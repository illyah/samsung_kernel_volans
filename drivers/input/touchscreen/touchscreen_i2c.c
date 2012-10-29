#include <linux/module.h>
#include <linux/kernel_stat.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include "touchscreen.h"
#include <linux/i2c-gpio.h>
#include <asm/gpio.h>

#include <linux/delay.h>
#include <mach/volans.h>

#include "dprintk.h"

#define I2C_M_WR				0x00

static struct i2c_client *g_client;

static int touch_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	return 0;
}

static int touch_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id touch_i2c_id[] = {
	{ "touch I2C", 0 },
	{ }
};



static struct i2c_driver touch_i2c_driver = {
	.driver = {
		.name = "touch I2C",
		.owner = THIS_MODULE,
	},
	.probe =    touch_i2c_probe,
	.remove =   touch_i2c_remove,
	.id_table = touch_i2c_id,
};

int i2c_tsp_sensor_read(u8 reg, u8 *val, unsigned int len )
{
	int id;
	int 	 err;
	struct 	 i2c_msg msg[1];
	unsigned char data[1];
	if( (g_client == NULL) || (!g_client->adapter) )
	{
		return -ENODEV;
	}


	
	msg->addr 	= g_client->addr;
	msg->flags 	= I2C_M_WR;
	msg->len 	= 1;
	msg->buf 	= data;
	*data       = reg;

	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) 
	{
		msg->flags = I2C_M_RD;
		msg->len   = len;
		msg->buf   = val;
		err = i2c_transfer(g_client->adapter, msg, 1);
	}else
	printk("[TSP][WARNING] i2c_transfer 1 fail : warning = %d \n", err);

	if (err >= 0) 
	{
		return 0;
	}else
	printk("[TSP][WARNING] i2c_transfer 2 fail : warning = %d \n", err);

	id = i2c_adapter_id(g_client->adapter);

	return err;
}

static void cypress_i2c_gpio_setscl_val(void *data, int state)
{
	struct i2c_gpio_platform_data *pdata = data;


	gpio_set_value(pdata->scl_pin, state);

	udelay(5);
	udelay(5);

}


static struct i2c_algo_bit_data ioc_data = {
	.setsda		= 0,
	.setscl		= cypress_i2c_gpio_setscl_val,
	.getsda		= 0,
	.getscl		= 0,
	.udelay		= 0,
	.timeout	= 0
};

int touch_add_i2c_device(const struct touch_setup_data *setup)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	int ret;

	ret = i2c_add_driver(&touch_i2c_driver);
	if (ret != 0) {
		printk("[TSP][ERR] can't add i2c driver\n");
		return ret;
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = setup->i2c_address;
	strlcpy(info.type, "touch", I2C_NAME_SIZE);

	adapter = i2c_get_adapter(setup->i2c_bus);
	if (!adapter) {
		printk("[TSP][ERR] can't get i2c adapter %d\n", setup->i2c_bus);
		goto err_driver;
	}

	


	g_client = i2c_new_device(adapter, &info);

	g_client->adapter->timeout = 0;
	g_client->adapter->retries = 0;
	
	i2c_put_adapter(adapter);
	if (!g_client) {
		printk("[TSP][ERR] can't add i2c device at 0x%x\n",
			(unsigned int)info.addr);
		goto err_driver;
	}

	return 0;

err_driver:
	i2c_del_driver(&touch_i2c_driver);
	return -ENODEV;
}

int i2c_tsp_sensor_write(u8 reg, u8 *val, unsigned int len)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[4];
	int i ;

	if(len > 3)
	{
		printk("[TSP][ERROR] %s() : excess of length(%d) !! \n", __FUNCTION__, len);
		return -1;
	}
	
	if( (g_client == NULL) || (!g_client->adapter) )
	{
		printk("[TSP][ERR] i2c_tsp_sensor_write() g_client is NULL !!!\n");
		return -ENODEV;
	}

	msg->addr = g_client->addr;
	printk("[TSP] g_client->addr = 0x%x\n", g_client->addr);
	msg->flags = I2C_M_WR;
	msg->len = len + 1;
	msg->buf = data;
	data[0] = reg;

	for (i = 0; i < len; i++)
	{
		data[i+1] = *(val+i);
	}

	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) 
	{
		return 0;
	}
	printk("[TSP][ERR] i2c_tsp_sensor_write - i2c_transfer err 0x%x\n", err);
	return err;
}

int i2c_tsp_sensor_write_reg(u8 address, int data)
{
	u8 i2cdata[1];

	i2cdata[0] = data;
	return i2c_tsp_sensor_write(address, i2cdata, 1);
}
