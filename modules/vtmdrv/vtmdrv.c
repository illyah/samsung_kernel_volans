/*
** =========================================================================
** File:
**     vtmdrv.c
**
** Description: 
**     VibeTonz Kernel Module main entry-point.
**
** Portions Copyright (c) 2008 Immersion Corporation. All Rights Reserved. 
**
** This file contains Original Code and/or Modifications of Original Code 
** as defined in and that are subject to the GNU Public License v2 - 
** (the 'License'). You may not use this file except in compliance with the 
** License. You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact 
** VibeTonzSales@immersion.com.
**
** The Original Code and all software distributed under the License are 
** distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
** INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
** FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see 
** the License for the specific language governing rights and limitations 
** under the License.
** =========================================================================
*/

#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>
#include "vtmdrv.h"
#include "ImmVibeSPI.c"

typedef struct
{
    int  state;
    struct  mutex lock;
}timer_state_t;

#define TIMER_ENABLED    1
#define TIMER_DISABLED   0

timer_state_t timer_state;

#define VERSION_STR " v2.0.92.2\n"                  
#define VERSION_STR_LEN 16                          
static char g_szDeviceName[   VIBE_MAX_DEVICE_NAME_LENGTH 
                            + VERSION_STR_LEN];     
static size_t g_cchDeviceName;                     

static char g_bIsPlaying = false;

#ifdef QA_TEST
#define FORCE_LOG_BUFFER_SIZE   128
#define TIME_INCREMENT          5
static int g_nTime = 0;
static int g_nForceLogIndex = 0;
static VibeInt8 g_nForceLog[FORCE_LOG_BUFFER_SIZE];
#endif

#if ((LINUX_VERSION_CODE & 0xFFFF00) < KERNEL_VERSION(2,6,0))
#error Unsupported Kernel version
#endif

#ifdef IMPLEMENT_AS_CHAR_DRIVER
static int g_nMajor = 0;
#endif

#include "VibeOSKernelLinuxTime.c"

static int open(struct inode *inode, struct file *file);
static int release(struct inode *inode, struct file *file);
static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos);
static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos);
static int ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static struct file_operations fops = 
{
    .owner =    THIS_MODULE,
    .read =     read,
    .write =    write,
    .ioctl =    ioctl,
    .open =     open,
    .release =  release
};

#ifndef IMPLEMENT_AS_CHAR_DRIVER
static struct miscdevice miscdev = 
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     MODULE_NAME,
	.fops =     &fops
};

#endif

static int suspend(struct platform_device *pdev, pm_message_t state);
static int resume(struct platform_device *pdev);
static struct platform_driver platdrv = 
{
    .suspend =  suspend,	
    .resume =   resume,	
    .driver = 
    {		
        .name = MODULE_NAME,	
    },	
};

static void platform_release(struct device *dev);
static struct platform_device platdev = 
{	
	.name =     MODULE_NAME,	
	.id =       -1,                    
	.dev = 
    {
		.platform_data = NULL, 		
		.release = platform_release,    
	},
};

MODULE_AUTHOR("singh.madhav@samsung.com");
MODULE_DESCRIPTION("VibeTonz Kernel Module");
MODULE_LICENSE("GPL v2");

int init_module(void)
{
    int nRet;   /* initialized below */
    DbgOut((KERN_INFO "vtmdrv: init_module.\n"));
    mutex_init(&(timer_state.lock));
    timer_state.state = TIMER_DISABLED;

#ifdef IMPLEMENT_AS_CHAR_DRIVER
    g_nMajor = register_chrdev(0, MODULE_NAME, &fops);
    if (g_nMajor < 0) 
    {
        DbgOut((KERN_ERR "vtmdrv: can't get major number.\n"));
        return g_nMajor;
    }
#else
    nRet = misc_register(&miscdev);
	if (nRet) 
    {
        DbgOut((KERN_ERR "vtmdrv: misc_register failed.\n"));
		return nRet;
	}
#endif

	nRet = platform_device_register(&platdev);
	if (nRet) 
    {
        DbgOut((KERN_ERR "vtmdrv: platform_device_register failed.\n"));
    }

	nRet = platform_driver_register(&platdrv);
	if (nRet) 
    {
        DbgOut((KERN_ERR "vtmdrv: platform_driver_register failed.\n"));
    }

    ImmVibeSPI_ForceOut_Initialize();
    VibeOSKernelLinuxInitTimer();

    /* Get device name */
    ImmVibeSPI_Device_GetName(g_szDeviceName, VIBE_MAX_DEVICE_NAME_LENGTH);

    /* Append version information and get buffer length */
    strcat(g_szDeviceName, VERSION_STR);
    g_cchDeviceName = strlen(g_szDeviceName);

    return 0;
}

void cleanup_module(void)
{
    DbgOut((KERN_INFO "vtmdrv: cleanup_module.\n"));

    VibeOSKernelLinuxTerminateTimer();
    ImmVibeSPI_ForceOut_Terminate();

	platform_driver_unregister(&platdrv);
	platform_device_unregister(&platdev);

#ifdef IMPLEMENT_AS_CHAR_DRIVER
    unregister_chrdev(g_nMajor, MODULE_NAME);
#else
    misc_deregister(&miscdev);
#endif
}

