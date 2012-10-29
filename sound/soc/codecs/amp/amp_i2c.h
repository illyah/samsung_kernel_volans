#ifndef __OMAP3430_AMP_I2C_HEADER__
#define __OMAP3430_AMP_I2C_HEADER__

#define AMP_INPUTMODECONTROL_ADDRESS				0x00
#define AMP_SPEAKERVOLUMECONTROL_ADDRESS			0x01
#define AMP_LEFTHEADPHONEVOLUMECONTROL_ADDRESS	0x02
#define AMP_RIGHTHEADPHONEVOLUMECONTROL_ADDRESS	0x03
#define AMP_OUTPUTMODECONTROL_ADDRESS				0x04

// add I2C flag : not defined under two line in linux/i2c.h header
#define I2C_DF_NOTIFY	0x01
#define I2C_M_WR		0x00

int amp_register_current_state(void);

int amp_read_byte(u8 reg, u8 *val);
int amp_write_byte(u8 reg, u8 val);

int amp_read_word(u8 reg, u16 *val);
int amp_write_word(u8 reg, u16 val);

int amp_power_up(void);

int i2c_amp_init(void);
void i2c_amp_exit(void);

#endif
