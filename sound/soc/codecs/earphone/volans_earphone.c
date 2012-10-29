#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>	
#include <linux/sysfs.h>	
#include <linux/device.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/atomic.h>
#include <plat/regs-gpio.h>
#include <plat/regs-adc.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/map.h>
#include <mach/volans.h>
#include "volans_earphone.h"
#include "../ak4671.h"

#define __USE_UDEV__

DCM_DETECT_STATE  gEarjack;
DCM_DETECT_STATE  gEarkey;

struct class *earphone_class;

unsigned int earjack_irq, earkey_irq;
unsigned int hcnt, lcnt;

static DECLARE_WAIT_QUEUE_HEAD( earphone_queue );
static DECLARE_WAIT_QUEUE_HEAD( earadc_work );

static unsigned char earjack_status;

struct workqueue_struct *earjack_irq_workq;
struct delayed_work 	 earjack_event_work;
struct workqueue_struct *earkey_irq_workq;
struct delayed_work 	 earkey_event_work;

static int dev_id = 2;
static struct device *earphone_dev_t;

#ifdef __USE_UDEV__

static int earPhone;
static int earKey;
static int earType;

static ssize_t earphone_show_value(struct device *dev, struct device_attribute *attr, char *buf) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
		 return snprintf(buf, sizeof(earPhone), "%d\n", earPhone);
 }
  
static ssize_t earphone_store_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
		 sscanf(buf, "%d", &earPhone);
		 return strnlen(buf, sizeof(earPhone));
}

static ssize_t earkey_show_value(struct device *dev, struct device_attribute *attr, char *buf) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
		 return snprintf(buf, sizeof(earKey), "%d\n", earKey);
 }
  
static ssize_t earkey_store_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
		 sscanf(buf, "%d", &earKey);
		 return strnlen(buf, sizeof(earKey));
 }
 
static ssize_t eartype_show_value(struct device *dev, struct device_attribute *attr, char *buf) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	return snprintf(buf, sizeof(earType), "%d\n", earType);
}

static ssize_t eartype_store_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	sscanf(buf, "%d", &earType);
	return strnlen(buf, sizeof(earType));
}

static DEVICE_ATTR(earPhone, S_IRUGO | S_IWUSR, earphone_show_value, earphone_store_value);
static DEVICE_ATTR(earKey, S_IRUGO | S_IWUSR, earkey_show_value, earkey_store_value);
static DEVICE_ATTR(earType, S_IRUGO | S_IWUSR, eartype_show_value, eartype_store_value);
 
static int sysfs_earkey_create_file(struct device *dev) {
        int result = 0;

	DPRINTK(KERN_INFO "%d\n", __LINE__);
    result = device_create_file(dev, &dev_attr_earPhone);
        result = device_create_file(dev, &dev_attr_earKey);
	result = device_create_file(dev, &dev_attr_earType);
    
        if (result)
                return result;
 
        return 0;
}

 static void sysfs_earkey_remove_file(struct device *dev) {
	DPRINTK(KERN_INFO "%d\n", __LINE__);
    device_remove_file(dev, &dev_attr_earPhone);
        device_remove_file(dev, &dev_attr_earKey);
	device_remove_file(dev, &dev_attr_earType);
}

#endif // __USE_UDEV__

void earjack_check(void)
{
	int pin_level = 0;
	DPRINTK(KERN_INFO "%d\n", __LINE__);

	pin_level = gpio_get_value(GPIO_DET_35);
	headset_status = pin_level;
	
	if(pin_level) {
		gEarjack.state = EARJACK_CONNECTED;
		gEarjack.low_count = 0;
		earjack_status = EARJACK_STATUS_CHANGE;
		dprintk(SND_DBG,"[EARPHONE] INT: 1\n");
	}
	else {
		dprintk(SND_DBG,"[EARPHONE] INT: 0\n");
	if (!(path_data.path & (IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC))) {
		mic_power_ctrl(0);	
	}
		gEarjack.state = EARJACK_DISCONNECTED;
		gEarjack.high_count = 0;
		earjack_status = EARJACK_STATUS_CHANGE;
	}
}

void earkey_check(void)
{
	int pin_level = 0;
	DPRINTK(KERN_INFO "%d\n", __LINE__);

	pin_level = gpio_get_value(GPIO_EAR_SEND_END);
	
	if(pin_level) {
		gEarkey.state = KEY_RELEASED;
		gEarkey.low_count = 0;
		DPRINTK("[EARPHONE] GPIO_EAR_SEND_END is High\n");
	}
	else {
		gEarkey.state = KEY_PRESSED;
		gEarkey.high_count = 0;
		DPRINTK("[EARPHONE] GPIO_EAR_SEND_END is LOW\n");
	}

}

