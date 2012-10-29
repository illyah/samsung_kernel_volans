
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include "KXSD9_regs.h"
#include "KXSD9_main.h"
#include "KXSD9_dev.h"
#include "common.h"



#include<mach/gpio.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>

enum
{
    eTRUE,
    eFALSE,
}dev_struct_status_t;

typedef struct
{
    unsigned short R_T;
    unsigned short BW;
    KXSD9_sensitivity_t sensitivity;
    KXSD9_zero_g_offset_t zero_g_offset;
}dev_settings_t;

typedef struct
{
    unsigned short dev_state;
    unsigned short mwup;
    unsigned short mwup_rsp;	
}dev_mode_t;

typedef struct 
{
    struct mutex lock;
	
    struct i2c_client const *client;

    dev_mode_t mode;
    dev_settings_t settings;

    unsigned short valid;	
}KXSD9_device_t;

int KXSD9_dev_init(struct i2c_client *client, KXSD9_module_param_t *);
int KXSD9_dev_exit(void);

void KXSD9_dev_mutex_init(void);

int KXSD9_dev_mwup_status(void);

int KXSD9_dev_get_sensitivity(KXSD9_sensitivity_t *);
int KXSD9_dev_get_zero_g_offset(KXSD9_zero_g_offset_t *);
int KXSD9_dev_get_acc(KXSD9_acc_t *);
int KXSD9_dev_get_default(KXSD9_default_t *);

int KXSD9_dev_enable(void);
int KXSD9_dev_disable(void);

extern int KXSD9_dev_mwup_enb(unsigned short);
extern int KXSD9_dev_mwup_dsb(void);
static void set_val(unsigned char *, unsigned char *, 
                              KXSD9_module_param_t *); 

static inline int get_X_acc(unsigned short *);
static inline int get_Y_acc(unsigned short *);
static inline int get_Z_acc(unsigned short *);
static int get_acc(char , char , unsigned short *);

static int i2c_read(unsigned char, unsigned char *);
static int i2c_write(unsigned char, unsigned char);
static KXSD9_device_t KXSD9_dev =
{
    .client = NULL,
    .valid = eFALSE,
};    

int KXSD9_dev_init(struct i2c_client *client, KXSD9_module_param_t *param)
{
    int ret = 0;

    unsigned char ctrl_regsc=0, ctrl_regsb=0;

    debug("KXSD9_dev_init called");

    s3c_gpio_cfgpin(GPIO_ACC_INT, S3C_GPIO_SFN(GPIO_ACC_INT_AF));

    mutex_lock(&(KXSD9_dev.lock));   

    KXSD9_dev.client = client;	

    CTRL_REGSB_BITSET_KXSD9_nor_op(&ctrl_regsb);
    CTRL_REGSB_BITSET_clk_ctrl_on(&ctrl_regsb);
	
    KXSD9_dev.mode.dev_state = ENABLED;	

    KXSD9_dev.settings.sensitivity.error_RT = 25;
    
    KXSD9_dev.settings.zero_g_offset.counts = 2048;
    KXSD9_dev.settings.zero_g_offset.error_RT = 205;
    
    set_val(&ctrl_regsb, &ctrl_regsc, param);

    if( (ret = i2c_write(CTRL_REGSB,ctrl_regsb)) < 0 )
    {
        debug("KXSD9_dev_init->i2c_write 1 failed");
    }
    else
    {
        mdelay(16);
        
        if( (ret = i2c_write(CTRL_REGSC,ctrl_regsc)) < 0 )
        {
            debug("KXSD9_dev_init->i2c_write 2 failed");
        }
        else
        {
            KXSD9_dev.valid = eTRUE;
        }
    }

    mutex_unlock(&(KXSD9_dev.lock));   	 

    debug("KXSD9_dev_init call over");
	 
    return ret;
}

