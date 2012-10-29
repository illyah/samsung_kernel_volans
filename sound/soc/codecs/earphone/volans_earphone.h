#include <linux/input.h>
#include "message.h"

/*********** Debug Macros START ********/
#undef EARJACK_DEBUG
//#define EARJACK_DEBUG

#undef DPRINTK
#ifdef EARJACK_DEBUG
#define DPRINTK(ARGS...)  printk(KERN_INFO "<%s>: ",__FUNCTION__);printk(ARGS)
#define FN_IN printk(KERN_INFO "[%s]: start\n", __FUNCTION__)
#define FN_OUT(n) printk(KERN_INFO "[%s]: end(%u)\n",__FUNCTION__, n)
#else
#define DPRINTK( x... )
#define FN_IN
#define FN_OUT(n)
#endif				/* DEBUG */
/*********** Debug Macros END **********/

#define EARJACK_NAME			"EARJACK"
#define EARKEY_NAME				"EARKEY"

#define EARJACK_CONNECT_HOLD_TIME		3
#define EARJACK_DISCONNECT_HOLD_TIME		0
#define EARKEY_PRESSED_HOLD_TIME		5
#define EARKEY_RELEASED_HOLD_TIME		1

#define EAR_DEV_MAJOR	 222
#define EAR_DEV_NAME     "earphone"
 
#define EARJACK_TIME_INTERVAL	HZ/100
#define EARKEY_TIME_INTERVAL	HZ/30

#define IRQ_DET_35			IRQ_EINT(10)
#define IRQ_EAR_SEND_END	IRQ_EINT(11)
#define EAR_ADC_CHANNEL		3
#define EARADC_SAMPLING_COUNT 	10

#define EARJACK_STATUS_NORMAL    0
#define EARJACK_STATUS_CHANGE    1

extern unsigned int headset_status;

extern void earjack_report_key(unsigned int keycode, int value);
extern int mic_power_ctrl(int mode);