void earphone_gpio_check(void)
{

	DPRINTK(KERN_INFO "%d\n", __LINE__);
	earjack_check();
	earkey_check();

	cancel_delayed_work(&earjack_event_work);
	queue_delayed_work(earjack_irq_workq, &earjack_event_work, msecs_to_jiffies(10));
	
	cancel_delayed_work(&earkey_event_work);
	queue_delayed_work(earkey_irq_workq, &earkey_event_work, msecs_to_jiffies(60));
}

static irqreturn_t earjack_detect_isr(int irq, void *dev_id)
{
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	earjack_check();

	cancel_delayed_work(&earjack_event_work);
	queue_delayed_work(earjack_irq_workq, &earjack_event_work, msecs_to_jiffies(10));
	
	return IRQ_HANDLED;
}

static irqreturn_t earkey_detect_isr(int irq, void *dev_id)
{
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	earkey_check();

	cancel_delayed_work(&earkey_event_work);
	queue_delayed_work(earkey_irq_workq, &earkey_event_work, msecs_to_jiffies(60));
	
	return IRQ_HANDLED;
}


extern void __iomem *s3c_adc_base_addr;
extern unsigned int s3c_adc_conv(unsigned int channel);

int get_average_adc_value(unsigned int *data, int count)
{
	int i=0, average, min = 0xFFFFFFFF, max=0, total=0;
	for(i=0; i<count; i++)
	{
		if(data[i] < min)
			min = data[i];
		if(data[i] > max)
			max = data[i];

		total += data[i];
	}
	average = (total - min - max)/(count-2);
	return average;
}


static int earadc_check(void)
{
	unsigned int i = 0, data[EARADC_SAMPLING_COUNT] ,temp = 0;
	void __iomem  *adc_base;
	DPRINTK(KERN_INFO "%d\n", __LINE__);

	mdelay(100);
	adc_base = s3c_adc_base_addr;
	
	writel( 0, adc_base + S3C_ADCCON); 
	writel( 0, adc_base + S3C_ADCDLY);  
	
	writel( (readl(adc_base+S3C_ADCDLY) & 0xff00) | 10, adc_base+S3C_ADCDLY);
	
	writel( readl(adc_base + S3C_ADCCON) | 
			S3C_ADCCON_RESSEL_10BIT | 
			S3C_ADCCON_PRSCEN | 
			S3C_ADCCON_PRSCVL(5) | 
			S3C_ADCCON_READ_START  
			, adc_base + S3C_ADCCON );	

	
	for(i=0; i < EARADC_SAMPLING_COUNT; i++) {
		data[i] = s3c_adc_conv(3);		
	}
	temp = get_average_adc_value(data, EARADC_SAMPLING_COUNT);

	if(temp > 260) {
		dprintk(SND_INF, "[EARJACK] >>> 4-wired Earjack : ADC3 Level - %d\n", temp);
		earType = 1;
	}
	else{
		dprintk(SND_INF, "[EARJACK] >>> 3-wired Earjack : ADC3 Level - %d\n", temp);
		earType = 0;
	}
	
	return 0;
}

void earphone_detect_scan_svc(void)
{
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	
	gEarjack.prev_state = UNKNOWN_STATUS;
	gEarkey.prev_state = UNKNOWN_STATUS;

	earphone_gpio_check();
}

