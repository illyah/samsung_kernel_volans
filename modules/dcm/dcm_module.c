#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <mach/gpio.h>
#include <linux/vmalloc.h>

#include <linux/kobject.h>	
#include <linux/sysfs.h>	
#include <linux/device.h>	
#include <linux/string.h>

#include <mach/hardware.h>
#include <linux/proc_fs.h>
#include <linux/irq.h>
#include "dcm_ext.h"
#include "dprintk.h"
#include <linux/suspend.h>

#define BATTERY_ADC_VF  4000
#define CONFIG_RFS_QUOTA 
#ifdef CONFIG_RFS_QUOTA 
#include <linux/version.h>
#include <linux/rfs_fs_sb.h>
#endif
#include <plat/pm.h>
int  (*SendMsgToDevMgr) ( int, int, int, int, int );

int pre_real_time_quota_status = 0xFF; 


#define SOUND_PATH_TEST
#define LCD_STATUS_CABC			0x200
#define LCD_STATUS_ALC			0x100
#define LCD_BACKLIGHT_VALUE		0xFF
#define MAX_SIZE				1000	


#ifdef USE_KERNEL_THREAD
#include "dcm_thread.h"
#define NTHREADS 1
kthread_t	dcm_thread[NTHREADS];
#endif

#define USE_FLAG_FOR_EVENT_SYNC
#ifdef USE_FLAG_FOR_EVENT_SYNC
unsigned int dcm_start_flag;
#endif

#if 1 
#include <linux/msdos_fs.h>
#include <linux/version.h>
#include <linux/fat.h>

#define FAT_START_ENT   2

#define DEV_STL		    0x0
#define DEV_MOVINAND    0x4

#define	LOW_MEMORY_NORMAL 		0x01
#define LOW_MEMORY_SOFT_WARNING 0x02
#define LOW_MEMORY_HARD_WARNING 0x04

#define FS_VFAT 		1
#define FS_RFS			2

#define SYSTEM_EVENT    0x00000001

int prev_real_time_quota_status = 0xFF;
int real_time_quota_status;

extern int omap3430_gpio_activity(int time_out_ms);
extern int twl4030_rtc_activity(int time_out_ms);
extern int gpio_last_activity_irq;

extern int alarm_boot;
#ifdef CONFIG_RFS_QUOTA
typedef void (FP_CLUSTER_USAGE_NOTIFY)(struct super_block *sb);
void register_cluster_usage_notify(FP_CLUSTER_USAGE_NOTIFY *p_func);
#endif
#include <linux/platform_device.h>
#ifdef DEBUG
#define mdebug(arg,...)	 printk(arg,## __VA_ARGS__)
#else
#define mdebug(arg,...)
#endif

#define ONENAND_WARNING_LEVEL 	10485760 /* 10 MB */
#define ONENAND_CRITICAL_LEVEL  1048576  /* 1 MB */

#define MOVINAND_WARNING_LEVEL 	104857600 /* 100 MB */
#define MOVINAND_CRITICAL_LEVEL  10485760  /* 10 MB */

struct class *mem_notify_class;


static unsigned long onenand_warning_level = ONENAND_WARNING_LEVEL;
static unsigned long onenand_critical_level = ONENAND_CRITICAL_LEVEL;
static unsigned long movinand_warning_level = MOVINAND_WARNING_LEVEL;
static unsigned long movinand_critical_level = MOVINAND_CRITICAL_LEVEL;

static ssize_t onenand_critical_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	return sprintf(buf, "%ld\n", onenand_critical_level);
}

static ssize_t onenand_critical_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	onenand_critical_level = simple_strtoul(buf, NULL, 10);
	return count;
}

static ssize_t onenand_warning_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	return sprintf(buf, "%ld\n", onenand_warning_level);
}

static ssize_t onenand_warning_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	onenand_warning_level = simple_strtoul(buf, NULL, 10);
	return count;
}

static ssize_t movinand_critical_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	return sprintf(buf, "%ld\n", movinand_critical_level);
}

static ssize_t movinand_critical_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	movinand_critical_level = simple_strtoul(buf, NULL, 10);
	return count;
}

