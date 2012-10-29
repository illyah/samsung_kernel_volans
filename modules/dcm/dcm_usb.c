/****************************************************************************
**
** COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR		: KyoungHOON Kim (khoonk)
**
*****************************************************************************
** 2006/06/13. khoonk	1.create usb detect routine
** 2006/06/14. khoonk	1.move isr routine to usb driver
*****************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include <asm/irq.h>
#include "dcm_ext.h"

#define USB_DETECT_HOLDTIME		 3

#define USB_NAME				 "DCM USB Detect"

#define USB_CHECK_TIMER_INTERVAL 1


#ifndef CONFIG_MICROUSBIC_INTR	
	#define usb_irq IRQ_UHOST	
#else
	#include <mach/microusbic.h>
	#define usb_irq IH_USBIC_BASE
#endif


#define __USE_UDEV__

#ifdef __USE_UDEV__

#include <linux/platform_device.h>


static int usb_probe (struct platform_device *pdev)
{
}

struct platform_device* usb_platform_device = NULL;
EXPORT_SYMBOL(usb_platform_device);


static struct platform_driver usb_platform_driver = {
 .probe   = usb_probe,
 .driver  = {
  .name = "usb",
 }
};
#endif // __USE_UDEV__

DCM_DETECT_STATE  gUsb;

static struct timer_list usb_check_timer;

static const int isr_id = 0x3;

irqreturn_t usb_detect_isr( int irq, void *dev_id )
{
	int *id = (int *)dev_id;

	int is_connected = 0;


	if( *id == isr_id )
	{
#ifndef CONFIG_MICROUSBIC_INTR	
#else
		is_connected = (get_usbic_state()==MICROUSBIC_USB_CABLE) ? 1:0;
#endif

		if(is_connected)
		{
			gUsb.state 		= USB_CONNECTED;
			gUsb.high_count = 0;
		}
		else 
		{
			gUsb.state 	   = USB_DISCONNECTED;
			gUsb.low_count = 0;
		}
	
#ifndef CONFIG_MICROUSBIC_INTR	
notify_event:		
#endif
		del_timer( &usb_check_timer );
		usb_check_timer.expires = jiffies + USB_CHECK_TIMER_INTERVAL;
		add_timer( &usb_check_timer );

		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

void usb_detect_scan_svc(void)
{
	gUsb.prev_state	= UNKNOWN_STATUS;

	usb_detect_isr( usb_irq , (void *)&isr_id );
}

void usb_status_scan(void)
{
	unsigned int check_usb_status = 0;
		
	if( gUsb.prev_state != gUsb.state )
	{
		switch(gUsb.state)
		{
			case USB_CONNECTED:
				gUsb.high_count++;	
				if(gUsb.high_count > USB_DETECT_HOLDTIME) 
				{
#ifdef __USE_UDEV__	
					if(usb_platform_device != NULL)
						kobject_uevent(&usb_platform_device->dev.kobj, KOBJ_ADD);
					else
						printk(KERN_ERR "[USB][ERR] usb_status_scan() device not ready!\n"); 	   
#endif
					gUsb.prev_state = gUsb.state;
				}
				else
				{
					check_usb_status = 1;
				}
				break;
			
			case USB_DISCONNECTED:
				gUsb.low_count++;	
				if(gUsb.low_count > USB_DETECT_HOLDTIME) 
				{	
#ifdef __USE_UDEV__					
					if(usb_platform_device != NULL)
						kobject_uevent(&usb_platform_device->dev.kobj, KOBJ_REMOVE);
					else
						printk(KERN_ERR "[USB][ERR] usb_status_scan() device not ready!\n"); 	   
#endif
					gUsb.prev_state = gUsb.state;
				}
				else 
				{
					check_usb_status = 1;
				}
				break;
			default:
				break;
		}
	}

	if( check_usb_status )
	{
		usb_check_timer.expires = jiffies + USB_CHECK_TIMER_INTERVAL;
		add_timer( &usb_check_timer );
	}

}

void usb_detect_init(void)
{
	gUsb.state 		= UNKNOWN_STATUS;
	gUsb.prev_state	= UNKNOWN_STATUS;

#ifdef __USE_UDEV__
	int result;
 
	result = platform_driver_register( &usb_platform_driver );
	 
	if(result)
	{
		printk("[DCM][ERR] usb_detect_init() - fail platform_driver_register() = %d\n", result);
		return;
	}
	 
	usb_platform_device = platform_device_register_simple( "usb", -1, NULL, 0 );
	if (IS_ERR(usb_platform_device))
	{
		printk("[DCM][ERR] usb_detect_init() - fail platform_device_register_simple() = %d\n", IS_ERR(usb_platform_device));
		return;
	}
#endif	// __USE_UDEV__


#ifndef CONFIG_MICROUSBIC_INTR	
#endif

	if( request_irq( usb_irq , usb_detect_isr, 
					 IRQF_DISABLED | IRQF_SHARED, USB_NAME, (void *)&isr_id ) )
	{
		printk("[DCM] ERROR! irq %u in use.\n", usb_irq );
	}
	else
	{
		printk("[DCM] found %s adapter at irq %u\n", USB_NAME , usb_irq );
	}
	
	init_timer( &usb_check_timer );
	usb_check_timer.expires  = jiffies + USB_CHECK_TIMER_INTERVAL;
	usb_check_timer.function = (void*) usb_status_scan;

	usb_detect_scan_svc();
}

void usb_detect_exit(void)
{
	free_irq(usb_irq , (void *)&isr_id);

	del_timer( &usb_check_timer );
}
