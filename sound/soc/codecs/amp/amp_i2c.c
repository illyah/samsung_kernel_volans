#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include <mach/hardware.h>
#include <mach/gpio.h>
#include <asm/irq.h>

#include "amp_i2c.h"

extern int amp_input_gain;
extern int amp_speaker_volume;
extern int amp_headphone_volume;

int amp_register_current_state(void);

extern int (*volans_amp_spk_mode_enable)(void);
extern int (*volans_amp_headphone_mode_enable)(void);
extern int (*volans_amp_receiver_mode_enable)(void);
extern int (*volans_amp_spk_mode_disable)(void);
extern int (*volans_amp_headphone_mode_disable)(void);
extern int (*volans_amp_receiver_mode_disable)(void);
extern int (*volans_amp_dualmode_enable)(void);
extern int (*volans_amp_get_output_mode)(void);
extern int (*amp_i2c_read)(u8 reg, u8 *val);
extern int (*amp_i2c_write)(u8 reg, u8 val);

static int i2c_amp_attach_adapter(struct i2c_adapter *adapter);
static int i2c_amp_probe_client(struct i2c_adapter *, int,  int);
static int i2c_amp_detach_client(struct i2c_client *client);
static int i2c_amp_suspend(struct device *, pm_message_t); 
static int i2c_amp_resume(struct device *);

#define	AMP_SLAVE_ADDRESS	( 0x9A >> 1 )

struct i2c_driver amp_driver =
{
#if 1
	.driver = {
		.name = "volans-amp",
		.owner	= THIS_MODULE,
		.suspend = &i2c_amp_suspend,
		.resume = &i2c_amp_resume,
	},
#endif
	//.flags	= I2C_DF_NOTIFY | I2C_M_IGNORE_NAK,
	.attach_adapter	= &i2c_amp_attach_adapter,
	.detach_client	= &i2c_amp_detach_client,

};

static struct i2c_client *g_client;

#if 0
static unsigned short ignore[] = { I2C_CLIENT_END };

static unsigned short normal_addr[] = {
	AMP_SLAVE_ADDRESS, 
	I2C_CLIENT_END 
};

static struct i2c_client_address_data addr_data = {
	.normal_i2c		= normal_addr,
	.probe			= ignore,
	.ignore			= ignore,
};
#endif 

static int i2c_amp_suspend(struct device *dev, pm_message_t state)
{	
	return 0;
}

static int i2c_amp_resume(struct device *dev)
{	
	return 0;
}

int amp_get_output_mode(void)
{
	u8 output_mode_control=0;
	
	amp_read_byte(AMP_OUTPUTMODECONTROL_ADDRESS	, &output_mode_control);

	return output_mode_control;
}

int amp_spk_mode_enable(void)
{
	u8 value=0;

	int i;
//	printk("amp_spk_mode_enable is called\n");
	value = amp_input_gain;
	amp_write_byte(AMP_INPUTMODECONTROL_ADDRESS, value);

//	value = 0xA1;
	value = 0x81;
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );

	msleep(1);
	
	value = amp_speaker_volume;
	
	for( i=0; i <= value; i++){
		amp_write_byte(AMP_SPEAKERVOLUMECONTROL_ADDRESS, i);
	}

	amp_register_current_state();

	return 0;
}

int amp_spk_mode_disable(void)
{
	u8 value=0;

	amp_write_byte(AMP_SPEAKERVOLUMECONTROL_ADDRESS, 1);
	
	msleep(1);
	
	amp_read_byte(AMP_OUTPUTMODECONTROL_ADDRESS, &value);
	value &= ~(0x1 << 7);
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );
	
	amp_register_current_state();
	return 0;
}

int amp_headphone_mode_enable(void)
{
	u8 value=0;

	int i;

	value = amp_input_gain;
	amp_write_byte(AMP_INPUTMODECONTROL_ADDRESS, value);
	
	value = 0xA5;
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );

	msleep(1);

	value = amp_headphone_volume;

	for(i=0; i <= value; i++){
		amp_write_byte(AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, i);
		amp_write_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, i);
	}
	
	amp_register_current_state();

	return 0;
}

int amp_dualmode_enable(void)
{
	u8 value=0;

	int i;

	value = 0x6A;
	amp_write_byte(AMP_INPUTMODECONTROL_ADDRESS, value);

	value = 0xA9;
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );

	msleep(1);
	
	value = amp_speaker_volume;
	
	for( i=0; i <= value; i++){
		amp_write_byte(AMP_SPEAKERVOLUMECONTROL_ADDRESS, i);
	}

	value = amp_headphone_volume;

	for(i=0; i <= value; i++){
		amp_write_byte(AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, i);
		amp_write_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, i);
	}
	
	amp_register_current_state();

	return 0;

}