static void set_val(unsigned char *ctrl_regsb, unsigned char *ctrl_regsc, 
                              KXSD9_module_param_t *param) 
{
    switch(param->mwup)
    {
        case ENABLED:
             CTRL_REGSB_BITSET_mwup_enb(ctrl_regsb);	
             KXSD9_dev.mode.mwup = ENABLED;
				
             debug("mwup = ENABLED");				
				
             switch( param->mwup_rsp )
             {
                case  LATCHED:
                default:						
                      CTRL_REGSC_BITSET_mwup_rsp_latchd(ctrl_regsc);	
                      KXSD9_dev.mode.mwup_rsp = LATCHED;
								  
                      debug("mwup_rsp = LATCHED");				  
                      break;
								  
                case  UNLATCHED:
                      CTRL_REGSC_BITSET_mwup_rsp_unlatchd(ctrl_regsc);	
                      KXSD9_dev.mode.mwup_rsp = UNLATCHED;	
								  
                      debug("mwup_rsp = UNLATCHED");										  
                      break;								  
             }
             break;
				
        case DISABLED:
        default:				
             CTRL_REGSB_BITSET_mwup_disb(ctrl_regsb);	
             KXSD9_dev.mode.mwup = DISABLED;
             KXSD9_dev.mode.mwup_rsp = DONT_CARE;
		
             debug("mwup = DISABLED");
             debug("mwup_rsp = DONT_CARE");						
             break;	
    }			

    switch(param->BW)
    {
        case BW50:
        default:			
             CTRL_REGSC_BITSET_BW50(ctrl_regsc);
             KXSD9_dev.settings.BW = BW50;

             debug("BW = BW50");				
             break;

        case BW100:
             CTRL_REGSC_BITSET_BW100(ctrl_regsc);
             KXSD9_dev.settings.BW = BW100;
				
             debug("BW = BW100");				
             break;

        case BW500:
             CTRL_REGSC_BITSET_BW500(ctrl_regsc);
             KXSD9_dev.settings.BW = BW500;	

             debug("BW = BW500");				
             break;

        case BW1000:
             CTRL_REGSC_BITSET_BW1000(ctrl_regsc);
             KXSD9_dev.settings.BW = BW1000;	

             debug("BW = BW1000");				
             break;				

        case BW2000:
             CTRL_REGSC_BITSET_BW2000(ctrl_regsc);
             KXSD9_dev.settings.BW = BW2000;	

             debug("BW = BW2000");				
             break;	

        case BW_no_filter:
             CTRL_REGSC_BITSET_BW_no_filter(ctrl_regsc);
             KXSD9_dev.settings.BW = BW_no_filter;		
				
             debug("BW = BW_no_filter");			
             break;
    }			

    switch(param->R_T)
    {
        case R2g_T1g:
        default:			
             CTRL_REGSC_BITSET_R2g_T1g(ctrl_regsc);
             KXSD9_dev.settings.R_T= R2g_T1g;	
             KXSD9_dev.settings.sensitivity.counts_per_g = 819;

             debug("R_T = R2g_T1g");
             debug("Sensitivity = 819 counts/g");
             break;

        case R2g_T1p5g:
             CTRL_REGSC_BITSET_R2g_T1p5g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R2g_T1p5g;	
             KXSD9_dev.settings.sensitivity.counts_per_g = 819;

             debug("R_T = R2g_T1p5g");
             debug("Sensitivity = 819 counts/g");
             break;

        case R4g_T2g:
             CTRL_REGSC_BITSET_R4g_T2g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R4g_T2g;
             KXSD9_dev.settings.sensitivity.counts_per_g = 410;

             debug("R_T = R4g_T2g");
             debug("Sensitivity = 410 counts/g");
             break;

        case R4g_T3g:
             CTRL_REGSC_BITSET_R4g_T3g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R4g_T3g;
             KXSD9_dev.settings.sensitivity.counts_per_g = 410;

             debug("R_T = R4g_T3g");
             debug("Sensitivity = 410 counts/g");
             break;			

        case R6g_T3g:
             CTRL_REGSC_BITSET_R6g_T3g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R6g_T3g;
             KXSD9_dev.settings.sensitivity.counts_per_g = 273;

             debug("R_T = R6g_T3g");
             debug("Sensitivity = 273 counts/g");
             break;

        case R6g_T4p5g:
             CTRL_REGSC_BITSET_R6g_T4p5g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R6g_T4p5g;
             KXSD9_dev.settings.sensitivity.counts_per_g = 273;

             debug("R_T = R6g_T4p5g");
             debug("Sensitivity = 273 counts/g");
             break;

        case R8g_T4g:
             CTRL_REGSC_BITSET_R8g_T4g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R8g_T4g;	
             KXSD9_dev.settings.sensitivity.counts_per_g = 205;

             debug("R_T = R8g_T4g");
             debug("Sensitivity = 205 counts/g");
             break;

        case R8g_T6g:
             CTRL_REGSC_BITSET_R8g_T6g(ctrl_regsc);
             KXSD9_dev.settings.R_T = R8g_T6g;
             KXSD9_dev.settings.sensitivity.counts_per_g = 205;

             debug("R_T = R8g_T6g");
             debug("Sensitivity = 205 counts/g");
             break;				
    }	    

}

