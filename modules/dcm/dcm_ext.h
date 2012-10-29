/****************************************************************************
**
** COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR		: KyoungHOON Kim (khoonk)
**
*****************************************************************************
**
*****************************************************************************/
#ifndef _DCM_EXT_
#define _DCM_EXT_

#include <linux/input.h>
#include "message.h"
extern int 	dcm_addQueue(DCM_QUEUE_TYPE *pval);
extern int 	dcm_deleteQueue(DCM_QUEUE_TYPE *pval);

extern void folder_detect_init(void);
extern void folder_detect_exit(void);
extern void folder_detect_isr(void);
extern void folder_detect_scan_svc(void);

extern void earphone_detect_init(void);
extern void earphone_detect_exit(void);
extern void usb_detect_init(void);
extern void usb_detect_exit(void);
extern void usb_detect_scan_svc(void);

extern int 	ledOnOffSet(LED_INFO *ledInfo);
extern void _ledInit(void);
extern void led_control(void);

#endif