static void check_earjack_irq_worker( struct work_struct* work )
{
	unsigned int check_earjack_key_status = 0;
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	
	if( gEarjack.prev_state != gEarjack.state )
	{
		switch(gEarjack.state)
		{
		case EARJACK_CONNECTED:
			gEarjack.high_count++;	
			if(gEarjack.high_count > EARJACK_CONNECT_HOLD_TIME) 
			{
				
				msleep(290);
				
				if(!gpio_get_value(GPIO_DET_35)) {
					if (!(path_data.path & (IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC))) {
						mic_power_ctrl(0);	
					}
					gEarjack.state = EARJACK_DISCONNECTED;
					gEarjack.high_count = 0;
					earjack_status = EARJACK_STATUS_CHANGE;
					wake_up_interruptible( &earphone_queue );
					break;
				}
				
				mic_power_ctrl(1);	
				earadc_check();
				
				if(!gpio_get_value(GPIO_DET_35)) {
					if (!(path_data.path & (IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC))) {
						mic_power_ctrl(0);	
					}
					gEarjack.state = EARJACK_DISCONNECTED;
					gEarjack.high_count = 0;
					earjack_status = EARJACK_STATUS_CHANGE;
					wake_up_interruptible( &earphone_queue );
					break;
				}

				earjack_status = EARJACK_STATUS_CHANGE;
				wake_up_interruptible( &earphone_queue );
#ifdef __USE_UDEV__
				dprintk(SND_INF, "[EARPHONE] EARJACK_CONNECTED \n");
				if(earphone_class != NULL){
					earPhone = 1;
					kobject_uevent(&earphone_dev_t->kobj, KOBJ_ADD);
				}else
					printk("[EARPHONE][ERR] earphone_class is NULL \n");
#endif
				gEarjack.prev_state 	= gEarjack.state;

				if(earType){
					s3c_gpio_cfgpin(GPIO_EAR_SEND_END, S3C_GPIO_SFN(GPIO_EAR_SEND_END_AF));
					s3c_gpio_setpull(GPIO_EAR_SEND_END, S3C_GPIO_PULL_NONE);
					earkey_irq = IRQ_EAR_SEND_END;
					set_irq_type(IRQ_EAR_SEND_END, IRQ_TYPE_EDGE_BOTH);
					
					if (request_irq(earkey_irq, earkey_detect_isr, IRQF_DISABLED, EARKEY_NAME, NULL)){
						printk("[EARPHONE] ERROR! irq %u in use.\n", IRQ_EAR_SEND_END);
					}else{
						dprintk(SND_INF, "[EARPHONE] found %s adapter at irq %u\n", EARKEY_NAME , IRQ_EAR_SEND_END);
					}
				}
				else{
					if (!(path_data.path & (IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC))) {
						mic_power_ctrl(0);	
					}
				}


			}
			else
			{
				check_earjack_key_status = 1;
			}
			break;
			
		case EARJACK_DISCONNECTED:
			gEarjack.low_count++;	
			if(gEarjack.low_count > EARJACK_DISCONNECT_HOLD_TIME) 
			{
				if(earType){
					free_irq(IRQ_EAR_SEND_END, (void *)0);
				}
				earjack_status = EARJACK_STATUS_CHANGE;
				wake_up_interruptible( &earphone_queue );
#ifdef __USE_UDEV__
				dprintk(SND_INF, "[EARPHONE] EARJACK_DISCONNECTED \n");
				if(earphone_class != NULL){
					earPhone = 0;
					kobject_uevent(&earphone_dev_t->kobj, KOBJ_REMOVE);
				} else
					printk("[EARPHONE][ERR] earphone_class is NULL \n");
#endif
				gEarjack.prev_state 	= gEarjack.state;
			}
			else
			{
				check_earjack_key_status = 1;
			}
			break;
		
		default:
			break;
		}
	}

	if( check_earjack_key_status )
		queue_delayed_work(earjack_irq_workq, &earjack_event_work, msecs_to_jiffies(10));

}

static void check_earkey_irq_worker( struct work_struct* work )
{
	unsigned int check_earjack_key_status = 0;
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	
	if( gEarkey.prev_state != gEarkey.state )
	{
		
		switch(gEarkey.state)
		{
			case KEY_PRESSED:
				gEarkey.high_count++;	
				if(gEarkey.high_count > EARKEY_PRESSED_HOLD_TIME) 
				{
					if(gpio_get_value(GPIO_DET_35))
					{
#ifdef __USE_UDEV__
					dprintk(SND_INF, "[EARPHONE] EARKEY_PRESSED \n");
					earjack_report_key(KEY_SEND, KEY_PRESSED);
					earKey = 1;
					if(earphone_class != NULL){
							kobject_uevent(&earphone_dev_t->kobj, KOBJ_CHANGE);
						} else
						printk("[EARPHONE][ERR] earphone_class is NULL \n");
#endif
					gEarkey.prev_state 	= gEarkey.state;
				}
				}
				else
				{
					check_earjack_key_status = 1;
				}
				break;
			case KEY_RELEASED:
				gEarkey.low_count++;	
				if(gEarkey.low_count > EARKEY_RELEASED_HOLD_TIME) 
				{
					if(gpio_get_value(GPIO_DET_35))
					{
#ifdef __USE_UDEV__
					dprintk(SND_INF,"[EARPHONE] EARKEY_RELEASED \n");
					earjack_report_key(KEY_SEND, KEY_RELEASED);
					earKey = 0;
					if(earphone_class != NULL){
							kobject_uevent(&earphone_dev_t->kobj, KOBJ_CHANGE);
						} else
						printk("[EARPHONE][ERR] earphone_class is NULL \n");
#endif
					gEarkey.prev_state 	= gEarkey.state;
				}
				}
				else
				{
					check_earjack_key_status = 1;
				}
				break;
			default:
				break;
		}

	}

	if( check_earjack_key_status )
		queue_delayed_work(earkey_irq_workq, &earkey_event_work, msecs_to_jiffies(60));

}

static unsigned int earphone_poll(struct file *filp, struct poll_table_struct *wait)
{	
	unsigned int mask = 0;
	DPRINTK(KERN_INFO "%d\n", __LINE__);

	poll_wait( filp, &earphone_queue, wait );
	
	if( earjack_status == EARJACK_STATUS_CHANGE )
	{
		earjack_status = EARJACK_STATUS_NORMAL;
		mask |= POLLIN | POLLRDNORM;
	}
	
	return mask;
}

