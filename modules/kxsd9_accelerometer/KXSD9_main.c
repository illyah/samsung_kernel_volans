
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

#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/stat.h>

#include <linux/ioctl.h>

#include "KXSD9_i2c_drv.h"
#include "KXSD9_dev.h"
#include "common.h"

#include<mach/gpio.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>

typedef struct
{
    struct list_head list;
    struct mutex list_lock;
    wait_queue_head_t waitq;
}KXSD9_queue_t;

typedef struct
{
    struct list_head node;
    
    atomic_t int_flag;

    wait_queue_t waitq_entry;
}fpriv_data_t;

#define KXSD9_IRQ IRQ_EINT(3)

#define KXSD9_IOC_MAGIC  0xF6
#define KXSD9_IOC_NR_MAX 6

#define KXSD9_IOC_GET_ACC _IOR(KXSD9_IOC_MAGIC, 0, KXSD9_acc_t)

#define KXSD9_IOC_GET_SENSITIVITY _IOR(KXSD9_IOC_MAGIC, 4, KXSD9_sensitivity_t)

#define KXSD9_IOC_GET_ZERO_G_OFFSET _IOR(KXSD9_IOC_MAGIC, 5, \
                                                  KXSD9_zero_g_offset_t)

#define KXSD9_IOC_DISB_MWUP _IO(KXSD9_IOC_MAGIC, 1)

#define KXSD9_IOC_ENB_MWUP _IOW(KXSD9_IOC_MAGIC, 2, unsigned short)

#define KXSD9_IOC_MWUP_WAIT _IO(KXSD9_IOC_MAGIC, 3)

#define KXSD9_IOC_GET_DEFAULT _IOR(KXSD9_IOC_MAGIC, 6, KXSD9_acc_t)


KXSD9_module_param_t *KXSD9_main_getparam(void);
void KXSD9_waitq_wkup_processes(void);


static int KXSD9_open (struct inode *, struct file *);
static int KXSD9_release (struct inode *, struct file *);
static int KXSD9_ioctl(struct inode *, struct file *, unsigned int,  unsigned long);

static void waitq_init(void);
static void waitq_list_insert_process( fpriv_data_t * );
static void waitq_prepare_to_wait( fpriv_data_t * );
static void waitq_list_remove_process( fpriv_data_t * );
static void waitq_finish_wait( fpriv_data_t * );

static irqreturn_t KXSD9_isr( int irq, void *unused );

static struct work_struct KXSD9_ws;

static unsigned short mwup = DISABLED;
static unsigned short mwup_rsp = DONT_CARE;
static unsigned short R_T = R2g_T1g;
static unsigned short BW = BW50;

static KXSD9_module_param_t KXSD9_module_param; 

static struct file_operations KXSD9_fops =
{
    .owner = THIS_MODULE,
    .open = KXSD9_open,
    .ioctl = KXSD9_ioctl,
    .release = KXSD9_release,
};

static struct miscdevice KXSD9_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "KXSD9",
    .fops = &KXSD9_fops,
};

static KXSD9_queue_t KXSD9_waitq;


KXSD9_module_param_t *KXSD9_main_getparam(void)
{
    return &KXSD9_module_param;
}

static int KXSD9_open (struct inode *inode, struct file *filp)
{
    int ret = 0;
    
    fpriv_data_t *fpriv;
    
    debug("KXSD9_open called");    

    if( (fpriv = (fpriv_data_t *)kzalloc(sizeof(fpriv_data_t), GFP_KERNEL)) == NULL )
    {
        debug("KXSD9_open: no memory");
        ret = -ENOMEM;
    }    
    else
    {
        atomic_set( &(fpriv->int_flag), RESET );
               
        filp->private_data = fpriv;
        
        nonseekable_open(inode, filp);
    }

    return ret;
}

static int KXSD9_release (struct inode *inode, struct file *filp)
{
    fpriv_data_t *fpriv = (fpriv_data_t *)filp->private_data;
    
    debug("KXSD9_release called");

    kfree( fpriv );
    
    return 0;
}

