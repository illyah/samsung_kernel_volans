#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <asm/irq.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <plat/regs-gpio.h>
#include <linux/i2c/pmic.h>



extern int usbic_i2c_init(void);
extern void usbic_i2c_exit(void);
extern int usbic_i2c_read_byte(u8 reg, u8 *val);
extern int usbic_i2c_write_byte(u8 reg, u8 val);

#define FAIRCHILD_I2C_SLAVE_ADDRESS                 0x4A 

#define FSA9480_I2C_REG_DEVICE_ID                   0x01
#define FSA9480_I2C_REG_CONTROL                     0x02
#define FSA9480_I2C_REG_INT_STATUS1                 0x03 
#define FSA9480_I2C_REG_INT_STATUS2                 0x04
#define FSA9480_I2C_REG_INT_MASK1                   0x05 
#define FSA9480_I2C_REG_INT_MASK2                   0x06 
#define FSA9480_I2C_REG_ADC                         0x07 
#define FSA9480_I2C_REG_TIMMING1                    0x08
#define FSA9480_I2C_REG_TIMMING2                    0x09
#define FSA9480_I2C_REG_DEVICE1                     0x0a
#define FSA9480_I2C_REG_DEVICE2                     0x0b
#define FSA9480_I2C_REG_MANUAL_SW1                  0x13
#define FSA9480_I2C_REG_MANUAL_SW2                  0x14 

#define FSA9480_I2C_REG_TIME_DELAY                  0x22
#define FSA9480_I2C_REG_DEVICE_MODE                 0x23
#define FSA9480_USB_CONNECTED                       0x01
#define FSA9480_USB_DISCONNECTED                    0x00 

#define IRQ_FSA9480                                 IRQ_EINT(9)       

#define FSA9480_DEVICE_AUDIO1                       0x01<<0
#define FSA9480_DEVICE_AUDIO2                       0x01<<1
#define FSA9480_DEVICE_USB                          0x01<<2
#define FSA9480_DEVICE_UART                         0x01<<3
#define FSA9480_DEVICE_CARKIT                       0x01<<4
#define FSA9480_DEVICE_USB_CHARGER                  0x01<<5
#define FSA9480_DEVICE_CHARGER                      0x01<<6
#define FSA9480_DEVICE_USBOTG                       0x01<<7 

#define FSA9480_DEVICE_JIG_USB_ON                   0x01<<0
#define FSA9480_DEVICE_JIG_USB_OFF                  0x01<<1
#define FSA9480_DEVICE_JIG_UART_ON                  0x01<<2
#define FSA9480_DEVICE_JIG_UART_OFF                 0x01<<3
#define FSA9480_DEVICE_PPD                          0x01<<4
#define FSA9480_DEVICE_TTY                          0x01<<5
#define FSA9480_DEVICE_AV                           0x01<<6

#define FSA9480_INT1_ATTACH                         0x01<<0
#define FSA9480_INT1_DETACH                         0x01<<1


#define FSA9480_SW1_USB_PATH_PDA_S1                 0x01<<5
#define FSA9480_SW1_USB_PATH_PDA_S2                 0x01<<2
#define FSA9480_SW1_USB_PATH_MODEM_S1               0x04<<5
#define FSA9480_SW1_USB_PATH_MODEM_S2               0x04<<2
#define FSA9480_SW1_VBUS_CHARGER                    0x01<<0

#define FSA9480_SW2_CHARGER_DET                     0x01<<4
#define FSA9480_SW2_BOOT_SW                         0x01<<3
#define FSA9480_SW2_JIG_ON                          0x01<<2

#define FSA9480_CTRL_AUTO_ACCESSORY                 0x01<<4
#define FSA9480_CTRL_DISABLE_RAW_DATA               0x01<<3
#define FSA9480_CTRL_AUTO_SWITCH                    0x01<<2
#define FSA9480_CTRL_WAIT                           0x01<<1
#define FSA9480_CTRL_INT_MASK                       0x01<<0

#define FSA9480_DEVICE_MODE_ACTIVE                  0x01<<1

#define USB_PATH_PDA                                1
#define USB_PATH_MODEM                              0

#define DEVICE_DISCONNECT                           0 
#define DEVICE_USB_ATTACH                           1
#define DEVICE_CHARGER_ATTACH                       2
#define DEVICE_JIG_UART_ATTACH                      3

u8 usb_status = FSA9480_USB_DISCONNECTED; 
u8 attDevice = DEVICE_DISCONNECT; 

struct work_struct usbic_workqueue; 

extern int usbsel;	
static struct platform_device *usb_ic_device = NULL;


static u32 usbic_device_init(void); 
static irqreturn_t usbic_irq_handler(int irq, void *dev_id);
static void usbic_workqueue_handler(struct work_struct * work);

