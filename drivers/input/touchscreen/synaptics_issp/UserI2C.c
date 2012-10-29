
#include "SynaI2C.h"
#include "UserI2C.h"

#include <linux/gpio.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/volans.h>

#include <linux/delay.h>

void
I2C_HAL_delay_10th_usec(unsigned short tenth_microsec)
{
  unsigned long usecs = (unsigned long) tenth_microsec;
  udelay(tenth_microsec);

}

void
I2C_HAL_SDA_IN_MODE(void)
{
  s3c_gpio_cfgpin(GPIO_TOUCH_SDA, S3C_GPIO_INPUT);

}

void
I2C_HAL_SCL_IN_MODE(void)
{
  s3c_gpio_cfgpin(GPIO_TOUCH_SCL, S3C_GPIO_INPUT);

}

void
I2C_HAL_SDA_OUT_MODE(void)
{
  s3c_gpio_cfgpin(GPIO_TOUCH_SDA, S3C_GPIO_OUTPUT);

}

void
I2C_HAL_SCL_OUT_MODE(void)
{
  s3c_gpio_cfgpin(GPIO_TOUCH_SCL, S3C_GPIO_OUTPUT);

}

void
I2C_HAL_I2C_SDA_LO(void)
{
  gpio_set_value(GPIO_TOUCH_SDA, GPIO_LEVEL_LOW);

}

void
I2C_HAL_I2C_SCL_LO(void)
{
  gpio_set_value(GPIO_TOUCH_SCL, GPIO_LEVEL_LOW);

}

unsigned char
I2C_HAL_BitRead_I2C_SDA(void)
{
  return gpio_get_value(GPIO_TOUCH_SDA);

}

unsigned char
I2C_HAL_BitRead_I2C_SCL(void)
{
  return gpio_get_value(GPIO_TOUCH_SCL);

}

unsigned char
I2C_HAL_BitRead_ATTN(void)
{
  return gpio_get_value(GPIO_TOUCH_IRQ);

}

void
SynaSleep(unsigned long MilliSeconds)
{
  mdelay(MilliSeconds);

}

unsigned char
SynaWaitForATTN(unsigned long MilliSeconds)
{

  while (((I2C_HAL_BitRead_ATTN() == 1) & (MilliSeconds < ATTN_TIMEOUT)))
    SynaSleep(1);

  if (I2C_HAL_BitRead_ATTN() == 0)
    return I2C_SUCCESS;
  printk("\nI2C_HAL_BitRead_ATTN : %d\n", I2C_HAL_BitRead_ATTN());

  return I2C_FAILURE;
}