static int KXSD9_ioctl(struct inode *inode, struct file *filp, unsigned int ioctl_cmd,  unsigned long arg)
{
    int ret = 0;
    void __user *argp = (void __user *)arg;   
	   
    if( _IOC_TYPE(ioctl_cmd) != KXSD9_IOC_MAGIC )
    {
        debug("Inappropriate ioctl 1 0x%x",ioctl_cmd);
        return -ENOTTY;
    }
    if( _IOC_NR(ioctl_cmd) > KXSD9_IOC_NR_MAX )
    {
        debug("Inappropriate ioctl 2 0x%x",ioctl_cmd);	
        return -ENOTTY;
    }
	
    switch (ioctl_cmd)
    {
        case KXSD9_IOC_GET_ACC:
            {
       	        KXSD9_acc_t *acc = NULL;

       	        debug("KXSD9 ioctl KXSD9_IOC_GET_ACC");

                if( (acc = (KXSD9_acc_t *)kzalloc(sizeof(KXSD9_acc_t), GFP_KERNEL)) == NULL )
                {
                    debug("KXSD9_ioctl: no memory");
                    ret = -ENOMEM;
                }
                else if( (ret = KXSD9_dev_get_acc(acc)) < 0 )
                {
                    debug("KXSD9_dev_get_acc failed");	
                }     
                else if( copy_to_user( argp, (const void *) acc, sizeof(KXSD9_acc_t) ) )
                {
                    ret = -EFAULT;
                }

                kfree(acc);
            }
            break;

        case KXSD9_IOC_GET_DEFAULT:
            {
       	        KXSD9_default_t *acc = NULL;

       	        debug("KXSD9 ioctl KXSD9_IOC_GET_DEFAULT");

                if( (acc = (KXSD9_default_t *)kzalloc(sizeof(KXSD9_default_t), GFP_KERNEL)) == NULL )
                {
                    debug("KXSD9_ioctl: no memory");
                    ret = -ENOMEM;
                }
                else if( (ret = KXSD9_dev_get_default(acc)) < 0 )
                {
                    debug("KXSD9_dev_get_default failed");	
                }     
                else if( copy_to_user( argp, (const void *) acc, sizeof(KXSD9_default_t) ) )
                {
                    ret = -EFAULT;
                }

                kfree(acc);
            }
            break;

        case KXSD9_IOC_GET_SENSITIVITY:
            {
       	        KXSD9_sensitivity_t *sensitivity = NULL;

       	        debug("KXSD9 ioctl KXSD9_IOC_GET_SENSITIVITY");

                if( (sensitivity = (KXSD9_sensitivity_t *)
                	    kzalloc(sizeof(KXSD9_sensitivity_t), GFP_KERNEL)) == NULL )
                {
                    debug("KXSD9_ioctl: no memory");
                    ret = -ENOMEM;
                }
                else if( (ret = KXSD9_dev_get_sensitivity(sensitivity)) < 0 )
                {
                    debug("KXSD9_dev_get_sensitivity failed");	
                }     
                else if( copy_to_user( argp, (const void *) sensitivity, sizeof(KXSD9_sensitivity_t) ) )
                {
                    ret = -EFAULT;
                }

                kfree(sensitivity);
            }
            break;

        case KXSD9_IOC_GET_ZERO_G_OFFSET:
            {
       	        KXSD9_zero_g_offset_t *offset = NULL;

       	        debug("KXSD9 ioctl KXSD9_IOC_GET_ZERO_G_OFFSET");

                if( (offset= (KXSD9_zero_g_offset_t *)
                	    kzalloc(sizeof(KXSD9_zero_g_offset_t), GFP_KERNEL)) == NULL )
                {
                    debug("KXSD9_ioctl: no memory");
                    ret = -ENOMEM;
                }
                else if( (ret = KXSD9_dev_get_zero_g_offset(offset)) < 0 )
                {
                    debug("KXSD9_dev_get_zero_g_offset failed");	
                }     
                else if( copy_to_user( argp, (const void *) offset, sizeof(KXSD9_zero_g_offset_t) ) )
                {
                    ret = -EFAULT;
                }

                kfree(offset);
            }
            break;            

        case KXSD9_IOC_DISB_MWUP:
        	
        	   debug("KXSD9 ioctl KXSD9_IOC_DISB_MWUP");

            ret = KXSD9_dev_mwup_disb();

	          break;

        case KXSD9_IOC_ENB_MWUP:
        	
        	   debug("KXSD9 ioctl KXSD9_IOC_ENB_MWUP");
            {
            	     unsigned short mwup_rsp;
            	    
                 if( copy_from_user( (void*) &mwup_rsp, argp, sizeof(int) ) )
                 {
                     ret = -EFAULT;
                 }
                 else
                 {
                     ret = KXSD9_dev_mwup_enb(mwup_rsp);
                 }           	    
        	   	}
	          break;

        case KXSD9_IOC_MWUP_WAIT:
        	
        	   debug("KXSD9 ioctl KXSD9_IOC_MWUP_WAIT");

            if( KXSD9_dev_mwup_status() != ENABLED )
        	   	{
        	   	    debug("motion wakeup feature is disabled");
        	   	    ret = -1;
        	   	}
        	   else
            {
            	    fpriv_data_t *fpriv = (fpriv_data_t *) filp->private_data;

                init_wait( &(fpriv->waitq_entry) );
            	    
                waitq_list_insert_process(fpriv);
                waitq_prepare_to_wait(fpriv);

                if( atomic_read( &(fpriv->int_flag) ) == RESET )
                {
                    schedule();
                }
                
                waitq_list_remove_process(fpriv);
                waitq_finish_wait(fpriv);

                atomic_set(&(fpriv->int_flag), RESET);
            }

	          break;
			
         default:
             debug("  ioctl default");
             ret = -ENOTTY;
             break;
    }
       
    return ret;
}

