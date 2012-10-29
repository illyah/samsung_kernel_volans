
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/time.h>

#include "Si4709_regs.h"
#include "Si4709_main.h"
#include "Si4709_dev.h"
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

#define RADIO_ON            1
#define RADIO_POWERDOWN     0

#define RADIO_SEEK_ON       1
#define RADIO_SEEK_OFF      0

#define FREQ_87500_kHz      8750
#define FREQ_76000_kHz      7600

#define RSSI_seek_th_MAX    0x7F
#define RSSI_seek_th_MIN    0x00

#define seek_SNR_th_DISB    0x00
#define seek_SNR_th_MIN     0x01  
#define seek_SNR_th_MAX     0x0F  

#define seek_FM_ID_th_DISB  0x00
#define seek_FM_ID_th_MAX   0x01  
#define seek_FM_ID_th_MIN   0x0F  

typedef struct 
{
    u32 frequency;
    u8 rsssi_val;
}channel_into_t;

typedef struct
{
    u16 band;
    u32 bottom_of_band;
    u16 channel_spacing;
    u32 timeout_RDS;      
    u32 seek_preset[NUM_SEEK_PRESETS];
    
}dev_settings_t;

typedef struct 
{
    struct mutex lock;
	
    struct i2c_client const *client;

    dev_state_t state;

    dev_settings_t settings;

    channel_into_t rssi_freq[NUM_SEEK_PRESETS_MAX];

    u16 registers[NUM_OF_REGISTERS];

    unsigned short valid;

    unsigned short valid_client_state;
}Si4709_device_t;


int Si4709_dev_init(struct i2c_client *);
int Si4709_dev_exit(void);

void Si4709_dev_mutex_init(void); 

int Si4709_dev_suspend(void);
int Si4709_dev_resume(void);

void Si4709_dev_work_func(struct work_struct *);

int Si4709_dev_powerup(void);
int Si4709_dev_powerdown(void);

int Si4709_dev_band_set(int);
int Si4709_dev_ch_spacing_set(int);

int Si4709_dev_chan_select(u32);
int Si4709_dev_chan_get(u32*);

int Si4709_dev_seek_up(u32*);
int Si4709_dev_seek_down(u32*);
int Si4709_dev_seek_auto(u32*);

int Si4709_dev_RSSI_seek_th_set(u8);
int Si4709_dev_seek_SNR_th_set(u8);
int Si4709_dev_seek_FM_ID_th_set(u8);
int Si4709_dev_cur_RSSI_get(u8*);
int Si4709_dev_AFCRL_get(u8*);
int Si4709_dev_VOLEXT_ENB(void);
int Si4709_dev_VOLEXT_DISB(void);
int Si4709_dev_volume_set(u8);
int Si4709_dev_volume_get(u8 *);
int Si4709_dev_MUTE_ON(void);
int Si4709_dev_MUTE_OFF(void);
int Si4709_dev_MONO_SET(void);
int Si4709_dev_STEREO_SET(void);
int Si4709_dev_RDS_ENABLE(void);
int Si4709_dev_RDS_DISABLE(void);
int Si4709_dev_RDS_timeout_set(u32);
int Si4709_dev_rstate_get(dev_state_t*);
int Si4709_dev_RDS_data_get(radio_data_t*);
int Si4709_dev_seek_snr_th_get(u8*);
int Si4709_dev_seek_cnt_th_get(u8*);
int Si4709_dev_rssi_seek_th_get(u8*);


static void wait(u32);

static void wait_RDS(void );

static int powerup(void);
static int powerdown(void);

static int seek(u32*, int);
static int tune_freq(u32);

static void get_cur_chan_freq(u32 *, u16);

static u16 freq_to_channel(u32);
static u32 channel_to_freq(u16);


static int i2c_read(u8);
static int i2c_write(u8);

static Si4709_device_t Si4709_dev =
{
    .client = NULL,
    .valid = eFALSE,
    .valid_client_state = eFALSE,
};

int Si4709_dev_wait_flag = NO_WAIT;
int Si4709_dev_rds_wait_flag = NO_WAIT;

int Si4709_dev_init(struct i2c_client *client)
{
    int ret = 0;

    debug("Si4709_dev_init called");

    mutex_lock(&(Si4709_dev.lock));
     
    Si4709_dev.client = client;

#if 1
	s3c_gpio_cfgpin(GPIO_FM_LDO_ON, S3C_GPIO_SFN(GPIO_FM_LDO_ON_AF));
	s3c_gpio_setpull(GPIO_FM_LDO_ON, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_FM_LDO_ON, GPIO_LEVEL_LOW);

	s3c_gpio_cfgpin(GPIO_FM_INT, S3C_GPIO_SFN(GPIO_FM_INT_AF));
	s3c_gpio_setpull(GPIO_FM_INT, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_FM_nRST, S3C_GPIO_SFN(GPIO_FM_nRST_AF));
	s3c_gpio_setpull(GPIO_FM_nRST, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_LOW);
#else
    s3c_gpio_cfgpin(GPIO_FM_LDO_ON, S3C_GPIO_SFN(GPIO_FM_LDO_ON_AF));
    gpio_set_value(GPIO_FM_LDO_ON, GPIO_LEVEL_HIGH);
    mdelay(1);
    s3c_gpio_cfgpin(GPIO_FM_nRST, S3C_GPIO_SFN(1));
    s3c_gpio_cfgpin(GPIO_FM_INT, S3C_GPIO_SFN(GPIO_FM_INT_AF));
  
    gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_LOW);
    mdelay(1);
    gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_HIGH);
    mdelay(2);
#endif

    Si4709_dev.state.power_state = RADIO_POWERDOWN;
    Si4709_dev.state.seek_state = RADIO_SEEK_OFF;

	Si4709_dev.valid_client_state = eTRUE;
    
    mutex_unlock(&(Si4709_dev.lock));   	 

    debug("Si4709_dev_init call over");
	 
    return ret;
}

int Si4709_dev_exit(void)
{
    int ret = 0;
	 
    debug("Si4709_dev_exit called");

    mutex_lock(&(Si4709_dev.lock));   

    Si4709_dev.client = NULL;	

    Si4709_dev.valid_client_state = eFALSE;
    Si4709_dev.valid = eFALSE;

    mutex_unlock(&(Si4709_dev.lock)); 

    gpio_set_value(GPIO_FM_LDO_ON, GPIO_LEVEL_LOW);
	gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_LOW);

    debug("Si4709_dev_exit call over");

    return ret;
}

void Si4709_dev_mutex_init(void)
{
    mutex_init(&(Si4709_dev.lock));
}

