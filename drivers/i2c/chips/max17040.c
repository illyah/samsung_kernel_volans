/*
 * max17040.c - Customized driver for MAX17040 Fuel Gauge.
 *
 * Copyright (C) 2009 Samsung Electronics.
 *
 * Author: Geun-Young, Kim <nenggun.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c/max17040.h>

/* global variables. */
static struct i2c_client *max17040_client = NULL;

static int max17040_write(u8 reg, u8 value)
{
	int ret;
	ret = i2c_smbus_write_byte_data(max17040_client, reg, value);

	if (ret < 0)
		pr_err("%s: error!(%d)\n", __func__, ret);

	return ret;
}

static int max17040_read(u8 reg)
{
	int ret;
	ret = i2c_smbus_read_byte_data(max17040_client, reg);

	if (ret < 0)
		pr_err("%s: error!(%d)\n", __func__, ret);

	return ret;
}

static void max17040_get_version(void)
{
	u8 msb, lsb;

	msb = max17040_read(MAX17040_VER_MSB);
	lsb = max17040_read(MAX17040_VER_LSB);

	pr_info("MAX17040 Fuel Gauge Version %d%d.\n", msb, lsb);
}

static void max17040_set_rcomp(u16 value)
{
	u8 msb, lsb;

	msb = (u8)(value >> 8);
	lsb = (u8)(value & 0x00ff);

	max17040_write(MAX17040_RCOMP_MSB, msb);
	max17040_write(MAX17040_RCOMP_LSB, lsb);
}

int max17040_get_data(unsigned char *buf, size_t count)
{
	if (!count || count > MAX17040_DATA_SIZE)
		return 0;

	memset(buf, 0, sizeof(buf));

	/* only vcell and soc value. */
	buf[MAX17040_VCELL_MSB] = max17040_read(MAX17040_VCELL_MSB);
	buf[MAX17040_VCELL_LSB] = max17040_read(MAX17040_VCELL_LSB);
	buf[MAX17040_SOC_MSB] = max17040_read(MAX17040_SOC_MSB);
	buf[MAX17040_SOC_LSB] = max17040_read(MAX17040_SOC_LSB);

	return count;
}
EXPORT_SYMBOL(max17040_get_data);

/* -------------------------------------------------------------------- */
static int max17040_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	if (!client)
		return -EINVAL;

	max17040_client = client;

	max17040_get_version();
	max17040_set_rcomp(0x9700);

	return 0;
}

static int max17040_remove(struct i2c_client *client)
{
	/* nothing.. */

	return 0;
}

static const struct i2c_device_id max17040_id[] = {
	{ "max17040", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max17040_id);

static struct i2c_driver max17040_i2c_driver = {
	.driver = {
		.name = "max17040",
		.owner = THIS_MODULE,
	},
	.probe = max17040_probe,
	.remove = max17040_remove,
	.id_table = max17040_id,
};

static int __init max17040_init(void)
{
	return i2c_add_driver(&max17040_i2c_driver);
}

static void __exit max17040_exit(void)
{
	i2c_del_driver(&max17040_i2c_driver);
}

module_init(max17040_init);
module_exit(max17040_exit);

MODULE_AUTHOR("Geun-Young, Kim <nenggun.kim@samsung.com>");
MODULE_DESCRIPTION("MAX17040 PMIC driver");
MODULE_LICENSE("GPL");

