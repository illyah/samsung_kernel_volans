#ifndef _KXSD9_DEV_H
#define _KXSD9_DEV_H

#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <mach/volans.h>

#include "KXSD9_main.h"

#define ENABLED    1
#define DISABLED   0

#define LATCHED    1
#define UNLATCHED  0
#define DONT_CARE  2

#define R2g_T1g       1
#define R2g_T1p5g     2
#define R4g_T2g       3
#define R4g_T3g       4
#define R6g_T3g       5
#define R6g_T4p5g     6
#define R8g_T4g       7
#define R8g_T6g       8

#define BW50          1
#define BW100         2 
#define BW500         3
#define BW1000        4
#define BW2000        5
#define BW_no_filter  6

#define DEFAULT_ACC_X1		2048
#define DEFAULT_ACC_Y1		1229
#define DEFAULT_ACC_Z1		2048
#define DEFAULT_ACC_X2		2048
#define DEFAULT_ACC_Y2		2048
#define DEFAULT_ACC_Z2		2867

typedef struct
{
    unsigned short X_acc;
    unsigned short Y_acc;
    unsigned short Z_acc;
}KXSD9_acc_t;

typedef struct 
{
    unsigned short x1;
    unsigned short y1;
    unsigned short z1;
    unsigned short x2;
    unsigned short y2;
    unsigned short z2;
}KXSD9_default_t;

typedef struct 
{
    unsigned short counts_per_g;
    unsigned short error_RT;
}KXSD9_sensitivity_t;

typedef struct
{
    unsigned short counts;
    unsigned short error_RT;
}KXSD9_zero_g_offset_t;

typedef struct
{
    unsigned short mwup_rsp;
}KXSD9_mwup_rsp_t;

extern int KXSD9_dev_init(struct i2c_client *, KXSD9_module_param_t *);
extern int KXSD9_dev_exit(void);

extern void KXSD9_dev_mutex_init(void); 

extern void KXSD9_dev_work_func(struct work_struct *);

extern int KXSD9_dev_get_sensitivity(KXSD9_sensitivity_t *);
extern int KXSD9_dev_get_zero_g_offset(KXSD9_zero_g_offset_t *);
extern int KXSD9_dev_get_acc(KXSD9_acc_t *);
extern int KXSD9_dev_get_default(KXSD9_default_t *);

extern int KXSD9_dev_enable(void);
extern int KXSD9_dev_disable(void);

extern int KXSD9_dev_mwup_enb(unsigned short);
extern int KXSD9_dev_mwup_disb(void);

extern int KXSD9_dev_mwup_status(void);

#endif