static irqreturn_t KXSD9_isr( int irq, void *unused )
{
    debug("KXSD9_isr called IRQ: %d",irq);  

    schedule_work(&KXSD9_ws);
    	
    return IRQ_HANDLED;
}

static void waitq_init(void)
{
    debug("waitq_init called");
    
    mutex_init( &(KXSD9_waitq.list_lock) );
    init_waitqueue_head( &(KXSD9_waitq.waitq) );
    INIT_LIST_HEAD( &(KXSD9_waitq.list) );
}

static void waitq_list_insert_process( fpriv_data_t *fpriv )
{
    debug("waitq_list_insert_process called");

    mutex_lock( &(KXSD9_waitq.list_lock) );
    list_add_tail(&(fpriv->node), &(KXSD9_waitq.list));
    mutex_unlock( &(KXSD9_waitq.list_lock) );
}

static void waitq_prepare_to_wait( fpriv_data_t *fpriv )
{
    debug("waitq_prepare_to_wait called");
    
    prepare_to_wait(&(KXSD9_waitq.waitq), &(fpriv->waitq_entry), TASK_INTERRUPTIBLE);
}

static void waitq_list_remove_process( fpriv_data_t *fpriv )
{
    debug("waitq_list_remove_process called");
    
    mutex_lock( &(KXSD9_waitq.list_lock) );
    list_del_init( &(fpriv->node) );
    mutex_unlock( &(KXSD9_waitq.list_lock) );
}

static void waitq_finish_wait( fpriv_data_t *fpriv )
{
    debug("waitq_finish_wait called");

    finish_wait(&(KXSD9_waitq.waitq), &(fpriv->waitq_entry));
}

void KXSD9_waitq_wkup_processes(void)
{
    struct list_head *node;
    fpriv_data_t *fpriv;

    debug("KXSD9_waitq_wkup_processes called");

    mutex_lock( &(KXSD9_waitq.list_lock) );

    list_for_each( node, &(KXSD9_waitq.list) )
    {
        debug("KXSD9_waitq_wkup_processes iteration");
    
        fpriv = list_entry(node, fpriv_data_t, node);
        atomic_set( &(fpriv->int_flag), SET );
    }

    mutex_unlock( &(KXSD9_waitq.list_lock) );

    wake_up_interruptible( &(KXSD9_waitq.waitq) );
}


int __init KXSD9_driver_init(void)
{
    int ret = 0;

    debug("KXSD9_driver_init called");  

    KXSD9_module_param.mwup = mwup;
    KXSD9_module_param.mwup_rsp = mwup_rsp;
    KXSD9_module_param.R_T = R_T;
    KXSD9_module_param.BW = BW;	

    debug("param: mwup = %d, mwup_rsp = %d, R_T = %d, BW = %d",
   	        mwup,mwup_rsp,R_T,BW);	

    KXSD9_dev_mutex_init();

    waitq_init();
	   
    if( (ret = misc_register(&KXSD9_misc_device)) < 0 )
    {
        error("KXSD9_driver_init misc_register failed");
        return ret; 	  	
    }

    INIT_WORK(&KXSD9_ws, (void (*) (struct work_struct *))KXSD9_dev_work_func);

    set_irq_type ( KXSD9_IRQ, IRQF_TRIGGER_RISING );

    if( (ret = request_irq(KXSD9_IRQ,KXSD9_isr,0 ,"KXSD9", (void *)NULL)) < 0 )
    {
        error("KXSD9_driver_init request_irq failed %d",KXSD9_IRQ);
        goto MISC_DREG;
    }
    if ( (ret = KXSD9_i2c_drv_init() < 0) ) 
    {
         goto MISC_IRQ_DREG;
    }

    debug("KXSD9_driver_init successful");  

    return ret;

MISC_IRQ_DREG:
    free_irq(KXSD9_IRQ, (void *)NULL);

MISC_DREG:
    misc_deregister(&KXSD9_misc_device);
		
    return ret; 
}


void __exit KXSD9_driver_exit(void)
{
    debug("KXSD9_driver_exit called");  
		  
    KXSD9_i2c_drv_exit();

    free_irq(KXSD9_IRQ, (void *)NULL);
    
    misc_deregister(&KXSD9_misc_device);
}

module_param(mwup, ushort, S_IRUGO|S_IWUSR);
module_param(mwup_rsp, ushort, S_IRUGO|S_IWUSR);
module_param(R_T, ushort, S_IRUGO|S_IWUSR);
module_param(BW, ushort, S_IRUGO|S_IWUSR);

module_init(KXSD9_driver_init);
module_exit(KXSD9_driver_exit);
MODULE_AUTHOR("Varun Mahajan <m.varun@samsung.com>");
MODULE_DESCRIPTION("KXSD9 accelerometer driver");
MODULE_LICENSE("GPL");


