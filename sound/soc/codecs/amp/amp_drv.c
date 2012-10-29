#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "amp_drv.h"

//#if (CONFIG_NOWPLUS_REV < CONFIG_NOWPLUS_REV02_N01)
//int amp_input_gain = 0x60;
//#else
int amp_input_gain = 0x40;
//#endif
int amp_speaker_volume = 0x1F;
int amp_headphone_volume = 0x1F;

int amp_ioctl(struct inode *node, struct file *f, unsigned int amp_ioctl_num, unsigned long ioctl_param)
{
	int ret = 0;
	int write_value=0;

	switch(amp_ioctl_num)
	{
		case AMP_INPUT_MODE_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			amp_write_byte(AMP_INPUTMODECONTROL_ADDRESS, write_value);
			amp_read_byte(AMP_INPUTMODECONTROL_ADDRESS, (u8*)&write_value);
			printk("input mode control value change to 0x%02x\n", write_value);
			break;
		case AMP_SPEAKER_VOLUME_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			amp_write_byte(AMP_SPEAKERVOLUMECONTROL_ADDRESS, write_value);
			amp_speaker_volume = write_value;
			printk("speaker volume change to %d\n", write_value);
			break;
		case AMP_LEFT_HEADPHONE_VOLUME_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			amp_write_byte(AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, write_value);
			amp_read_byte(AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, (u8*)&write_value);
			printk("input mode control value change to 0x%02x\n", write_value);
			break;
		case AMP_RIGHT_HEADPHONE_VOLUME_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			amp_write_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, write_value);
			amp_read_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, (u8*)&write_value);
			printk("input mode control value change to 0x%02x\n", write_value);
			break;
		case AMP_OUTPUT_MODE_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			amp_write_byte(AMP_OUTPUTMODECONTROL_ADDRESS, write_value);
			amp_read_byte(AMP_OUTPUTMODECONTROL_ADDRESS, (u8*)&write_value);
			printk("input mode control value change to 0x%02x\n", write_value);
			break;
		case AMP_INPUT_GAIN_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			if(write_value == 0) write_value = 0x60;
			if(write_value == 1) write_value = 0x65;
			if(write_value == 2) write_value = 0x6A;
			amp_write_byte(AMP_INPUTMODECONTROL_ADDRESS, write_value);
			amp_input_gain = write_value;
			printk("input mode control value change to 0x%02x\n", write_value);
			break;
		case AMP_HEADPHONE_VOLUME_CONTROL:
			ret = copy_from_user(&write_value, (int *)ioctl_param, sizeof(int));
			amp_write_byte(AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS, write_value);
			amp_write_byte(AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS, write_value);
			amp_headphone_volume = write_value;
			printk("headphone volume change to %d\n", write_value);
			break;
	}

	return 0;

}

struct file_operations amp_fops =
{
	.owner = THIS_MODULE,
	.ioctl = amp_ioctl,
};

static struct miscdevice amp_device = {
	.minor = 141,
	.name = "amp",
	.fops = &amp_fops,
};

int amp_init(void)
{
	int result;

	result = misc_register(&amp_device);
	if (result < 0) return result;

	i2c_amp_init();
	
	return 0;
}

void amp_exit(void)
{
	misc_deregister(&amp_device);
	i2c_amp_exit();

}

//module_init(amp_init);
//module_exit(amp_exit);

//MODULE_AUTHOR("In Bum, Choi <inbum.choi@samsung.com>");
//MODULE_DESCRIPTION("OMAP3430 Sound AMP Driver");
//MODULE_LICENSE("GPL");
