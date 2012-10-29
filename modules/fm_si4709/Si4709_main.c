
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/wait.h>
#include <linux/stat.h>
#include <linux/ioctl.h>
#include <linux/workqueue.h>

#include "Si4709_i2c_drv.h"
#include "Si4709_dev.h"
#include "Si4709_ioctl.h"
#include "common.h"

#include<mach/gpio.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>

#define Si4709_DEBUG
 #define Si4709_IRQ IRQ_EINT(18) 
static int Si4709_open (struct inode *, struct file *);
static int Si4709_release (struct inode *, struct file *);
static int Si4709_ioctl(struct inode *, struct file *, unsigned int,  unsigned long);

static irqreturn_t Si4709_isr( int irq, void *unused );


static struct file_operations Si4709_fops =
{
    .owner = THIS_MODULE,
    .open = Si4709_open,
    .ioctl = Si4709_ioctl,
    .release = Si4709_release,
};

static struct miscdevice Si4709_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "Si4709",
    .fops = &Si4709_fops,
};

wait_queue_head_t Si4709_waitq;
wait_queue_head_t Si4709_rds_waitq;

static struct work_struct Si4709_ws;


static int Si4709_open (struct inode *inode, struct file *filp)
{
    debug("Si4709_open called");    

    return nonseekable_open(inode, filp);
}

static int Si4709_release (struct inode *inode, struct file *filp)
{
    debug("Si4709_release called");

    return 0;
}