int Si4709_dev_powerup(void)
{
    int ret = 0;
    u32 value = 100;

    debug("Si4709_dev_powerup called");

    mutex_lock(&(Si4709_dev.lock)); 

    if( Si4709_dev.valid_client_state == eFALSE )
    {
        debug("Si4709_dev_powerup called when DS (state, client) is invalid");
	       ret = -1;	  
    }    
    else if( (ret = powerup()) < 0 )
    {
        debug("powerup failed");
    }
    else
    {
        POWERCFG_BITSET_RDSM_HIGH(&Si4709_dev.registers[POWERCFG]);
        POWERCFG_BITSET_SKMODE_LOW(&Si4709_dev.registers[POWERCFG]);
        SYSCONFIG1_BITSET_STCIEN_HIGH(&Si4709_dev.registers[SYSCONFIG1]);
        SYSCONFIG1_BITSET_RDSIEN_LOW(&Si4709_dev.registers[SYSCONFIG1]);
        SYSCONFIG1_BITSET_RDS_HIGH(&Si4709_dev.registers[SYSCONFIG1]);
        SYSCONFIG1_BITSET_GPIO_STC_RDS_INT(&Si4709_dev.registers[SYSCONFIG1]);
        SYSCONFIG1_BITSET_RESERVED(&Si4709_dev.registers[SYSCONFIG1]);
        
        SYSCONFIG2_BITSET_SEEKTH(&Si4709_dev.registers[SYSCONFIG2],0);
        SYSCONFIG2_BITSET_VOLUME(&Si4709_dev.registers[SYSCONFIG2],0x0F);
        SYSCONFIG2_BITSET_BAND_87p5_108_MHz(&Si4709_dev.registers[SYSCONFIG2]);
        Si4709_dev.settings.band = BAND_87500_108000_kHz;
        Si4709_dev.settings.bottom_of_band = FREQ_87500_kHz;
        
        SYSCONFIG2_BITSET_SPACE_100_KHz(&Si4709_dev.registers[SYSCONFIG2]);
        Si4709_dev.settings.channel_spacing = CHAN_SPACING_100_kHz;

        SYSCONFIG3_BITSET_SKSNR(&Si4709_dev.registers[SYSCONFIG3],1);
        
        SYSCONFIG3_BITSET_RESERVED(&Si4709_dev.registers[SYSCONFIG3]);

        Si4709_dev.settings.timeout_RDS = msecs_to_jiffies(value);

        if( (ret = i2c_write(SYSCONFIG3)) < 0 )
        {
            debug("Si4709_dev_powerup i2c_write 1 failed");
        }
        else
        {
             Si4709_dev.valid = eTRUE;
        }

    }

    mutex_unlock(&(Si4709_dev.lock)); 

    return ret;
}


int Si4709_dev_powerdown(void)
{
    int ret = 0;

    debug("Si4709_dev_powerdown called");

    mutex_lock(&(Si4709_dev.lock)); 

    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_powerdown called when DS is invalid");
	       ret = -1;	  
    }    
    else if ( ( ret = powerdown()) < 0 )
    {
        debug("powerdown failed");
    }

    mutex_unlock(&(Si4709_dev.lock)); 

    return ret;
}

int Si4709_dev_suspend(void)
{
    int ret = 0;
	 
    debug("Si4709_dev_suspend called");

    mutex_lock(&(Si4709_dev.lock)); 

    if( Si4709_dev.valid_client_state== eFALSE )
    {
        debug("Si4709_dev_suspend called when DS (state, client) is invalid");
	       ret = -1;	  
    }    
    
    mutex_unlock(&(Si4709_dev.lock)); 

    debug("Si4709_dev_enable call over");

    return ret;
}

int Si4709_dev_resume(void)
{
    int ret = 0;
	 

    mutex_lock(&(Si4709_dev.lock));   

    if( Si4709_dev.valid_client_state == eFALSE )
    {
        debug("Si4709_dev_resume called when DS (state, client) is invalid");
	       ret = -1;	  
    }    
	 
    mutex_unlock(&(Si4709_dev.lock)); 


    return ret;
}

void Si4709_dev_work_func(struct work_struct *unused)
{
        if( (i2c_read(STATUSRSSI)) < 0 )
        {
            debug("%s failed 1", __FUNCTION__); 
        return;
        }

    debug("STATUSRSSI: 0x%x",Si4709_dev.registers[STATUSRSSI]);    
    
    if( (Si4709_dev_wait_flag == SEEK_WAITING) || (Si4709_dev_wait_flag == TUNE_WAITING))
    {
        if (STATUSRSSI_SEEK_TUNE_STATUS(Si4709_dev.registers[STATUSRSSI]) == COMPLETE) 
        {
            debug("SEEK_TUNE_COMPLETE");
            Si4709_dev_wait_flag = WAIT_OVER;
            wake_up_interruptible(&Si4709_waitq);        
        }
    }

    if(Si4709_dev_rds_wait_flag == RDS_WAITING)
    {
        if (STATUSRSSI_RDS_READY_STATUS
        	   (Si4709_dev.registers[STATUSRSSI]) == NEW_RDS_GROUP_READY) 
    {
            debug("NEW_RDS_GROUP_READY");
        Si4709_dev_rds_wait_flag = WAIT_OVER;
        wake_up_interruptible(&Si4709_rds_waitq);
    }
}
}


int Si4709_dev_band_set(int band)
{
    int ret = 0;
    u16 sysconfig2 =0; 
    u16 prev_band = 0;
    u32 prev_bottom_of_band = 0;
   
    debug("Si4709_dev_band_set called");
 
    mutex_lock(&(Si4709_dev.lock));   
     sysconfig2 = Si4709_dev.registers[SYSCONFIG2];     
     prev_band = Si4709_dev.settings.band;
     prev_bottom_of_band = Si4709_dev.settings.bottom_of_band;
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_band_set called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        switch (band)
        {
            case BAND_87500_108000_kHz:
            	   SYSCONFIG2_BITSET_BAND_87p5_108_MHz(&Si4709_dev.registers[SYSCONFIG2]);
            	   Si4709_dev.settings.band = BAND_87500_108000_kHz;
            	   Si4709_dev.settings.bottom_of_band = FREQ_87500_kHz;
            	   break;

            case BAND_76000_108000_kHz:
                SYSCONFIG2_BITSET_BAND_76_108_MHz(&Si4709_dev.registers[SYSCONFIG2]);
                Si4709_dev.settings.band = BAND_76000_108000_kHz;
                Si4709_dev.settings.bottom_of_band = FREQ_76000_kHz;
                break;

            case BAND_76000_90000_kHz:	
                SYSCONFIG2_BITSET_BAND_76_90_MHz(&Si4709_dev.registers[SYSCONFIG2]);
                Si4709_dev.settings.band = BAND_76000_90000_kHz;
                Si4709_dev.settings.bottom_of_band = FREQ_76000_kHz;
                break;

            default:
            	   ret = -1;
        }

        if(ret == 0)
        {
            if( (ret = i2c_write(SYSCONFIG2)) < 0 )
            {
                debug("Si4709_dev_band_set i2c_write 1 failed");
                Si4709_dev.settings.band = prev_band;
                Si4709_dev.settings.bottom_of_band = prev_bottom_of_band;
                Si4709_dev.registers[SYSCONFIG2] = sysconfig2;
            }
        }
    }
    
    mutex_unlock(&(Si4709_dev.lock)); 
 
    return ret;
}

