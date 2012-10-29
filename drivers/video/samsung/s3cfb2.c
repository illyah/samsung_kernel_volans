











#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/ctype.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <plat/clock.h>
#include "s3cfb2.h"

#define VOLANS_LCD_ACTIVE_VIEW_WIDTH		43	
#define VOLANS_LCD_ACTIVE_VIEW_HEIGHT		70	


//#define __CONFIG_SAMSUNG_BUG_FIX__


#include <plat/irqs.h>

extern void s3cfb_set_lcd_info(struct s3cfb_global *ctrl);


#include "samsung_boot_logo.h"

static struct s3cfb_global *ctrl;



#include <plat/power-domain.h>
#include <plat/regs-gpio.h>
#include <plat/pm.h>

static struct platform_device *lcd_pdev = NULL;
static int lcd_pm_state = 0;

static struct sleep_save s3c_lcd_save[] = {
	SAVE_ITEM(S3C64XX_GPICON),
	SAVE_ITEM(S3C64XX_GPIDAT),
	SAVE_ITEM(S3C64XX_GPIPUD),
	SAVE_ITEM(S3C64XX_GPJCON),
	SAVE_ITEM(S3C64XX_GPJDAT),
	SAVE_ITEM(S3C64XX_GPJPUD),
};


struct s3c_platform_fb *to_fb_plat(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return (struct s3c_platform_fb *) pdev->dev.platform_data;
}

#ifndef CONFIG_FRAMEBUFFER_CONSOLE
static int s3cfb_draw_logo(struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct fb_var_screeninfo *var = &fb->var;


	memcpy(ctrl->fb[pdata->default_win]->screen_base, \
		arr_samsung_boot_logo, fix->line_length * var->yres);

	return 0;
}
#else
int fb_is_primary_device(struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct s3cfb_window *win = fb->par;

	lcd_dbg("[%d] checking for primary device\n", win->id);

	if (win->id == pdata->default_win)
		return 1;
	else
		return 0;
}
#endif

static irqreturn_t s3cfb_irq_frame(int irq, void *dev_id)
{
	s3cfb_clear_interrupt(ctrl);

	ctrl->wq_count++;
	wake_up_interruptible(&ctrl->wq);

	return IRQ_HANDLED;
}

static int s3cfb_enable_window(int id)
{
	struct s3cfb_window *win = ctrl->fb[id]->par;

	lcd_dbg(" enable %d window\n", win->id);

	if (s3cfb_window_on(ctrl, id)) {
		win->enabled = 0;
		lcd_err( "Cannot enable %d window", win->id);
		return -EFAULT;
	} else {
		win->enabled = 1;
		return 0;
	}
}

static int s3cfb_disable_window(int id)
{
	struct s3cfb_window *win = ctrl->fb[id]->par;

	lcd_dbg(" disable %d window\n", win->id);

	if (s3cfb_window_off(ctrl, id)) {
		win->enabled = 1;
		lcd_err( "Cannot disable %d window", win->id);
		return -EFAULT;
	} else {
		win->enabled = 0;
		return 0;
	}
}

static int s3cfb_init_global(void)
{

	lcd_dbg(" initialize global vars\n");
	
	ctrl->output = OUTPUT_RGB;
	ctrl->rgb_mode = MODE_RGB_P;

	ctrl->wq_count = 0;
	init_waitqueue_head(&ctrl->wq);
	mutex_init(&ctrl->lock);

	s3cfb_set_output(ctrl);
	s3cfb_set_display_mode(ctrl);
	s3cfb_set_polarity(ctrl);
	s3cfb_set_timing(ctrl);
	s3cfb_set_lcd_size(ctrl);

	return 0;
}

static int s3cfb_map_video_memory(struct fb_info *fb)
{
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;

#ifdef __CONFIG_SAMSUNG_BUG_FIX__
		if (win->path == DATA_PATH_FIFO || win->id == 0)
			return 0;
#else
		if (win->path == DATA_PATH_FIFO)
			return 0;
#endif


	fb->screen_base = dma_alloc_writecombine(ctrl->dev, \
				PAGE_ALIGN(fix->smem_len), \
				(unsigned int *) &fix->smem_start, GFP_KERNEL);
	if (!fb->screen_base)
	{
		lcd_err( "Cannot map video memory for %d window", win->id);
		return -ENOMEM;
	}
	else
		lcd_dbg(" [fb%d] dma: 0x%08x, cpu: 0x%08x, size: 0x%08x\n", \
			win->id, (unsigned int) fix->smem_start, \
			(unsigned int) fb->screen_base, fix->smem_len);

	memset(fb->screen_base, 0, fix->smem_len);

	return 0;
}

