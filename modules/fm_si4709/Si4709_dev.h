#ifndef _Si4709_DEV_H
#define _Si4709_DEV_H

#include <linux/i2c.h>
#include <linux/workqueue.h>

typedef struct
{
    int power_state;
    int seek_state;
}dev_state_t;


typedef struct
{
    u16 rdsa;
    u16 rdsb;
    u16 rdsc;
    u16 rdsd;
    u8  curr_rssi;
    u32 curr_channel;
}radio_data_t;

#define NUM_SEEK_PRESETS    20
#define NUM_SEEK_PRESETS_MAX  50

#define SEEK_WAITING     1
#define NO_WAIT               2
#define TUNE_WAITING    4
#define WAIT_OVER          0
#define RDS_WAITING      5

#define BAND_87500_108000_kHz   1
#define BAND_76000_108000_kHz   2
#define BAND_76000_90000_kHz    3

#define CHAN_SPACING_200_kHz   20
#define CHAN_SPACING_100_kHz   10
#define CHAN_SPACING_50_kHz    5

extern int Si4709_dev_wait_flag;
extern int Si4709_dev_rds_wait_flag;

extern int Si4709_dev_init(struct i2c_client *);
extern int Si4709_dev_exit(void);

extern void Si4709_dev_mutex_init(void); 

extern int Si4709_dev_suspend(void);
extern int Si4709_dev_resume(void);

extern int Si4709_dev_powerup(void);
extern int Si4709_dev_powerdown(void);

extern int Si4709_dev_band_set(int);
extern int Si4709_dev_ch_spacing_set(int);

extern int Si4709_dev_chan_select(u32);
extern int Si4709_dev_chan_get(u32*);

extern int Si4709_dev_seek_up(u32*);
extern int Si4709_dev_seek_down(u32*);
extern int Si4709_dev_seek_auto(u32*);

extern int Si4709_dev_RSSI_seek_th_set(u8);
extern int Si4709_dev_seek_SNR_th_set(u8);
extern int Si4709_dev_seek_FM_ID_th_set(u8);
extern int Si4709_dev_cur_RSSI_get(u8*);
extern int Si4709_dev_AFCRL_get(u8*);
extern int Si4709_dev_VOLEXT_ENB(void);
extern int Si4709_dev_VOLEXT_DISB(void);
extern int Si4709_dev_volume_set(u8);
extern int Si4709_dev_volume_get(u8*);
extern int Si4709_dev_MUTE_ON(void);
extern int Si4709_dev_MUTE_OFF(void);
extern int Si4709_dev_MONO_SET(void);
extern int Si4709_dev_STEREO_SET(void);
extern int Si4709_dev_rstate_get(dev_state_t*);
extern int Si4709_dev_RDS_data_get(radio_data_t*);
extern int Si4709_dev_RDS_ENABLE(void);
extern int Si4709_dev_RDS_DISABLE(void);
extern int Si4709_dev_RDS_timeout_set(u32);
extern int Si4709_dev_seek_snr_th_get(u8*);
extern int Si4709_dev_seek_cnt_th_get(u8*);
extern int Si4709_dev_rssi_seek_th_get(u8*);

extern void Si4709_dev_work_func(struct work_struct *);


#endif