static ssize_t movinand_warning_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	return sprintf(buf, "%ld\n", movinand_warning_level);
}

static ssize_t movinand_warning_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	movinand_warning_level = simple_strtoul(buf, NULL, 10);
	return count;
}

static ssize_t quota_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	mdebug("%s : %d\n", __func__, __LINE__);
	return sprintf(buf, "%d\n", real_time_quota_status);
}

static DEVICE_ATTR(onenand_critical, S_IRUGO | S_IWUSR, onenand_critical_level_show, onenand_critical_level_store);
static DEVICE_ATTR(onenand_warning, S_IRUGO | S_IWUSR, onenand_warning_level_show, onenand_warning_level_store);
static DEVICE_ATTR(movinand_critical, S_IRUGO | S_IWUSR, movinand_critical_level_show, movinand_critical_level_store);
static DEVICE_ATTR(movinand_warning, S_IRUGO | S_IWUSR, movinand_warning_level_show, movinand_warning_level_store);
static DEVICE_ATTR(quota_info, S_IRUGO, quota_info_show, NULL);

static int low_memnotify_probe(struct platform_device *dev)
{
	mdebug("%s : %d\n", __func__, __LINE__);

	if (device_create_file(&dev->dev, &dev_attr_onenand_critical)) {
		printk(KERN_ERR "failed to create sysfs file for threshold\n");
		return -1;
	}   

	if (device_create_file(&dev->dev, &dev_attr_onenand_warning)) {
		printk(KERN_ERR "failed to create sysfs file for threshold\n");
		return -1;
	}   

	if (device_create_file(&dev->dev, &dev_attr_movinand_critical)) {
		printk(KERN_ERR "failed to create sysfs file for threshold\n");
		return -1;
	}   

	if (device_create_file(&dev->dev, &dev_attr_movinand_warning)) {
		printk(KERN_ERR "failed to create sysfs file for threshold\n");
		return -1;
	}   

	if (device_create_file(&dev->dev, &dev_attr_quota_info)) {
		printk(KERN_ERR "failed to create sysfs file for threshold\n");
		return -1;
	}   


	return 0;
}

static struct platform_driver low_memnotify_driver = {
	.probe          = low_memnotify_probe,
	.driver         = {
		.name   =   "low_mem_notify",
		.owner  =   THIS_MODULE,
	},
};

static struct platform_device low_memnotify_device = {
	.name   =   "low_mem_notify",
	.id     =   0,
	.dev    =   {
		.platform_data = NULL,
	},
};

void dcm_notify_quota_info(struct super_block *sb)
{
	unsigned long total, free;
	unsigned long warning_clusters, critical_clusters;
	unsigned long warning_bytes, critical_bytes;
	int quota_status, dev_name, fs_name;

	struct msdos_sb_info *vfat_info;
	struct rfs_sb_info *rfs_info;

	if(!strcmp(sb->s_id, "mmca1")) { 
		dev_name = DEV_MOVINAND;
		fs_name = FS_RFS;
		warning_bytes = movinand_warning_level;
		critical_bytes = movinand_critical_level;
	}
	else if(!strcmp(sb->s_id, "stl9")) { 
		dev_name = DEV_STL;
		fs_name = FS_RFS;
		warning_bytes = onenand_warning_level;
		critical_bytes = onenand_critical_level;
	} 
	else {
		return;
	}

	switch(fs_name) {
		case FS_VFAT:
			vfat_info = sb->s_fs_info;
			total = vfat_info->max_cluster - FAT_START_ENT;
			free = vfat_info->free_clusters;
			break;
		case FS_RFS:
			rfs_info = (struct rfs_sb_info *) sb->s_fs_info;

			warning_clusters = warning_bytes >> rfs_info->cluster_bits;
			critical_clusters = critical_bytes >> rfs_info->cluster_bits;

			total = rfs_info->num_clusters;
			free = rfs_info->num_clusters - rfs_info->num_used_clusters;
			break;
		default: 
			return;
	}

	if(free > warning_clusters ) {
		quota_status = LOW_MEMORY_NORMAL;
	} else {
		if(free <= critical_clusters ) {
			quota_status = LOW_MEMORY_HARD_WARNING;
		}
		else {
			quota_status = LOW_MEMORY_SOFT_WARNING;
		}
	}
	#endif

	real_time_quota_status &= ~(0xF << dev_name);

	real_time_quota_status |= (quota_status << dev_name);

	if(prev_real_time_quota_status ^ real_time_quota_status)
	{
			SendMsgToDevMgr( SYSTEM_EVENT, real_time_quota_status, 0, 0, 0 );
		printk("<%s:%d> real_time_quota_status : 0x%08x\n", __FUNCTION__, __LINE__, real_time_quota_status);
	}

	prev_real_time_quota_status = real_time_quota_status;
}