static int s3cfb_unmap_video_memory(struct fb_info *fb)
{
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;

	if (fix->smem_start) {
		dma_free_writecombine(ctrl->dev, fix->smem_len, \
			fb->screen_base, fix->smem_start);
		fix->smem_start = 0;
		fix->smem_len = 0;
		lcd_dbg("[fb%d] video memory released\n", win->id);
	}

	return 0;
}

static int s3cfb_set_bitfield(struct fb_var_screeninfo *var)
{
	switch (var->bits_per_pixel) {
	case 16:
		if (var->transp.length == 1) {
			var->red.offset = 10;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 5;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 15;
		} else if (var->transp.length == 4) {
			var->red.offset = 8;
			var->red.length = 4;
			var->green.offset = 4;
			var->green.length = 4;
			var->blue.offset = 0;
			var->blue.length = 4;
			var->transp.offset = 12;
		} else {
			var->red.offset = 11;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 6;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 0;
		}
		break;

	case 24:
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;

	case 32:
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 24;
		break;
	}

	return 0;
}

static int s3cfb_set_alpha_info(struct fb_var_screeninfo *var,
				struct s3cfb_window *win)
{

	lcd_dbg(" setting alpha info\n");
	
	if (var->transp.length > 0)
		win->alpha.mode = PIXEL_BLENDING;
	else {
		win->alpha.mode = PLANE_BLENDING;
		win->alpha.channel = 0;
		win->alpha.value = S3CFB_AVALUE(0xf, 0xf, 0xf);
	}

	return 0;
}

static int s3cfb_check_var(struct fb_var_screeninfo *var,
				struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;
	struct s3cfb_lcd *lcd = ctrl->lcd;

	lcd_dbg("[fb%d] check_var\n", win->id);

	if (var->bits_per_pixel != 16 && var->bits_per_pixel != 24 && \
		var->bits_per_pixel != 32) {
		lcd_err( " invalid bits per pixel\n");
		return -EINVAL;
	}

	if (var->xres > lcd->width)
		var->xres = lcd->width;

	if (var->yres > lcd->height)
		var->yres = lcd->height;

	if (var->xres_virtual != var->xres)
		var->xres_virtual = var->xres;

	if (var->yres_virtual > var->yres * (fb->fix.ypanstep + 1))
		var->yres_virtual = var->yres * (fb->fix.ypanstep + 1);

	if (var->xoffset != 0)
		var->xoffset = 0;

	if (var->yoffset + var->yres > var->yres_virtual)
		var->yoffset = var->yres_virtual - var->yres;

	if (var->width != VOLANS_LCD_ACTIVE_VIEW_WIDTH)
		var->width = VOLANS_LCD_ACTIVE_VIEW_WIDTH;

	if (var->height != VOLANS_LCD_ACTIVE_VIEW_HEIGHT)
		var->height = VOLANS_LCD_ACTIVE_VIEW_HEIGHT;

	if (win->x + var->xres > lcd->width)
		win->x = lcd->width - var->xres;

	if (win->y + var->yres > lcd->height)
		win->y = lcd->height - var->yres;

	
	if (win->id != pdata->default_win) {
		fix->line_length = var->xres_virtual * var->bits_per_pixel / 8;
		fix->smem_len = fix->line_length * var->yres_virtual;
	}

	s3cfb_set_bitfield(var);
	s3cfb_set_alpha_info(var, win);

	return 0;	
}

static int s3cfb_set_par(struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct s3cfb_window *win = fb->par;

	lcd_dbg(" [fb%d] set_par\n", win->id);

	if ((win->id != pdata->default_win) && !fb->fix.smem_start)
		s3cfb_map_video_memory(fb);

	s3cfb_set_window_control(ctrl, win->id);
	s3cfb_set_window_position(ctrl, win->id);
	s3cfb_set_window_size(ctrl, win->id);
	s3cfb_set_buffer_address(ctrl, win->id);
	s3cfb_set_buffer_size(ctrl, win->id);

	if (win->id > 0)
		s3cfb_set_alpha_blending(ctrl, win->id);

	return 0;	
}

static int s3cfb_blank(int blank_mode, struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;

	lcd_dbg("get into the blank screen\n");

	s3cfb_disable_window(win->id);

	return 0;
}

static int s3cfb_pan_display(struct fb_var_screeninfo *var, 
				struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;

	if (var->yoffset + var->yres > var->yres_virtual) {
		err("invalid yoffset value\n");
		return -EINVAL;
	}

	fb->var.yoffset = var->yoffset;

	lcd_dbg("[fb%d] yoffset for pan display: %d\n", win->id, \
		var->yoffset);

	s3cfb_set_buffer_address(ctrl, win->id);

	return 0;
}

