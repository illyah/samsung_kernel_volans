/* linux/arch/arm/plat-s3c/include/plat/spi.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_SPI_H
#define __PLAT_SPI_H __FILE__

#define CS_ACT_LOW		0
#define CS_ACT_HIGH		1

#define CS_FLOAT	-1
#define CS_LOW		0
#define CS_HIGH		1

struct sam_spi_pdata {
	int cs_level;		/* Current level of the CS line */
	int cs_act_high;	/* If the slave device is CS_Active_High */
	int cs_pin;	/* GPIO Pin number used as the ChipSelect for this Slave */
	int cs_mode;
	void (*cs_config)(int pin, int mode, int pull); /* Configure the GPIO in appropriate mode(output) initially */
	void (*cs_set)(int pin, int lvl); /* CS line control */
};

struct sam_spi_mstr_info {
	int num_slaves;
};

#define DECLARE_SPIPDATA(set, cfg, ah, pin, mode, bus, slv)	static struct sam_spi_pdata sam_slv_pdata_b##bus##s##slv = {	\
									.cs_level     = CS_FLOAT,			\
									.cs_act_high  = ah,				\
									.cs_pin       = pin,				\
									.cs_mode      = mode,				\
									.cs_set       = set,				\
									.cs_config    = cfg,				\
								}
extern void samspi_set_info(unsigned id, struct sam_spi_mstr_info *info);

#endif /* __PLAT_SPI_H */