#define DCM_LOG_LEVEL 0

#if (DCM_LOG_LEVEL >= 2)
#define FN_IN printk("%s Entry\n",__FUNCTION__);
#define FN_OUT(ARG) printk("%s[%d]:Exit(%d)\n",__FUNCTION__,__LINE__,ARG);
#else
#define FN_IN
#define FN_OUT(ARG)
#endif

#define SWITCH_USB_SEL		0
#define SWITCH_UART_SEL		1


#define PDA_UART_SEL_LEVEL				GPIO_LEVEL_LOW
#define PHONE_UART_SEL_LEVEL			GPIO_LEVEL_HIGH
#define PDA_UART_SEL_PAD_OFF_LEVEL		PAD_OFF_DRIVE_LOW
#define PHONE_UART_SEL_PAD_OFF_LEVEL	PAD_OFF_DRIVE_HIGH

static int switch_val = 3;
static int nation_val = 0;

#define NATION_PROC_ENTRY	"nation"

extern void (*usbsel_notify)(int);

extern void dcm_parameter_init(void);
extern void dcm_parameter_exit(void);

extern int save_parameter_value(void);
extern void read_parameter_value(int idx, void *value);
extern void set_parameter_value(int idx, void *value);

extern void touchscreen_poweronoff(int onoff);

#define DCM_NAME					"dcm"
#define DCM_MINOR					134
#define	tolower(x)					x

static struct proc_dir_entry* 		printflag_entry = NULL;

typedef struct lcd_info     
{
	struct kobject kobj;
}lcd_info_t;

lcd_info_t lcd_info;

int notify_finish_sleep(struct notifier_block *nb, unsigned long type, void *data)
{
	if(PM_POST_SUSPEND!=type)
		return NOTIFY_DONE;
	return NOTIFY_OK;
}

struct notifier_block dcm_notifier =
{
	.notifier_call=notify_finish_sleep,
	.priority=1,
};
unsigned int MyStrToULL(char* pStr, unsigned char nBase)
{
	unsigned char nPos=0;
	unsigned char c;
	unsigned int nVal = 0;
	unsigned char nCnt=0;
	unsigned int n=0;
	unsigned int len;

	FN_IN;
	if (pStr == NULL)
		return(0);

	for(len=1; len < strlen(pStr); len++) {
		c = tolower(*(pStr + len));
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
			continue;
		else
			break;
	}

	for (nPos=0 ; nPos < len ; nPos++) {
		c = tolower(*(pStr + len - 1 - nPos));
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c = c - 'a' + 10;
		else
			break;

		for (nCnt=0, n=1 ; nCnt < nPos ; nCnt++)
			n *= nBase;
		nVal += (n * c);
	} 

	FN_OUT(nVal);
	return(nVal);    
}

