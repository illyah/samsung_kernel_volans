
#ifndef	_SYNAI2C_H 
#define	_SYNAI2C_H

#include "UserI2C.h"

#define		I2C_SUCCESS				0x00
#define		I2C_FAILURE				0x01

#define I2C_TRUE		1
#define	I2C_FALSE	0

void
I2C_HAL_delay_10th_usec(unsigned short tenth_usec);

void
I2C_HAL_SDA_IN_MODE(void);
void
I2C_HAL_SCL_IN_MODE(void);

void
I2C_HAL_SDA_OUT_MODE(void);
void
I2C_HAL_SCL_OUT_MODE(void);

void
I2C_HAL_I2C_SDA_LO(void);
void
I2C_HAL_I2C_SCL_LO(void);

unsigned char
I2C_HAL_BitRead_I2C_SDA(void);
unsigned char
I2C_HAL_BitRead_I2C_SCL(void);

unsigned char
I2C_HAL_BitRead_ATTN(void);

#endif