int KXSD9_dev_exit(void)
{
    int ret = 0;
	 
    debug("KXSD9_dev_exit called");

    mutex_lock(&(KXSD9_dev.lock));   

    KXSD9_dev.client = NULL;	

    KXSD9_dev.valid = eFALSE;

    mutex_unlock(&(KXSD9_dev.lock)); 

    debug("KXSD9_dev_exit call over");

    return ret;
}

void KXSD9_dev_mutex_init(void)
{
    mutex_init(&(KXSD9_dev.lock));
}

int KXSD9_dev_mwup_status(void)
{
    int ret;
    
    mutex_lock(&(KXSD9_dev.lock)); 

    if( (KXSD9_dev.valid == eFALSE) ||( KXSD9_dev.mode.dev_state == DISABLED ) )
    {
        ret = N_OK;
    }
    else
    {
        ret = KXSD9_dev.mode.mwup;
    }
    
    mutex_unlock(&(KXSD9_dev.lock)); 

    return ret;
}

int KXSD9_dev_enable(void)
{
    int ret = 0;
    unsigned char reg_data;	 
	 
    debug("KXSD9_dev_enable called");

    mutex_lock(&(KXSD9_dev.lock));   

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_enable called when DS is invalid");
	 ret = -1;	  
    }
    else if( (ret = i2c_read(CTRL_REGSB,&reg_data)) < 0 )
    {
        debug("KXSD9_dev_enable->i2c_read failed");
    }
    else	 	
    {
        CTRL_REGSB_BITSET_KXSD9_nor_op(&reg_data);
        CTRL_REGSB_BITSET_clk_ctrl_on(&reg_data);		

        if( (ret = i2c_write(CTRL_REGSB, reg_data)) < 0 )
        {
            debug("KXSD9_dev_enable->i2c_write failed");
        }
        else
        {
            mdelay(16);
            
            KXSD9_dev.mode.dev_state = ENABLED;
        }
    } 
	 
    mutex_unlock(&(KXSD9_dev.lock)); 

    debug("KXSD9_dev_enable call over");

    return ret;
}

int KXSD9_dev_disable(void)
{
    int ret = 0;
    unsigned char reg_data;	 
	 
    debug("KXSD9_dev_disable called");

    mutex_lock(&(KXSD9_dev.lock));   

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_disable called when DS is invalid");
        ret = -1;	  
    }
    else if( (ret = i2c_read(CTRL_REGSB,&reg_data)) < 0 )
    {
        debug("KXSD9_dev_disable->i2c_read failed");
    }
    else	 	
    {
        CTRL_REGSB_BITSET_KXSD9_standby(&reg_data);
        CTRL_REGSB_BITSET_clk_ctrl_off(&reg_data);

        if( (ret = i2c_write(CTRL_REGSB, reg_data)) < 0 )
        {
            debug("KXSD9_dev_disable->i2c_write failed");
        }
        else
        {
            KXSD9_dev.mode.dev_state = DISABLED;
        }
    } 
	 
    mutex_unlock(&(KXSD9_dev.lock)); 

    debug("KXSD9_dev_disable call over");

    return ret;
}