static int Si4709_ioctl(struct inode *inode, struct file *filp, 
	                        unsigned int ioctl_cmd,  unsigned long arg)
{
    int ret = 0;
    void __user *argp = (void __user *)arg;   
	   
    if( _IOC_TYPE(ioctl_cmd) != Si4709_IOC_MAGIC )
    {
        debug("Inappropriate ioctl 1 0x%x",ioctl_cmd);
        return -ENOTTY;
    }
    if( _IOC_NR(ioctl_cmd) > Si4709_IOC_NR_MAX )
    {
        debug("Inappropriate ioctl 2 0x%x",ioctl_cmd);	
        return -ENOTTY;
    }
	
    switch (ioctl_cmd)
    {
        case Si4709_IOC_POWERUP:
            if( (ret = Si4709_dev_powerup()) < 0 )
            {
                debug("Si4709_IOC_POWERUP failed");
            }
            break;

        case Si4709_IOC_POWERDOWN:
            if( (ret = Si4709_dev_powerdown()) < 0 )
            {
                debug("Si4709_IOC_POWERDOWN failed");
            }
            break;

        case Si4709_IOC_BAND_SET:
        	   {
        	   	   int band;
        	   	   
        	   	   if( copy_from_user((void*) &band, argp, sizeof(int)) )
        	   	   {
        	   	       ret = -EFAULT;
        	   	   }
                else if( (ret = Si4709_dev_band_set(band)) < 0)
                {
                    debug("Si4709_IOC_BAND_SET failed");
                }
        	   }
            break;
            
        case Si4709_IOC_CHAN_SPACING_SET:
        	   {
        	   	   int ch_spacing;
        	   	   
        	   	   if( copy_from_user((void*) &ch_spacing, argp, sizeof(int)) )
        	   	   {
        	   	       ret = -EFAULT;
        	   	   }
                else if ((ret = Si4709_dev_ch_spacing_set(ch_spacing)) < 0)
                {
                    debug("Si4709_IOC_CHAN_SPACING_SET failed");
                }
        	   }
            break;

        case Si4709_IOC_CHAN_SELECT:
        	   {
        	   	   u32 frequency;

        	   	   if( copy_from_user((void*) &frequency, argp, sizeof(u32)) )
        	   	   {
        	   	       ret = -EFAULT;
        	   	   }
                else if ( (ret = Si4709_dev_chan_select(frequency)) < 0 )
                {
                    debug("Si4709_IOC_CHAN_SELECT failed");
                }
        	   }
            break;            
            
        case Si4709_IOC_CHAN_GET:
         	  {
        	   	   u32 frequency;
        	   	   
                if( (ret = Si4709_dev_chan_get(&frequency)) < 0)
                {
                    debug("Si4709_IOC_CHAN_GET failed");
                }
                else if( copy_to_user(argp, (void*) &frequency, sizeof(u32)) )
                {
                    ret = -EFAULT;
                }
        	   } 
            break;

        case Si4709_IOC_SEEK_UP:
            {
                u32 frequency;
             
                if( (ret = Si4709_dev_seek_up(&frequency)) < 0)
                {
                    debug("Si4709_IOC_SEEK_UP failed");
                }
                else if( copy_to_user(argp, (void*) &frequency, sizeof(u32)) )
                {
                    ret = -EFAULT;
                }
            } 
            break;

        case Si4709_IOC_SEEK_DOWN:
            {
                u32 frequency;
             
                if( (ret = Si4709_dev_seek_down(&frequency)) < 0)
                {
                    debug("Si4709_IOC_SEEK_DOWN failed");
                }
                else if( copy_to_user(argp, (void*) &frequency, sizeof(u32)) )
                {
                    ret = -EFAULT;
                }
            }   
            break;

        case Si4709_IOC_SEEK_AUTO:
        		  {
        		  	    u32 *seek_preset_user;
        		  	    int i = 0;
			debug("Si4709_IOC_SEEK_AUTO called");
                 if( (seek_preset_user = (u32 *)kzalloc(sizeof(u32) * NUM_SEEK_PRESETS, 
                 	             GFP_KERNEL)) == NULL )
                 {
                     debug("Si4709_ioctl: no memory");
                     ret = -ENOMEM;
                 }            		   
                 else
                 {
                     if((ret = Si4709_dev_seek_auto(seek_preset_user)) < 0)
                     {
                         debug("Si4709_IOC_SEEK_AUTO failed");
                     }

                     else if ( copy_to_user(argp, (u32*) seek_preset_user, sizeof(int) * NUM_SEEK_PRESETS) )
                     {
                         ret = -EFAULT;
                     }
                     
                     kfree(seek_preset_user);
                 }
        	   }
            break;

        case Si4709_IOC_RSSI_SEEK_TH_SET:
            {
                u8 RSSI_seek_th;
         
                if( copy_from_user((void*) &RSSI_seek_th, argp, sizeof(u8)) )
                {
                    ret = -EFAULT;
                }
                else if ( (ret = Si4709_dev_RSSI_seek_th_set(RSSI_seek_th)) < 0 )
                {
                    debug("Si4709_IOC_RSSI_SEEK_TH_SET failed");
                }
            }
            break;   

        case Si4709_IOC_SEEK_SNR_SET:
            {
                u8 seek_SNR_th;
         
                if( copy_from_user((void*) &seek_SNR_th, argp, sizeof(u8)) )
                {
                    ret = -EFAULT;
                }
                else if( (ret = Si4709_dev_seek_SNR_th_set(seek_SNR_th)) < 0 )
                {
                    debug("Si4709_IOC_SEEK_SNR_SET failed");
                }
            }
            break;

        case Si4709_IOC_SEEK_CNT_SET:
            {
                u8 seek_FM_ID_th;
         
                if( copy_from_user((void*) &seek_FM_ID_th, argp, sizeof(u8)) )
                {
                    ret = -EFAULT;
                }
                else if ( (ret = Si4709_dev_seek_FM_ID_th_set(seek_FM_ID_th)) < 0 )
                {
                    debug("Si4709_IOC_SEEK_CNT_SET failed");
                }
            }  
            break;

        case Si4709_IOC_CUR_RSSI_GET:
            {
                u8  curr_rssi;
             
                if( (ret = Si4709_dev_cur_RSSI_get(&curr_rssi)) < 0)
                {
                    debug("Si4709_IOC_CUR_RSSI_GET failed");
                }
                else if( copy_to_user(argp, (void*) &curr_rssi, sizeof(u8 )) )
                {
                    ret = -EFAULT;
                }
                
            }     
            break;

	  case Si4709_IOC_AFCRL_GET:
            {
                u8  curr_afcrl;
             
                if( (ret = Si4709_dev_AFCRL_get(&curr_afcrl)) < 0)
                {
                    debug("Si4709_IOC_AFCRL_GET failed");
                }
                else if( copy_to_user(argp, (void*) &curr_afcrl, sizeof(u8 )) )
                {
                    ret = -EFAULT;
                }
                
            }     
            break;

        case Si4709_IOC_VOLEXT_ENB:
        	   if( (ret = Si4709_dev_VOLEXT_ENB()) < 0 )
        	   {
        	       debug("Si4709_IOC_VOLEXT_ENB failed");
        	   }  
            break;

        case Si4709_IOC_VOLEXT_DISB:
        	   if( (ret = Si4709_dev_VOLEXT_DISB()) < 0 )
        	   {
        	       debug("Si4709_IOC_VOLEXT_DISB failed");
        	   }
            break;

        case Si4709_IOC_VOLUME_SET:
            {
                u8 volume;
         
                if( copy_from_user((void*) &volume, argp, sizeof(u8)) )
                {
                    ret = -EFAULT;
                }
                else if ( (ret = Si4709_dev_volume_set(volume)) < 0 )
                {
                    debug("Si4709_IOC_VOLUME_SET failed");
                }
            }  
            break;

        case Si4709_IOC_VOLUME_GET:
            {
                u8 volume;
             
                if( (ret = Si4709_dev_volume_get(&volume)) < 0)
                {
                    debug("Si4709_IOC_VOLUME_GET failed");
                }
                else if( copy_to_user(argp, (void*) &volume, sizeof(u8)) )
                {
                    ret = -EFAULT;
                }
            }  
            break;

        case Si4709_IOC_MUTE_ON:
        	   if( (ret = Si4709_dev_MUTE_ON()) < 0 )
        	   {
        	       debug("Si4709_IOC_MUTE_ON failed");
        	   }  
            break;

        case Si4709_IOC_MUTE_OFF:
        	   if( (ret = Si4709_dev_MUTE_OFF()) < 0 )
        	   {
        	       debug("Si4709_IOC_MUTE_OFF failed");
        	   }    
            break;

        case Si4709_IOC_MONO_SET:
        	   if( (ret = Si4709_dev_MONO_SET()) < 0 )
        	   {
        	       debug("Si4709_IOC_MONO_SET failed");
        	   }   
            break;

        case Si4709_IOC_STEREO_SET:
        	   if( (ret = Si4709_dev_STEREO_SET()) < 0 )
        	   {
        	       debug("Si4709_IOC_STEREO_SET failed");
        	   }   
            break; 

        case Si4709_IOC_RSTATE_GET:
            {
                dev_state_t dev_state;
             
                if( (ret = Si4709_dev_rstate_get(&dev_state)) < 0)
                {
                    debug("Si4709_IOC_RSTATE_GET failed");
                }
                else if( copy_to_user(argp, (void*) &dev_state, sizeof(dev_state_t)) )
                {
                    ret = -EFAULT;
                }
            }    
            break;

        case Si4709_IOC_RDS_DATA_GET:
            {
                radio_data_t data;
             
                if( (ret = Si4709_dev_RDS_data_get(&data)) < 0)
                {
                    debug("Si4709_IOC_RDS_DATA_GET failed");
                }
                else if( copy_to_user(argp, (void*) &data, sizeof(radio_data_t)) )
                {
                    ret = -EFAULT;
                }
            }     
            break;
            
        case Si4709_IOC_RDS_ENABLE:
         	 if( (ret = Si4709_dev_RDS_ENABLE()) < 0 )
        	   {
        	       debug("Si4709_IOC_STEREO_SET failed");
        	   }   
            break; 

         case Si4709_IOC_RDS_DISABLE:
         	 if( (ret = Si4709_dev_RDS_DISABLE()) < 0 )
        	   {
        	       debug("Si4709_IOC_STEREO_SET failed");
        	   }   
            break; 

          case Si4709_IOC_RDS_TIMEOUT_SET:
          		{
          			   u32  time_out;
          			   debug("Switch case:Si4709_IOC_RDS_TIMEOUT_SET called");
          			   if( copy_from_user((void*) &time_out, argp, sizeof(u32)) )
                {
                    ret = -EFAULT;
                }
                else if ( (ret = Si4709_dev_RDS_timeout_set(time_out)) < 0 )
                {
                    debug("Si4709_IOC_RDS_TIMEOUT_SET failed");
                }
          	 }    
            break; 
           
	  case Si4709_IOC_SEEK_SNR_GET:
	  	{
			u8 snr_th;
			if( (ret = Si4709_dev_seek_snr_th_get(&snr_th)) < 0)
                	{
                    		debug("Si4709_IOC_SEEK_SNR_GET failed");
                	}
                	else if( copy_to_user(argp, (void*) &snr_th, sizeof(u8)) )
                	{
                    		ret = -EFAULT;
                	}
			
	  	}
	  break;

	  case Si4709_IOC_SEEK_CNT_GET :
	  	{
			u8 cnt_th;
			if( (ret = Si4709_dev_seek_cnt_th_get(&cnt_th)) < 0)
                	{
                    		debug("Si4709_IOC_SEEK_CNT_GET failed");
                	}
                	else if( copy_to_user(argp, (void*) &cnt_th, sizeof(u8)) )
                	{
                    		ret = -EFAULT;
                	}
			
	  	}
	  break;

	  case Si4709_IOC_RSSI_SEEK_TH_GET:
	  	{
			u8 rssi_th;
			if( (ret = Si4709_dev_rssi_seek_th_get(&rssi_th)) < 0)
                	{
                    		debug("Si4709_IOC_RSSI_SEEK_TH_GET failed");
                	}
                	else if( copy_to_user(argp, (void*) &rssi_th, sizeof(u8)) )
                	{
                    		ret = -EFAULT;
                	}			 			
	  	}
	  break;
	  
           
        default:
            debug("  ioctl default");
            ret = -ENOTTY;
            break;
    }

    return ret;
}

