#ifndef __DCM_HEADER__
#define __DCM_HEADER__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/device.h>


#define DCM_LOG_LEVEL	0

#if (DCM_LOG_LEVEL >= 2)
#define FN_IN printk("%s Entry\n", __FUNCTION__);
#define FN_OUT(ARG) printk("%s[%d]:Exit(%d)\n", __FUNCTION__, __LINE__, ARG);
#else
#define FN_IN 
#define FN_OUT(ARG)
#endif

#define DCM_MAJOR	241
#define DCM_NAME	"dcm"

#define	IOC_DCM_TEST	0

//static int dcm_start(void);
//static void dcm_end(void);
static int dcm_open (struct inode *inode, struct file *filp);
static ssize_t dcm_read (struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t dcm_write (struct file *filp, char const *buf, size_t count, loff_t *f_pos);
static int dcm_release (struct inode *inode, struct file *filp);
static int dcm_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

//static int dcm_probe( struct platform_device* pdev );
//static int dcm_suspend( struct platform_device* pdev );
//static int dcm_resume( struct platform_device* pdev );
static int __init dcm_init(void);
static void __exit dcm_exit(void);

struct file_operations dcm_fops = {
	.owner	= THIS_MODULE,
	.open	= dcm_open,
	.read	= dcm_read,
	.write	= dcm_write,
	.ioctl	= dcm_ioctl,
//	.poll	= dcm_poll,
	.release= dcm_release,
};

struct class *dcm_class;
/*
static struct platform_device *dcm_device;

static struct platform_driver dcm_driver = {
	.probe 	 = dcm_probe,
	.suspend = dcm_suspend,
	.resume  = dcm_resume,
	.driver  = {
		.name = DCM_NAME, 
	}
};
*/
extern  int  keypad_init( void );
extern  void keypad_exit( void );

extern int s3c_ts_init(void); 
extern void s3c_ts_exit(void); 

#endif


