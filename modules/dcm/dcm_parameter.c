/*
 * dcm_parameter.c
 *
 * Parameter read & save driver on parameter partition.
 *
 * COPYRIGHT(C) Samsung Electronics Co.Ltd. 2006-2010 All Right Reserved.  
 *
 * Author: Jeonghean Min <jeonghwan.min@samsung.com>	@LDK@
 *
 * 20080226. Supprot both UFD & BML layer.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/blkpg.h>
#include <linux/hdreg.h>
#include <linux/genhd.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <mach/hardware.h>

#include "dcm_ext.h"

#define SECTOR_BITS		9

#define PARAM_LEN		(32 * 2048) 

#ifdef DCM_PARAM_DEBUG
static char * get_param_idx_string(int idx)
{
	switch(idx){
		case __SERIAL_SPEED:	return "serial_speed";
		case __LOAD_RAMDISK:	return "load ramdisk";
		case __BOOT_DELAY:		return "boot delay";
		case __LCD_LEVEL:		return "lcd level";
		case __SWITCH_SEL:		return "switch select";
		case __PHONE_DEBUG_ON:	return "phone debug on";
		case __BL_DIM_TIME:		return "backlight dim time";
		case __MELODY_MODE:		return "melody mode";
		case __DOWNLOAD_MODE:	return "download mode";
		case __NATION_SEL:		return "nation select";
		case __PARAM_INT_11:	return "param int 11";
		case __PARAM_INT_12:	return "param int 12";
		case __PARAM_INT_13:	return "param int 13";
		case __PARAM_INT_14:	return "param int 14";
		case __VERSION:			return "version";
		case __CMDLINE:			return "cmdline";
		case __PARAM_STR_2:		return "param str 2";
		case __PARAM_STR_3:		return "param str 3";
		case __PARAM_STR_4:		return "param str 4";
		default:	return "Unknown";	
	}
}

static void dump_param_status(status_t * st)
{
	int i;

	printk("==============================================\n");
	printk("param magic : %d\n", st->param_magic);
	printk("param version : %d\n", st->param_version);

	for(i=0; i< (MAX_PARAM - MAX_STRING_PARAM);i++)
	{
		printk("param = %s , value = %d\n", 
				get_param_idx_string(st->param_list[i].ident),
				st->param_list[i].value);
	}

	for(i=0; i< MAX_STRING_PARAM; i++)
	{
		printk("param = %s , value = %s\n",
				get_param_idx_string(st->param_str_list[i].ident),
				st->param_str_list[i].value);
	}
	printk("==============================================\n");
}
#endif


int write_param_block(unsigned int dev_id, unsigned char *addr)
{
	unsigned int i, err;
	unsigned int first;
	unsigned int nBytesReturned = 0;
	unsigned char *pbuf, *buf;
	
	if (addr == NULL) {
		printk(KERN_ERR "%s: wrong address\n", __FUNCTION__);
		return -EINVAL;
	}	
		
	buf = kmalloc(PARAM_LEN, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	err = 0; 
	pbuf = buf;
	pbuf = buf;
	
fail2:
	return err;
	
}

int read_param_block(unsigned int dev_id, unsigned char *addr)
{
	unsigned int i, err;
	unsigned int first;
	
	if (addr == NULL){
		printk(KERN_ERR "%s: wrong address\n", __FUNCTION__);
		return -EINVAL;
	}	
		
	return 0;
}



static status_t param_status;

static status_t param_default_status;

static int load_parameter_value(void)
{
	unsigned char *addr = NULL;
	unsigned int err = 0, dev_id = 0;

	status_t *status;

	addr = kmalloc(PARAM_LEN, GFP_KERNEL);
	if (!addr)
		return -ENOMEM;
	err = read_param_block(dev_id, addr);
	if (err) {
		printk(KERN_ERR "%s: read param error\n", __FUNCTION__);
		goto fail;
	}

	status = (status_t *)addr;

	if ((status->param_magic == PARAM_MAGIC) &&
			(status->param_version == PARAM_VERSION)) {
		memcpy(&param_status, (char *)status, sizeof(status_t));			
	}
	else {
		printk(KERN_ERR "%s: no param info\n", __FUNCTION__);
		err = -1;
	}	

fail:
	kfree(addr);
	return err;
}

int save_parameter_value(void)
{
	unsigned int err = 0, dev_id = 0;
	unsigned char *addr = NULL;

	addr = kmalloc(PARAM_LEN, GFP_KERNEL);
	if (!addr)
		return -ENOMEM;
	memset(addr, 0, PARAM_LEN);
	memcpy(addr, &param_status, sizeof(status_t));
	
	err = write_param_block(dev_id, addr);
	if (err) {
		printk(KERN_ERR "%s: read param error\n", __FUNCTION__);
		goto fail;
	}

#ifdef DCM_PARAM_DEBUG
	dump_param_status(&param_status);
#endif

fail:
	kfree(addr);
	return err;
}
EXPORT_SYMBOL(save_parameter_value);

void set_parameter_value(int idx, void *value)
{
	int i, str_i;

#ifdef DCM_PARAM_DEBUG
	if(idx < (MAX_PARAM - MAX_STRING_PARAM))
	{
		printk("[dcm] set_parameter_value : idx = %s , value = %d\n",
				get_param_idx_string(idx), *(int *)value);
	}
	else
	{
		printk("[dcm] set_parameter_value : idx = %s , value = %s\n",
				get_param_idx_string(idx), (char *)value);
	}
#endif

	for (i = 0; i < MAX_PARAM; i++) {
		if (i < (MAX_PARAM - MAX_STRING_PARAM)) {	
			if(param_status.param_list[i].ident == idx) {
				param_status.param_list[i].value = *(int *)value;
			}
		}
		else {
			str_i = (i - (MAX_PARAM - MAX_STRING_PARAM));
			if(param_status.param_str_list[str_i].ident == idx) {
				strlcpy(param_status.param_str_list[str_i].value, 
					(char *)value, PARAM_STRING_SIZE);
			}
		}
	}
}
EXPORT_SYMBOL(set_parameter_value);

void read_parameter_value(int idx, void *value)
{
	int i, str_i;

	for (i = 0 ; i < MAX_PARAM; i++) {
		if (i < (MAX_PARAM - MAX_STRING_PARAM)) {	
			if(param_status.param_list[i].ident == idx) {
				*(int *)value = param_status.param_list[i].value;
			}
		}
		else {
			str_i = (i - (MAX_PARAM - MAX_STRING_PARAM));
			if(param_status.param_str_list[str_i].ident == idx) {
				strlcpy((char *)value, 
					param_status.param_str_list[str_i].value, PARAM_STRING_SIZE);
			}
		}
	}
}
EXPORT_SYMBOL(read_parameter_value);

void dcm_parameter_init(void)
{
	int ret;

	memset(&param_default_status, 0, sizeof(status_t));

	param_default_status.param_magic = PARAM_MAGIC;
	param_default_status.param_version = PARAM_VERSION;
	param_default_status.param_list[0].ident = __SERIAL_SPEED;
	param_default_status.param_list[0].value = TERMINAL_SPEED;
	param_default_status.param_list[1].ident = __LOAD_RAMDISK;
	param_default_status.param_list[1].value = LOAD_RAMDISK;
	param_default_status.param_list[2].ident = __BOOT_DELAY;
	param_default_status.param_list[2].value = BOOT_DELAY;
	param_default_status.param_list[3].ident = __LCD_LEVEL;
	param_default_status.param_list[3].value = LCD_LEVEL;
	param_default_status.param_list[4].ident = __SWITCH_SEL;
	param_default_status.param_list[4].value = SWITCH_SEL;
	param_default_status.param_list[5].ident = __PHONE_DEBUG_ON;
	param_default_status.param_list[5].value = PHONE_DEBUG_ON;
	param_default_status.param_list[6].ident = __BL_DIM_TIME;
	param_default_status.param_list[6].value = BL_DIM_TIME;
	param_default_status.param_list[7].ident = __MELODY_MODE;
	param_default_status.param_list[7].value = MELODY_MODE;
	param_default_status.param_list[8].ident = __DOWNLOAD_MODE;
	param_default_status.param_list[8].value = DOWNLOAD_MODE;
	param_default_status.param_list[9].ident = __NATION_SEL;
	param_default_status.param_list[9].value = NATION_SEL;
	param_default_status.param_str_list[0].ident = __VERSION;
	strlcpy(param_default_status.param_str_list[0].value,
				VERSION_LINE, PARAM_STRING_SIZE);
	param_default_status.param_str_list[1].ident = __CMDLINE;

	ret = load_parameter_value();
	if (ret < 0) {
		printk(KERN_ERR "%s: relocated default param\n", __FUNCTION__);
		memcpy(&param_status, &param_default_status, sizeof(status_t));			
	}
}

void dcm_parameter_exit(void)
{
}