static irqreturn_t Si4709_isr( int irq, void *unused )
{
    debug("Si4709_isr called IRQ: %d",irq);  

	if (gpio_get_value(GPIO_FM_INT) != GPIO_LEVEL_LOW)
		return IRQ_HANDLED;

    schedule_work(&Si4709_ws);

    return IRQ_HANDLED;
}





void debug_ioctls(void) 
{

    debug("------------------------------------------------");

}




int __init Si4709_driver_init(void)
{
    int ret = 0;

    debug("Si4709_driver_init called");  

    Si4709_dev_mutex_init();

    if( (ret = misc_register(&Si4709_misc_device)) < 0 )
    {
        error("Si4709_driver_init misc_register failed");
        return ret; 	  	
    }
    
    if ( (ret = Si4709_i2c_drv_init() < 0) ) 
    {
         goto MISC_DREG;
    }

    init_waitqueue_head(&Si4709_waitq);
    init_waitqueue_head(&Si4709_rds_waitq);

    INIT_WORK(&Si4709_ws, (void (*) (struct work_struct *))Si4709_dev_work_func);
	
	set_irq_type (Si4709_IRQ, IRQ_TYPE_EDGE_BOTH);

    if( (ret = request_irq(Si4709_IRQ,Si4709_isr,0 ,"Si4709", (void *)NULL)) < 0 )
    {
        error("Si4709_driver_init request_irq failed %d",Si4709_IRQ);
        goto MISC_DREG;
    }
   

    debug("Si4709_driver_init successful");  

    return ret;

MISC_DREG:
    misc_deregister(&Si4709_misc_device);
		
    return ret; 
}


void __exit Si4709_driver_exit(void)
{
    debug("Si4709_driver_exit called");  
		  
    Si4709_i2c_drv_exit();

    free_irq(Si4709_IRQ, (void *)NULL);
    
    misc_deregister(&Si4709_misc_device);
}

module_init(Si4709_driver_init);
module_exit(Si4709_driver_exit);
MODULE_AUTHOR("Varun Mahajan <m.varun@samsung.com>");
MODULE_DESCRIPTION("Si4709 FM tuner driver");
MODULE_LICENSE("GPL");


