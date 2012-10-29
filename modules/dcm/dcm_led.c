/****************************************************************************
**
** COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR		: KyoungHOON Kim (khoonk)
**
*****************************************************************************
** 2006/06/14. khoonk	create led_control routine
*****************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include "dcm_ext.h"
#include "message.h"

#include <mach/gpio.h>

LED_STATE	gLedState[MAX_LED_NUM];

typedef enum {
		DCM_LED_COLOR_GREEN 	= 0x01,
		DCM_LED_COLOR_BLUE 		= 0x02,
		DCM_LED_COLOR_CYAN		= 0x03,
	    DCM_LED_COLOR_RED		= 0x04,
		DCM_LED_COLOR_YELLOW	= 0x05,
		DCM_LED_COLOR_MAGENTA	= 0x06,
		DCM_LED_COLOR_WHITE		= 0x07, 	

}emDCMLedColor; 


static int led_pins[MAX_LED_NUM] =
{
};

void _ledInit(void)
{
	int i;
	for( i = 0; i < MAX_LED_NUM; i++ )
	{
	}
}

void _ledOn(int ledNum)
{
	return;
}

void _ledOff(int ledNum)
{
	return;
}

int ledOnOffSet(LED_INFO *ledInfo)
{
	if( ledInfo->ledNum <= DCM_LED_COLOR_WHITE )
	{

		printk("ledNum(%d), mode(%d), onCount(%d), offCount(%d)\n", ledInfo->ledNum, ledInfo->mode, ledInfo->onCount, ledInfo->offCount);
		switch(ledInfo->mode)
		{
			case LED_ON	:
				break;
				
			case LED_OFF :
				break;
		}
	}
	return 0;	
}

unsigned int tmpCnt = 0;

int ledSingleCntl(int numLed)
{
	switch(LEDSTATE(numLed))
	{
		case LED_ON 	:
		case LED_OFF	:
			break;

		case LED_BLINK	:
			
			if(LED_BLINK_STATE(numLed) == LED_ON)
			{
				if(LED_COUNT(numLed) > LED_ON_COUNT(numLed))
				{
					LED_COUNT(numLed) = 0;	/* count clear */
					_ledOff(numLed);	/* led off */
					LED_BLINK_STATE(numLed) = LED_OFF;			
				}
				else
					LED_COUNT(numLed)++;
			}
			else	/* LED off state */
			{
				if(LED_COUNT(numLed) > LED_OFF_COUNT(numLed))
				{
					LED_COUNT(numLed) = 0;	/* count clear */
					_ledOn(numLed);	/* led on */
					LED_BLINK_STATE(numLed) = LED_ON;			
				}
				else
					LED_COUNT(numLed)++;

			}
			break;

		case LED_BLINK2 :
			
			if(LED_BLINK_STATE(numLed) == LED_ON)
			{
				if(LED_COUNT(numLed) > LED_ON_COUNT(numLed))
				{
					LED_COUNT(numLed) = 0;	/* count clear */
					_ledOff(numLed);	/* led off */
					LED_BLINK_STATE(numLed) = LED_OFF;			
					tmpCnt = 0;
				}
				else
				{
					if(!(tmpCnt % 3))
					{
						if(LED_BLINK_CUR_STATE(numLed) == LED_ON)
						{
							_ledOff(numLed);	/* led off */
							LED_BLINK_CUR_STATE(numLed) = LED_OFF;
						}
						else
						{
							_ledOn(numLed);	/* led off */
							LED_BLINK_CUR_STATE(numLed) = LED_ON;
						}
					}
					tmpCnt++;
					LED_COUNT(numLed)++;
				}
			}
			else	/* LED off state */
			{
				if(LED_COUNT(numLed) > LED_OFF_COUNT(numLed))
				{
					LED_COUNT(numLed) = 0;	/* count clear */
					_ledOn(numLed);	/* led on */
					LED_BLINK_STATE(numLed) = LED_ON;			
				}
				else
					LED_COUNT(numLed)++;

			}
			break;

		case LED_ON_TIMEOUT:
			LED_COUNT(numLed)++;
			if(LED_COUNT(numLed) >= LED_TIMEOUT(numLed)){
				LED_COUNT(numLed) = 0;	/* count clear */
				_ledOff(numLed);	/* led off */
				LEDSTATE(numLed) = LED_OFF;
			}
			break;

		default :
			break;
	}
	return 0;
}

void led_control(void)
{
	int i;

	for(i = 0; i < MAX_LED_NUM; i++)
	{
		ledSingleCntl(i);		
	}	
}