static inline unsigned int __chan_to_field(unsigned int chan,
						struct fb_bitfield bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf.length;

	return chan << bf.offset;
}

static int s3cfb_setcolreg(unsigned int regno, unsigned int red,
				unsigned int green, unsigned int blue,
				unsigned int transp, struct fb_info *fb)
{
	unsigned int *pal = (unsigned int *) fb->pseudo_palette;
	unsigned int val = 0;

	lcd_dbg(" setting color regs \n");

	if (regno < 16) {
		
		val |= __chan_to_field(red, fb->var.red);
		val |= __chan_to_field(green, fb->var.green);
		val |= __chan_to_field(blue, fb->var.blue);
		val |= __chan_to_field(transp, fb->var.transp);			

		pal[regno] = val;
	}

	return 0;
}

#ifdef __CONFIG_SAMSUNG_BUG_FIX__

static int s3cfb_open(struct fb_info *fb, int user)
{
	struct s3cfb_window *win = fb->par;
	int ret = 0;

	lcd_dbg("opening fb%d\n", win->id);

	mutex_lock(&ctrl->lock);

	if (atomic_read(&win->in_use)) {
		lcd_dbg( "[WARNING][fb%d] you are trying to open multiple instances, " \
			"allowed temporary\n", win->id);
		ret = 0;
	} else
		atomic_inc(&win->in_use);

	mutex_unlock(&ctrl->lock);

	return ret;
}
#else	
static int s3cfb_open(struct fb_info *fb, int user)
{
	struct s3cfb_window *win = fb->par;
	int ret = 0;

	lcd_dbg("opening fb%d\n", win->id);

	mutex_lock(&ctrl->lock);

	if (atomic_read(&win->in_use))
	{
		lcd_err( "[fb%d] not allowed to open multiple instance. \n", win->id);
		ret = -EBUSY;
	}
	else
		atomic_inc(&win->in_use);

	mutex_unlock(&ctrl->lock);

	return ret;
}

#endif	


static int s3cfb_release_window(struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct s3cfb_window *win = fb->par;

	lcd_dbg("closing fb%d window\n", win->id);

	if (win->id != pdata->default_win) {
		s3cfb_disable_window(win->id);
		s3cfb_unmap_video_memory(fb);
		s3cfb_set_buffer_address(ctrl, win->id);
	}

	win->x = 0;
	win->y = 0;

	return 0;
}

static int s3cfb_release(struct fb_info *fb, int user)
{
	struct s3cfb_window *win = fb->par;

	lcd_dbg("releasing fb%d\n", win->id);

#ifndef __CONFIG_SAMSUNG_BUG_FIX__
	s3cfb_disable_window(win->id);
#endif
	s3cfb_release_window(fb);

	mutex_lock(&ctrl->lock);
	atomic_dec(&win->in_use);
	mutex_unlock(&ctrl->lock);

	return 0;
}

static int s3cfb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	
	return 0;
}

static int s3cfb_wait_for_vsync(void)
{
	int count = ctrl->wq_count;

	lcd_dbg(" waiting for VSYNC interrupt\n");

	wait_event_interruptible_timeout(ctrl->wq, \
			count != ctrl->wq_count, HZ / 10);

	lcd_dbg(" got a VSYNC interrupt\n");

	return 0;
}