int KXSD9_dev_mwup_enb(unsigned short mwup_rsp)
{
    int ret = 0;
    unsigned char ctrl_regsb, ctrl_regsc, ctrl_regsb_bkup;
    unsigned short mwup_prev, mwup_rsp_prev;

    debug("KXSD9_dev_mwup_enb called");

    mutex_lock(&(KXSD9_dev.lock));   

    mwup_prev = KXSD9_dev.mode.mwup;
    mwup_rsp_prev = KXSD9_dev.mode.mwup_rsp;

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_mwup_enb called with dev DS invalid");
        ret = -1;
    }
    else if( KXSD9_dev.mode.dev_state == DISABLED )
    {
        debug("KXSD9_dev_mwup_enb: dev is DISABLED");
        ret = -1;		
    }
    else if( (ret = i2c_read(CTRL_REGSB, &ctrl_regsb)) < 0 )
    {
        debug("KXSD9_dev_mwup_enb i2c_read1 failed");
    }
    else if( (ret = i2c_read(CTRL_REGSC, &ctrl_regsc)) < 0 )
    {
        debug("KXSD9_dev_mwup_enb i2c_read2 failed");
    }
    else
    {
        ctrl_regsb_bkup = ctrl_regsb;
        
        CTRL_REGSB_BITSET_mwup_enb(&ctrl_regsb);	
        KXSD9_dev.mode.mwup = ENABLED;

        switch( mwup_rsp )
        {
            case  LATCHED:
                CTRL_REGSC_BITSET_mwup_rsp_latchd(&ctrl_regsc);	
                KXSD9_dev.mode.mwup_rsp = LATCHED;
	  
                break;
								  
            case  UNLATCHED:
                CTRL_REGSC_BITSET_mwup_rsp_unlatchd(&ctrl_regsc);	
                KXSD9_dev.mode.mwup_rsp = UNLATCHED;	
									  
                break;		

             default:  
             	  ret = -EINVAL;
             	  debug("KXSD9_dev_mwup_enb invalid argument");
             	  break;
         }

         if( ret == 0 )
         {
             if( (ret = i2c_write(CTRL_REGSB,ctrl_regsb)) < 0 )
             {
                 debug("KXSD9_dev_mwup_enb->i2c_write 1 failed");
             }
             else if( (ret = i2c_write(CTRL_REGSC,ctrl_regsc)) < 0 )
             {
                 debug("KXSD9_dev_mwup_enb->i2c_write 2 failed");
                 
                  if( (ret = i2c_write(CTRL_REGSB, ctrl_regsb_bkup)) < 0 )
                  {
                      debug("FATAL: the device is marked invalid");
                      KXSD9_dev.valid = eFALSE;
                  }
             }
         }
    }

    if(ret < 0)
    {
        KXSD9_dev.mode.mwup = mwup_prev;
        KXSD9_dev.mode.mwup_rsp = mwup_rsp_prev;
   	 }

    debug("mwup = %s", (KXSD9_dev.mode.mwup == ENABLED) ? "ENABLED" : "DISABLED");
    debug("mwup_rsp = %s", 
    (KXSD9_dev.mode.mwup_rsp == LATCHED) ? "LATCHED" : 
    ((KXSD9_dev.mode.mwup_rsp == UNLATCHED) ? "UNLATCHED": "DONT_CARE"));

    mutex_unlock(&(KXSD9_dev.lock)); 

    return ret;         
}

