/*
 * ak4671_path.c  --  AK4671 Test Path Implementation.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <sound/soc.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/i2c.h>
#include "ak4671.h"

struct ak4671_path_data path_data = {0,};
struct audio_info_data curr_audio_info;
struct audio_info_data prev_audio_info;

bool adc_flag = false, dac_flag = false;
bool call_path =false;
bool same_path_flag = false;
bool codec_power_status = false;
unsigned int amp_state = 0;

void ak4671_reg_print(struct snd_soc_codec *codec)
{
	unsigned char temp = 0;
	int i = 0;
	printk("\n***** AK4671 registers *****\n");
	for(i=0; i<0x5B; i++)
	{
		if(0 == (i%8)) printk("\n");
		temp = codec->read(codec, i);
		printk(" %02x ", temp);
	}
	printk("\n");
}

void amp_reg_write(unsigned char addr, unsigned char value)
{
	if(amp_i2c_write != NULL)
		amp_i2c_write(addr, value);
	else 
		printk("[SOUND:ERROR] amp_i2c_write is Null pointer, check amp module\n");
}

void amp_reg_read(unsigned char addr, unsigned char *value)
{
	if(amp_i2c_read != NULL) {
		if(amp_i2c_read(addr, value))
			printk("[SOUND:ERROR]: failed to get of reg in amp_i2c_read \n");
	}
	else 
		printk("[SOUND:ERROR] amp_i2c_read is Null pointer, check amp module\n");
}

void amp_reg_print(void)
{
	unsigned char temp = 0, i=0;
	printk("\n***** MAX9877 registers *****\n");
	for(i=0; i<5; i++) {
		amp_reg_read(i, &temp);
		printk(" %02x",temp);
	}
	printk("\n");
}

int set_amp(int power, int direction)
{
	unsigned char temp;

	if(power){
		amp_reg_read(0x00, &temp);
		temp = temp & 0x0F;
		temp |= 0x40;
		amp_reg_write(0x00, temp);	// ZCD = 1, Single ended.
		
		if((direction == SPEAKER) && (amp_state == 0)) {
			amp_state = SPEAKER;
			amp_reg_write(0x04, 0x81);	// SPK = INA1 + INA2
		} 
		else if((direction == HEADPHONE) && (amp_state == 0)) {
			amp_state = HEADPHONE;
			amp_reg_write(0x04, 0x85);	// HPL = INB1, HPR = INB2
		}
		else if ( ((direction == SPEAKER) && (amp_state == HEADPHONE)) ||
	          ((direction == HEADPHONE) && (amp_state == SPEAKER)) ) {
			amp_state = DUALMODE;
			amp_reg_write(0x04, 0x83);	//SPK = INB1 + INB2, HPL = INB1, HPR = INB2
		}
		else {
			return 0;
		}

	} 
	else {
		amp_state = 0;
		amp_reg_write(0x04, 0x40);
	}
	return 1;
}

void toggle_amp(struct snd_soc_codec *codec, int direction)
{

	if(direction == SPEAKER){
		set_amp(OFF, 0);
		set_amp(ON, HEADPHONE);
	}
	else if(direction == HEADPHONE) {
		set_amp(OFF, 0);
		set_amp(ON, SPEAKER);
	}

	return ;
}

static void set_vcom(struct snd_soc_codec *codec)
{
	u8 temp;
	
	temp = codec->read(codec, AK4671_AD_DA_PM);
	if( !(temp & 0x01) ) {
		codec->write(codec, AK4671_AD_DA_PM, 0x01);
		udelay(1);
	}
}

void connect_signal(struct snd_soc_codec *codec, unsigned int path)
{
	unsigned char temp = 0;
	if((IN1_MAIN_MIC | IN1_SUB_MIC) & path) {

			codec->write(codec, AK4671_MIC_SIG, 0x9C);
			
		if(OUT3_MSM_MIC & path) {
			temp = codec->read(codec, AK4671_LOUT3_PM);
			codec->write(codec, AK4671_LOUT3_PM, (temp | 0x20));
			temp = codec->read(codec, AK4671_LOUT3_SIG);
			codec->write(codec, AK4671_LOUT3_SIG, (temp | 0x02));
		} 
		else if(OUT1_RCV & path) {
			temp = codec->read(codec, AK4671_LOUT1_PM);
			codec->write(codec, AK4671_LOUT1_PM, (temp | 0x20));			
			temp = codec->read(codec, AK4671_LOUT1_SIG);
			codec->write(codec, AK4671_LOUT1_SIG, (temp | 0x02));
		} 
		else if((OUT2_SPK_L & path) || (OUT2_SPK_R & path)) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x02));
		} 
		else if(OUT2_HP & path) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x02));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x02));
		} 
		else if(OUT_BT & path) {
			printk("\n [SOUND][ERROR] Path from MIC to BT is not prepared  \n");
		}
	}
	
	else if(IN3_EAR_MIC & path) {

			codec->write(codec, AK4671_MIC_SIG, 0xCE); 
			
		if(OUT3_MSM_MIC & path) {
			temp = codec->read(codec, AK4671_LOUT3_PM);
			codec->write(codec, AK4671_LOUT3_PM, (temp | 0x20));
			temp = codec->read(codec, AK4671_LOUT3_SIG);
			codec->write(codec, AK4671_LOUT3_SIG, (temp | 0x08));
		} 
		
		else if(OUT1_RCV & path) {
			temp = codec->read(codec, AK4671_LOUT1_PM);
			codec->write(codec, AK4671_LOUT1_PM, (temp | 0x20));			
			temp = codec->read(codec, AK4671_LOUT1_SIG);
			codec->write(codec, AK4671_LOUT1_SIG, (temp | 0x08));
		} 
		
		else if((OUT2_SPK_L & path) || (OUT2_SPK_R & path)) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x08));
		} 
		
		else if(OUT2_HP & path) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x08));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x08));
		} 
		else if(OUT_BT & path) {
			printk("\n [SOUND][ERROR] Path from MIC to BT is not prepared  \n");
		}
	}

	else if(IN2_FMRADIO & path) {
		
		codec->write(codec, AK4671_MIC_SIG, 0x05);
			
		if(OUT2_HP & path) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x04));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x04));
		} 
		
		else if((OUT2_SPK_L & path) || (OUT2_SPK_R & path)) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x04));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x04));
		} 
		
		else if(OUT1_RCV & path) {
			printk("\n [SOUND][ERROR] Path from FMRADIO to RCV is not prepared  \n");
		} 
		
		else if(OUT_BT & path) {
			printk("\n [SOUND][ERROR] Path from FMRADIO to BT is not prepared  \n");
		} 
		
		else if(OUT3_MSM_MIC & path) {
			printk("\n [SOUND][ERROR] Path from FMRADIO to MSMMIC is not prepared  \n");
		}
	}

	else if(IN4_MSM_SPK & path) {

			temp = codec->read(codec, AK4671_MIC_SIG);
			if(!((temp == 0x9C) || (temp == 0xCE))) {
				codec->write(codec, AK4671_MIC_SIG, 0x9C); 
			}
		
		if(OUT1_RCV & path) {
			temp = codec->read(codec, AK4671_LOUT1_PM);
			codec->write(codec, AK4671_LOUT1_PM, (temp | 0x20));			
			temp = codec->read(codec, AK4671_LOUT1_SIG);
			codec->write(codec, AK4671_LOUT1_SIG, (temp | 0x10));
		} 
		
		else if((OUT2_SPK_L & path) || (OUT2_SPK_R & path)) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x10));
		} 
		
		else if(OUT2_HP & path) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x10));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x10));
		} 
		
		else if(OUT_BT & path) {
			if( IN_AP & path_data.path) {
			temp = codec->read(codec, 0x15);
				temp &= ~0x0c;
				codec->write(codec, 0x15, (temp | 0x08)); 
			} 
			else {
				temp = codec->read(codec, 0x15);
				temp &= ~0x0c;
				codec->write(codec, 0x15, (temp | 0x04)); 
			}

			temp = codec->read(codec, AK4671_MIC_SIG);
			if(!((temp == 0x9C) || (temp == 0xCE))) {
				codec->write(codec, AK4671_MIC_SIG, 0x9C); 
			}
			temp = codec->read(codec, 0x19);
			codec->write(codec, 0x19, (temp | 0x41)); 
		} 
		
		else if(OUT3_MSM_MIC & path) {
			printk("\n [SOUND][ERROR] Path from MSMSPK to MSMMIC isn't prepared  \n");
		}
	}
	else if(IN_AP & path) {
		
		if(OUT1_RCV & path) {
			temp = codec->read(codec, AK4671_LOUT1_PM);
			codec->write(codec, AK4671_LOUT1_PM, (temp | 0x20));			
			
			temp = codec->read(codec, AK4671_LOUT1_SIG);
			codec->write(codec, AK4671_LOUT1_SIG, (temp | 0x01));
			temp = codec->read(codec, AK4671_ROUT1_SIG);
			codec->write(codec, AK4671_ROUT1_SIG, (temp | 0x01));
		} 
		
		else if((OUT2_SPK_L & path) || (OUT2_SPK_R & path)) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x01));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x01));
			temp = codec->read(codec, AK4671_LOUT2_PM);
			codec->write(codec, AK4671_LOUT2_PM, (temp & (~0x08)));
		} 
		
		else if(OUT2_HP & path) {
			temp = codec->read(codec, AK4671_LOUT2_SIG);
			codec->write(codec, AK4671_LOUT2_SIG, (temp | 0x01));
			temp = codec->read(codec, AK4671_ROUT2_SIG);
			codec->write(codec, AK4671_ROUT2_SIG, (temp | 0x01));
			temp = codec->read(codec, AK4671_LOUT2_PM);
			codec->write(codec, AK4671_LOUT2_PM, (temp & (~0x08)));
		}
		
		else if(OUT_BT & path) {
			printk("[OUT_BT] %s \n",__func__);
			if(IN4_MSM_SPK & path_data.path) {
				printk("[MSM_SPK][OUT_BT] %s \n",__func__);
				temp = codec->read(codec, 0x15);
				temp &= ~0x0c;
				codec->write(codec, 0x15, (temp | 0x08)); 
			}
			temp = codec->read(codec, 0x19);
			codec->write(codec, 0x19, (temp | 0x41)); // SRC-A = MIX Rch
		} 
		else if(OUT3_MSM_MIC & path) {
			printk("\n [SOUND][ERROR] Path from AP to MSMMIC is not prepared  \n");
		}
	}
	
	else if(IN_BT & path) {
		
		if(OUT3_MSM_MIC & path) {
			temp = codec->read(codec, AK4671_LOUT3_PM);
			codec->write(codec, AK4671_LOUT3_PM, (temp | 0x20));
		
			temp = codec->read(codec, AK4671_LOUT3_SIG);
			codec->write(codec, AK4671_LOUT3_SIG, (temp | 0x01));
		
			temp = codec->read(codec, 0x15);
			codec->write(codec, 0x15, (temp | 0x10)); 
		
			temp = codec->read(codec, 0x59);
			codec->write(codec, 0x59, (temp | 0x10));
		} 
		
		else if(OUT_AP & path) {
			temp = codec->read(codec, 0x59);
			codec->write(codec, 0x59, (temp | 0x10));
		}
		
		else if(OUT1_RCV & path) {
			printk("\n [SOUND][ERROR] Path from BT to OUT1_RCV is not prepared\n");
		} 
		
		else if((OUT2_SPK_L & path) || (OUT2_SPK_R & path)) {
			printk("\n [SOUND][ERROR] Path from BT to SPK is not prepared\n");
		} 
		
		else if(OUT2_HP & path) {
			printk("\n [SOUND][ERROR] Path from BT to HP is not prepared\n");
		}
	}
	
	else {
		printk("[SOUND][WARNING] %s \n",__func__);
	}
}

void input_power_up(struct snd_soc_codec *codec, unsigned int path)
{
	unsigned char temp = 0;

	if((OUT1_RCV | OUT2_SPK_L | OUT2_SPK_R | OUT2_HP | OUT3_MSM_MIC) & path) {
			
		if(IN_AP & path) {
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0xc0) );
		}

		else if((IN1_MAIN_MIC | IN1_SUB_MIC) & path)	{
			temp = codec->read(codec, AK4671_MIX_PM1);
			codec->write(codec, AK4671_MIX_PM1, (temp | 0x03));
		}
		else if(IN3_EAR_MIC & path)	{
			temp = codec->read(codec, AK4671_MIX_PM1);
			codec->write(codec, AK4671_MIX_PM1, (temp | 0x30));
		}
		else if(IN4_MSM_SPK & path)	{
			temp = codec->read(codec, AK4671_MIX_PM1);
			codec->write(codec, AK4671_MIX_PM1, (temp | 0xC0));
		}
			
		else if( IN2_FMRADIO & path) {
			temp = codec->read(codec, AK4671_MIX_PM1);
			codec->write(codec, AK4671_MIX_PM1, (temp | 0x0C));
		}

		else if(IN_BT & path) {
		temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0x40) );
			
			temp = codec->read(codec, 0x53);
			if(!(temp & 0x04)) {
			codec->write(codec, 0x53, (temp | 0x06));
				msleep(260);
			}
			else
				codec->write(codec, 0x53, (temp | 0x02));
		}
	}
			
	else if(OUT_AP & path) {
			
		if((IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC) & path) {
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0x14)); 
		}
		else if(IN4_MSM_SPK & path)	{
		temp = codec->read(codec, AK4671_AD_DA_PM);
		codec->write(codec, AK4671_AD_DA_PM, (temp | 0x28));
		}
		else if( IN2_FMRADIO & path) {
		temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0x3C)); 
		}
		else if (IN_BT & path)	{
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0x40));

			temp = codec->read(codec, 0x53);
			if(!(temp & 0x04)) {
				codec->write(codec, 0x53, (temp | 0x06));
				msleep(260);
			}
			else
				codec->write(codec, 0x53, (temp | 0x02));
	}
}

	else if(OUT_BT & path) {
		if(IN_AP & path) {
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0x80));
		}

		if(IN4_MSM_SPK & path ) {
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp | 0xA8));
		}
	}
	
	else {
		printk("[SOUND][WARNING] %s \n",__func__);
        }
}

void input_power_down(struct snd_soc_codec *codec, unsigned int path)
{
	unsigned char temp = 0;

	if(OUT_AP & path) {
			
		if((IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC) & path) {
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp & ~0x14)); 
		}
		else if(IN4_MSM_SPK & path)	{
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp & ~0x28));
		}
		else if( IN2_FMRADIO & path) {
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp & ~0x3C)); 
		}
		else if (IN_BT & path)	{
			temp = codec->read(codec, AK4671_AD_DA_PM);
			codec->write(codec, AK4671_AD_DA_PM, (temp & ~0x40));

			temp = codec->read(codec, 0x53);
			if(!(temp & 0x04)) {
				codec->write(codec, 0x53, (temp | 0x06));
				msleep(260);
			}
			else
				codec->write(codec, 0x53, (temp | 0x02));
		}
	}
	else {
		printk("[SOUND][WARNING] %s \n",__func__);
	}
}

void output_power_up(struct snd_soc_codec *codec, unsigned int path)
{
	unsigned char temp = 0;

	if(OUT1_RCV & path) {
	
		temp = codec->read(codec, AK4671_LOUT1_PM);
		codec->write(codec, AK4671_LOUT1_PM, (temp | 0x04));
	
		temp = codec->read(codec, AK4671_LOUT1_PM);
		codec->write(codec, AK4671_LOUT1_PM, (temp | 0x03));
		msleep(10);

		temp = codec->read(codec, AK4671_LOUT1_PM);
		codec->write(codec, AK4671_LOUT1_PM, (temp & (~0x04)));
	
	}
	else if((OUT2_SPK_L | OUT2_SPK_R) & path) {
	
		temp = codec->read(codec, AK4671_LOUT2_PM);
		codec->write(codec, AK4671_LOUT2_PM, (temp | 0x63));
		
		temp = codec->read(codec, AK4671_LOUT2_PM);
		codec->write(codec, AK4671_LOUT2_PM, (temp | 0x04));
		
		msleep(100);
		
		set_amp(ON, SPEAKER);
	}
	else if(OUT2_HP & path) {
	
		temp = codec->read(codec, AK4671_LOUT2_PM);
		codec->write(codec, AK4671_LOUT2_PM, (temp | 0x63));
		
		temp = codec->read(codec, AK4671_LOUT2_PM);
		codec->write(codec, AK4671_LOUT2_PM, (temp | 0x04));
		
		msleep(100);
		
		set_amp(ON, HEADPHONE);
	}
	else if(OUT3_MSM_MIC & path) {

		temp = codec->read(codec, AK4671_LOUT3_PM);
		codec->write(codec, AK4671_LOUT3_PM, (temp | 0x04));

		temp = codec->read(codec, AK4671_LOUT3_PM);
		codec->write(codec, AK4671_LOUT3_PM, (temp | 0x03));
				
		msleep(200);
			
		temp = codec->read(codec, AK4671_LOUT3_PM);
		codec->write(codec, AK4671_LOUT3_PM, (temp & (~0x04)));
	}
	else if(OUT_BT & path) {
		if((IN4_MSM_SPK | IN_AP) & path) {
		temp = codec->read(codec, 0x53);
		if(!(temp & 0x04)) {
			codec->write(codec, 0x53, (temp | 0x05));
			msleep(260);			
		}
		else
			codec->write(codec, 0x53, (temp | 0x01));
		}
	}
	else if(OUT_AP & path) {
	}
	else {
		printk("[SOUND][WARNING] %s, path: %x \n",__func__,path);
	}
		
}

void mic_power_up(struct snd_soc_codec *codec)
{

	if(path_data.path & IN1_MAIN_MIC)
	{
		mic_select_ctrl(MAIN_MIC);
	}
	else if(path_data.path & IN1_SUB_MIC)
	{
		mic_select_ctrl(SUB_MIC);
	}
	if (path_data.path & (IN1_MAIN_MIC | IN1_SUB_MIC | IN3_EAR_MIC)) {
		mic_power_ctrl(1);
		DPRINTK("MICBAIS ON\n");
	} else if(headset_status == 0) { 
		mic_power_ctrl(0);   
		DPRINTK("MICBAIS OFF\n");
	}

}

void codec_power_up(void)
{
	if (!codec_power_ctrl(POWER_ON))
	{
		printk("%s:%d -- codec power on failed!\n", __func__, __LINE__);
	}
	
	codec_power_status = true;
}

static void set_codec_pll(struct snd_soc_codec *codec, unsigned int path)
{
	unsigned char temp,temp2 ;

	if(path & (IN_AP | OUT_AP)) {	
		temp = codec->read(codec, AK4671_PLL_MODE0);
		temp2 = codec->read(codec, AK4671_PLL_MODE1);

		if((temp != pll_mode0) || (temp2 != pll_mode1)) {
			DPRINTK("[SOUND:PLL] set pll ========\n");
	        codec->write(codec, AK4671_PLL_MODE0, pll_mode0 );
                codec->write(codec, AK4671_PLL_MODE1, pll_mode1 );
                msleep(40);
                codec->write(codec, AK4671_FORMAT, audio_format);
		}
	}
}

static void set_route(struct snd_soc_codec *codec, unsigned int path)
{
	unsigned char temp = 0;
	
	if ((IN_OFF | OUT_OFF) & path ) {
		
		temp = codec->read(codec, AK4671_MODE2);
		temp |= 0x4;
		codec->write(codec, AK4671_MODE2, temp);
		
		set_amp(OFF, 0);
		
		codec->write(codec, AK4671_AD_DA_PM, 0x00 );
		
		codec_power_ctrl(POWER_OFF);
		udelay(1);
		
		set_gain_table_flag = false;
		codec_power_status = false;

	}
	else {
	
		if(codec_power_status == false)
		{
			codec_power_up();
		}
		
		set_codec_pll(codec, path);

		if(set_gain_table_flag == 0) {
			DPRINTK("[SOUND]previous gain : %d\n", prev_sound_scenario);
			DPRINTK("[SOUND]current gain  : %d\n", curr_sound_scenario);
		        ak4671_select_gain_table(codec);
			set_gain_table_flag = true;
			prev_sound_scenario = curr_sound_scenario;
		}
		
		connect_signal(codec, path);	
		set_vcom(codec);
		input_power_up(codec, path);
		output_power_up(codec, path);

	}
}
int check_direction(unsigned int dir)
{
	if(dir & (~PATH_MASK)) {
		printk("[PATH:WARN]wrong direction [0x%08x]\n", dir);
		return 0;
	}
	else if ((OUT_OFF & dir) && (!(IN_OFF & dir)) ){
		printk("[PATH:WARN]wrong direction [0x%08x]\n", dir);
		return 0;
	}
	else if (dir == 0x0) {
		printk("[PATH:WARN]wrong direction [0x%08x]\n", dir);
		return 0;
	}
	return 1;
}

void set_sound_path(struct snd_soc_codec *codec, unsigned int direction)
{
	unsigned int i;
	DPRINTK("[%s]\n", __func__);
	DPRINTK("current path:0x%08x, add path:0x%08x\n",path_data.path,direction);
	
	if(!check_direction(direction)) return;

	if( (direction & IN_OFF) && (direction & OUT_OFF)) {
	    set_route(codec, direction);
		dprintk(SND_INF, "[PATH:OFF]cur:0x%08x , new:0x%08x\n", path_data.path, direction);
		path_data.path = path_data.path_count = 0;
		for(i=0; i<MAX_PATH_COUNT; i++) {
			path_data.prev_path[i] = 0;
		}
	}
	else if (direction & HEADSET_DELETE) {
		if(path_data.path & IN_AP) {
			if(path_data.path & (OUT2_SPK_L | OUT2_SPK_R)) {
				amp_reg_write(0x04, 0x81);	// SPK = INA1 + INA2

				path_data.path &= ~(OUT2_HP);
				for(i=0; i<MAX_PATH_COUNT; i++) {
					if( path_data.prev_path[i] == (IN_AP | OUT2_HP)) {
						path_data.prev_path[i] = 0;
						path_data.path_count--;
						amp_state = SPEAKER;	// amp state update
					}
				}
			}
			else {
				set_amp(OFF, 0);
				path_data.path &= ~(IN_AP | OUT2_HP);
				for(i=0; i<MAX_PATH_COUNT; i++) {
					if( path_data.prev_path[i] == (IN_AP | OUT2_HP)) {
						path_data.prev_path[i] = 0;
						path_data.path_count--;
						amp_state = 0;	// amp_state update
					}
				}
			}
		}
		
	}
	else if (direction & TOGGLE_SPK_EAR) {
		if(path_data.path & (OUT2_SPK_L | OUT2_SPK_R)) {
			toggle_amp(codec, SPEAKER);
			path_data.path &= ~(OUT2_SPK_L | OUT2_SPK_R);
			path_data.path |= OUT2_HP;

			for(i=0; i<MAX_PATH_COUNT; i++) {
				if( path_data.prev_path[i] & (OUT2_SPK_L | OUT2_SPK_R)) {
						path_data.prev_path[i] = 0;
						path_data.path_count--;
				}
			}
			for(i=0; i<MAX_PATH_COUNT; i++) {
				if(path_data.prev_path[i] == 0) {
					path_data.prev_path[i] = (IN_AP | OUT2_HP);
					break;
				}
			}
		}
		else if(path_data.path & (OUT2_HP)) {
			toggle_amp(codec, HEADPHONE);
			path_data.path &= ~(OUT2_HP);
			path_data.path |= (OUT2_SPK_L | OUT2_SPK_R);

			for(i=0; i<MAX_PATH_COUNT; i++) {
				if( path_data.prev_path[i] == (IN_AP | OUT2_HP)) {
						path_data.prev_path[i] = 0;
						path_data.path_count--;
				}
			}
			for(i=0; i<MAX_PATH_COUNT; i++) {
				if(path_data.prev_path[i] == 0) {
					path_data.prev_path[i] = (IN_AP | OUT2_SPK_L | OUT2_SPK_R);
					break;
				}
			}
		}
		else
			printk("[PATH:ERROR] at the toggle path..\n");
	}
	else if (direction != path_data.path) {
		for(i=0; i<MAX_PATH_COUNT; i++) {
			if(path_data.prev_path[i] == direction) {
				dprintk(SND_INF, "[PATH:SAME]prev[%d]:0x%08x, new:0x%08x\n", i, path_data.prev_path[i], direction);
				return;
			}
		}
		set_route (codec, direction);
		dprintk(SND_INF, "[PATH:SET]cur:0x%08x , new:0x%08x\n", path_data.path, direction);

		for(i=0; i<MAX_PATH_COUNT; i++) {
			if(path_data.prev_path[i] == 0) {
				path_data.prev_path[i] = direction;
				break;
			}
		}
		
		for(i=0; i<MAX_PATH_COUNT; i++) {
			if(path_data.prev_path[i] != 0) {
				path_data.path_count++;
			}
		}
		
		path_data.path |= direction;
    }
	else {
		dprintk(SND_INF, "[PATH:SAME1]cur:0x%08x , new:0x%08x\n", path_data.path, direction);
	}

	dprintk(SND_INF, "[PATH:status]cur:0x%08x , new:0x%08x\n", path_data.path, direction);

	mic_power_up(codec);

	for(i=0; i<MAX_PATH_COUNT; i++) {
		dprintk(SND_DBG, "prev_path[%d]: 0x%08x\n", i, path_data.prev_path[i]);
	}

}

int get_sound_path(void)
{
	return path_data.path;
}


void set_capture_path(struct snd_soc_codec *codec, unsigned int direction)
{
	unsigned int i;

	if ((IN_OFF & direction) && (!(OUT_OFF & direction))) {
		
		input_power_down(codec, direction);
		dprintk(SND_INF, "[CAPTURE OFF] cur:0x%08x, new:0x%08x\n", path_data.path, direction );
		path_data.path &= (~(IN_OFF | OUT_AP));
		
		for(i=0; i<MAX_PATH_COUNT; i++) {
			if(path_data.prev_path[i] == direction) {
				path_data.prev_path[i] = 0;
				return;
			}
		}

		for(i=0; i<MAX_PATH_COUNT; i++) {
			DPRINTK("prev_path: 0x%08x\n",path_data.prev_path[i]);
		}
		
		dprintk(SND_INF, "[CAPTURE:STATUS] cur:0x%08x, new:0x%08x\n", path_data.path, direction );
	}
	else
		set_sound_path(codec, direction);

}

void set_audio_info(struct snd_soc_codec *codec)
{
	int i,j;
	same_path_flag = false;
	
	curr_sound_scenario = curr_audio_info.gain_info;
		
	for(i=0; i<MAX_PATH_COUNT; i++) 
	{
		if(curr_audio_info.path[i] != 0) 
		{
			
			if(curr_audio_info.path[i]==(IN_OFF | OUT_OFF)) 
			{
				set_sound_path(codec, curr_audio_info.path[i]);
				DPRINTK("[PATH:OFF]reset path data\n");
			}
			
			else if(!check_direction(curr_audio_info.path[i])) 
			{
				DPRINTK("[SKIP] wrong path\n");
			} 
			else 
			{	
				for(j=0; j<MAX_PATH_COUNT; j++) 
				{
					if(	prev_audio_info.path[j] == curr_audio_info.path[i])
						same_path_flag = true;	
				}
				
				if(same_path_flag == false) 
				{	
					set_sound_path(codec, curr_audio_info.path[i]);
					DPRINTK("set audio path: %x\n",curr_audio_info.path[i]);
				} 
				
				else 
				{ 
					same_path_flag = false;
					DPRINTK("[SKIP]already connected : %x\n",curr_audio_info.path[i]);
				}
			}
		}
	}

}






