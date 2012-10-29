/*****************************************************************************
** kernel/kheart-beat/khb_main.h
******************************************************************************/

#ifndef _KHB_MAIN_H_
#define _KHB_MAIN_H_

#include <linux/module.h> 
#include "khb_lcd.h"

/**** debugging related ***/
//#define khb_dbg_msg(MSG...)            printk(MSG)
#define khb_dbg_msg(MSG...)          

/*** LCD ***/
#define KHB_MIN_BOX_W	3
#define KHB_MIN_BOX_H	3

#define KHB_DEFAULT_FONT_SIZE	0
#define KHB_ENLARGED_FONT		1

#define KHB_DISCARD_PREV_MSG	0
#define KHB_RETAIN_PREV_MSG		1

int kernel_alive_indicator(int x, int y, int rowHeight, int colHeight, int mode);
int display_dbg_msg(char* txtMsg, unsigned char fontSize, unsigned char retainPrevMsg);

extern int s3cfb_direct_ioctl(int id, unsigned int cmd, unsigned long arg);

#endif /* #ifndef _KHB_MAIN_H_  */