int KXSD9_dev_mwup_disb(void)
{
    int ret = 0;
    unsigned char ctrl_regsb;
    unsigned short mwup_prev;
    
    debug("KXSD9_dev_mwup_disb called");
    
    mutex_lock(&(KXSD9_dev.lock));  

    mwup_prev = KXSD9_dev.mode.mwup;

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_mwup_disb called with dev DS invalid");
        ret = -1;
    }
    else if( KXSD9_dev.mode.dev_state == DISABLED )
    {
        debug("KXSD9_dev_mwup_disb: dev is DISABLED");
        ret = -1;		
    }
    else if( KXSD9_dev.mode.mwup == DISABLED )
    {
        debug("mwup already disabled");
    }
    else if( (ret = i2c_read(CTRL_REGSB, &ctrl_regsb)) < 0 )
    {
        debug("KXSD9_dev_mwup_enb i2c_read failed");
    }
    else 
    {
        CTRL_REGSB_BITSET_mwup_disb(&ctrl_regsb);	
        KXSD9_dev.mode.mwup = DISABLED;    

        if( (ret = i2c_write(CTRL_REGSB,ctrl_regsb)) < 0 )
        {
             debug("KXSD9_dev_mwup_disb->i2c_write failed");
        }        
    }

    if(ret < 0)
    {
        KXSD9_dev.mode.mwup = mwup_prev;
    }
    else
    {
        KXSD9_waitq_wkup_processes();
    }
    
    mutex_unlock(&(KXSD9_dev.lock)); 

    return ret;
}

void KXSD9_dev_work_func(struct work_struct *unused)
{
    unsigned char data;
	
    debug("KXSD9_dev_work_func called");

    mutex_lock(&(KXSD9_dev.lock));

    if( KXSD9_dev.valid == eFALSE)
    {
        debug("KXSD9_dev_work_func called with DS invalid");
    }
    else if( KXSD9_dev.mode.dev_state == DISABLED )
    {
        debug("Error: KXSD9_dev_work_func called with device DISABLED");
    }
    else if( i2c_read(CTRL_REGSA,&data) < 0)
    {
        debug("KXSD9_dev_work_func->i2c_read failed");
    }
    else
    {
        if (KXSD9_dev.mode.mwup == ENABLED )
        { 
            if( KXSD9_dev.mode.mwup_rsp == LATCHED )
            	{
            	    KXSD9_dev.mode.mwup = DISABLED;
            	    debug("mwup: DISABLED");
            	}
        }
        debug("  CTRL_REGSA: 0x%x",data);
    }

    KXSD9_waitq_wkup_processes();
    
    mutex_unlock(&(KXSD9_dev.lock));	
}

int KXSD9_dev_get_sensitivity(KXSD9_sensitivity_t *sensitivity)
{
    int ret = 0;

    mutex_lock(&(KXSD9_dev.lock));

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_get_sensitivity called with dev DS invalid");
        ret = -1;
    }
    else 
    {
        sensitivity->counts_per_g = KXSD9_dev.settings.sensitivity.counts_per_g;
        sensitivity->error_RT = KXSD9_dev.settings.sensitivity.error_RT;
    }

    mutex_unlock(&(KXSD9_dev.lock));
	
    return ret;
}

int KXSD9_dev_get_zero_g_offset(KXSD9_zero_g_offset_t *offset)
{
    int ret = 0;

    mutex_lock(&(KXSD9_dev.lock));

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_get_zero_g_offset called with dev DS invalid");
        ret = -1;
    }
    else 
    {
        offset->counts= KXSD9_dev.settings.zero_g_offset.counts;
        offset->error_RT = KXSD9_dev.settings.zero_g_offset.error_RT;
    }

    mutex_unlock(&(KXSD9_dev.lock));
	
    return ret;
}