static int open(struct inode *inode, struct file *file) 
{
    DbgOut((KERN_INFO "vtmdrv: open.\n"));
    if (!try_module_get(THIS_MODULE)) return -ENODEV;
    return 0; 
}

static int release(struct inode *inode, struct file *file) 
{
    DbgOut((KERN_INFO "vtmdrv: release.\n"));

    /* 
    ** Reset force and stop timer when the driver is closed, to make sure
    ** no dangling semaphore remains in the system, especially when the
    ** driver is run outside of immvibed for testing purposes.
    */
    VibeOSKernelLinuxStopTimer();
    module_put(THIS_MODULE);
    return 0; 
}

static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    const size_t nBufSize = (g_cchDeviceName > (size_t)(*ppos)) ? min(count, g_cchDeviceName - (size_t)(*ppos)) : 0;
    /* End of buffer, exit */
    if (0 == nBufSize) return 0;

    if (0 != copy_to_user(buf, g_szDeviceName + (*ppos), nBufSize)) 
    {
        /* Failed to copy all the data, exit */
        DbgOut((KERN_ERR "vtmdrv: copy_to_user failed.\n"));
        return 0;
    }

    /* Update file position and return copied buffer size */
    *ppos += nBufSize;
    return nBufSize;
}

static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    char cBuffer[1];
    mutex_lock(&(timer_state.lock));
    if(timer_state.state==TIMER_DISABLED)
    {
       GPtimer_enable();
       timer_state.state = TIMER_ENABLED;
    }
    mutex_unlock(&(timer_state.lock));
   
    *ppos = 0;  /* file position not used, always set to 0 */

    /* 
    ** Prevent unauthorized caller to write data. 
    ** VibeTonz service is the only valid caller.
    */
    if (file->private_data != (void*)VTMDRV_MAGIC_NUMBER) 
    {
        DbgOut((KERN_ERR "vtmdrv: unauthorized write.\n"));
        return 0;
    }

    /* Check buffer size */
    if (count != 1) 
    {
        DbgOut((KERN_ERR "vtmdrv: invalid write buffer size.\n"));
        return 0;
    }

    if (0 != copy_from_user(cBuffer, buf, count)) 
    {
        /* Failed to copy all the data, exit */
        DbgOut((KERN_ERR "vtmdrv: copy_from_user failed.\n"));
        return 0;
    }

    g_bIsPlaying = true;

    ImmVibeSPI_ForceOut_Set(cBuffer[0]);

#ifdef QA_TEST
    g_nForceLog[g_nForceLogIndex++] = cBuffer[0];
    if (g_nForceLogIndex >= FORCE_LOG_BUFFER_SIZE)
    {
        int i;
        for (i=0; i<FORCE_LOG_BUFFER_SIZE; i++)
        {
            printk("<6>%d\t%d\n", g_nTime, g_nForceLog[i]);
            g_nTime += TIME_INCREMENT;
        }
        g_nForceLogIndex = 0;
    }
#endif
#if 1
    /* Start the timer after receiving new output force */
    VibeOSKernelLinuxStartTimer();
#endif

    return count;
}

static int ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
        case VTMDRV_STOP_KERNEL_TIMER:
         {
          ImmVibeSPI_ForceOut_Set(0); /* just in case ... */
          VibeOSKernelLinuxStopTimer();
          mutex_lock(&(timer_state.lock));
          if(timer_state.state==TIMER_ENABLED)
            {
              GPtimer_disable();
              timer_state.state = TIMER_DISABLED;
             }
           mutex_unlock(&(timer_state.lock));
#if 0            
          VibeOSKernelLinuxStopTimer();
#endif          
          g_bIsPlaying = false;

#ifdef QA_TEST
            if (g_nForceLogIndex)
            {
                int i;
                for (i=0; i<g_nForceLogIndex; i++)
                {
                    printk("<6>%d\t%d\n", g_nTime, g_nForceLog[i]);
                    g_nTime += TIME_INCREMENT;
                }
            }
            g_nTime = 0;
            g_nForceLogIndex = 0;
#endif
           }
            break;

        case VTMDRV_IDENTIFY_CALLER:
            if (VTMDRV_MAGIC_NUMBER == arg) file->private_data = (void*)VTMDRV_MAGIC_NUMBER;
            break;
    }

    return 0;
}

static int suspend(struct platform_device *pdev, pm_message_t state) 
{
    if (g_bIsPlaying)
    {
        DbgOut((KERN_INFO "vtmdrv: can't suspend, still playing effects.\n"));
        return -EBUSY;
    }
    else
    {
        DbgOut((KERN_INFO "vtmdrv: suspend.\n"));
        return 0;
    }
}

static int resume(struct platform_device *pdev) 
{	
    DbgOut((KERN_INFO "vtmdrv: resume.\n"));
	
	s3c_pwm_timer_clk();
	s3c_pwm_timer_reload_on();
	s3c_pwm_timer_prescaler_set();
    	return 0;   /* can resume */
}

static void platform_release(struct device *dev) 
{	
    DbgOut((KERN_INFO "vtmdrv: platform_release.\n"));
}