static int volans_usb_ic_probe (struct platform_device *dev);
static int volans_usb_ic_remove (struct platform_device *dev);
#ifdef CONFIG_PM
static int volans_usb_ic_suspend (struct platform_device *dev, pm_message_t state);
static int volans_usb_ic_resume (struct platform_device *dev);
#endif 

extern int (*connected_device_status)(void);
static void volans_usb_power_control(int ctl)
{
	pmic_power_control(POWER_USB_OTGi, ctl);
	pmic_power_control(POWER_USB_OTG, ctl);
}

int volans_get_charger_status(void)
{
    return attDevice; 
}
EXPORT_SYMBOL(volans_get_charger_status); 



static int usbic_set_usb_path(u32 usbpathValue)
{
    u8 control = 0;
    u8 swValue = 0; 
    u8 i2cRead = 0;
    usbic_i2c_read_byte(FSA9480_I2C_REG_CONTROL, &control); 
    if (usbpathValue == USB_PATH_PDA) {	
		volans_usb_power_control(PMIC_POWER_ON);

        gpio_set_value(GPIO_CP_USB_ON,GPIO_LEVEL_LOW);
		control |= FSA9480_CTRL_AUTO_SWITCH;
		usbic_i2c_write_byte(FSA9480_I2C_REG_CONTROL,control);
        usbic_i2c_read_byte(FSA9480_I2C_REG_DEVICE1,&i2cRead); 
        if(i2cRead & FSA9480_DEVICE_USB){
            printk("FSA9480 : usb connected .. \n"); 
            usb_status = FSA9480_USB_CONNECTED; 
            kobject_uevent(&usb_ic_device->dev.kobj, KOBJ_ADD);
        }
        usbsel = USB_PATH_PDA;
	}

	else {		
		volans_usb_power_control(PMIC_POWER_OFF);

        gpio_set_value(GPIO_CP_USB_ON,GPIO_LEVEL_HIGH);
        swValue = 0x49;
        printk("Lunch swValue %x\n",swValue);
        usbic_i2c_write_byte(FSA9480_I2C_REG_MANUAL_SW1,swValue);
        control &= ~(FSA9480_CTRL_AUTO_SWITCH);
		usbic_i2c_write_byte(FSA9480_I2C_REG_CONTROL,control);
        kobject_uevent(&usb_ic_device->dev.kobj, KOBJ_REMOVE);
        usbsel = USB_PATH_MODEM;
	}
}


u32 interrupt_count = 0;

static u32 usbic_device_init(void)
{
    int result; 
    u8 i2cRead = 0;
    u8 control = 0;
    u8 temp; 
    u8 device_mode = 0;

    u32 filterstatus;
    s3c_gpio_cfgpin(GPIO_INTB, GPIO_INTB_AF);
	s3c_gpio_setpull(GPIO_INTB, S3C_GPIO_PULL_DOWN);
    set_irq_type(IRQ_FSA9480,IRQ_TYPE_EDGE_FALLING);
    INIT_WORK(&usbic_workqueue,usbic_workqueue_handler);
    
    filterstatus = __raw_readl(S3C64XX_EINT0FLTCON1);
    printk("[MICROUSBIC] init_hw_setting filter value : %x\n",filterstatus);
    filterstatus |= 0x01<<7; 
    __raw_writel(filterstatus,S3C64XX_EINT0FLTCON1);
    
    temp = 0; 
    usbic_i2c_read_byte(FSA9480_I2C_REG_TIME_DELAY,&temp);
    temp = (temp&0xf0)|0x07;  
    usbic_i2c_write_byte(FSA9480_I2C_REG_TIME_DELAY,temp);
    
    usbic_i2c_write_byte(FSA9480_I2C_REG_INT_MASK1,~(FSA9480_INT1_ATTACH|FSA9480_INT1_DETACH));
    usbic_i2c_write_byte(FSA9480_I2C_REG_INT_MASK2,0xff);

    usbic_i2c_read_byte(FSA9480_I2C_REG_CONTROL,&control);
    control &= ~(FSA9480_CTRL_INT_MASK);                            
    usbic_i2c_write_byte(FSA9480_I2C_REG_CONTROL,control);
   
    result = request_irq(IRQ_FSA9480, usbic_irq_handler,IRQF_DISABLED, "FSA9480", NULL);
    if (result) {
	    printk("fsa9480 request irq failed.. \n"); 
        return -1; 
    }
    return 0;
}


