











#ifndef _S3CFB_H
#define _S3CFB_H

#ifdef __KERNEL__
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <plat/fb.h>
#include "dprintk.h"
#endif





#define S3CFB_NAME		"s3cfb"

#define info(args...)		do { printk(KERN_INFO S3CFB_NAME ": " args); } while (0)
#define err(args...)		do { printk(KERN_ERR  S3CFB_NAME ": " args); } while (0)

#define S3CFB_AVALUE(r, g, b)	(((r & 0xf) << 8) | ((g & 0xf) << 4) | ((b & 0xf) << 0))
#define S3CFB_CHROMA(r, g, b)	(((r & 0xff) << 16) | ((g & 0xff) << 8) | ((b & 0xff) << 0))






//#define CONFIG_FIX_FIMD_FLSAHING_ISSUE





enum s3cfb_data_path_t {
	DATA_PATH_DMA,
	DATA_PATH_FIFO,
};

enum s3cfb_alpha_t {
	PLANE_BLENDING,
	PIXEL_BLENDING,
};

enum s3cfb_chroma_dir_t {
	CHROMA_FG,
	CHROMA_BG,
};

enum s3cfb_output_t {
	OUTPUT_RGB,
	OUTPUT_ITU,
	OUTPUT_I80LDI0,
	OUTPUT_I80LDI1,
	OUTPUT_TV,
	OUTPUT_TV_RGB,
	OUTPUT_TV_I80LDI0,
	OUTPUT_TV_I80LDI1,
};

enum s3cfb_rgb_mode_t {
	MODE_RGB_P = 0,
	MODE_BGR_P = 1,
	MODE_RGB_S = 2,
	MODE_BGR_S = 3,
};













struct s3cfb_alpha {
	enum 		s3cfb_alpha_t mode;
	int		channel;
	unsigned int	value;
};











struct s3cfb_chroma {
	int 		enabled;
	int 		blended;
	unsigned int	key;
	unsigned int	comp_key;
	unsigned int	alpha;
	enum 		s3cfb_chroma_dir_t dir;
};








struct s3cfb_lcd_polarity {
	int	rise_vclk;
	int	inv_hsync;
	int	inv_vsync;
	int	inv_vden;
};











struct s3cfb_lcd_timing {
	int	h_fp;
	int	h_bp;
	int	h_sw;
	int	v_fp;
	int	v_fpe;
	int	v_bp;
	int	v_bpe;
	int	v_sw;
};












struct s3cfb_lcd {
	int	width;
	int	height;
	int	bpp;
	int	freq;
	struct 	s3cfb_lcd_timing timing;
	struct 	s3cfb_lcd_polarity polarity;

	void 	(*init_ldi)(void);
};















struct s3cfb_window {
	int		id;
	int		enabled;
	atomic_t		in_use;
	wait_queue_head_t	wq;
	int		x;
	int		y;
	enum 		s3cfb_data_path_t path;
	int		local_channel;
	int		dma_burst;
	unsigned int	pseudo_pal[16];
	struct		s3cfb_alpha alpha;
	struct		s3cfb_chroma chroma;
};













struct s3cfb_global {
	
	void __iomem		*regs;
	struct mutex		lock;
	struct device		*dev;
	struct clk		*clock;
	int			irq;
	wait_queue_head_t	wq;
	unsigned int		wq_count;
	struct fb_info		**fb;

	
	int			enabled;
	int			dsi;
	int			interlace;
	
	enum s3cfb_output_t 	output;
	enum s3cfb_rgb_mode_t	rgb_mode;
	struct s3cfb_lcd 	*lcd;
};






struct s3cfb_user_window {
	int x;
	int y;
};

struct s3cfb_user_plane_alpha {
	int 		channel;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};

struct s3cfb_user_chroma {
	int 		enabled;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};