static ssize_t earphone_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	unsigned char a;
	DPRINTK(KERN_INFO "%d\n", __LINE__);

	a = 48 + gEarjack.state; 

	if( copy_to_user( buf, (void *)&a, 1 ) )
	{
		printk( "earphone copy_to_user error\n" );
		return -EFAULT;
	}

	return 1;
}   


static struct file_operations earphone_fops =
{
	.owner   = THIS_MODULE,
	.read    = earphone_read,
	.poll    = earphone_poll,
};


int earphone_detect_init(void)
{
	int result, value;
	
#ifdef __USE_UDEV__
	int err = 0;
#endif
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	
	gEarjack.state 			= UNKNOWN_STATUS;
	gEarjack.prev_state 	= EARJACK_DISCONNECTED;
	gEarkey.state 			= UNKNOWN_STATUS;
	gEarkey.prev_state 		= KEY_RELEASED;

	earjack_status = UNKNOWN_STATUS;

	value = __raw_readl(S3C64XX_EINT0FLTCON1);
	value |= (1 << 15);						  
	value &= ~(1 << 14);					 
	__raw_writel(value, S3C64XX_EINT0FLTCON1);

	s3c_gpio_cfgpin(GPIO_DET_35, S3C_GPIO_SFN(GPIO_DET_35_AF));
	s3c_gpio_setpull(GPIO_DET_35, S3C_GPIO_PULL_NONE);
	earjack_irq = IRQ_DET_35;
	set_irq_type(IRQ_DET_35, IRQ_TYPE_EDGE_BOTH);

	INIT_DELAYED_WORK( &earjack_event_work, check_earjack_irq_worker );
	if( ( earjack_irq_workq = create_singlethread_workqueue( "earjack" ) ) == NULL )
	{
		printk( "fail to create irq workqueue for getting earjack state\n" );
	}

	INIT_DELAYED_WORK( &earkey_event_work, check_earkey_irq_worker );
	if( ( earkey_irq_workq = create_singlethread_workqueue( "earkey" ) ) == NULL )
	{
		printk( "fail to create irq workqueue for getting earkey state\n" );
	}

	if (request_irq(earjack_irq, earjack_detect_isr, 
				IRQF_DISABLED , EARJACK_NAME, (void *)&dev_id))
		printk("[EARPHONE] ERROR! irq %u in use.\n", IRQ_DET_35);
	else
		printk("[EARPHONE] found %s adapter at irq %u\n",EARJACK_NAME,IRQ_DET_35);

	result = register_chrdev( EAR_DEV_MAJOR, EAR_DEV_NAME, &earphone_fops );

	if( result < 0 )
	{
		printk( "earphone rgister_chrdev err\n" );
		return result;
	}
	
	earphone_class = class_create( THIS_MODULE, "earphone_class" );

	if( IS_ERR( earphone_class ) )
	{
		printk( "earphone class_create err\n" );
		unregister_chrdev( EAR_DEV_MAJOR, EAR_DEV_NAME );
		return IS_ERR(earphone_class);
	}

	earphone_dev_t = device_create( earphone_class, NULL, MKDEV(EAR_DEV_MAJOR, 0), NULL, "earphone" );

	if( IS_ERR( earphone_dev_t ) )
	{
		printk( "earphone device_create err\n" );
		return IS_ERR(earphone_dev_t);
	}

	headset_status = gpio_get_value(GPIO_DET_35); 

	queue_delayed_work(earjack_irq_workq, &earjack_event_work, msecs_to_jiffies(10));
	queue_delayed_work(earkey_irq_workq, &earkey_event_work, msecs_to_jiffies(30));

#ifdef __USE_UDEV__
 
	err = sysfs_earkey_create_file(earphone_dev_t);
	if (err) {
		printk("[EARPHONE][ERR] earphone_detect_init() - sysfs_create_file error\n");
		return err;
	}
return 0;
#endif	// __USE_UDEV__
}

void earphone_detect_exit(void)
{
	DPRINTK(KERN_INFO "%d\n", __LINE__);
	free_irq(IRQ_DET_35, (void *)&dev_id);

	device_destroy(earphone_class, MKDEV(EAR_DEV_MAJOR, 0));
	class_destroy(earphone_class);

	unregister_chrdev( EAR_DEV_MAJOR, EAR_DEV_NAME );

#ifdef __USE_UDEV__
	sysfs_earkey_remove_file(earphone_dev_t);
#endif

	cancel_rearming_delayed_workqueue(earjack_irq_workq, &earjack_event_work);
	destroy_workqueue(earjack_irq_workq);

	cancel_rearming_delayed_workqueue(earkey_irq_workq, &earkey_event_work);
	destroy_workqueue(earkey_irq_workq);

}