static int s3cfb_ioctl(struct fb_info *fb, unsigned int cmd,
			unsigned long arg)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct fb_var_screeninfo *var = &fb->var;
	struct s3cfb_window *win = fb->par, *win_temp;
	struct s3cfb_lcd *lcd = ctrl->lcd;
	int ret = 0, i;

	union {
		struct s3cfb_user_window user_window;
		struct s3cfb_user_plane_alpha user_alpha;
		struct s3cfb_user_chroma user_chroma;
		int vsync;
	} p;

	switch (cmd) {
	case FBIO_WAITFORVSYNC:
		s3cfb_wait_for_vsync();
		break;

	case S3CFB_WIN_ON:
		s3cfb_enable_window(win->id);
		break;

	case S3CFB_WIN_OFF:
		s3cfb_disable_window(win->id);
		break;

	case S3CFB_WIN_OFF_ALL:
		for (i = 0; i < pdata->nr_wins; i++) {
			win_temp = ctrl->fb[i]->par;
			s3cfb_disable_window(win_temp->id);
		}
		break;

	case S3CFB_WIN_POSITION:
		if (copy_from_user(&p.user_window, \
			(struct s3cfb_user_window __user *) arg, \
			sizeof(p.user_window)))
			ret = -EFAULT;
		else {
			if (p.user_window.x < 0)
				p.user_window.x = 0;

			if (p.user_window.y < 0)
				p.user_window.y = 0;

			if (p.user_window.x + var->xres > lcd->width)
				win->x = lcd->width - var->xres;
			else
				win->x = p.user_window.x;

			if (p.user_window.y + var->yres > lcd->height)
				win->y = lcd->height - var->yres;
			else
				win->y = p.user_window.y;

			s3cfb_set_window_position(ctrl, win->id);
		}
		break;

	case S3CFB_WIN_SET_PLANE_ALPHA:
		if (copy_from_user(&p.user_alpha, \
			(struct s3cfb_user_plane_alpha __user *) arg, \
			sizeof(p.user_alpha)))
			ret = -EFAULT;
		else {
			win->alpha.mode = PLANE_BLENDING;
			win->alpha.channel = p.user_alpha.channel;
			win->alpha.value = \
				S3CFB_AVALUE(p.user_alpha.red, \
					p.user_alpha.green, \
					p.user_alpha.blue);

			s3cfb_set_alpha_blending(ctrl, win->id);
		}
		break;

	case S3CFB_WIN_SET_CHROMA:
		if (copy_from_user(&p.user_chroma, \
			(struct s3cfb_user_chroma __user *) arg, \
			sizeof(p.user_chroma)))
			ret = -EFAULT;
		else {
			win->chroma.enabled = p.user_chroma.enabled;
			win->chroma.key = S3CFB_CHROMA(p.user_chroma.red, \
						p.user_chroma.green, \
						p.user_chroma.blue);

			s3cfb_set_chroma_key(ctrl, win->id);
		}
		break;

	case S3CFB_SET_VSYNC_INT:
		if (get_user(p.vsync, (int __user *) arg))
			ret = -EFAULT;
		else {
			if (p.vsync)
				s3cfb_set_global_interrupt(ctrl, 1);

			s3cfb_set_vsync_interrupt(ctrl, p.vsync);
		}
		break;
	}

	return ret;
}

struct fb_ops s3cfb_ops = {
	.owner		= THIS_MODULE,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_check_var	= s3cfb_check_var,
	.fb_set_par	= s3cfb_set_par,
	.fb_blank	= s3cfb_blank,
	.fb_pan_display	= s3cfb_pan_display,
	.fb_setcolreg	= s3cfb_setcolreg,
	.fb_cursor	= s3cfb_cursor,
	.fb_ioctl	= s3cfb_ioctl,
	.fb_open	= s3cfb_open,
	.fb_release	= s3cfb_release,
};


int s3cfb_open_fifo(int id, int ch, int (*do_priv)(void *), void *param)
{
	struct s3cfb_window *win = ctrl->fb[id]->par;

	lcd_dbg(" [fb%d] open fifo\n", win->id);

	win->path = DATA_PATH_FIFO;
	win->local_channel = ch;

	if (do_priv) {
		if (do_priv(param)) {
			lcd_err( " failed to run for private fifo open\n");
			s3cfb_enable_window(id);
			return -EFAULT;
		}
	}

	s3cfb_set_window_control(ctrl, id);
	s3cfb_enable_window(id);

	return 0;
}


int s3cfb_close_fifo(int id, int (*do_priv)(void *), void *param, int sleep)
{
	struct s3cfb_window *win = ctrl->fb[id]->par;

	lcd_dbg(" [fb%d] close fifo\n", win->id);

	if (sleep)
		win->path = DATA_PATH_FIFO;
	else
		win->path = DATA_PATH_DMA;


#ifndef CONFIG_FIX_FIMD_FLSAHING_ISSUE
	s3cfb_check_line_count(ctrl);
	s3cfb_display_off(ctrl);
	s3cfb_disable_window(id);

	if (do_priv) {
		if (do_priv(param)) {
			lcd_err( " failed to run for private fifo close\n");
			s3cfb_enable_window(id);
			s3cfb_display_on(ctrl);
			return -EFAULT;
		}
	}

	s3cfb_display_on(ctrl);
#else
	
	s3cfb_check_line_count100(ctrl);   
	s3cfb_disable_window(id);
	s3cfb_frame_off(ctrl);
	s3cfb_check_line_count(ctrl);
	s3cfb_disable_local_win0();

	if (do_priv) {
		if (do_priv(param)) {
			lcd_err( " failed to run for private fifo close\n");
			s3cfb_enable_window(id);
			s3cfb_display_on(ctrl);
			return -EFAULT;
		}
	}

	s3cfb_frame_on(ctrl);
#endif


	return 0;
}