static int proc_read_printflag(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	FN_IN;
	len = sprintf(page, "------------------------------\n");
	len += sprintf(page+len, "iPrintFlag  = %016LX\n",iPrintFlag);
	len += sprintf(page+len, "------------+-----------------\n");
	len += sprintf(page+len, "DCM         |                %LX\n",( iPrintFlag&0x000000000000000fLL) );
	len += sprintf(page+len, "PowerControl|               %LX \n",( iPrintFlag&0x00000000000000f0LL )>>0x4);
	len += sprintf(page+len, "Camera      |              %LX  \n",( iPrintFlag&0x0000000000000f00LL )>>0x8);
	len += sprintf(page+len, "LCD         |             %LX   \n",( iPrintFlag&0x000000000000f000LL )>>0xc);
	len += sprintf(page+len, "Sound       |            %LX    \n",( iPrintFlag&0x00000000000f0000LL )>>0x10);
	len += sprintf(page+len, "Dpram       |           %LX     \n",( iPrintFlag&0x0000000000f00000LL )>>0x14);
	len += sprintf(page+len, "MultiPDP    |          %LX      \n",( iPrintFlag&0x000000000f000000LL )>>0x18);
	len += sprintf(page+len, "USB         |         %LX       \n",( iPrintFlag&0x00000000f0000000LL )>>0x1c);
	len += sprintf(page+len, "Bluez       |        %LX        \n",( iPrintFlag&0x0000000f00000000LL )>>0x20 );
	len += sprintf(page+len, "WirelessLAN |       %LX         \n",( iPrintFlag&0x000000f000000000LL )>>0x24 );
	len += sprintf(page+len, "MMC/SD      |      %LX          \n",( iPrintFlag&0x00000f0000000000LL )>>0x28 );
	len += sprintf(page+len, "FileSystem  |     %LX           \n",( iPrintFlag&0x0000f00000000000LL )>>0x2c );
	len += sprintf(page+len, "OOM         |    %LX            \n",( iPrintFlag&0x000f000000000000LL )>>0x30 );
	len += sprintf(page+len, "reserved    |   %LX             \n",( iPrintFlag&0x00f0000000000000LL )>>0x34 );
	len += sprintf(page+len, "reserved    |  %LX              \n",( iPrintFlag&0x0f00000000000000LL )>>0x38 );
	len += sprintf(page+len, "reserved    | %LX               \n",( iPrintFlag&0xf000000000000000LL )>>0x3c );
	len += sprintf(page+len, "------------------------------\n");

	FN_OUT(len);

	return len;

}

static int proc_write_printflag(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len; 
	unsigned long long ret;
	unsigned char value[20];

	FN_IN;
	if(count > 19)
		len = 19;
	else
		len = count;

	if(copy_from_user(value, buffer, len)) {
		return -EFAULT;
	}
	value[len] = '\0';

	if(strncmp(value,"0x", 2) == 0)
		ret = MyStrToULL(value+2, 16);
	else
		ret = MyStrToULL(value, 10);

	iPrintFlag = ret;

	FN_OUT(len);
	return len;
}

static __inline__ void proc_print_create(void) {
	FN_IN;
	printflag_entry = create_proc_entry("iPrintFlag",0644,NULL);
	if(printflag_entry == NULL) {
		remove_proc_entry("iPrintFlag", NULL);
	}
	else {
		printflag_entry->data = &iPrintFlag;
		printflag_entry->read_proc = proc_read_printflag;
		printflag_entry->write_proc = proc_write_printflag;
		printflag_entry->owner = THIS_MODULE;
	}
	FN_OUT(0);
}

static __inline__ void proc_print_remove(void) {
	FN_IN;
	remove_proc_entry("iPrintFlag",NULL);
	FN_OUT(0);
}

DECLARE_WAIT_QUEUE_HEAD(dcm_queue);
DCM_QUEUE_TYPE			dcmQueue[DCM_QUEUE_SIZE];

static unsigned int 	dcm_queue_head = 0;	
static unsigned int 	dcm_queue_tail = 0;	

int dcm_isQueueEmpty(void)
{
	return dcm_queue_head == dcm_queue_tail;
}

void dcm_clearQueue(void)
{
	dcm_queue_head = 0;
	dcm_queue_tail = 0;
	memset( dcmQueue, 0x00, sizeof( dcmQueue ) ); 
}

int dcm_addQueue(DCM_QUEUE_TYPE *pval)
{
    int i;

	i = (dcm_queue_tail + 1) % DCM_QUEUE_SIZE;
	if (i != dcm_queue_head) 
	{
        memcpy((char *)&dcmQueue[i], (char *)pval, sizeof(DCM_QUEUE_TYPE));
        dcm_queue_tail = i;
		wake_up_interruptible(&dcm_queue);
    }

	return 0;
}

