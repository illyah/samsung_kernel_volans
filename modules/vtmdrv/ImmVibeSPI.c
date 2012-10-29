/*
** =========================================================================
** File:
**     ImmVibeSPI.c
**
** Description: 
**     Device-dependent functions called by Immersion VibeTonz API
**     to control PWM duty cycle, amp enable/disable, save IVT file, etc...
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

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/leds.h>
#include <asm/mach-types.h>

#include <asm/irq.h>
#include <mach/map.h>
#include <plat/regs-timer.h>
#include <mach/regs-irq.h>
#include <asm/mach/time.h>
#include <mach/tick.h>

#include <mach/hardware.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include<mach/gpio.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include "vtmdrv.h"

#ifdef IMMVIBESPIAPI
#undef IMMVIBESPIAPI
#endif

#define IMMVIBESPIAPI static
#define PWM_DUTY_MAX   580  
#define MANUAL_ON	   1
#define MANUAL_OFF     0

static bool g_bAmpEnabled = false;
static struct clk *clk;
int manual_update;

void s3c_pwm_timer_start(void);
void s3c_pwm_timer_stop(void);
void s3c_pwm_timer_clk(void);
void s3c_pwm_timer_manual_on(void);
void s3c_pwm_timer_manual_off(void);
void s3c_pwm_timer_reload_on(void);
void s3c_pwm_timer_reload_off(void);
void s3c_pwm_timer_prescaler_set(void);


static int ReqGPTimer(void)
{
	s3c_gpio_cfgpin(GPIO_VIB_PWM, S3C_GPIO_SFN(2));
	s3c_gpio_setpull(GPIO_VIB_PWM, S3C_GPIO_PULL_NONE);
	
	s3c_gpio_cfgpin(GPIO_VIB_EN, S3C_GPIO_SFN(1));
	s3c_gpio_setpull(GPIO_VIB_EN, S3C_GPIO_PULL_NONE);
	s3c_pwm_timer_clk();
	s3c_pwm_timer_reload_on();
	s3c_pwm_timer_prescaler_set();
	manual_update=MANUAL_OFF;
    	return 0;
}


static void GPTimerSetValue(unsigned long load,unsigned long cmp)
{
	 __raw_writel(load, S3C2410_TCNTB(1));
	 __raw_writel(cmp, S3C2410_TCMPB(1));
}


IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpDisable(void)
{


    if (g_bAmpEnabled)
    {
        DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_AmpDisable.\n"));
        g_bAmpEnabled = false;
	gpio_set_value(GPIO_VIB_EN, GPIO_LEVEL_LOW);
    }
	 return VIBE_S_SUCCESS;
}

IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpEnable(void)
{

    if (!g_bAmpEnabled)
    {
        DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_AmpEnable.\n"));
        g_bAmpEnabled = true;
	    gpio_set_value(GPIO_VIB_EN, GPIO_LEVEL_HIGH);
    	}
    return VIBE_S_SUCCESS;
}

IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Initialize(void)
{

    DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_Initialize.\n"));
    g_bAmpEnabled = true;   
    ImmVibeSPI_ForceOut_AmpDisable();
    ReqGPTimer();
    return VIBE_S_SUCCESS;
}

IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Terminate(void)
{

    DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_Terminate.\n"));
    ImmVibeSPI_ForceOut_AmpDisable();
    return VIBE_S_SUCCESS;
}

 IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Set(VibeInt8 nForce)
{
    VibeUInt32 nTmp;
    if (nForce == 0)
    	{ 
        	ImmVibeSPI_ForceOut_AmpDisable();
        	s3c_pwm_timer_stop();
			manual_update=MANUAL_OFF;
    	}
    else
    	{
        	ImmVibeSPI_ForceOut_AmpEnable();
        	nTmp=(580*nForce)/254 +580/2;
			if(manual_update==MANUAL_OFF)
				{	
					s3c_pwm_timer_manual_on();
					manual_update=MANUAL_ON;
				}
        	GPTimerSetValue(PWM_DUTY_MAX, nTmp);
        	s3c_pwm_timer_start();  
			if(manual_update==MANUAL_ON)
				{
					s3c_pwm_timer_manual_off(); 
      			}
		}
    return VIBE_S_SUCCESS;
}

IMMVIBESPIAPI VibeStatus ImmVibeSPI_Device_GetName(char *szDevName, int nSize)
{

    if ((!szDevName) || (nSize < 1)) return VIBE_E_FAIL;
    DbgOut((KERN_DEBUG "ImmVibeSPI_Device_GetName.\n"));
    strncpy(szDevName, "Generic Linux Device", nSize-1);
    szDevName[nSize - 1] = '\0';   
    return VIBE_S_SUCCESS;
}


void s3c_pwm_timer_start(void)
{
	unsigned long tcon;
	tcon = __raw_readl(S3C2410_TCON);
	tcon |= S3C2410_TCON_T1START;
	__raw_writel(tcon, S3C2410_TCON);
}

void s3c_pwm_timer_stop(void)
{
	unsigned long tcon;
	tcon = __raw_readl(S3C2410_TCON);
	tcon &= ~S3C2410_TCON_T1START ;
	__raw_writel(tcon, S3C2410_TCON);
}

void s3c_pwm_timer_manual_on(void)
{
	unsigned long tcon;
	tcon = __raw_readl(S3C2410_TCON);
	tcon |= S3C2410_TCON_T1MANUALUPD;
	__raw_writel(tcon, S3C2410_TCON);
}

void s3c_pwm_timer_manual_off(void)
{
	unsigned long tcon;
	tcon = __raw_readl(S3C2410_TCON);
	tcon &= ~S3C2410_TCON_T1MANUALUPD;
	__raw_writel(tcon, S3C2410_TCON);
}

void s3c_pwm_timer_reload_on(void)
{
	unsigned long tcon;
	tcon = __raw_readl(S3C2410_TCON);
	tcon |= S3C2410_TCON_T1RELOAD;
	__raw_writel(tcon, S3C2410_TCON);
}
void s3c_pwm_timer_reload_off(void)
{
	unsigned long tcon;
    tcon = __raw_readl(S3C2410_TCON);
    tcon &= ~S3C2410_TCON_T1RELOAD;
    __raw_writel(tcon, S3C2410_TCON);
}

void s3c_pwm_timer_prescaler_set(void)
{
	unsigned long tcfg0;
	tcfg0 = __raw_readl(S3C2410_TCFG0);
	tcfg0 &= ~255; 
	tcfg0 |= 4 ;
	__raw_writel(tcfg0, S3C2410_TCFG0);
}

void s3c_pwm_timer_clk(void)
{
	unsigned long pclk=0;
	clk = clk_get(NULL, "timers");
	if (IS_ERR(clk))
    	panic("failed to get clock for system timer");
	pclk = clk_get_rate(clk);
}

void GPtimer_disable(void)
{
	clk_disable(clk);
}

void GPtimer_enable()
{
	clk_enable(clk);
}