int Si4709_dev_ch_spacing_set(int ch_spacing)
{
    int ret = 0;
    u16 sysconfig2 = 0; 
    u16 prev_ch_spacing = 0;
    
    debug("Si4709_dev_disable called");
  
    mutex_lock(&(Si4709_dev.lock));   
     sysconfig2 = Si4709_dev.registers[SYSCONFIG2]; 
     prev_ch_spacing = Si4709_dev.settings.channel_spacing;
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_band_set called when DS is invalid");
        ret = -1;   
    }
    else
    {
        switch (ch_spacing)
        {
            case CHAN_SPACING_200_kHz:
                SYSCONFIG2_BITSET_SPACE_200_KHz(&Si4709_dev.registers[SYSCONFIG2]);
                Si4709_dev.settings.channel_spacing = CHAN_SPACING_200_kHz;
                break;
 
            case CHAN_SPACING_100_kHz:
                SYSCONFIG2_BITSET_SPACE_100_KHz(&Si4709_dev.registers[SYSCONFIG2]);
                Si4709_dev.settings.channel_spacing = CHAN_SPACING_100_kHz;
                break;
 
            case CHAN_SPACING_50_kHz: 
                SYSCONFIG2_BITSET_SPACE_50_KHz(&Si4709_dev.registers[SYSCONFIG2]);
                Si4709_dev.settings.channel_spacing = CHAN_SPACING_50_kHz;
                break;
 
            default:
                ret = -1;
        }
 
        if(ret == 0)
        {
            if( (ret = i2c_write(SYSCONFIG2)) < 0 )
            {
                debug("Si4709_dev_ch_spacing_set i2c_write 1 failed");
                Si4709_dev.settings.channel_spacing = prev_ch_spacing;
                Si4709_dev.registers[SYSCONFIG2] = sysconfig2;
            }
        }
    }
    
    mutex_unlock(&(Si4709_dev.lock)); 
  
    return ret;
}

int Si4709_dev_chan_select(u32 frequency)
{
    int ret = 0;
  
    debug("Si4709_dev_chan_select called");
 
    mutex_lock(&(Si4709_dev.lock));   

    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_chan_select called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        Si4709_dev.state.seek_state = RADIO_SEEK_ON; 
        
        ret = tune_freq(frequency);
        debug("Si4709_dev_chan_select called1");
        Si4709_dev.state.seek_state = RADIO_SEEK_OFF; 
    }

    mutex_unlock(&(Si4709_dev.lock)); 
 
    return ret;
}

int Si4709_dev_chan_get(u32 *frequency)
{
    int ret = 0;

    
    debug("Si4709_dev_chan_get called");
  
    mutex_lock(&(Si4709_dev.lock));   

    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_chan_get called when DS is invalid");
	       ret = -1;	  
    }
    else if( (ret = i2c_read(READCHAN)) < 0 )
    {
        debug("Si4709_dev_chan_get i2c_read failed");
    }
    else
    {
        get_cur_chan_freq(frequency, Si4709_dev.registers[READCHAN]);
    }
    
    mutex_unlock(&(Si4709_dev.lock)); 
  
    return ret;
}

int Si4709_dev_seek_up(u32 *frequency)
{
    int ret = 0;
   
    debug("Si4709_dev_seek_up called");
   
    mutex_lock(&(Si4709_dev.lock)); 

    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_seek_up called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        Si4709_dev.state.seek_state = RADIO_SEEK_ON; 
        
        ret = seek(frequency, 1);

        Si4709_dev.state.seek_state = RADIO_SEEK_OFF; 
    }

    mutex_unlock(&(Si4709_dev.lock)); 
   
    return ret;
}

int Si4709_dev_seek_down(u32 *frequency)
{
    int ret = 0;
   
    debug("Si4709_dev_seek_down called");
   
    mutex_lock(&(Si4709_dev.lock));   

    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_seek_down called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        Si4709_dev.state.seek_state = RADIO_SEEK_ON;   

        ret = seek(frequency, 0);
 
        Si4709_dev.state.seek_state = RADIO_SEEK_OFF;
    }

    mutex_unlock(&(Si4709_dev.lock)); 
   
    return ret;
}



int Si4709_dev_seek_auto(u32 *seek_preset_user)
{

   int ret = 0;
   int i =0;
   int j = 0;
   channel_into_t temp;
   u32 local_freq = 0;

     debug("Si4709_dev_seek_auto called");
     
     if( Si4709_dev.valid == eFALSE )
       {
            debug("Si4709_dev_seek_down called when DS is invalid");
	          ret = -1;	  
        }
    
    else
    	  {
       	      if( (ret = tune_freq(FREQ_87500_kHz)) == 0 )
                   {
                        debug("Si4709_dev_seek_auto tune_freq success");
             get_cur_chan_freq(&(Si4709_dev.rssi_freq[0].frequency), 
             	                   Si4709_dev.registers[READCHAN]);
                        Si4709_dev_cur_RSSI_get(&(Si4709_dev.rssi_freq[0].rsssi_val));
                    } 
       	      else
       	      	{
       	      	    debug("tunning failed, seek auto failed");
       	      	    ret =-1;
       	     goto OUT;
       	      	}

        for(i=0;i<NUM_SEEK_PRESETS_MAX; i++)
              	{       
								if( (ret = seek(&local_freq,1)) == 0 )
                    	    	{
                get_cur_chan_freq(&(Si4709_dev.rssi_freq[i].frequency), 
                                    Si4709_dev.registers[READCHAN]);
                if(Si4709_dev.rssi_freq[i].frequency == 0)
                    Si4709_dev.rssi_freq[i].rsssi_val = 0;
                else	
                               Si4709_dev_cur_RSSI_get(&(Si4709_dev.rssi_freq[i].rsssi_val));
                           	}
                    	    else
                    	      	{
                    	    	     debug("seek failed");
                    	      	}
              	}
       
        for(i=0;i<NUM_SEEK_PRESETS_MAX;i++)
        {
            if(Si4709_dev.rssi_freq[i].frequency == 0)
            	 Si4709_dev.rssi_freq[i].rsssi_val = 0;
            else if(Si4709_dev.rssi_freq[i].rsssi_val == 0)
            	 Si4709_dev.rssi_freq[i].frequency = 0;
        }        
    

        for(i=0;i<(NUM_SEEK_PRESETS_MAX-1);i++)
              	{       
            for(j=i+1;j<NUM_SEEK_PRESETS_MAX;j++)
                    	    	{
                if(Si4709_dev.rssi_freq[j].frequency==Si4709_dev.rssi_freq[i].frequency)
                {
                    Si4709_dev.rssi_freq[j].frequency = 0;
                    Si4709_dev.rssi_freq[j].rsssi_val = 0;
                    	      	}
              	}
                           	}
                    	    
        for(i=0;i<NUM_SEEK_PRESETS_MAX;i++)
                    	      	{
            debug("rssi_freq[%d].frequency = %u, rssi_freq[%d].rsssi_val = %u",
            	i,Si4709_dev.rssi_freq[i].frequency,i,Si4709_dev.rssi_freq[i].rsssi_val);
              	}
       

       
        for(i=0;i<(NUM_SEEK_PRESETS_MAX-1);i++)
                {
            for(j=i+1;j<NUM_SEEK_PRESETS_MAX;j++)
                        {
                            if(  Si4709_dev.rssi_freq[j].rsssi_val>Si4709_dev.rssi_freq[i].rsssi_val)
                            	{
                            	    temp=Si4709_dev.rssi_freq[i];
                            	    Si4709_dev.rssi_freq[i]=Si4709_dev.rssi_freq[j];
                            	    Si4709_dev.rssi_freq[j]=temp;
                            	}
            }
                     	 }


        for(i=0;i<NUM_SEEK_PRESETS_MAX;i++)
        {
            debug("rssi_freq[%d].frequency = %u, rssi_freq[%d].rsssi_val = %u",
            	i,Si4709_dev.rssi_freq[i].frequency,i,Si4709_dev.rssi_freq[i].rsssi_val);
               	  }

        for(i=0;i<NUM_SEEK_PRESETS;i++)
             	{
             	     Si4709_dev.settings.seek_preset[i]=Si4709_dev.rssi_freq[i].frequency;
            debug("seek_preset[%d](frequency) = %u, rssi_freq[%d].rsssi_val = %u",
            	i,Si4709_dev.settings.seek_preset[i],i,Si4709_dev.rssi_freq[i].rsssi_val);
             	}

    }
                
    if(ret == 0)
        memcpy(seek_preset_user, Si4709_dev.settings.seek_preset , 
               sizeof(int)*NUM_SEEK_PRESETS);
     
OUT:
    return ret;
}