static void usbic_workqueue_handler(struct work_struct * work)
{
    u8 interruptStatus1;
    u8 interruptStatus2; 
    u8 deviceStatus1; 
    u8 deviceStatus2; 
    u32 status; 
    u8 device_mode;
    
    printk("[USB IC] workqueue start .. \n");
   
    status = gpio_get_value(GPIO_INTB);
    printk("[USB IC] INTB value : %x\n",status);
    
    if (status) {
        return ;
    }

    usbic_i2c_read_byte(FSA9480_I2C_REG_INT_STATUS1,&interruptStatus1);
    usbic_i2c_read_byte(FSA9480_I2C_REG_INT_STATUS2,&interruptStatus2);
    
    interruptStatus1 &= (FSA9480_INT1_ATTACH|FSA9480_INT1_DETACH);
    printk("[USB IC]interrupt status : %d\n",interruptStatus1);
    
    if(interruptStatus1 == FSA9480_INT1_ATTACH) {
        printk("[USB IC] Accessary Attached \n");
        usbic_i2c_read_byte(FSA9480_I2C_REG_DEVICE1,&deviceStatus1); 
        usbic_i2c_read_byte(FSA9480_I2C_REG_DEVICE2,&deviceStatus2);
        printk("[USB IC] Device 1 : %x\n",deviceStatus1);
        printk("[USB IC] Device 2 : %x\n",deviceStatus2);
        
        if (deviceStatus1 == FSA9480_DEVICE_USB){
            if (usbsel == USB_PATH_PDA) {
                volans_usb_power_control(PMIC_POWER_ON);
                kobject_uevent(&usb_ic_device->dev.kobj, KOBJ_ADD);
                usb_status = FSA9480_USB_CONNECTED; 
            }
            printk("[USB IC] USB Attached .. \n");
            attDevice = DEVICE_USB_ATTACH; 
        }
        else if (deviceStatus1 == FSA9480_DEVICE_CHARGER) {
            printk("[USB IC] Dedicated Charger Attached .. \n");
            attDevice = DEVICE_CHARGER_ATTACH; 
        }
        else if (deviceStatus1 == FSA9480_DEVICE_CARKIT) {
            printk("[USB IC] Dedicate 5W Charger Attached .. \n");
            attDevice = DEVICE_CHARGER_ATTACH; 
        }
        else if (deviceStatus2 == FSA9480_DEVICE_JIG_UART_OFF) {
            printk("[USB IC] JIG UART OFF Attached .. \n");
            kobject_uevent(&usb_ic_device->dev.kobj, KOBJ_CHANGE);
            attDevice = DEVICE_JIG_UART_ATTACH; 
        }
        else {
            printk("[USB IC] Undefied Device Attached .. \n");
        }
    }
    
    else if (interruptStatus1 == FSA9480_INT1_DETACH){
        printk("[USB IC] Accessary Detached \n");
        if (usb_status == FSA9480_USB_CONNECTED) {
            printk("[USB IC] USB Detached\n");
            kobject_uevent(&usb_ic_device->dev.kobj, KOBJ_REMOVE);
            usb_status = FSA9480_USB_DISCONNECTED; 
			/* added by hobac. */
			volans_usb_power_control(PMIC_POWER_OFF);
        }
        else if (attDevice == DEVICE_CHARGER_ATTACH) {
            printk("[USB IC] Dedicate Charger Detached .. \n");         
            attDevice = DEVICE_DISCONNECT; 
        }
        else if (attDevice == DEVICE_JIG_UART_ATTACH) {
            printk("[USB IC] JIG UART OFF Detached .. \n");
            kobject_uevent(&usb_ic_device->dev.kobj, KOBJ_CHANGE);
            attDevice = DEVICE_DISCONNECT; 
        }
        else {
            printk("Error.. Undefined Device Detached.. \n"); 
        }
    }
}


static irqreturn_t usbic_irq_handler(int irq, void *dev_id)
{
	schedule_work(&usbic_workqueue);
	return IRQ_HANDLED;
}


static ssize_t usbic_usbstatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{   
    return sprintf(buf,"%d",usb_status);
}
    
static struct device_attribute usbstatus = 
                    __ATTR(usb-status,S_IRUGO,usbic_usbstatus_show,NULL);

static ssize_t usbic_jigstatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{   
    return sprintf(buf,"%d",attDevice == DEVICE_JIG_UART_ATTACH ? 1 : 0);
}
    
static struct device_attribute jigstatus = 
                    __ATTR(jig-status,S_IRUGO,usbic_jigstatus_show,NULL);



static ssize_t usbic_usbpath_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf,"%s",usbsel ? "PDA" : "MODEM");
}

static ssize_t usbic_devicestatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{   
    u8 deviceStatus1; 

    usbic_i2c_read_byte(FSA9480_I2C_REG_DEVICE1,&deviceStatus1); 
    return sprintf(buf,"%02x\n",deviceStatus1);
}
    