int dcm_deleteQueue(DCM_QUEUE_TYPE *pval)
{
	if( !dcm_isQueueEmpty() ) {
		dcm_queue_head = (dcm_queue_head + 1) % DCM_QUEUE_SIZE;
		memcpy((char *)pval, (char *)&dcmQueue[dcm_queue_head], sizeof(DCM_QUEUE_TYPE));
		return 0;
	}
	return -1;
}

int dcm_SendMsgToDevMgr(int id,int p1,int p2,int p3,int p4)
{
	t_callMsg *p_snd_msg,snd_msg;

	p_snd_msg = &snd_msg;
	memset(p_snd_msg, 0, sizeof(t_callMsg));

	S_STID	= TASKID_DCM;
	S_DTID	= TASKID_DEVMGR;
	S_MSGID	= id;
	S_SIZE	= sizeof(t_callMsg);

	S_PARA1	= p1;
	S_PARA2	= p2;
	S_PARA3	= p3;
	S_PARA4	= p4;
	dcm_addQueue(p_snd_msg);
	
	return 0;
}


#ifdef CONFIG_RFS_QUOTA 
extern struct super_block *rfs_s;
#endif

static void dcm_10ms_service(void)
{
#ifdef CONFIG_RFS_QUOTA 
	struct rfs_sb_info *rfs_info;
	unsigned long total, free;
		rfs_info = (struct rfs_sb_info *) rfs_s->s_fs_info;
	total = rfs_info->num_clusters;
	free = rfs_info->num_clusters - rfs_info->num_used_clusters;

#endif

	printk("[DCM] dcm_10ms_service called\n");

}

static unsigned int	dcm_poll(struct file *, struct poll_table_struct *);
static ssize_t		dcm_read(struct file *, char *, size_t , loff_t *);

static int dcm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

static unsigned int dcm_poll(struct file *fp, poll_table * wait)
{
	FN_IN;
	poll_wait(fp, &dcm_queue, wait);
	if (dcm_queue_head != dcm_queue_tail)
		return (POLLIN | POLLRDNORM);
	FN_OUT(0);
	return 0;
}
#define DCMQ_VMALLOC
static ssize_t dcm_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
	int cnt = 0;
	int retval;

#ifndef DCMQ_VMALLOC
	DCM_QUEUE_TYPE popd_key;
	DCM_QUEUE_TYPE userbuf[DCM_QUEUE_SIZE]; 
	memset( (char*)&userbuf[0], 0x00, DCM_QUEUE_SIZE * sizeof(DCM_QUEUE_TYPE ));
#else
	DCM_QUEUE_TYPE *popd_key = ( DCM_QUEUE_TYPE* ) vmalloc( sizeof(DCM_QUEUE_TYPE) );
	DCM_QUEUE_TYPE *userbuf  = ( DCM_QUEUE_TYPE* ) vmalloc( (DCM_QUEUE_SIZE *sizeof(DCM_QUEUE_TYPE) ) );
	memset((char *)userbuf,0x00,DCM_QUEUE_SIZE*sizeof(DCM_QUEUE_TYPE));
#endif

	FN_IN;
	if (dcm_queue_head == dcm_queue_tail)
		interruptible_sleep_on(&dcm_queue);  

	for (cnt = 0; cnt < DCM_QUEUE_SIZE; cnt++)
	{ 
#ifndef DCMQ_VMALLOC
		if (dcm_deleteQueue(&popd_key) == -1) break;
		memcpy((char *)&userbuf[cnt],(char *)&popd_key,sizeof(DCM_QUEUE_TYPE));
#else
		if (dcm_deleteQueue(popd_key) == -1) break;
		memcpy((char *)(userbuf+cnt),(char *)popd_key,sizeof(DCM_QUEUE_TYPE));
#endif

	}

	if( copy_to_user(buf, (char *)&userbuf[0], cnt*sizeof(DCM_QUEUE_TYPE)) )
	{
	
	}

	retval = (cnt)*sizeof(DCM_QUEUE_TYPE);
#ifdef DCMQ_VMALLOC
	vfree(popd_key);
	vfree(userbuf);
