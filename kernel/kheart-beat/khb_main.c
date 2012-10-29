/*****************************************************************************
 * kernel/kheart-beat/khb_main.c
 * 
 * This file contains functions which aid the kernel debugging
 ******************************************************************************/

#include "khb_main.h"
#include "khb_lcd.h"


/*
 * This function toggles the display colour of the rowHeight x colHeight matrix on the
 * left corner of the screen.
 */
int kernel_alive_indicator(int x, int y, int colHeight,  int rowHeight, int mode)
{
	return -1;
	static int flag1 = COLOR_WHITE;
	static int flag2 = COLOR_RED;
	struct fb_fix_screeninfo fixinfo;

	khb_dbg_msg("entered kernel alive indicator function\n");

/*	if(s3cfb_direct_ioctl(2, FBIOGET_FSCREENINFO, (unsigned long)&fixinfo) < 0)
	{
		khb_dbg_msg("s3cfb_direct_ioctl failed\n");
		return -1;
	}
*/
	fbpMem = phys_to_virt(fixinfo.smem_start);
	if(fbpMem != NULL)
	{
		rowHeight = (rowHeight<0)?KHB_MIN_BOX_W:rowHeight;
		rowHeight = (rowHeight>PANEL_W)?PANEL_W:rowHeight;
		colHeight = (colHeight<0)?KHB_MIN_BOX_H:colHeight;
		colHeight = (colHeight>PANEL_H)?PANEL_H:colHeight;

		if(mode==0) {
			lcd_FilledRectangle(x,y,colHeight, rowHeight, flag1);
			flag1 = ~flag1;
		}
		else if(mode==1) {
			lcd_FilledRectangle(x,y,colHeight, rowHeight, flag2);
			flag2 = ~flag2;
		}

		//display_dbg_msg(3,3,COLOR_RED, "This is a test msg");
	}
	else
	{
		khb_dbg_msg("fbpMem NULL\n");
	}

return 0;
}

EXPORT_SYMBOL(kernel_alive_indicator);

/*
 * txtMsg	 :	text message to display
 * color	 :	Background colour
 * fontSize	 :  0 for default font size; 1 for x4 magnification 
 * retainPrevMSg : 0 to overwrite the previous message; 1 to retain previously dispalyed message
 */

int display_dbg_msg(char* txtMsg, unsigned char fontSize, unsigned char retainPrevMsg)
{ 
	return -1;
	int colHeight=3;
	static int rowHeight=0;
	struct fb_fix_screeninfo fixinfo;

	if(txtMsg == NULL)
	{
		khb_dbg_msg("txtMsg pointer is NULL\n");
		return -1;
	}

/*	if(s3cfb_direct_ioctl(2, FBIOGET_FSCREENINFO, (unsigned long)&fixinfo) < 0)
	{
		khb_dbg_msg("s3cfb_direct_ioctl failed\n");
		return -1;
	}
*/
	if((fbpMem = phys_to_virt(fixinfo.smem_start)) == NULL)
	{
		khb_dbg_msg("fbpMem is NULL\n");
		return -1;
	}

	/* display debug screen */
	if(!retainPrevMsg)
	{
		rowHeight = 3;
	}
	else
	{
		rowHeight += 8*(fontSize+2);
	}

	rowHeight = (rowHeight<0)?KHB_MIN_BOX_W:rowHeight;
	rowHeight = (rowHeight>PANEL_H)?PANEL_H:rowHeight;

	/* display debug message */
	lcd_draw_font(colHeight, rowHeight, COLOR_BLACK, COLOR_WHITE, strlen(txtMsg), (unsigned char*)txtMsg, fontSize);

	return 0;
}

EXPORT_SYMBOL(display_dbg_msg);