int amp_headphone_mode_disable(void)
{
	u8 value=0;

	amp_write_byte(AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, 1);
	amp_write_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, 1);

	msleep(1);

	amp_read_byte(AMP_OUTPUTMODECONTROL_ADDRESS, &value);
	value &= ~(0x1 << 7);
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );
	
	amp_register_current_state();
	return 0;
}

int amp_receiver_mode_enable(void)
{
	u8 value=0;
	
	value = amp_input_gain;
	amp_write_byte(AMP_INPUTMODECONTROL_ADDRESS, value);

	value = 0xE9;
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );

	amp_register_current_state();
	return 0;
}

int amp_receiver_mode_disable(void)
{
	u8 value=0;
	
	amp_read_byte(AMP_OUTPUTMODECONTROL_ADDRESS, &value);
	value &= ~(0x1 << 7);
	amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, value );
	
	amp_register_current_state();
	return 0;
}

int amp_register_current_state(void)
{
	u8 input_mode_control=0;
	u8 speaker_volume_control=0;
	u8 left_headphone_volume_control=0;
	u8 right_headphone_volume_control=0;
	u8 output_mode_control=0;

	amp_read_byte(AMP_INPUTMODECONTROL_ADDRESS, &input_mode_control);
	amp_read_byte(AMP_SPEAKERVOLUMECONTROL_ADDRESS,	&speaker_volume_control);
	amp_read_byte( AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, &left_headphone_volume_control);
	amp_read_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, &right_headphone_volume_control);
	amp_read_byte(AMP_OUTPUTMODECONTROL_ADDRESS	, &output_mode_control);

	printk("#####################################\n");
	printk("#      SOUND AMP CURRENT STATE      #\n");
	printk("#####################################\n");
	printk("input mode control : 0x%02x\n", input_mode_control);
	printk("speaker volume control : 0x%02x\n", speaker_volume_control);
	printk("left headphone volume control : 0x%02x\n", left_headphone_volume_control);
	printk("right headphone volume control : 0x%02x\n", right_headphone_volume_control);
	printk("output mode control : 0x%02x\n", output_mode_control);
	printk("#####################################\n");

	return 0;
}

int amp_read_byte(u8 reg, u8 *val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[1];

	if( (g_client == NULL) || (!g_client->adapter) )
		return -ENODEV;

	msg->addr = g_client->addr;
	msg->flags = g_client->flags & I2C_M_TEN;
	msg->flags |= I2C_M_WR;
	//msg->flags = 0;
	msg->len = 1;
	msg->buf = data;
	*data = reg;
	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) {
		//mdelay(10);
		msg->flags = g_client->flags & I2C_M_TEN;
		msg->flags |= I2C_M_RD;
		err = i2c_transfer(g_client->adapter, msg, 1);
	}

	if (err >= 0) {
		*val = *data;
		return 0;
	}

	return err;
}

int amp_write_byte(u8 reg, u8 val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[2];

	if( (g_client == NULL) || (!g_client->adapter) )
		return -ENODEV;

	msg->addr = g_client->addr;
	msg->flags = g_client->flags & I2C_M_TEN;
	msg->flags |= I2C_M_WR;
	//msg->flags = 0;
	msg->len = 2;
	msg->buf = data;
	data[0] = reg;
	data[1] = val;
	
	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) return 0;

	return err;
}

#if 0
int amp_read_word(u8 reg, u16 *val)
{
	int ret = 0;
	struct i2c_msg msg[1];
	unsigned char data[2];

	if (!g_client->adapter)
		return -ENODEV;

	msg->addr = g_client->addr;
//	msg->flags = I2C_M_WR;
	msg->len = 1;
	msg->buf = data;
	*data = reg;
	ret= i2c_transfer(g_client->adapter, msg, 1);
	if (ret>= 0) {
		mdelay(3);
		msg->len = 2;
		msg->flags = I2C_M_RD;
		ret = i2c_transfer(g_client->adapter, msg, 1);
	}
	if (ret >= 0) {
		/* low byte comes first so need to swap */
		*val = data[0] + (data[1] << 8);
		return ret;
	}

	return ret;
}