int Si4709_dev_RSSI_seek_th_set(u8 seek_th)
{
    int ret = 0;
    u16 sysconfig2 = 0;
       
    debug("Si4709_dev_RSSI_seek_th_set called");
     
    mutex_lock(&(Si4709_dev.lock));   
    sysconfig2 = Si4709_dev.registers[SYSCONFIG2];
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_RSSI_seek_th_set called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        SYSCONFIG2_BITSET_SEEKTH(&Si4709_dev.registers[SYSCONFIG2], seek_th);
        
        if( (ret = i2c_write( SYSCONFIG2 )) < 0 )
        {
            debug("Si4709_dev_RSSI_seek_th_set i2c_write 1 failed");
            Si4709_dev.registers[SYSCONFIG2] = sysconfig2;
        }
	
    }
    
    mutex_unlock(&(Si4709_dev.lock)); 
     
    return ret;
}


int Si4709_dev_seek_SNR_th_set(u8 seek_SNR)
{
    int ret = 0;
    u16 sysconfig3 = 0;
        
    debug("Si4709_dev_seek_SNR_th_set called");
      
    mutex_lock(&(Si4709_dev.lock));   
    
    sysconfig3 = Si4709_dev.registers[SYSCONFIG3]; 
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_seek_SNR_th_set called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        SYSCONFIG3_BITSET_SKSNR(&Si4709_dev.registers[SYSCONFIG3], seek_SNR);
        SYSCONFIG3_BITSET_RESERVED(&Si4709_dev.registers[SYSCONFIG3]);
        

        if( (ret = i2c_write( SYSCONFIG3 )) < 0 )
        {
           debug("Si4709_dev_seek_SNR_th_set i2c_write 1 failed");
           Si4709_dev.registers[SYSCONFIG3] = sysconfig3;
        }
	
    }
    
    mutex_unlock(&(Si4709_dev.lock)); 
      
    return ret;
}



int Si4709_dev_seek_FM_ID_th_set(u8 seek_FM_ID_th)
{
    int ret = 0;
    u16 sysconfig3 = 0;
        
    debug("Si4709_dev_seek_FM_ID_th_set called");
       
    mutex_lock(&(Si4709_dev.lock));   
    
    sysconfig3 = Si4709_dev.registers[SYSCONFIG3];
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_seek_SNR_th_set called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        SYSCONFIG3_BITSET_SKCNT(&Si4709_dev.registers[SYSCONFIG3], seek_FM_ID_th);
        SYSCONFIG3_BITSET_RESERVED(&Si4709_dev.registers[SYSCONFIG3]);

        if( (ret = i2c_write( SYSCONFIG3 )) < 0 )
        {
            debug("Si4709_dev_seek_FM_ID_th_set i2c_write 1 failed");
            sysconfig3 = Si4709_dev.registers[SYSCONFIG3];
        }
	
    }       
        
    mutex_unlock(&(Si4709_dev.lock)); 
       
    return ret;
}


int Si4709_dev_seek_snr_th_get(u8 *seek_SNR)
{
    int ret = 0;
        
    debug("Si4709_dev_seek_SNR_th_get called");
      
    mutex_lock(&(Si4709_dev.lock));   
    
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_seek_SNR_th_get called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        if( (ret = i2c_read(SYSCONFIG3)) < 0 )
        {
            debug("Si4709_dev_seek_snr_th_get i2c_read 1 failed");
        }
        else
        {
              
            *seek_SNR= SYSCONFIG3_GET_SKSNR(Si4709_dev.registers[SYSCONFIG3]);
            
        }
    }     
    
    mutex_unlock(&(Si4709_dev.lock)); 
      
    return ret;
}


int Si4709_dev_seek_cnt_th_get(u8 * seek_cnt)
{
    int ret = 0;
        
    debug("Si4709_dev_seek_cnt_th_get called");
      
    mutex_lock(&(Si4709_dev.lock));   
    
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_seek_cnt_th_get called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        if( (ret = i2c_read(SYSCONFIG3)) < 0 )
        {
            debug("Si4709_dev_seek_cnt_th_get 1 failed");
        }
        else
        {
              
            *seek_cnt= SYSCONFIG3_GET_SKCNT(Si4709_dev.registers[SYSCONFIG3]);
            
        }
    }     
    
    mutex_unlock(&(Si4709_dev.lock)); 
      
    return ret;
}

int Si4709_dev_rssi_seek_th_get(u8 *rssi_TH)
{
	int ret = 0;
	       
	debug("Si4709_dev_rssi_seek_th_get called");
	     
	mutex_lock(&(Si4709_dev.lock));   
	   
	   
	if( Si4709_dev.valid == eFALSE )
	{
		debug("Si4709_dev_rssi_seek_th_get called when DS is invalid");
		ret = -1; 	 
	}
	else
	{
		if( (ret = i2c_read(SYSCONFIG2)) < 0 )
	        {
			debug("Si4709_dev_rssi_seek_th_get i2c_read 1 failed");
	        }
	        else
	        {
		     
		   *rssi_TH= SYSCONFIG2_GET_SEEKTH(Si4709_dev.registers[SYSCONFIG2]);
		   
		   
	       }
    }       
        
    mutex_unlock(&(Si4709_dev.lock)); 
       
    return ret;


}



