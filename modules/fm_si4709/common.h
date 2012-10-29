#ifndef _COMMON_H
#define _COMMON_H

#include <linux/kernel.h>
#include <linux/types.h>

#define GPIO_LEVEL_HIGH             1
#define GPIO_LEVEL_LOW              0
//#define Si4709_DEBUG

#define error(fmt,arg...) printk(KERN_CRIT fmt "\n",## arg)

#ifdef Si4709_DEBUG
#define debug(fmt,arg...) printk(KERN_CRIT "--------" fmt "\n",## arg)
#else
#define debug(fmt,arg...)
#endif

#define YES  1
#define NO  0

#endif

