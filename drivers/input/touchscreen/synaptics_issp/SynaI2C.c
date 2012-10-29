
#include "SynaI2C.h"
#include "UserI2C.h"
#include "SynaAbstractionLayer.h"
#include <linux/kernel.h>

#define I2C_READ		0x01	

#define I2C_QDEL I2C_HAL_delay_10th_usec(40);		

#define I2C_HDEL	I2C_HAL_delay_10th_usec(47); 

#define	I2C_TLOW_DELAY         	I2C_HAL_delay_10th_usec(47);

#define	I2C_THIGH_DELAY        	I2C_HAL_delay_10th_usec(40);

#define	I2C_TBUF_DELAY         	I2C_HAL_delay_10th_usec(47);

#define	I2C_TSU_STA_DELAY      	I2C_HAL_delay_10th_usec(47);

#define	I2C_THD_STA_DELAY      	I2C_HAL_delay_10th_usec(40);

#define	I2C_TSU_STO_DELAY      	I2C_HAL_delay_10th_usec(40);

unsigned char
I2C_Set_ReadAddress(unsigned char uRmiAddress);

void
I2C_SDA_HIGH(void);
unsigned char
I2C_SCL_HIGH(void);

void
I2C_SDA_LOW(void);
void
I2C_SCL_LOW(void);

void
I2C_SCL_TOGGLE(void);
void
I2C_START(void);
void
I2C_STOP(void);

unsigned char
I2C_PutByte(unsigned char bByte);

unsigned char
I2C_GetByte(unsigned char bLast);

void
I2C_SCL_TOGGLE(void)
{
  I2C_HDEL;
  I2C_SCL_HIGH();
  I2C_HDEL;
  I2C_SCL_LOW();
}

void
I2C_START(void)
{
  I2C_SDA_LOW();
  I2C_QDEL;
  I2C_SCL_LOW();
}

void
I2C_STOP(void)
{
  I2C_SDA_LOW();
  I2C_HDEL;
  I2C_SCL_HIGH();
  I2C_QDEL;
  I2C_SDA_HIGH();
  I2C_HDEL;
}

void
I2C_SDA_LOW(void)
{
  I2C_HAL_SDA_OUT_MODE();
  I2C_HAL_I2C_SDA_LO();
}

void
I2C_SCL_LOW(void)
{
  I2C_HAL_SCL_OUT_MODE();
  I2C_HAL_I2C_SCL_LO();
}

void
I2C_SDA_HIGH(void)
{
  I2C_HAL_SDA_IN_MODE();
}

unsigned char
I2C_SCL_HIGH(void)
{
  signed long bDelay = I2C_HAL_CLK_STRETCH_LIMIT;

  I2C_HAL_SCL_IN_MODE();

  while ((I2C_HAL_BitRead_I2C_SCL() == 0) && (bDelay > 0))
    {
      bDelay--;
      SynaSleep(1);
    }

  if (bDelay == 0)
    {
      printk("[TSP][ERROR] I2C_SCL_HIGH fail. bDelay==0\n");
      return 1;
    }
  return 0;
}

unsigned char
I2C_PutByte(unsigned char Byte)
{
  signed char Counter;

  for (Counter = 7; Counter >= 0; Counter--)
    {
      if (Byte & (1 << Counter))
        {
          I2C_SDA_HIGH();
        }
      else
        {
          I2C_SDA_LOW();
        }

      I2C_SCL_TOGGLE();
    }

  I2C_SDA_HIGH();

  I2C_HDEL;
  if (I2C_SCL_HIGH())
    return I2C_FAILURE;
  I2C_HDEL;

  Byte = I2C_HAL_BitRead_I2C_SDA();

  I2C_SCL_LOW();
  I2C_SDA_HIGH();

  if (Byte == 0)
    return I2C_SUCCESS;

  return I2C_FAILURE;
}

unsigned char
I2C_GetByte(unsigned char Last)
{
  signed char Counter;
  unsigned char Val, Byte = 0;

  I2C_SDA_HIGH();

  for (Counter = 7; Counter >= 0; Counter--)
    {
      I2C_HDEL;
      if (I2C_SCL_HIGH())
        return I2C_FAILURE;
      Val = I2C_HAL_BitRead_I2C_SDA();

      Byte <<= 1;
      if (Val)
        Byte |= 1;
      I2C_HDEL;
      I2C_SCL_LOW();
    }

  if (Last)
    I2C_SDA_HIGH();
  else
    I2C_SDA_LOW();

  I2C_SCL_TOGGLE();
  I2C_SDA_HIGH();

  return Byte;

}

unsigned int
SynaWriteRegister(unsigned short uRmiAddress, unsigned char Buffer[],
    unsigned int BytesToSend)
{
  unsigned char Count = 0;

  I2C_START();

  if (I2C_PutByte(I2CAddr7Bit << 1) == I2C_FAILURE)
    {
      I2C_STOP();
      return I2C_FAILURE;
    }

  if (I2C_PutByte((unsigned char) uRmiAddress) == I2C_FAILURE)
    {
      I2C_STOP();
      return I2C_FAILURE;
    }

  while (Count < BytesToSend)
    {
      if (I2C_PutByte(Buffer[Count++]) == I2C_FAILURE)
        {
          I2C_STOP();
          return I2C_FAILURE;
        }
    }

  I2C_STOP();

  return I2C_SUCCESS;
}

unsigned int
SynaReadRegister(unsigned short uRmiAddress, unsigned char Buffer[],
    unsigned int BytesToRead)
{
  unsigned char Count = 0;

  I2C_START();

  I2C_Set_ReadAddress(uRmiAddress);

  I2C_START();

  if (I2C_PutByte(I2CAddr7Bit << 1 | I2C_READ) == I2C_FAILURE)
    {
      I2C_STOP();
      return I2C_FAILURE;
    }

  while (Count < BytesToRead)
    {
      Buffer[Count] = I2C_GetByte(Count == (BytesToRead - 1));
      Count++;
    }

  I2C_STOP();

  return I2C_SUCCESS;

}

unsigned char
I2C_Set_ReadAddress(unsigned char uRmiAddress)
{

  if (I2C_PutByte(I2CAddr7Bit << 1) == I2C_FAILURE)
    {
      I2C_STOP();
      return I2C_FAILURE;
    }

  if (I2C_PutByte((unsigned char) uRmiAddress) == I2C_FAILURE)
    {
      I2C_STOP();
      return I2C_FAILURE;
    }

  I2C_STOP();

  return I2C_SUCCESS;

}