int Si4709_dev_cur_RSSI_get(u8 *curr_rssi)
{
    int ret = 0;
         
    debug("Si4709_dev_cur_RSSI_get called");
        
    mutex_lock(&(Si4709_dev.lock));   
   
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_cur_RSSI_get called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        if( (ret = i2c_read(STATUSRSSI)) < 0 )
        {
            debug("Si4709_dev_cur_RSSI_get i2c_read 1 failed");
        }
        else
        {
              
            *curr_rssi= STATUSRSSI_RSSI_SIGNAL_STRENGTH(Si4709_dev.registers[STATUSRSSI]);
            
        }
    }     
    mutex_unlock(&(Si4709_dev.lock)); 
        
    return ret;
}

int Si4709_dev_AFCRL_get(u8 *curr_afcrl)
{
    int ret = 0;
         
    debug("Si4709_dev_AFCRL_get called");
        
    mutex_lock(&(Si4709_dev.lock));   
   
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_AFCRL_get called when DS is invalid");
	       ret = -1;	  
    }
    else
    {
        if( (ret = i2c_read(STATUSRSSI)) < 0 )
        {
            debug("Si4709_dev_AFCRL_get i2c_read 1 failed");
        }
        else
        {
              
            *curr_afcrl= STATUSRSSI_BITGET_AFCRL(Si4709_dev.registers[STATUSRSSI]);
            
        }
    }     
    mutex_unlock(&(Si4709_dev.lock)); 
        
    return ret;
}


int Si4709_dev_VOLEXT_ENB(void)
{
    int ret = 0;
    u16 sysconfig3 = 0;
          
    debug("Si4709_dev_VOLEXT_ENB called");
         
    mutex_lock(&(Si4709_dev.lock)); 
    
    sysconfig3 = Si4709_dev.registers[SYSCONFIG3];    
     
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_VOLEXT_ENB called when DS is invalid");
        ret = -1;   
    }
    else
    {
        SYSCONFIG3_BITSET_VOLEXT_ENB(&Si4709_dev.registers[SYSCONFIG3]);
        SYSCONFIG3_BITSET_RESERVED(&Si4709_dev.registers[SYSCONFIG3]);

        if( (ret = i2c_write( SYSCONFIG3 )) < 0 )
        {
            debug("Si4709_dev_VOLEXT_ENB i2c_write 1 failed");
            Si4709_dev.registers[SYSCONFIG3] = sysconfig3;
        }
    }
          
    mutex_unlock(&(Si4709_dev.lock)); 
         
    return ret;
}

int Si4709_dev_VOLEXT_DISB(void)
{
    int ret = 0;
    u16 sysconfig3 = 0;
           
    debug("Si4709_dev_VOLEXT_DISB called");
          
    mutex_lock(&(Si4709_dev.lock));  
    
    sysconfig3 = Si4709_dev.registers[SYSCONFIG3];    
    
    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_VOLEXT_DISB called when DS is invalid");
         ret = -1;   
    }
    else
    {
         SYSCONFIG3_BITSET_VOLEXT_DISB(&Si4709_dev.registers[SYSCONFIG3]);
         SYSCONFIG3_BITSET_RESERVED(&Si4709_dev.registers[SYSCONFIG3]);
       
         if( (ret = i2c_write( SYSCONFIG3 )) < 0 )
         {
             debug("Si4709_dev_VOLEXT_DISB i2c_write 1 failed");
             Si4709_dev.registers[SYSCONFIG3] = sysconfig3;
         }
    }
          
    mutex_unlock(&(Si4709_dev.lock)); 
          
    return ret;
}

int Si4709_dev_volume_set(u8 volume)
{
    int ret = 0;
    u16 sysconfig2 = 0;
            
    debug("Si4709_dev_volume_set called");
           
    mutex_lock(&(Si4709_dev.lock));   
    
    sysconfig2 = Si4709_dev.registers[SYSCONFIG2];       
    
    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_volume_set called when DS is invalid");
         ret = -1;   
    }
    else
    {
         SYSCONFIG2_BITSET_VOLUME(&Si4709_dev.registers[SYSCONFIG2], volume);
         
         if( (ret = i2c_write( SYSCONFIG2 )) < 0 )
         {
             debug("Si4709_dev_volume_set i2c_write 1 failed");
             Si4709_dev.registers[SYSCONFIG2] = sysconfig2;
         }
    }
    
    mutex_unlock(&(Si4709_dev.lock)); 
           
    return ret;
}

int Si4709_dev_volume_get(u8 *volume)
{
    int ret = 0;

    debug("Si4709_dev_volume_get called");
            
    mutex_lock(&(Si4709_dev.lock));   

    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_volume_get called when DS is invalid");
         ret = -1;   
    }
    else
    {
         *volume = SYSCONFIG2_GET_VOLUME(Si4709_dev.registers[SYSCONFIG2]);
    }            
             
    mutex_unlock(&(Si4709_dev.lock)); 
            
    return ret;
}

int Si4709_dev_MUTE_ON(void)
{
    int ret = 0;
    u16 powercfg = 0;
              
    debug("Si4709_dev_MUTE_ON called");
             
    mutex_lock(&(Si4709_dev.lock));   
    powercfg = Si4709_dev.registers[POWERCFG];
    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_MUTE_ON called when DS is invalid");
         ret = -1;   
    }
    else
    {
         POWERCFG_BITSET_DMUTE_LOW(&Si4709_dev.registers[POWERCFG]);
         POWERCFG_BITSET_RESERVED(&Si4709_dev.registers[POWERCFG]);

         if( (ret = i2c_write( POWERCFG )) < 0 )
         {
             debug("Si4709_dev_MUTE_ON i2c_write 1 failed");
             Si4709_dev.registers[POWERCFG] = powercfg;
         }
    }                   
              
    mutex_unlock(&(Si4709_dev.lock)); 
             
    return ret;
}

int Si4709_dev_MUTE_OFF(void)
{
    int ret = 0;
    u16 powercfg = 0;
              
    debug("Si4709_dev_MUTE_OFF called");
              
    mutex_lock(&(Si4709_dev.lock));   
    
    powercfg = Si4709_dev.registers[POWERCFG];
    
    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_MUTE_OFF called when DS is invalid");
         ret = -1;   
    }
    else
    {
         POWERCFG_BITSET_DMUTE_HIGH(&Si4709_dev.registers[POWERCFG]);
         POWERCFG_BITSET_RESERVED(&Si4709_dev.registers[POWERCFG]);
 
         if( (ret = i2c_write( POWERCFG )) < 0 )
         {
             debug("Si4709_dev_MUTE_OFF i2c_write 1 failed");
             Si4709_dev.registers[POWERCFG] = powercfg;
         }
    }                   
               
    mutex_unlock(&(Si4709_dev.lock)); 
              
    return ret;
}

