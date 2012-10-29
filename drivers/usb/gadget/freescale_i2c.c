#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <asm/irq.h>

static int usbic_i2c_suspend(struct i2c_client *client, pm_message_t mesg);
static int usbic_i2c_resume(struct i2c_client *client); 
static int usbic_i2c_probe (struct i2c_client *client, const struct i2c_device_id *id);
static int usbic_i2c_remove(struct i2c_client *clinet);
static int __usbic_i2c_read_byte(u8 reg, u8 *val);
static int __usbic_i2c_write_byte(u8 reg, u8 val);


static const struct i2c_device_id usbic_drv_id_table[] = 
{
    {"USBIC",0},
    {}, 
};

MODULE_DEVICE_TABLE(i2c,usbic_drv_id_table);

#define I2C_M_WR		0x00


static struct i2c_driver usbic_i2c_driver = 
{
    .driver = {
        .name       = "microusbic",
        .owner      = THIS_MODULE,
    },
    .probe      = usbic_i2c_probe,
    .remove     = usbic_i2c_remove, 
    .suspend    = usbic_i2c_suspend,
    .resume     = usbic_i2c_resume,
    .id_table   = usbic_drv_id_table,
};

static struct i2c_client *usbic_client; 

int usbic_i2c_read_byte(u8 reg, u8 *val)
{
	
    int retry_count = 3;
    while(retry_count){
        if(__usbic_i2c_read_byte(reg,val) == 0){
            break;
        }
        retry_count--;
    }

	printk("[USBIC i2c read: reg: %d, val: %p]\n",reg,val);

    if (retry_count == 0) {
        printk("Critical..  micro usb ic read failed ..\n");
        return -1; 
    }
    return 0;
}


int usbic_i2c_write_byte(u8 reg, u8 val)
{
    int retry_count = 3;
    while(retry_count){
        if(__usbic_i2c_write_byte(reg,val) == 0){
            break;
        }
        retry_count--;
    }
	printk("[USBIC i2c write: reg: %d, val: %d]\n",reg,val);
    if (retry_count == 0) {
        printk("Critical..  micro usb ic write failed ..\n");
        return -1; 
    }
    return 0;
}


static int __usbic_i2c_read_byte(u8 reg, u8 *val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[1];

	if( (usbic_client == NULL) || (!usbic_client->adapter) )
		return -ENODEV;

	msg->addr = usbic_client->addr;
	msg->flags = usbic_client->flags & I2C_M_TEN;
	msg->flags |= I2C_M_WR;

	msg->len = 1;
	msg->buf = data;
	*data = reg;
	err = i2c_transfer(usbic_client->adapter, msg, 1);

	if (err >= 0) {
		msg->flags = usbic_client->flags & I2C_M_TEN;
		msg->flags |= I2C_M_RD;
		err = i2c_transfer(usbic_client->adapter, msg, 1);
	}

	if (err >= 0) {
		*val = *data;
		return 0;
	}

	return err;
}

static int __usbic_i2c_write_byte(u8 reg, u8 val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[2];

	if( (usbic_client == NULL) || (!usbic_client->adapter) )
		return -ENODEV;

	msg->addr = usbic_client->addr;
	msg->flags = I2C_M_WR;

	msg->len = 2;
	msg->buf = data;
	data[0] = reg;
	data[1] = val;
	
	err = i2c_transfer(usbic_client->adapter, msg, 1);
	if (err >= 0) return 0;

	return err;
}


static int usbic_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
    return 0; 
}

static int usbic_i2c_resume(struct i2c_client *client)
{
    return 0; 
}


static int usbic_i2c_probe (struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    u8 temp; 
    printk("fsa9480_probe called .. \n"); 
    if(strcmp(client->name, "USBIC") == 0) 
    {
        usbic_client = client; 
        return 0;
    }
    printk("usbic_i2c_probe failed .. \n"); 
    return -1; 
}
static int usbic_i2c_remove(struct i2c_client *client)
{
    return 0; 
}



int usbic_i2c_init(void)
{
    int ret;

    if ((ret =i2c_add_driver(&usbic_i2c_driver))){
        printk("fairchild microusb ic i2c : Driver registration failed, module not inserted.\n");
        return ret;
    }

    return 0;
}

void usbic_i2c_exit(void)
{
    i2c_del_driver(&usbic_i2c_driver);
}