int KXSD9_dev_get_acc(KXSD9_acc_t *acc)
{
    int ret = 0;

    mutex_lock(&(KXSD9_dev.lock));

    if( KXSD9_dev.valid == eFALSE )
    {
        debug("KXSD9_dev_get_acc called with dev DS invalid");
        ret = -1;
    }
    else if( KXSD9_dev.mode.dev_state == DISABLED )
    {
        debug("KXSD9_dev_get_acc: dev is DISABLED");
        ret = -1;		
    }
    else if( KXSD9_dev.mode.mwup == ENABLED )
    {
        debug("KXSD9_dev_get_acc: trying to read the acc when the motion wakeup feature is enabled");
        ret = -1;
    }
    else if( acc == NULL )
    {
        ret = -1;
    }
    else
    {
        if( (ret = get_X_acc(&(acc->X_acc)) ) < 0 )
        {
            debug("get_X_acc failed");
        }
        else if( (ret = get_Y_acc(&(acc->Y_acc)) ) < 0 )
        {
            debug("get_X_acc failed");	 
        }
        else if( (ret = get_Z_acc(&(acc->Z_acc)) ) < 0 )
        {
            debug("get_X_acc failed");	 
        }
    }

    mutex_unlock(&(KXSD9_dev.lock));
	
    return ret;
}


int KXSD9_dev_get_default(KXSD9_default_t *acc)
{
    int ret = 0;

	acc->x1 = DEFAULT_ACC_X1;
	acc->y1 = DEFAULT_ACC_Y1;
	acc->z1 = DEFAULT_ACC_Z1;
	acc->x2 = DEFAULT_ACC_X2;
	acc->y2 = DEFAULT_ACC_Y2;
	acc->z2 = DEFAULT_ACC_Z2;

    return ret;
}


static inline int get_X_acc(unsigned short *X_acc)
{
    return get_acc(XOUT_H, XOUT_L, X_acc);
}

static inline int get_Y_acc(unsigned short *Y_acc)
{
    return get_acc(YOUT_H, YOUT_L, Y_acc);
}

static inline int get_Z_acc(unsigned short *Z_acc)
{
    return get_acc(ZOUT_H, ZOUT_L, Z_acc);
}

static int get_acc(char reg_H, char reg_L, unsigned short *acc)
{
    int ret = 0;
    char datah,datal;	 
    unsigned short aux = 0;

    if( (ret = i2c_read( reg_H, &datah)) < 0 )
    {
        debug("i2c_read(_H) failed");
    }
    else
    {	 	
        if( (ret = i2c_read( reg_L , &datal)) < 0 )
        {
            debug("i2c_read(_L) failed");
        }
        else
        {
            aux = datah;
            aux<<= 8;
            aux|= datal;
            aux>>= 4;
            aux&= 0x0FFF;
            *acc = aux;
        }
    }
    return ret;
}

static int i2c_read(unsigned char reg, unsigned char *data)
{
    int ret = 0;
    struct i2c_msg msg[1];
    unsigned char aux[1];

    msg[0].addr	= KXSD9_dev.client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    aux[0] = reg;
    msg[0].buf = aux;

    ret = i2c_transfer(KXSD9_dev.client->adapter, msg, 1);

    if( ret < 0 )
    {
        debug("i2c_read->i2c_transfer 1 failed");
    }
    else 
    {
        msg[0].flags = I2C_M_RD;
        msg[0].len   = 1;
        msg[0].buf   = data;
        ret = i2c_transfer(KXSD9_dev.client->adapter, msg, 1);

        if( ret < 0 )
        {
            debug("i2c_read->i2c_transfer 2 failed");
        }
    }

    if( ret >= 0 )
    {
        return 0;
    }

    return ret;
}

static int i2c_write( unsigned char reg, unsigned char data )
{
    int ret = 0;
    struct i2c_msg msg[1];
    unsigned char buf[2];

    msg[0].addr	= KXSD9_dev.client->addr;
    msg[0].flags = 0;
    msg[0].len = 2;

    buf[0] = reg;
    buf[1] = data;
    msg[0].buf = buf;

    ret = i2c_transfer(KXSD9_dev.client->adapter, msg, 1);

    if( ret < 0 )
    {
        debug("i2c_write->i2c_transfer failed");
    }

    if( ret >= 0 )
    {
        return 0;
    }

    return ret;
}