int Si4709_dev_MONO_SET(void)
{
    int ret = 0;
    u16 powercfg = 0;
               
    debug("Si4709_dev_MONO_SET called");
               
    mutex_lock(&(Si4709_dev.lock)); 
    
    powercfg = Si4709_dev.registers[POWERCFG];
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_MONO_SET called when DS is invalid");
        ret = -1;   
    }
    else
    {
        POWERCFG_BITSET_MONO_HIGH(&Si4709_dev.registers[POWERCFG]);
        POWERCFG_BITSET_RESERVED(&Si4709_dev.registers[POWERCFG]);
 
        if( (ret = i2c_write( POWERCFG )) < 0 )
        {
            debug("Si4709_dev_MONO_SET i2c_write 1 failed");
            Si4709_dev.registers[POWERCFG] = powercfg;
        }
    }                   
                
    mutex_unlock(&(Si4709_dev.lock)); 
              
    return ret;
}

int Si4709_dev_STEREO_SET(void)
{
    int ret = 0;
    u16 powercfg = 0;
                
    debug("Si4709_dev_STEREO_SET called");
                
    mutex_lock(&(Si4709_dev.lock));   
    
    powercfg = Si4709_dev.registers[POWERCFG];
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_STEREO_SET called when DS is invalid");
        ret = -1;   
    }
    else
    {
        POWERCFG_BITSET_MONO_LOW(&Si4709_dev.registers[POWERCFG]);
        POWERCFG_BITSET_RESERVED(&Si4709_dev.registers[POWERCFG]);
  
        if( (ret = i2c_write( POWERCFG )) < 0 )
        {
            debug("Si4709_dev_STEREO_SET i2c_write 1 failed");
            Si4709_dev.registers[POWERCFG] = powercfg; 
        }
    }                   
                
    mutex_unlock(&(Si4709_dev.lock)); 
               
    return ret;
}

int Si4709_dev_RDS_ENABLE(void)
{
    u16 sysconfig1 = 0;
    int ret = 0;
             
    debug("Si4709_dev_RDS_ENABLE called");
            
    mutex_lock(&(Si4709_dev.lock));   
    
    sysconfig1 = Si4709_dev.registers[SYSCONFIG1];
    
    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_RDS_ENABLEcalled when DS is invalid");
         ret = -1;   
    }
    else
    {
     	   SYSCONFIG1_BITSET_RDS_HIGH(&Si4709_dev.registers[SYSCONFIG1]);

    	    if( (ret = i2c_write(SYSCONFIG1)) < 0 )
         {
            debug("Si4709_dev_RDS_ENABLE i2c_write 1 failed");
            Si4709_dev.registers[SYSCONFIG1] = sysconfig1;
         }
   	} 	   

    mutex_unlock(&(Si4709_dev.lock)); 
   
    return ret;
}



int Si4709_dev_RDS_DISABLE(void)
{
    u16 sysconfig1 = 0;
    int ret = 0;
             
    debug("Si4709_dev_RDS_DISABLE called");
            
    mutex_lock(&(Si4709_dev.lock));   
    
    sysconfig1 = Si4709_dev.registers[SYSCONFIG1];
    
    if( Si4709_dev.valid == eFALSE )
    {
         debug("Si4709_dev_RDS_DISABLE called when DS is invalid");
         ret = -1;   
    }
    else
    {
     	   SYSCONFIG1_BITSET_RDS_LOW(&Si4709_dev.registers[SYSCONFIG1]);

         if( (ret = i2c_write(SYSCONFIG1)) < 0 )
         {
             debug("Si4709_dev_RDS_DISABLE i2c_write 1 failed");
             Si4709_dev.registers[SYSCONFIG1] = sysconfig1;
         }
   	} 	   

    mutex_unlock(&(Si4709_dev.lock)); 

    return ret;
}
     

int Si4709_dev_rstate_get(dev_state_t *dev_state)
{
    int ret = 0;
                
    debug("Si4709_dev_rstate_get called");
                
    mutex_lock(&(Si4709_dev.lock));   

    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_rstate_get called when DS is invalid");
        ret = -1;   
    }
    else
    {
        dev_state->power_state = Si4709_dev.state.power_state;
        dev_state->seek_state = Si4709_dev.state.seek_state;
    }             

    mutex_unlock(&(Si4709_dev.lock)); 
                
    return ret;
}

int Si4709_dev_RDS_data_get(radio_data_t *data)
{
    int ret = 0;
    u16 sysconfig1 = 0;
                 
    debug("Si4709_dev_RDS_data_get called");
                
    
    sysconfig1 = Si4709_dev.registers[SYSCONFIG1];
    
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_RDS_data_get called when DS is invalid");
        ret = -1;   
    }
    else
    {
        SYSCONFIG1_BITSET_RDSIEN_HIGH(&Si4709_dev.registers[SYSCONFIG1]);

        if( (ret = i2c_write(SYSCONFIG1)) < 0 )
        {
            debug("Si4709_dev_RDS_data_get i2c_write 1 failed");
            Si4709_dev.registers[SYSCONFIG1] = sysconfig1;
        }
      
        else
        {
        	if((ret = i2c_read(SYSCONFIG1)) < 0) {
			debug("Si4709_dev_RDS_data_get i2c_write 2 failed");
        	}
        		
              debug("sysconfig1: 0x%x",Si4709_dev.registers[SYSCONFIG1] );
             sysconfig1 = Si4709_dev.registers[SYSCONFIG1];
            
            Si4709_dev_rds_wait_flag = RDS_WAITING;
        
            wait_RDS();
			
		if((ret = i2c_read(STATUSRSSI)) < 0) {
			             debug("Si4709_dev_RDS_data_get i2c_read 1 failed");
        	}

            debug("statusrssi: 0x%x",Si4709_dev.registers[STATUSRSSI] );
       	    SYSCONFIG1_BITSET_RDSIEN_LOW(&Si4709_dev.registers[SYSCONFIG1]);

            if ( (ret = i2c_write(SYSCONFIG1)) < 0 )
            {
                debug("Si4709_dev_RDS_data_get i2c_write 2 failed");
                Si4709_dev.registers[SYSCONFIG1] = sysconfig1;
            }
            else if(Si4709_dev_rds_wait_flag == WAIT_OVER)
            {
                 Si4709_dev_rds_wait_flag = NO_WAIT;
              
                 if	( (ret = i2c_read(RDSD)) < 0 )
                 {
                     debug("Si4709_dev_RDS_data_get i2c_read 4 failed");
                 }
                 else 
                 {
                     data->rdsa = Si4709_dev.registers[RDSA];
                     data->rdsb = Si4709_dev.registers[RDSB];
                     data->rdsc = Si4709_dev.registers[RDSC];
                     data->rdsd = Si4709_dev.registers[RDSD];

                     get_cur_chan_freq(&(data->curr_channel), Si4709_dev.registers[READCHAN]);
                     debug("curr_channel: %u",data->curr_channel);
                     data->curr_rssi = 
                    	STATUSRSSI_RSSI_SIGNAL_STRENGTH(Si4709_dev.registers[STATUSRSSI]);
                     debug("curr_rssi:%u",(u32)data->curr_rssi );
                 }
           	}
            else
           	{
        	       debug("Si4709_dev_RDS_data_get failure no int or timeout");
        	       Si4709_dev_rds_wait_flag = NO_WAIT;
        	       ret = -1;
        	   }
        	} 
    } 

                
    return ret;
}