#endif

	FN_OUT(retval);
	return retval; 
}
#ifdef DCM_IOCTL_DUMP
static char * get_cmd_string(u_int cmd)
{
	switch (cmd) {
	case IOC_DCM_START:					return "IOC_DCM_START";
	case IOC_LED_SET:					return "IOC_LED_SET";
	case IOC_TCH_GET_CAL:				return "IOC_TCH_GET_CAL";
	case IOC_TCH_SET_CAL:				return "IOC_TCH_SET_CAL";
	case IOC_TCH_ENABLE:				return "IOC_TCH_ENABLE";
	case IOC_TCH_DISABLE:				return "IOC_TCH_DISABLE";
	case IOC_LCD_GET_BRT:				return "IOC_LCD_GET_BRT";
	case IOC_LCD_SET_BRT:				return "IOC_LCD_SET_BRT";
	case IOC_LCD_GET_ALC:				return "IOC_LCD_GET_ALC";
	case IOC_LCD_SET_ALC:				return "IOC_LCD_SET_ALC";
	case IOC_LCD_GET_CABC:				return "IOC_LCD_GET_CABC";
	case IOC_LCD_SET_CABC:				return "IOC_LCD_SET_CABC";
	case IOC_LCD_POW_ON:				return "IOC_LCD_POW_ON";
	case IOC_LCD_POW_OFF:				return "IOC_LCD_POW_OFF";
	case IOC_BAT_GET_STATUS:			return "IOC_BAT_GET_STATUS";
	case IOC_BAT_SET_PHONE_CRITICAL:	return "IOC_BAT_SET_PHONE_CRITICAL";
	case IOC_UART_SET:					return "IOC_UART_SET";
	case IOC_UART_GET:					return "IOC_UART_GET";
	case IOC_USB_SET:					return "IOC_USB_SET";
	case IOC_USB_GET:					return "IOC_USB_GET";
	case IOC_VIB_SET:					return "IOC_VIB_SET";
	case IOC_SET_PARAM:					return "IOC_SET_PARAM";
	case IOC_READ_PARAM:				return "IOC_READ_PARAM";
	case IOC_SAVE_PARAM:				return "IOC_SAVE_PARAM";
	case IOC_SND_PATH:					return "IOC_SND_PATH";
	case IOC_ACC_GET_DATA:				return "IOC_ACC_GET_DATA";
	case IOC_SET_TOUCH_INTERRUPT:		return "IOC_SET_TOUCH_INTERRUPT";
	case IOC_SET_KEYPAD_INTERRUPT:		return "IOC_SET_KEYPAD_INTERRUPT";
	case IOC_COMBO_EN_SET:				return "IOC_COMBO_EN_SET";
	case IOC_SET_IPM:					return "IOC_SET_IPM";
	case IOC_SET_USER_OOM:				return "IOC_SET_USER_OOM";
	case IOC_SEND_CHARGER_STATUS:		return "IOC_SEND_CHARGET_STATUS";
	case IOC_SET_SLEEP_MODE: 			return "IOC_SET_SLEEP_MODE";
	case IOC_SET_WAKEUP_SRC:			return "IOC_SET_WAKEUP_SRC";
	case IOC_GET_WAKEUP_SRC:			return "IOC_GET_WAKEUP_SRC";
	case IOC_GET_WAKEUP_DONE_SRC:		return "IOC_GET_WAKEUP_DONE_SRC";
	default: 					return "Unknown";
	}
}
#endif


extern int aat1231_1_lcd_backlight_get_level(void);
extern int aat1231_1_lcd_backlight_set_level(int level);
extern void lcd_power_on(void);
extern void lcd_power_off(void);
extern void aat1231_1_lcd_backlight_power_on_off(int onoff);

