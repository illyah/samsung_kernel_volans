/*
 * Synaptics RMI4 touchscreen driver
 *
 * Copyright (C) 2010
 * Author: Joerie de Gram <j.de.gram@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>
#include <linux/irq.h>
#include <linux/delay.h>

#define TOUCH_IRQ  IRQ_EINT_GROUP(2, 7)
#define ABS_X_MAX       240
#define ABS_Y_MAX       400
#define LONG(x) ((x)/BITS_PER_LONG)

static s32 s_finger_status, s_finger_contact, s_x_msb, s_y_msb, s_xy_lsb;

static struct workqueue_struct *synaptics_wq;

struct synaptics_rmi4_data {
        struct input_dev *input;
        struct i2c_client *client;
        struct hrtimer timer;
        struct work_struct work;
        unsigned int irq;
        unsigned char f01_control_base;
        unsigned char f01_data_base;
        unsigned char f11_data_base;
};

static enum hrtimer_restart synaptics_rmi4_timer_func(struct hrtimer *timer)
{
        struct synaptics_rmi4_data *ts = container_of(timer, struct synaptics_rmi4_data, timer);

        queue_work(synaptics_wq, &ts->work);

        hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);

        return HRTIMER_NORESTART;
}

static void synaptics_rmi4_work(struct work_struct *work)
{
        struct synaptics_rmi4_data *ts = container_of(work, struct synaptics_rmi4_data, work);
        s32 finger_status, finger_contact, x_msb, y_msb, xy_lsb;
        finger_status = i2c_smbus_read_byte_data(ts->client, ts->f11_data_base + 0);
        x_msb = i2c_smbus_read_byte_data(ts->client, ts->f11_data_base + 1);
        y_msb = i2c_smbus_read_byte_data(ts->client, ts->f11_data_base + 2);
        xy_lsb = i2c_smbus_read_byte_data(ts->client, ts->f11_data_base + 3);
        finger_contact = i2c_smbus_read_byte_data(ts->client, ts->f11_data_base + 5);

        s_x_msb = (x_msb << 4)| (xy_lsb & 0x0f);
        s_y_msb = (y_msb << 4)| ((xy_lsb & 0xf0) >> 4);

        s_xy_lsb = xy_lsb;
        s_finger_contact = finger_contact;
        s_finger_status = finger_status;

        input_report_abs(ts->input, ABS_X, (x_msb << 4)| (xy_lsb & 0x0f));
        input_report_abs(ts->input, ABS_Y, (y_msb << 4)| ((xy_lsb & 0xf0) >> 4));
        input_report_abs(ts->input, ABS_PRESSURE, finger_status);
        input_report_abs(ts->input, ABS_TOOL_WIDTH,finger_status);
        input_report_key(ts->input, BTN_TOUCH, finger_status);//(finger_status == 0x01 || finger_status == 0x02 || finger_status == 0x03) ? 1 : 0);

        input_sync(ts->input);

        if(ts->irq) {
                i2c_smbus_read_word_data(ts->client, ts->f01_data_base + 1);
                enable_irq(ts->irq);
        }
}

static void synaptics_rmi4_irq_setup(struct synaptics_rmi4_data *ts)
{
        i2c_smbus_write_byte_data(ts->client, ts->f01_control_base + 1, 0x04);
        pr_info("synaptics-rmi4: irq setup succesful\n");
}

static irqreturn_t synaptics_rmi4_irq_handler(int irq, void *dev_id)
{
        struct synaptics_rmi4_data *ts = dev_id;

        disable_irq_nosync(ts->irq);
        queue_work(synaptics_wq, &ts->work);

        return IRQ_HANDLED;
}

static ssize_t sysfs_touchcontrol_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
  return 0;
}

static ssize_t
sysfs_touchcontrol_show(struct device *dev, struct device_attribute *attr,
    char *buf)
{
  printk("[TSP] finger status: %d, x_msb: %d, y_msb: %d, xy_lsb: %d, finger_contact: %d\n", s_finger_status, s_x_msb, s_y_msb, s_xy_lsb, s_finger_contact);

  return 0;
}

static DEVICE_ATTR(touchcontrol, S_IRUGO | S_IWUSR, sysfs_touchcontrol_show, sysfs_touchcontrol_store);

static void touchscreen_hw_init()
{
  s3c_gpio_cfgpin(GPIO_TOUCH_LDO_EN, S3C_GPIO_OUTPUT);
  s3c_gpio_setpull(GPIO_TOUCH_LDO_EN, S3C_GPIO_PULL_NONE);

  s3c_gpio_setpull(GPIO_TOUCH_SCL, S3C_GPIO_PULL_UP);
  s3c_gpio_setpull(GPIO_TOUCH_SDA, S3C_GPIO_PULL_UP);

  s3c_gpio_cfgpin(GPIO_TOUCH_IRQ, S3C_GPIO_SFN(GPIO_TOUCH_IRQ_AF));
  s3c_gpio_setpull(GPIO_TOUCH_IRQ, S3C_GPIO_PULL_UP);
  set_irq_type(TOUCH_IRQ, IRQ_TYPE_EDGE_FALLING);
  printk("[TSP] irq %d\n",TOUCH_IRQ);
  udelay(10);
}

static int __devinit synaptics_rmi4_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        struct synaptics_rmi4_data *ts;
        struct input_dev *input_dev;
        int err;

        if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_WORD_DATA)) {
                return -EIO;
        }

        touchscreen_hw_init();
        gpio_set_value(GPIO_TOUCH_LDO_EN, GPIO_LEVEL_HIGH);

        ts = kzalloc(sizeof(struct synaptics_rmi4_data), GFP_KERNEL);
        input_dev = input_allocate_device();

        if (!ts || !input_dev) {
                pr_err("synaptics-rmi4: failed to allocate memory\n");
                err = -ENOMEM;
                goto err_free_mem;
        }

        ts->irq = TOUCH_IRQ;
        ts->f01_control_base = 0x23;
        ts->f01_data_base = 0x13;
        ts->f11_data_base = 0x15; /* FIXME */

        ts->client = client;
        ts->input = input_dev;

        INIT_WORK(&ts->work, synaptics_rmi4_work);

        input_dev->name = "Synaptics RMI4 touchscreen";
        input_dev->phys = "synaptics-rmi4/input0";
        input_dev->id.bustype = BUS_I2C;

        ts->input->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY); //BIT_MASK(EV_SYN) |
        ts->input->absbit[LONG(ABS_X)] = BIT(ABS_X);
        ts->input->absbit[LONG(ABS_Y)] = BIT(ABS_Y);
        ts->input->absbit[LONG(ABS_PRESSURE)] = BIT(ABS_PRESSURE);
        ts->input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

        input_set_abs_params(ts->input, ABS_X, 0, 240, 0, 0);
        input_set_abs_params(ts->input, ABS_Y, 0, 400, 0, 0);
        input_set_abs_params(ts->input, ABS_PRESSURE, 0, 255, 0, 0);
        input_set_abs_params(ts->input, ABS_TOOL_WIDTH, 0, 15, 0, 0);

        input_set_drvdata(ts->input, ts);

        err = input_register_device(input_dev);
        if (err) {
                pr_err("synaptics-rmi4: failed to register input device\n");
                goto err_free_mem;
        }

        i2c_set_clientdata(client, ts);


        if(ts->irq) {
                //err = request_irq(gpio_to_irq(ts->irq), synaptics_rmi4_irq_handler, IRQF_TRIGGER_FALLING, "SYNAPTICS_ATTN", ts);
            err = request_irq(TOUCH_IRQ, synaptics_rmi4_irq_handler, IRQF_TRIGGER_FALLING, "SYNAPTICS_ATTN", ts);
        }

        err = device_create_file(&input_dev->dev, &dev_attr_touchcontrol);

        if(!err && ts->irq) {
                synaptics_rmi4_irq_setup(ts);
        } else {
                pr_info("synaptics-rmi4: GPIO IRQ missing, falling back to polled mode\n");
                ts->irq = 0;

                hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
                ts->timer.function = synaptics_rmi4_timer_func;
                hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
        }

        return 0;

 err_free_mem:
        input_free_device(input_dev);
        kfree(ts);
        return err;
}