int Si4709_dev_RDS_timeout_set(u32 time_out)
{
    int ret = 0; 
    u32 jiffy_count = 0;
    
    debug("Si4709_dev_RDS_timeout_set called");
    
    jiffy_count = msecs_to_jiffies(time_out);

    debug("jiffy_count%d",jiffy_count);

    mutex_lock(&(Si4709_dev.lock));   
     
    if( Si4709_dev.valid == eFALSE )
    {
        debug("Si4709_dev_RDS_timeout_set called when DS is invalid");
        ret = -1;   
    }
    else
    {
        Si4709_dev.settings.timeout_RDS = jiffy_count;
    }

    mutex_unlock(&(Si4709_dev.lock));  

    return ret;
}
    

static int powerup(void)
{
    int ret=0;
    u16 powercfg = Si4709_dev.registers[POWERCFG];

	gpio_set_value(GPIO_FM_LDO_ON, GPIO_LEVEL_HIGH);
	mdelay(1);
	gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_HIGH);
	mdelay(1);

	 gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_LOW);
	 gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_HIGH);   

    POWERCFG_BITSET_DMUTE_HIGH( &Si4709_dev.registers[POWERCFG] );
    POWERCFG_BITSET_ENABLE_HIGH( &Si4709_dev.registers[POWERCFG] );
    POWERCFG_BITSET_DISABLE_LOW( &Si4709_dev.registers[POWERCFG] );
    POWERCFG_BITSET_RESERVED( &Si4709_dev.registers[POWERCFG] );
 
    if( (ret = i2c_write(POWERCFG)) < 0 )
    {
        debug("powerup->i2c_write 1 failed");
        Si4709_dev.registers[POWERCFG] = powercfg;
    }
    else
    {
        mdelay(110);
 
        Si4709_dev.state.power_state = RADIO_ON;
    }

    return ret;
}

static int powerdown(void)
{
    int ret = 0;
    u16 test1 = Si4709_dev.registers[TEST1], 
    	   sysconfig1 = Si4709_dev.registers[SYSCONFIG1],
    	   powercfg = Si4709_dev.registers[POWERCFG];

    SYSCONFIG1_BITSET_GPIO_LOW(&Si4709_dev.registers[SYSCONFIG1]);
    SYSCONFIG1_BITSET_RESERVED( &Si4709_dev.registers[SYSCONFIG1] );
 
    POWERCFG_BITSET_DMUTE_LOW( &Si4709_dev.registers[POWERCFG] );
    POWERCFG_BITSET_ENABLE_HIGH( &Si4709_dev.registers[POWERCFG] );
    POWERCFG_BITSET_DISABLE_HIGH( &Si4709_dev.registers[POWERCFG] );
    POWERCFG_BITSET_RESERVED( &Si4709_dev.registers[POWERCFG] );

    if( (ret = i2c_write( TEST1 )) < 0 )
    {
        debug("powerdown->i2c_write 1 failed");
        Si4709_dev.registers[SYSCONFIG1] = sysconfig1;
        Si4709_dev.registers[POWERCFG] = powercfg;
        Si4709_dev.registers[TEST1] = test1;
    }
    else
    {
        Si4709_dev.state.power_state = RADIO_POWERDOWN;
    }

	gpio_set_value(GPIO_FM_LDO_ON, GPIO_LEVEL_LOW);
	gpio_set_value(GPIO_FM_nRST, GPIO_LEVEL_LOW);

    return ret;
}

static int seek(u32 *frequency, int up)
{
    int ret = 0, i = 0;
    u16 powercfg = Si4709_dev.registers[POWERCFG];
    u16 channel = 0;
    int valid_station_found = 0;
 
    if( up ) 
    {
        POWERCFG_BITSET_SEEKUP_HIGH(&Si4709_dev.registers[POWERCFG]);
    }
    else
    {
        POWERCFG_BITSET_SEEKUP_LOW(&Si4709_dev.registers[POWERCFG]);
    }
        
    POWERCFG_BITSET_SEEK_HIGH(&Si4709_dev.registers[POWERCFG]);
    POWERCFG_BITSET_RESERVED(&Si4709_dev.registers[POWERCFG]);

    if( (ret = i2c_write(POWERCFG)) < 0 )
    {
        debug("seek i2c_write 1 failed");
        Si4709_dev.registers[POWERCFG] = powercfg;
    }
    else
    {
        Si4709_dev_wait_flag = SEEK_WAITING;

        wait(15000);

        if(Si4709_dev_wait_flag == SEEK_WAITING)
        {
        Si4709_dev_wait_flag = NO_WAIT;
            for(i = 0; i<20; i++) 
            {
        if( (ret = i2c_read(STATUSRSSI)) < 0 )
        {
                    debug("%s failed 1", __FUNCTION__);
                    break;
                }
                if(STATUSRSSI_SEEK_TUNE_STATUS
                	 (Si4709_dev.registers[STATUSRSSI]) == COMPLETE)
                	 break;
                debug("**** %s after wait loop ****",__FUNCTION__);
                mdelay(60);
            }
            if(i == 20) 
            	 ret = -1;
        }

        Si4709_dev_wait_flag = NO_WAIT;

        if( (i2c_read(STATUSRSSI)) < 0 )
        {
            debug("seek i2c_read 2 failed");
            ret = -1;
        }
        else
        {
            if ( STATUSRSSI_SF_BL_STATUS(Si4709_dev.registers[STATUSRSSI]) 
            	    == SEEK_SUCCESSFUL )
            {
                valid_station_found = 1;
            }

            powercfg = Si4709_dev.registers[POWERCFG];
                    
            POWERCFG_BITSET_SEEK_LOW(&Si4709_dev.registers[POWERCFG]);
            POWERCFG_BITSET_RESERVED(&Si4709_dev.registers[POWERCFG]);
 
            if( (i2c_write(POWERCFG)) < 0 )
            {
                debug("seek i2c_write 2 failed");
                Si4709_dev.registers[POWERCFG] = powercfg;
                ret = -1;
            }
            else if(ret == 0)
            {
                do
                {
                    if( (ret = i2c_read(STATUSRSSI)) < 0 )
                    {
                        debug("seek i2c_read 3 failed"); 
                        break;
                    }
                    debug("%s: infinite loop",__FUNCTION__);
                }while( STATUSRSSI_SEEK_TUNE_STATUS(Si4709_dev.registers[STATUSRSSI]) != CLEAR );

                if( ret == 0 && valid_station_found == 1 )
                {
                    if( (ret = i2c_read( READCHAN )) < 0 )
                    {
                        debug("seek i2c_read 4 failed");
                    }
                    else
                    {
                        channel = READCHAN_GET_CHAN(Si4709_dev.registers[READCHAN]);
                        *frequency = channel_to_freq(channel);
                    }
                }
                else
                {
                   
                *frequency = 0;
                	}
            }
        }    
    }

    return ret;
}