#define FBIO_WAITFORVSYNC		_IO  ('F', 32)
#define S3CFB_WIN_ON			_IO  ('F', 200)
#define S3CFB_WIN_OFF			_IO  ('F', 201)
#define S3CFB_WIN_OFF_ALL		_IO  ('F', 202)
#define S3CFB_WIN_POSITION		_IOW ('F', 203, struct s3cfb_user_window)
#define S3CFB_WIN_SET_PLANE_ALPHA	_IOW ('F', 204, struct s3cfb_user_plane_alpha)
#define S3CFB_WIN_SET_CHROMA		_IOW ('F', 205, struct s3cfb_user_chroma)
#define S3CFB_SET_VSYNC_INT		_IOW ('F', 206, unsigned int)




#define LCD_DEBUG(fmt, ...)						\
	do {								\
			dprintk(LCD_DBG, "[FIMD] %s() "		\
				fmt, __FUNCTION__, ##__VA_ARGS__);			\
	} while(0)

#define LCD_INFO(fmt, ...)						\
	do {								\
			dprintk(LCD_INF, "[FIMD][INFO] %s() "		\
				fmt, __FUNCTION__, ##__VA_ARGS__);			\
	} while (0)

#define LCD_WARN(fmt, ...)						\
	do {								\
			printk("[FIMD][WARNING] %s() "		\
				fmt, __FUNCTION__, ##__VA_ARGS__);			\
	} while (0)


#define LCD_ERROR(fmt, ...)						\
	do {								\
			printk("[FIMD][ERROR] %s() "		\
				fmt, __FUNCTION__, ##__VA_ARGS__);			\
	} while (0)


#define lcd_dbg(fmt, ...)		LCD_DEBUG(fmt, ##__VA_ARGS__)
#define lcd_info(fmt, ...)		LCD_INFO(fmt, ##__VA_ARGS__)
#define lcd_warn(fmt, ...)		LCD_WARN(fmt, ##__VA_ARGS__)
#define lcd_err(fmt, ...)		LCD_ERROR(fmt, ##__VA_ARGS__)






extern int soft_cursor(struct fb_info *info, struct fb_cursor *cursor);
extern void s3cfb_set_lcd_info(struct s3cfb_global *ctrl);
extern struct s3c_platform_fb *to_fb_plat(struct device *dev);
extern void s3cfb_check_line_count(struct s3cfb_global *ctrl);
extern int s3cfb_set_output(struct s3cfb_global *ctrl);
extern int s3cfb_set_display_mode(struct s3cfb_global *ctrl);
extern int s3cfb_display_on(struct s3cfb_global *ctrl);
extern int s3cfb_display_off(struct s3cfb_global *ctrl);
extern int s3cfb_frame_off(struct s3cfb_global *ctrl);
extern int s3cfb_set_clock(struct s3cfb_global *ctrl);
extern int s3cfb_set_polarity(struct s3cfb_global *ctrl);
extern int s3cfb_set_timing(struct s3cfb_global *ctrl);
extern int s3cfb_set_lcd_size(struct s3cfb_global *ctrl);
extern int s3cfb_set_global_interrupt(struct s3cfb_global *ctrl, int enable);
extern int s3cfb_set_vsync_interrupt(struct s3cfb_global *ctrl, int enable);
extern int s3cfb_clear_interrupt(struct s3cfb_global *ctrl);
extern int s3cfb_window_on(struct s3cfb_global *ctrl, int id);
extern int s3cfb_window_off(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_window_control(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_alpha_blending(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_window_position(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_window_size(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_buffer_address(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_buffer_size(struct s3cfb_global *ctrl, int id);
extern int s3cfb_set_chroma_key(struct s3cfb_global *ctrl, int id);

#ifdef CONFIG_FIX_FIMD_FLSAHING_ISSUE
extern int s3cfb_disable_local_win0();
extern int s3cfb_frame_on(struct s3cfb_global *ctrl);
extern void s3cfb_check_line_count100(struct s3cfb_global *ctrl);

#endif
#endif 
