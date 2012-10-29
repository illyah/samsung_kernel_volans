#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/irq_regs.h>


static ssize_t 
rb_dump_show(struct device *dev, struct device_attribute *attr, char *buf);

static ssize_t 
rb_dump_store(struct device *dev, struct device_attribute *attr, 
	                    const char *buf, size_t count);

static DEVICE_ATTR(rb_dump, S_IRUGO | S_IWUSR,rb_dump_show, rb_dump_store);

extern void dump_ram_buffer(void);

static ssize_t 
rb_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dump_ram_buffer();
	return 0;
}

static ssize_t 
rb_dump_store(struct device *dev, struct device_attribute *attr, 
	                    const char *buf, size_t count)
{
	int ret=0;
	char *cbuf;
	
	if(NULL == buf || count == 0 || count > PAGE_SIZE)
			return -1;

	cbuf = kzalloc(count+1, GFP_KERNEL);
	memcpy(cbuf, buf, count);
	cbuf[count] = '\0';
	ret = printk("%s",cbuf);
	kfree(cbuf);
	
	return (ret-3);
}


static int rb_dump_write(struct file *file, const char *data,
		size_t len, loff_t *ppose)
{
	return 0;
}
static int rb_dump_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int rb_dump_ioctl(struct inode *inode, struct file *filp, unsigned int ioctl_cmd,  unsigned long arg)
{
return 0;
}

static struct file_operations rb_dump_fops =
{
.owner = THIS_MODULE,
};

static struct miscdevice rb_dump_device = {
.minor = MISC_DYNAMIC_MINOR,
.name = "rb_dump",
.fops = &rb_dump_fops,
};

static dev_t rb_dump_devnum;
struct class *rb_dump_class;
struct cdev rb_dump_cdev;

int __init rb_dump_init(void)
{
	int ret = 0;
	if((ret = misc_register(&rb_dump_device)) < 0)
	{
		 printk("rb_dump misc_register failed\n");
	}
	else if( (ret = device_create_file(rb_dump_device.this_device, &dev_attr_rb_dump )) < 0 )
    {
        printk("rb-dump device create file failed\n");
    }
	printk("rb_dump_init done\n");
	return ret;
}

void __exit rb_dump_exit(void)
{
    device_remove_file( rb_dump_device.this_device, &dev_attr_rb_dump );
	misc_deregister(&rb_dump_device);
	printk("rb_dump_exit done\n");

}


module_init(rb_dump_init);
module_exit(rb_dump_exit);
MODULE_AUTHOR("basavaraj <basavarj.d@partner.samsung.com>");
MODULE_DESCRIPTION("ram buff dump driver");
MODULE_LICENSE("GPL");
