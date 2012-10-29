#ifndef _COMMON_H
#define _COMMON_H

#include <linux/kernel.h>

#define KXSD9_DEBUG
#undef KXSD9_DEBUG

#define error(fmt,arg...) printk(KERN_CRIT fmt "\n",## arg)

#ifdef KXSD9_DEBUG
#define debug(fmt,arg...) printk(KERN_CRIT "--------" fmt "\n",## arg)
#else
#define debug(fmt,arg...)
#endif

#define SET   1
#define RESET 0

#define OK  1
#define N_OK  -1

#define YES  1
#define NO  0

#endif