static struct device_attribute devicestatus = 
                    __ATTR(device-status,S_IRUGO,usbic_devicestatus_show,NULL);

static ssize_t usbic_usbpath_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (strcmp(buf,"PDA") == 0){
        printk("set usb path to PDA \n"); 
        if (usbsel != USB_PATH_PDA){
            usbic_set_usb_path(USB_PATH_PDA);
            printk("set Done.. \n");
        }

    }
    else if (strcmp(buf,"MODEM") == 0){
        printk("set usb path to MODEM \n"); 
        if (usbsel != USB_PATH_MODEM){
            usbic_set_usb_path(USB_PATH_MODEM);
            printk("set Done.. \n");
        }
    }
    else{
        printk("Undefied value .. \n"); 
    }
    return count; 
    
}

static struct device_attribute usbpath = 
                __ATTR(usb-path,S_IRUGO|S_IWUSR,usbic_usbpath_show,usbic_usbpath_store);

static struct attribute *usbic_sysfs_attrs[] = {
	&usbstatus.attr,
    &usbpath.attr,
    &jigstatus.attr,
	&devicestatus.attr,
	NULL,
};


static struct attribute_group attr_usbic_sysfs_group = {
    .name = "usbic",
	.attrs = usbic_sysfs_attrs,
};

static struct platform_driver volans_usb_ic_driver = {
    .probe      = volans_usb_ic_probe,
    .remove     = volans_usb_ic_remove, 
#ifdef CONFIG_PM 
    .suspend    = volans_usb_ic_suspend, 
    .resume     = volans_usb_ic_resume, 
#else 
    .suspend    = NULL,
    .resume     = NULL,    
#endif 
    .driver = {
        .name = "usb-ic",
    },
};


static int volans_usb_ic_probe (struct platform_device *dev)
{
    int result; 
    u8 status1, status2; 
    printk("volans_usb_ic_probe() called \n"); 
    if (usbic_i2c_init() != 0){
        printk("fsa9480 i2c init failed .. \n");
        goto usbic_init_fail1;
    }
    if (usbic_device_init() != 0){
        printk("fsa9480 init failed .. \n"); 
        goto usbic_init_fail2; 
    }
    
    usbic_i2c_read_byte(FSA9480_I2C_REG_DEVICE1,&status1); 
    usbic_i2c_read_byte(FSA9480_I2C_REG_DEVICE2,&status2);
    if ((status1 != 0)|((status2 != 0))) {
        if (status1 & FSA9480_DEVICE_USB){
            if (usbsel == USB_PATH_PDA) { 
				volans_usb_power_control(PMIC_POWER_ON);
                usb_status = FSA9480_USB_CONNECTED; 
            }
            attDevice = DEVICE_USB_ATTACH; 
        }

        else if (status1 & FSA9480_DEVICE_CHARGER){
            attDevice = DEVICE_CHARGER_ATTACH; 
        }

		else if (status1 & FSA9480_DEVICE_CARKIT) {
			attDevice = DEVICE_CHARGER_ATTACH;
		}

        else if (status2 & FSA9480_DEVICE_JIG_UART_OFF){
            attDevice = DEVICE_JIG_UART_ATTACH; 
			volans_usb_power_control(PMIC_POWER_ON);
        }
    }
    usb_ic_device = dev; 
    sysfs_create_group(&usb_ic_device->dev.kobj,&attr_usbic_sysfs_group); 

	connected_device_status = volans_get_charger_status;
    return 0;
    
usbic_init_fail2 : 
    usbic_i2c_exit(); 
usbic_init_fail1 :     
    platform_driver_unregister(&volans_usb_ic_driver);   
    return -1; 
}

static int volans_usb_ic_remove (struct platform_device *dev)
{
    free_irq(IRQ_FSA9480,NULL);
    printk("usbic_exit() called \n"); 
    usbic_i2c_exit(); 
    platform_driver_unregister(&volans_usb_ic_driver);   

	connected_device_status = NULL;
}

#ifdef CONFIG_PM
static int volans_usb_ic_suspend (struct platform_device *dev, pm_message_t state)
{
    return 0; 
}

static int volans_usb_ic_resume (struct platform_device *dev)
{
    printk("volans_usb_ic_resume called \n");
    schedule_work(&usbic_workqueue);
    return 0; 
}
#endif 



int usbic_init(void)
{
   return platform_driver_register(&volans_usb_ic_driver);
}


void usbic_exit(void)
{
    platform_driver_unregister(&volans_usb_ic_driver);   
}


module_init(usbic_init);
module_exit(usbic_exit); 

MODULE_AUTHOR("Min Woo, KIM <minwoo7945.kim@samsung.com>");
MODULE_DESCRIPTION("Voalns project microusb ic driver "); 
MODULE_LICENSE("GPL"); 