static int tune_freq(u32 frequency)
{
    int ret = 0, i = 0;
    u16 channel = Si4709_dev.registers[CHANNEL];

    debug("tune_freq called");

    Si4709_dev.registers[CHANNEL] = freq_to_channel(frequency);

    CHANNEL_BITSET_TUNE_HIGH(&Si4709_dev.registers[CHANNEL]);
    CHANNEL_BITSET_RESERVED(&Si4709_dev.registers[CHANNEL]);

    Si4709_dev_wait_flag = TUNE_WAITING;

    if( (ret = i2c_write(CHANNEL)) < 0 )
    {
        debug("tune_freq i2c_write 1 failed");
        Si4709_dev.registers[CHANNEL] = channel;
        Si4709_dev_wait_flag = NO_WAIT;
    }
    else
    {
        wait(600);

        if(Si4709_dev_wait_flag == TUNE_WAITING)
        {
            for(i = 0; i<12; i++) 
            {
                if( (ret = i2c_read(STATUSRSSI)) < 0 )
                {
                    debug("%s failed 1", __FUNCTION__);
                    break;
                }
                if(STATUSRSSI_SEEK_TUNE_STATUS
                	 (Si4709_dev.registers[STATUSRSSI]) == COMPLETE)
                	 break;
                debug("**** %s after wait loop ****",__FUNCTION__);
                mdelay(60);
            }
            if(i == 12) 
            	 ret = -1;
        }

        Si4709_dev_wait_flag = NO_WAIT;

        channel = Si4709_dev.registers[CHANNEL];
    
        CHANNEL_BITSET_TUNE_LOW(&Si4709_dev.registers[CHANNEL]);
        CHANNEL_BITSET_RESERVED(&Si4709_dev.registers[CHANNEL]);
        
        if( (i2c_write(CHANNEL)) < 0 )
        {
            debug("tune_freq i2c_write 2 failed");
            Si4709_dev.registers[CHANNEL] = channel;
            ret = -1;
        }
        else if( ret == 0 )
        {
            do
            {
                if( (ret = i2c_read(STATUSRSSI)) < 0 )
                {
                     debug("tune_freq i2c_read 1 failed"); 
                     break;
                }
                debug("%s: infinite loop",__FUNCTION__);
            }while( STATUSRSSI_SEEK_TUNE_STATUS(Si4709_dev.registers[STATUSRSSI]) != CLEAR );  
        }
    }

    return ret;
}

static void get_cur_chan_freq(u32 *frequency, u16 readchan)
{

    u16 channel = 0;
     debug("get_cur_chan_freq called"); 
    channel = READCHAN_GET_CHAN(readchan);

    *frequency = channel_to_freq(channel);

    debug("frequency-> %u",*frequency);  
}

static u16 freq_to_channel(u32 frequency)
{
    u16 channel;

    if( frequency < Si4709_dev.settings.bottom_of_band )
    {
        frequency = Si4709_dev.settings.bottom_of_band;
    }

    channel = (frequency - Si4709_dev.settings.bottom_of_band) 
    	         / Si4709_dev.settings.channel_spacing;

    return channel;
}

static u32 channel_to_freq(u16 channel)
{
    u32 frequency;

    frequency = Si4709_dev.settings.bottom_of_band +
    	           Si4709_dev.settings.channel_spacing * channel;

    return frequency;
}


static void wait(u32 ms)
{
    u32 timeout = msecs_to_jiffies(ms);
    wait_event_interruptible_timeout(Si4709_waitq, 
    (Si4709_dev_wait_flag == WAIT_OVER),timeout);
}

static void wait_RDS(void)
{
   wait_event_interruptible_timeout(Si4709_rds_waitq, 
  	(Si4709_dev_rds_wait_flag == WAIT_OVER),Si4709_dev.settings.timeout_RDS);
}

static int i2c_read( u8 reg )
{
   u8 idx, reading_reg = STATUSRSSI;
  	u8 data[NUM_OF_REGISTERS * 2], data_high, data_low;
  	int msglen = 0, ret = 0;

  	for(idx = 0; idx < NUM_OF_REGISTERS * 2; idx++)
		{
		    data[idx] = 0x00;
  	} 

	  msglen = reg - reading_reg + 1;

	  if(msglen > 0)
	  {
		    msglen = msglen * 2;
	  } 
	  else
		{
		    msglen = (msglen + NUM_OF_REGISTERS) * 2;
	  } 

	  ret = i2c_master_recv(Si4709_dev.client, data, msglen);

	  if(ret == msglen) 
	  {
		    idx = 0;
		    do 
		    {
			       data_high	= data[idx];
			       data_low	= data[idx+1];

			       Si4709_dev.registers[reading_reg] = 0x0000;
			       Si4709_dev.registers[reading_reg] = (data_high << 8) + data_low;
			       reading_reg = (reading_reg + 1) & RDSD;
			       idx = idx + 2;
		     } while(reading_reg != ((reg +1) & RDSD));

        ret = 0;
	  }
	  else 
	  {
		      ret = -1;		
	  }

	  return ret;
}    

static int i2c_write( u8 reg )
{
	   u8 writing_reg = POWERCFG;
	   u8 data[NUM_OF_REGISTERS * 2];
	   int i, msglen = 0, ret = 0;

	   for(i = 0; i < NUM_OF_REGISTERS * 2; i++)
	   	{
		      data[i] = 0x00;
	   	}
	   
	   do 
	   	{
		     data[msglen++] = (u8)(Si4709_dev.registers[writing_reg] >> 8);
		     data[msglen++] = (u8)(Si4709_dev.registers[writing_reg] & 0xFF);

	      	writing_reg = (writing_reg +1) & RDSD;
	    } while(writing_reg != ((reg + 1) & RDSD));

	    ret = i2c_master_send(Si4709_dev.client, ( const char *)data, msglen);

    	if(ret == msglen) 
    	{
		     ret = 0;
	    } 
    	else 
    	{
				  ret = -1;
	    }

    	return ret;
}    