static int __devexit synaptics_rmi4_remove(struct i2c_client *client)
{
        struct synaptics_rmi4_data *ts = i2c_get_clientdata(client);
        input_unregister_device(ts->input);

        if(ts->irq) {
                free_irq(ts->irq, ts);
        } else {
                hrtimer_cancel(&ts->timer);
        }

        kfree(ts);

        return 0;
}

static struct i2c_device_id synaptics_rmi4_idtable[] = {
        { "synaptics-rmi4", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, synaptics_rmi4_idtable);

static struct i2c_driver synaptics_rmi4_driver = {
        .probe          = synaptics_rmi4_probe,
        .remove         = __devexit_p(synaptics_rmi4_remove),
        .id_table       = synaptics_rmi4_idtable,
        .driver = {
                .owner  = THIS_MODULE,
                .name   = "synaptics-rmi4"
        },
};

static int __init synaptics_rmi4_init(void)
{
        synaptics_wq = create_singlethread_workqueue("synaptics_wq");

        if (!synaptics_wq) {
                return -ENOMEM;
        }

        return i2c_add_driver(&synaptics_rmi4_driver);
}

static void __exit synaptics_rmi4_exit(void)
{
        i2c_del_driver(&synaptics_rmi4_driver);
}

module_init(synaptics_rmi4_init);
module_exit(synaptics_rmi4_exit);

MODULE_AUTHOR("Joerie de Gram <j.de.gram@gmail.com");
MODULE_DESCRIPTION("Synaptics RMI4 touchscreen driver");
MODULE_LICENSE("GPL");
