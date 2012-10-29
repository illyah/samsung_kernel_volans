/*
** =========================================================================
** File:
**     VibeOSKernelLinuxTime.c
**
** Description: 
**     Time helper functions for Linux.
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

#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include "vtmdrv.h"

#define TIMER_VALUE                     5000000       
#define WATCHDOG_TIMEOUT                10            


static bool g_bTimerStarted = false;
static struct hrtimer h_timer;
ktime_t kt;

static int g_nWatchdogCounter = 0;

DECLARE_MUTEX(g_hMutex);

static void VibeOSKernelLinuxStartTimer(void);
static void VibeOSKernelLinuxStopTimer(void);

static inline int  sem_is_locked(struct semaphore *sem)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&sem->lock, flags);
	ret= (sem->count) != 1;
	spin_unlock_irqrestore(&sem->lock, flags);
	return ret;
}

static int VibeOSKernelTimerProc(struct hrtimer * local_timer)
{
    if (!g_bTimerStarted)
        return;

    if (sem_is_locked(&g_hMutex))
    {
        up(&g_hMutex);
    }
    else
    {
        if ((++g_nWatchdogCounter) > WATCHDOG_TIMEOUT)
        {
            ImmVibeSPI_ForceOut_Set(0);
            VibeOSKernelLinuxStopTimer();

            g_nWatchdogCounter = 0;
        }
    }
    return 0;
}

static void VibeOSKernelLinuxInitTimer(void)
{
     hrtimer_init(&h_timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);

    h_timer.function  = VibeOSKernelTimerProc;

}

static void VibeOSKernelLinuxStartTimer(void)
{
   
   g_nWatchdogCounter = 0;
    if (!g_bTimerStarted)
    {
        if (!sem_is_locked(&g_hMutex))
        	down_interruptible(&g_hMutex); 
         g_bTimerStarted = true;
    	}
			
    
        kt.tv.nsec = TIMER_VALUE;
        hrtimer_start(&h_timer,kt,HRTIMER_MODE_REL);

    down_interruptible(&g_hMutex);  
   }

static void VibeOSKernelLinuxStopTimer(void)
{
      hrtimer_cancel(&h_timer); 
    if (g_bTimerStarted)
    {
        g_bTimerStarted = false;
    }
     
} 

static void VibeOSKernelLinuxTerminateTimer(void)
{
    VibeOSKernelLinuxStopTimer();
    if (sem_is_locked(&g_hMutex)) up(&g_hMutex);
  
}