void s3cfb_enable_local(int id, int in_yuv, int ch)
{
	struct s3cfb_window *win = ctrl->fb[id]->par;

	win->path = DATA_PATH_FIFO;
	win->local_channel = ch;

	lcd_dbg(" [fb%d] enabling local path\n", win->id);

	s3cfb_set_vsync_interrupt(ctrl, 1);
	s3cfb_wait_for_vsync();
	s3cfb_set_vsync_interrupt(ctrl, 0);

	s3cfb_set_window_control(ctrl, id);
}


void s3cfb_enable_dma(int id)
{
	struct s3cfb_window *win = ctrl->fb[id]->par;

	win->path = DATA_PATH_DMA;

	lcd_dbg(" [fb%d] enabling fifo path\n", win->id);

	s3cfb_set_vsync_interrupt(ctrl, 1);
	s3cfb_wait_for_vsync();
	s3cfb_set_vsync_interrupt(ctrl, 0);

	s3cfb_display_off(ctrl);
	s3cfb_set_window_control(ctrl, id);
	s3cfb_display_on(ctrl);
}

int s3cfb_direct_ioctl(int id, unsigned int cmd, unsigned long arg)
{
	struct fb_info *fb = ctrl->fb[id];
	struct fb_var_screeninfo *var = &fb->var;	
	struct s3cfb_window *win = fb->par;
	struct s3cfb_lcd *lcd = ctrl->lcd;
	struct s3cfb_user_window user_win;
	void *argp = (void *) arg;
	int ret = 0;

	switch (cmd) {
	case FBIO_ALLOC:
		win->path = (enum s3cfb_data_path_t) argp;
		break;

	case FBIOGET_FSCREENINFO:
		ret = memcpy(argp, &fb->fix, sizeof(fb->fix)) ? 0 : -EFAULT;
		break;

	case FBIOGET_VSCREENINFO:
		ret = memcpy(argp, &fb->var, sizeof(fb->var)) ? 0 : -EFAULT;
		break;

	case FBIOPUT_VSCREENINFO:
		ret = s3cfb_check_var((struct fb_var_screeninfo *) argp, fb);
		if (ret) {
			err("invalid vscreeninfo\n");
			break;
		}

		ret = memcpy(&fb->var, (struct fb_var_screeninfo *) argp, \
				sizeof(fb->var)) ? 0 : -EFAULT;
		if (ret) {
			err("failed to put new vscreeninfo\n");
			break;
		}

		ret = s3cfb_set_par(fb);
		break;

	case S3CFB_WIN_POSITION:
		ret = memcpy(&user_win, (struct s3cfb_user_window __user *) arg, \
				sizeof(user_win)) ? 0 : -EFAULT;
		if (ret) {
			err("failed to S3CFB_WIN_POSITION.\n");
			break;
		}

		if (user_win.x < 0)
			user_win.x = 0;

		if (user_win.y < 0)
			user_win.y = 0;

		if (user_win.x + var->xres > lcd->width)
			win->x = lcd->width - var->xres;
		else
			win->x = user_win.x;

		if (user_win.y + var->yres > lcd->height)
			win->y = lcd->height - var->yres;
		else
			win->y = user_win.y;

		s3cfb_set_window_position(ctrl, win->id);

		break;

	



	default:
		ret = s3cfb_ioctl(fb, cmd, arg);
		break;
	}

	return ret;
}