static int dcm_ioctl(struct inode * inode, struct file *filp, u_int cmd, u_long arg) 
{
	int err = 1;
	int level;
	int earphone = 0;
	FN_IN;

#ifdef DCM_IOCTL_DUMP
	printk("[dcm] dcm_ioctl : 0x%08x %s\n", cmd, get_cmd_string(cmd));
#endif

	switch (cmd) {
#ifdef USE_FLAG_FOR_EVENT_SYNC
	case IOC_DCM_START:
		printk( "[DCM] IOC_DCM_START arg = %d\n", (int)arg );
		dcm_clearQueue();
		dcm_start_flag = arg;
		dcm_10ms_service();
		break;
#endif
	case IOC_LED_SET:
		{
		}
		break;
	case IOC_TCH_GET_CAL:
		break;
	case IOC_TCH_SET_CAL:
		break;
	case IOC_TCH_ENABLE:
		break;
	case IOC_TCH_DISABLE:
		break;
	case IOC_LCD_GET_BRT:
		{
			level = aat1231_1_lcd_backlight_get_level();
			printk("[DCM] lcd backlight level = %d\n",level);
			if (copy_to_user((int *)arg, &level, sizeof(int)))
				err = -EFAULT;
		}
		break;
	case IOC_LCD_SET_BRT:
		{
			level = (int)arg;
			if(level == 0)
				aat1231_1_lcd_backlight_power_on_off(0);
			else
				aat1231_1_lcd_backlight_set_level(level);
			printk("[DCM] lcd backlight level = %d\n", level);
		}
		break;
	case IOC_LCD_GET_ALC:
		{
			printk("[DCM] not support IOC_LCD_GET_ALC\n");
		}
		break;
	case IOC_LCD_SET_ALC:
		{
			printk("[DCM] not support IOC_LCD_SET_ALC\n");
		}
		break;
	case IOC_LCD_GET_CABC:
		{
			printk("[DCM] not support IOC_LCD_GET_CABC\n");

		}
		break;
	case IOC_LCD_SET_CABC:
		{
			printk("[DCM] not support IOC_LCD_SET_CABC\n");
		}
		break;

	case IOC_LCD_POW_ON:
		{
			lcd_power_on();	
			printk("[DCM] lcd power on!\n");
			touchscreen_poweronoff(1);
		}
		break;
	case IOC_LCD_POW_OFF:
		{
			lcd_power_off(); 
			printk("[DCM] lcd power off!\n");
			touchscreen_poweronoff(0);
		}
		break;
	case IOC_BAT_GET_STATUS:
		{
            level = 0; 
            if (copy_to_user((int *)arg, &level, sizeof(int)))
				err =  -EFAULT;
		}
		break;
        
    case IOC_BAT_GET_SOC:
        {
            level = 0;
            if (copy_to_user((int *)arg, &level, sizeof(int)))
				err =  -EFAULT;
		}
		break;
	case IOC_BAT_SET_PHONE_CRITICAL:
		{
		}
		break;
	case IOC_UART_SET:
		{
			if ((arg < 0) || (1 < arg)) {
				return -EINVAL;
			}

	
		}
		break;
	case IOC_UART_GET:
		{
				err =  -EFAULT;
		}
		break;
	case IOC_USB_SET:
		{
			if ((arg < 0) || (1 < arg)) {
				return -EINVAL;
			}

		}
		break;
	case IOC_USB_GET:
		{
		}
		break;
	case IOC_VIB_SET:
		err = 10;
		{
		}
		break;
	case IOC_SET_PARAM:
		{
			param_int_t param_int_info;
			if (copy_from_user(&param_int_info,
					(const char __user *)arg, sizeof(param_int_t)))
				return -EFAULT;
			set_parameter_value(param_int_info.ident,
				(void *)&param_int_info.value);		
			if (save_parameter_value() < 0)
				return -EFAULT;
		}
		break;
	case IOC_READ_PARAM:
		{
			param_int_t param_int_info;
			if (copy_from_user(&param_int_info,
					(const char __user *)arg, sizeof(param_int_t)))
				return -EFAULT;
			read_parameter_value(param_int_info.ident,
				(void *)&param_int_info.value);		

#ifdef DCM_IOCTL_DUMP
			if(param_int_info.ident < (MAX_PARAM - MAX_STRING_PARAM))
				printk("[dcm] read param : id %d, value : %d\n", param_int_info.ident, param_int_info.value);
			else
				printk("[dcm] read param : id %d, value : %s\n", param_int_info.ident, param_int_info.value);
#endif

			if (copy_to_user((char __user *)arg,
					&param_int_info, sizeof(param_int_t)))
				return -EFAULT;
		}
		break;
	case IOC_SAVE_PARAM:
		{
			if (save_parameter_value() < 0) 
				return -EFAULT;
		}
		break;
#ifdef SOUND_PATH_TEST
	case IOC_SND_PATH:
		{
		}
		break;
#endif
	case IOC_ACC_GET_DATA:
		{
		}
		break;

	case IOC_SET_TOUCH_INTERRUPT:
		{	
		}
	case IOC_SET_KEYPAD_INTERRUPT:
		{
		}

		break;
	case IOC_COMBO_EN_SET:
		break;
	case IOC_SET_IPM:
		{	
		}
		break; 
	case IOC_SET_USER_OOM:
		break;
	case IOC_SEND_CHARGER_STATUS:
		break;

	case IOC_SET_SLEEP_MODE: 
		err= pm_suspend(PM_SUSPEND_MEM);
		break;
	case IOC_SET_WAKEUP_SRC:

		break;
	case IOC_GET_WAKEUP_SRC:

		break;
	case IOC_GET_WAKEUP_DONE_SRC:

		break;
	case IOC_EARPHONE_STATUS:
		{
			earphone = gpio_get_value(GPIO_DET_35);
			if (copy_to_user((int *)arg, &earphone, sizeof(int))){
				err = -EFAULT;
			}
		}
		break;
	case IOC_EARKEY_STATUS:
		{
			earphone = gpio_get_value(GPIO_EAR_SEND_END);
			if (copy_to_user((int *)arg, &earphone, sizeof(int))){
				err = -EFAULT;
			}
		}
		break;
	default:
		{
			err = -EFAULT;
		}		
		break;
	}

	FN_OUT(err);
	return err;
}