int amp_write_word(u8 reg, u16 val)
{
	int ret;
	struct i2c_msg msg[1];
	unsigned char data[3];
	int retry = 0;

	if (!g_client->adapter) {
		return -ENODEV;
	}
												
again:
	msg->addr = g_client->addr;
//	msg->flags = I2C_M_WR;
	msg->len = 3;
	msg->buf = data;
	data[0] = reg;
	/* low byte goes out first */
	data[1] = (u8) (val & 0xff);
	data[2] = (u8) (val >> 8);
	ret = i2c_transfer(g_client->adapter, msg, 1);
	if (ret >= 0) {
		return 0;
	}

	if (retry <= 5) {
		retry++;
		mdelay(20);
		goto again;
	}
	return ret;
}
#endif


static int i2c_amp_attach_adapter(struct i2c_adapter *adapter)
{
#if 1
	int addr = AMP_SLAVE_ADDRESS;
	int id=0;

	id = adapter->nr;
	
	if(id == 3) { 
		printk("amp module I2C id: %d\n", id);
		i2c_amp_probe_client(adapter, addr, 0); 
	}

	return 0;
#else
	return i2c_probe(adapter, &addr_data, &i2c_amp_probe_client);
#endif
}

static int i2c_amp_probe_client(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *new_client;
	int err = 0;

   	if ( !i2c_check_functionality(adapter,I2C_FUNC_SMBUS_BYTE_DATA) ) {
		printk(KERN_INFO "byte op is not permited.\n");
		goto ERROR0;
	}

	new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL );

	if ( !new_client )  {
		err = -ENOMEM;
		goto ERROR0;
	}

	new_client->addr = address;	
 	new_client->adapter = adapter;
	new_client->driver = &amp_driver;
	new_client->flags = I2C_DF_NOTIFY | I2C_M_IGNORE_NAK;


	g_client = new_client;

	strlcpy(new_client->name, "amp", I2C_NAME_SIZE);

/*	pr_info("amp: TRY attach Slave %s on Adapter %s [%x]\n",
				new_client->name, adapter->name, err);
*/
	if ((err = i2c_attach_client(new_client)))
		goto ERROR1;

//	pr_info("amp: after i2c_attach_client  \n");
	
	msleep(1);

	//amp_dualmode_enable();
//	amp_register_current_state();

	volans_amp_spk_mode_enable = amp_spk_mode_enable;
	volans_amp_headphone_mode_enable = amp_headphone_mode_enable;
	volans_amp_receiver_mode_enable = amp_receiver_mode_enable;
	volans_amp_spk_mode_disable = amp_spk_mode_disable;
	volans_amp_headphone_mode_disable = amp_headphone_mode_disable;
	volans_amp_receiver_mode_disable = amp_receiver_mode_disable;
	volans_amp_dualmode_enable = amp_dualmode_enable;
	volans_amp_get_output_mode = amp_get_output_mode;
	amp_i2c_read = amp_read_byte;
	amp_i2c_write = amp_write_byte;
	return 0;

	ERROR1:
		printk("ERROR i2c_attach_client \n");
		kfree(new_client);
		return err;

	ERROR0:
    	return err;
}


static int i2c_amp_detach_client(struct i2c_client *client)
{
	int err;

  	/* Try to detach the client from i2c space */
	if ((err = i2c_detach_client(client))) {
        return err;
	}

	kfree(client); /* Frees client data too, if allocated at the same time */
	g_client = NULL;
	return 0;
}


int i2c_amp_init(void)
{
	int ret;

	if ( (ret =i2c_add_driver(&amp_driver) ) ) {
		printk("sound amp: Driver registration failed, module not inserted.\n");
		return ret;
	}

	return 0;
}

void i2c_amp_exit(void)
{
	i2c_del_driver(&amp_driver);
}

//EXPORT_SYMBOL(amp_read_byte);
//EXPORT_SYMBOL(amp_write_byte);
//EXPORT_SYMBOL(amp_get_output_mode);
//EXPORT_SYMBOL(amp_spk_mode_enable);
//EXPORT_SYMBOL(amp_spk_mode_disable);
//EXPORT_SYMBOL(amp_headphone_mode_enable);
//EXPORT_SYMBOL(amp_dualmode_enable);
//EXPORT_SYMBOL(amp_headphone_mode_disable);
//EXPORT_SYMBOL(amp_receiver_mode_enable);
//EXPORT_SYMBOL(amp_receiver_mode_disable);