static int s3cfb_init_fbinfo(int id)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	struct fb_info *fb = ctrl->fb[id];
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct fb_var_screeninfo *var = &fb->var;
	struct s3cfb_window *win = fb->par;
	struct s3cfb_alpha *alpha = &win->alpha;
	struct s3cfb_lcd *lcd = ctrl->lcd;
	struct s3cfb_lcd_timing *timing = &lcd->timing;
	
	memset(win, 0, sizeof(struct s3cfb_window));
	platform_set_drvdata(to_platform_device(ctrl->dev), fb);
	strcpy(fix->id, S3CFB_NAME);

	
	win->id = id;
	win->path = DATA_PATH_DMA;
	win->dma_burst = 16;
	alpha->mode = PLANE_BLENDING;

	
	fb->fbops = &s3cfb_ops;
	fb->flags = FBINFO_FLAG_DEFAULT;
	fb->pseudo_palette = &win->pseudo_pal;
	fix->xpanstep = 0;
	fix->ypanstep = pdata->nr_buffers[id] - 1;
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->accel = FB_ACCEL_NONE;
	fix->visual = FB_VISUAL_TRUECOLOR;
	var->xres = lcd->width;
	var->yres = lcd->height;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres + (var->yres * fix->ypanstep);
	var->bits_per_pixel = 32;
	var->xoffset = 0;
	var->yoffset = 0;
	var->width = VOLANS_LCD_ACTIVE_VIEW_WIDTH;
	var->height = VOLANS_LCD_ACTIVE_VIEW_HEIGHT;
	var->transp.length = 0;

	fix->line_length = var->xres_virtual * var->bits_per_pixel / 8;
	fix->smem_len = fix->line_length * var->yres_virtual;

	var->nonstd = 0;
	var->activate = FB_ACTIVATE_NOW;
	var->vmode = FB_VMODE_NONINTERLACED;
	var->hsync_len = timing->h_sw;
	var->vsync_len = timing->v_sw;
	var->left_margin = timing->h_fp;
	var->right_margin = timing->h_bp;
	var->upper_margin = timing->v_fp;
	var->lower_margin = timing->v_bp;

	var->pixclock = lcd->freq * (var->left_margin + var->right_margin + \
			var->hsync_len + var->xres) * \
			(var->upper_margin + var->lower_margin + \
			var->vsync_len + var->yres);

	lcd_dbg(" pixclock: %d\n", var->pixclock);

	s3cfb_set_bitfield(var);
	s3cfb_set_alpha_info(var, win);

	lcd_dbg(" [fb%d] initialize frame info\n", win->id);

	return 0;
}