static struct file_operations dcm_fops = {
	.owner	= THIS_MODULE,
	.ioctl	= dcm_ioctl,
	.read	= dcm_read,
	.poll	= dcm_poll,
};

static struct miscdevice dcm_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DCM_NAME,
	.fops	= &dcm_fops,
};
#ifdef USE_MAX1237 
extern int max1237_init(void);
#endif

static int proc_read_nation(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n",	nation_val);

	return len;
}

static void parameter_init(void)
{
	dcm_parameter_init();

	read_parameter_value(__SWITCH_SEL, &switch_val);

	read_parameter_value(__NATION_SEL, &nation_val);

	create_proc_read_entry("nation", 0, 0, proc_read_nation, NULL);

	return;
}

static void parameter_exit(void)
{
	remove_proc_entry("nation", NULL);

	dcm_parameter_exit();

	return;
}

static int __init dcm_init(void)
{
	int ret;

	FN_IN;
    if((ret=misc_register(&dcm_device))<0)
	{
		printk("dcm module misc_register failed!\n");
		return ret;
	}	
	if (platform_driver_register(&low_memnotify_driver)) {
		printk(KERN_ERR "failed to register memnotify driver\n");
		return -ENODEV;
	}

	if (platform_device_register(&low_memnotify_device)) {
		printk(KERN_ERR "failed to register memnotify device\n");
		return -ENODEV;
	}

	parameter_init();
	
	proc_print_create();

	SendMsgToDevMgr = dcm_SendMsgToDevMgr;
#ifdef CONFIG_FAT_QUOTA
	set_notify_func(dcm_notify_quota_info);
#endif
#ifdef CONFIG_RFS_QUOTA
	register_cluster_usage_notify(dcm_notify_quota_info);
#endif
	
	FN_OUT(0);

	return 0;
}

static void __exit dcm_close(void)
{
	FN_IN;


	parameter_exit();

	misc_deregister(&dcm_device);

	platform_driver_unregister(&low_memnotify_driver);
        platform_device_unregister(&low_memnotify_device);
	FN_OUT(0);
}

module_init(dcm_init);
module_exit(dcm_close);

MODULE_AUTHOR("KyoungHOON Kim"); 
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("platform specific h/w control interface");