static int s3cfb_alloc_framebuffer(void)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	int ret, i;

	lcd_dbg(" allocation of framebuffer\n");

	
	ctrl->fb = (struct fb_info **) kmalloc(pdata->nr_wins * \
			sizeof(struct fb_info *), GFP_KERNEL);
	if (!ctrl->fb) {
		lcd_err( " not enough memory for framebuffers\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	
	for (i = 0; i < pdata->nr_wins; i++) {
		ctrl->fb[i] = framebuffer_alloc(sizeof(struct s3cfb_window), \
				ctrl->dev);
		if (!ctrl->fb[i]) {
			lcd_err( " not enough memory for %d framebuffer\n", i);
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		ret = s3cfb_init_fbinfo(i);
		if (ret) {
			lcd_err( " failed to allocate memory for fb%d\n", i);
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		if (i == pdata->default_win) {
			if (s3cfb_map_video_memory(ctrl->fb[i])) {
				lcd_err( " failed to map video memory " \
					"for default window (%d)\n", i);
				ret = -ENOMEM;
				goto err_alloc_fb;
			}
		}
	}

	return 0;

err_alloc_fb:
	for (i = 0; i < pdata->nr_wins; i++) {
		if (ctrl->fb[i])
			framebuffer_release(ctrl->fb[i]);
	}

	kfree(ctrl->fb);

err_alloc:
	return ret;
}

int s3cfb_register_framebuffer(void)
{
	struct s3c_platform_fb *pdata = to_fb_plat(ctrl->dev);
	int ret, i;

	lcd_dbg(" registering framebuffer\n");


	for (i = 0; i < pdata->nr_wins; i++) {
		ret = register_framebuffer(ctrl->fb[i]);
		if (ret) {
			lcd_err( " failed to register framebuffer device\n");
			return -EINVAL;
		}

#ifndef CONFIG_FRAMEBUFFER_CONSOLE
		if (i == pdata->default_win) {
			s3cfb_check_var(&ctrl->fb[i]->var, ctrl->fb[i]);
			s3cfb_set_par(ctrl->fb[i]);
			s3cfb_draw_logo(ctrl->fb[i]);
		}
#endif
	}

	return 0;
}

static int s3cfb_sysfs_show_win_power(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct s3c_platform_fb *pdata = to_fb_plat(dev);
	struct s3cfb_window *win;
	char temp[16];
	int i;

	for (i = 0; i < pdata->nr_wins; i++) {
		win = ctrl->fb[i]->par;
		sprintf(temp, "[fb%d] %s\n", i, win->enabled ? "on" : "off");
		strcat(buf, temp);
	}

	return strlen(buf);
}

static int s3cfb_sysfs_store_win_power(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	struct s3c_platform_fb *pdata = to_fb_plat(dev);
	char temp[4] = {0, };
	const char *p = buf;
	int id, to;

	while (*p != '\0') {
		if (!isspace(*p))
			strncat(temp, p, 1);
		p++;
	}

	if (strlen(temp) != 2)
		return -EINVAL;

	id = simple_strtoul(temp, NULL, 10) / 10;
	to = simple_strtoul(temp, NULL, 10) % 10;

	if (id < 0 || id > pdata->nr_wins)
		return -EINVAL;

	if (to != 0 && to != 1)
		return -EINVAL;

	if (to == 0)
		s3cfb_disable_window(id);
	else
		s3cfb_enable_window(id);

	return len;
}

static DEVICE_ATTR(win_power, 0644, \
			s3cfb_sysfs_show_win_power,
			s3cfb_sysfs_store_win_power);

static int s3cfb_probe(struct platform_device *pdev)
{
	struct s3c_platform_fb *pdata;
	struct resource *res;
	int ret = 0;

#ifdef CONFIG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_LCD);

	if (s3c_wait_blk_pwr_ready(S3C64XX_BLK_F))
		return -1;
#endif	


	
	ctrl = kzalloc(sizeof(struct s3cfb_global), GFP_KERNEL);

	if (!ctrl) {
		printk(" failed to allocate for global fb structure\n");
		goto err_global;
	}

	ctrl->dev = &pdev->dev;
	s3cfb_set_lcd_info(ctrl);

	
	pdata = to_fb_plat(&pdev->dev);

	
	ctrl->clock = clk_get(&pdev->dev, pdata->clk_name);
	if (IS_ERR(ctrl->clock)) {
		lcd_err( " failed to get fimd clock source\n");
		ret = -EINVAL;
		goto err_clk;
	}

	clk_enable(ctrl->clock);

	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		lcd_err( " failed to get io memory region\n");
		ret = -EINVAL;
		goto err_io;
	}

	
	res = request_mem_region(res->start, \
		res->end - res->start + 1, pdev->name);
	if (!res) {
		lcd_err( " failed to request io memory region\n");
		ret = -EINVAL;
		goto err_io;
	}

	
	ctrl->regs = ioremap(res->start, res->end - res->start + 1);
	if (!ctrl->regs) {
		lcd_err( " failed to remap io region\n");
		ret = -EINVAL;
		goto err_io;
	}

	
	ctrl->irq = platform_get_irq(pdev, 0);

	if (request_irq(IRQ_LCD_VSYNC, s3cfb_irq_frame, IRQF_DISABLED, \
				pdev->name, ctrl)) {
		lcd_err( " request_irq failed\n");
		ret = -EINVAL;
		goto err_irq;
	}

	
	s3cfb_init_global();
	s3cfb_display_on(ctrl);
	
	s3cfb_check_line_count(ctrl);

	
	if (s3cfb_alloc_framebuffer())
	{
		lcd_err( " s3cfb_alloc_framebuffer failed\n");
		goto err_alloc;
	}

	if (s3cfb_register_framebuffer())
	{
		lcd_err( " s3cfb_register_framebuffer failed\n");
		goto err_alloc;
	}

	s3cfb_set_clock(ctrl);
	s3cfb_enable_window(pdata->default_win);

	ret = device_create_file(&(pdev->dev), &dev_attr_win_power);
	if (ret < 0)
		lcd_err( " failed to add sysfs entries\n");

	lcd_dbg(" registered successfully in probe\n");

	
	lcd_pdev = pdev;
	lcd_pm_state = 1;
	return 0;

err_alloc:
	free_irq(ctrl->irq, ctrl);

err_irq:
	iounmap(ctrl->regs);

err_io:
	clk_disable(ctrl->clock);

err_clk:
	clk_put(ctrl->clock);

err_global:
	return ret;
}

static int s3cfb_remove(struct platform_device *pdev)
{
	struct s3c_platform_fb *pdata = to_fb_plat(&pdev->dev);
	struct fb_info *fb;
	int i;

	lcd_dbg(" removing s3cfb\n");

	free_irq(ctrl->irq, ctrl);
	iounmap(ctrl->regs);
	clk_disable(ctrl->clock);
	clk_put(ctrl->clock);

	for (i = 0; i < pdata->nr_wins; i++) {
		fb = ctrl->fb[i];

		
		if (fb) {
			s3cfb_unmap_video_memory(fb);
			s3cfb_set_buffer_address(ctrl, i);
			framebuffer_release(fb);
		}
	}

	kfree(ctrl->fb);
	kfree(ctrl);

	return 0;
}

int s3cfb_suspend(struct platform_device *pdev, pm_message_t state)
{

	lcd_dbg(" suspending...\n");
	
	if (lcd_pm_state != 0) {
		s3cfb_display_off(ctrl);
		clk_disable(ctrl->clock);

		s3c6410_pm_do_save(s3c_lcd_save, ARRAY_SIZE(s3c_lcd_save));

		__raw_writel(0x0, S3C64XX_GPICON);
		__raw_writel(0x55555555, S3C64XX_GPIPUD);
		__raw_writel(0x0, S3C64XX_GPJCON);
		__raw_writel(0x55555555, S3C64XX_GPJPUD);

#ifdef CONFIG_DOMAIN_GATING
		s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_LCD);
#endif	
	}
	else
	{
		lcd_dbg( "[WARNING] Not suspending, lcd_pm_state=%d\n", lcd_pm_state);
	}

	return 0;
}

int s3cfb_resume(struct platform_device *pdev)
{
	struct s3c_platform_fb *pdata = to_fb_plat(&pdev->dev);
	struct fb_info *fb;
	struct s3cfb_window *win;
	int i;

	lcd_dbg(" wake up from suspend\n");

	if (pdata->cfg_gpio)
		pdata->cfg_gpio(pdev);

#ifdef CONFIG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_LCD);

	if (s3c_wait_blk_pwr_ready(S3C64XX_BLK_F))
	{
		lcd_err( " cannot resume since S3C64XX_BLK_F is not ready\n");
		return -1;
	}
#endif	

	clk_enable(ctrl->clock);
	s3cfb_init_global();
	s3cfb_set_clock(ctrl);
	s3cfb_display_on(ctrl);

	for (i = 0; i < pdata->nr_wins; i++) {
		fb = ctrl->fb[i];
		win = fb->par;

		if (win->enabled) {
			lcd_dbg(" trying to enable %d window \n", i);
			s3cfb_check_var(&fb->var, fb);
			s3cfb_set_par(fb);
			s3cfb_enable_window(win->id);
		}
	}

	
	s3c6410_pm_do_restore(s3c_lcd_save, ARRAY_SIZE(s3c_lcd_save));
	lcd_pm_state = 1;

	return 0;
}

static struct platform_driver s3cfb_driver = {
	.probe		= s3cfb_probe,
	.remove		= s3cfb_remove,
	.suspend	= s3cfb_suspend,
	.resume		= s3cfb_resume,
	.driver		= {
		.name	= S3CFB_NAME,
		.owner	= THIS_MODULE,
	},
};

static int s3cfb_register(void)
{
	platform_driver_register(&s3cfb_driver);

	return 0;
}

static void s3cfb_unregister(void)
{
	platform_driver_unregister(&s3cfb_driver);
}

module_init(s3cfb_register);
module_exit(s3cfb_unregister);
	
MODULE_AUTHOR("Jinsung, Yang <jsgood.yang@samsung.com>");
MODULE_DESCRIPTION("Samsung Display Controller (FIMD) driver");
MODULE_LICENSE("GPL");


void s3cfb_pre_suspend(void)
{

	lcd_dbg(" s3cfb_pre_suspend...\n");
	
	s3cfb_display_off(ctrl);
	clk_disable(ctrl->clock);

	
	s3c6410_pm_do_save(s3c_lcd_save, ARRAY_SIZE(s3c_lcd_save));

	__raw_writel(0x0, S3C64XX_GPICON);
	__raw_writel(0x55555555, S3C64XX_GPIPUD);
	__raw_writel(0x0, S3C64XX_GPJCON);
	__raw_writel(0x55555555, S3C64XX_GPJPUD);

#ifdef CONFIG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_LCD);
#endif	

	
	lcd_pm_state = 0;
}

void s3cfb_post_resume(void)
{

	lcd_dbg(" s3cfb_post_resume...\n");
	
	if (lcd_pm_state == 0) {
		struct s3c_platform_fb *pdata = to_fb_plat(&lcd_pdev->dev);
		struct fb_info *fb;
		struct s3cfb_window *win;
		int i;

#ifdef CONFIG_DOMAIN_GATING
		s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_LCD);

		if (s3c_wait_blk_pwr_ready(S3C64XX_BLK_F)) {
			lcd_err( " cannot post_resume since S3C64XX_BLK_F is not ready\n");
			
		}
#endif	

		clk_enable(ctrl->clock);
		s3cfb_init_global();
		s3cfb_set_clock(ctrl);
		s3cfb_display_on(ctrl);

		
		s3c6410_pm_do_restore(s3c_lcd_save, ARRAY_SIZE(s3c_lcd_save));

		for (i = 0; i < pdata->nr_wins; i++) {
			fb = ctrl->fb[i];
			win = fb->par;

			if (win->enabled) {
				s3cfb_check_var(&fb->var, fb);
				s3cfb_set_par(fb);
				s3cfb_enable_window(win->id);
			}
		}
	}
	else
	{
		lcd_dbg( "[WARNING] Not post_resume, lcd_pm_state=%d\n", lcd_pm_state);
	}
}
