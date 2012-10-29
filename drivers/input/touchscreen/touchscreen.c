
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>
#include <linux/platform_device.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>
#include <plat/gpio-bank-o.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <linux/firmware.h>
#include "dprintk.h"
#include "message.h"
#include <plat/s3c64xx-dvfs.h>

#include "touchscreen.h"

#define TOUCH_IRQ_EINT	IRQ_EINT_GROUP(2, 7)

#define TOUCHSCREEN_NAME			"touchscreen"
#define DEFAULT_PRESSURE_UP		0
#define DEFAULT_PRESSURE_DOWN		256

#define __TOUCH_DRIVER_MAJOR_VER__ 1
#define __TOUCH_DRIVER_MINOR_VER__ 0

#define SYNAPTICS_VERSION_ADDR	0x71
#define SYNAPTICS_READ_ADDR	0x12
#define SYNAPTICS_READ_CNT	14

#define LATEST_FIRMWARE_VERSION		0x17

#define LONG(x) ((x)/BITS_PER_LONG)


#define MAX_TOUCH_X_RESOLUTION	240
#define MAX_TOUCH_Y_RESOLUTION	400


enum EnTouchSleepStatus
{
  EN_TOUCH_SLEEP_MODE = 0, EN_TOUCH_WAKEUP_MODE = 1
};

enum EnTouchPowerStatus
{
  EN_TOUCH_POWEROFF_MODE = 0, EN_TOUCH_POWERON_MODE = 1
};

enum EnTouchDriveStatus
{
  EN_TOUCH_USE_DOZE_MODE = 0, EN_TOUCH_USE_NOSLEEP_MODE = 1
};

static struct touchscreen_t tsp;
static struct work_struct tsp_work;
static struct workqueue_struct *tsp_wq;
static int g_touch_onoff_status = 0;
static int g_enable_touchscreen_handler = 0;
static void
(*touchscreen_read)(struct work_struct *work) = NULL;

struct semaphore sem_touch_onoff;
struct semaphore sem_touch_handler;

struct timeval g_current_time;

static u16 old_x_position = -1;
static u16 old_y_position = -1;
static u16 old_x2_position = -1;
static u16 old_y2_position = -1;
static int finger_switched = 0;
static int old_finger_type = 0;

static u8 g_version_read_addr = 0;
static u8 g_synaptics_read_addr = 0;
static int g_synaptics_read_cnt = 0;

static u8 g_synaptics_device_control_addr = 0;

static int g_sleep_onoff_status = EN_TOUCH_WAKEUP_MODE;
struct semaphore sem_sleep_onoff;

extern unsigned char SynaFirmware[11010];

extern int
i2c_tsp_sensor_read(u8 reg, u8 *val, unsigned int len);
extern int
touch_add_i2c_device(const struct touch_setup_data *setup);
extern unsigned int
SynaDoReflash(void);

extern int
i2c_tsp_sensor_write_reg(u8 address, int data);

static void
synaptics_touchscreen_read(struct work_struct *work);
void
synaptics_touchscreen_start_triggering(void);

static irqreturn_t
touchscreen_handler(int irq, void *dev_id);
static void
touchscreen_poweroff(void);
static void
touchscreen_poweron(void);
static void
touchscreen_hw_init(void);
static int
synaptics_set_drive_mode_bit(enum EnTouchDriveStatus drive_status);

int
check_firmware_image(char *file_path)
{
  struct file *check_file;

  check_file = filp_open(file_path, O_RDONLY, 0);

  if (IS_ERR(check_file))
    {
      printk("[TSP][WARNING] there is no touch firmware.(%s)\n", file_path);
      return -1;
    }
  else
    {
      filp_close(check_file, NULL);
    }
  return 0;
}

static void
issp_request_firmware(char* update_file_name)
{
  int idx_src = 0;
  int idx_dst = 0;

  struct device *dev = &tsp.inputdevice->dev;
  const struct firmware * fw_entry;

  memset(SynaFirmware, 0, sizeof(SynaFirmware));

  request_firmware(&fw_entry, update_file_name, dev);
  do
    {
      SynaFirmware[idx_dst] = fw_entry->data[idx_src];
      idx_src++;
      idx_dst++;
    }
  while (idx_src < 11008);

  SynaFirmware[11008] = 0xFF;
  SynaFirmware[11009] = 0xFF;
}

static int
touch_firmware_update(char* firmware_file_name)
{
  int ret = 0;
  char firmware_full_path[50] =
    { 0, };
  unsigned long touch_handler_flag;

  ret = check_firmware_image("/mnt/mmc/mmc_vtouch.img");

  if (ret == -1)
    strcpy(firmware_full_path, "/lib/firmware/");
  else
    strcpy(firmware_full_path, "/mnt/mmc/");

  strcat(firmware_full_path, firmware_file_name);

  ret = check_firmware_image(firmware_full_path);

  if (ret == -1)
    return -1;

  local_irq_save(touch_handler_flag);
  if (g_enable_touchscreen_handler == 1)
    {
      g_enable_touchscreen_handler = 0;
      free_irq(tsp.irq, &tsp);
    }
  local_irq_restore(touch_handler_flag);

  issp_request_firmware(firmware_file_name);

  SynaDoReflash();
  touchscreen_poweroff();
  touchscreen_poweron();
  synaptics_touchscreen_start_triggering();
  printk("[TSP] touch firmware update done...\n");
  return 1;
}

static ssize_t
sysfs_issp_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
  int fw_upgrade;

  char filename[50] =
    { '\0', };
  u8 data[2];

  int rst = 0;
  int ret = 0;

  touchscreen_poweron();

  rst = sscanf(buf, "%d", &fw_upgrade);

  if (fw_upgrade == 1)
    {
      printk("[TSP] start update touch firmware(%d) !!\n", fw_upgrade);

      ret = touch_firmware_update("touch.img");
    }
  else if (fw_upgrade == 2)
    {
      sscanf(buf + 1, "%50s", filename);
      if (filename[0] == (char) '\0')
        {
          printk("[TSP][ERROR] please input update file name.\n");
          return -EINVAL;
        }
      else
        {
          printk("[TSP] update file name is \"%s\"\n", filename);
        }
      printk("[TSP] start update touch firmware(%d) !!\n", fw_upgrade);

      ret = touch_firmware_update(filename);
    }
  else if (fw_upgrade == 3)
    {
      printk("[TSP] touch firmware auto update\n");

      i2c_tsp_sensor_read(0x71, data, 1);
      printk("[TSP] current firmware S/W revision info = %d\n", data[0]);

      if (LATEST_FIRMWARE_VERSION > data[0])
        {
          ret = touch_firmware_update("touch.img");

          printk("[TSP] latest touch firmware installed : %d\n",
              LATEST_FIRMWARE_VERSION);

          display_dbg_msg("touch firmware update finished !",
              KHB_DEFAULT_FONT_SIZE, KHB_DISCARD_PREV_MSG);
        }
      else
        {
          printk("[TSP] No excution touch firmware update.\n");
          printk("[TSP] already installed latest touch firmware : %d\n",
              data[0]);
        }
    }
  else if (fw_upgrade == 4)
    {
      printk("[TSP] start update touch firmware(%d) !!\n", fw_upgrade);
      ret = touch_firmware_update("mmc_vtouch.img");
    }
  else
    {
      printk("[TSP][ERROR] wrong touch firmware update command\n");
      ret = -1;
    }

  if (ret < 0)
    printk("[TSP][ERROR] %s() firmware update fail !!\n", __FUNCTION__);
  else
    printk("[TSP] %s() firmware update success !!\n", __FUNCTION__);

  return count;
}

static ssize_t
sysfs_issp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  u8 data[2] =
    { 0, };

  i2c_tsp_sensor_read(0x71, data, 1);

  return sprintf(buf,
      "touch driver version : %d.%d, firmware S/W Revision Info = %d\n",
      __TOUCH_DRIVER_MAJOR_VER__, __TOUCH_DRIVER_MINOR_VER__, data[0]);
}

static DEVICE_ATTR(issp, S_IRUGO | S_IWUSR, sysfs_issp_show, sysfs_issp_store);

void
synaptics_touchscreen_start_triggering(void)
{
  u8 *data = NULL;
  int ret = 0;

  data = kmalloc(g_synaptics_read_cnt, GFP_KERNEL);
  if (data == NULL)
    {
      printk("[TSP][ERROR] %s() kmalloc fail\n", __FUNCTION__);
      return;
    }

  ret = i2c_tsp_sensor_read(g_synaptics_read_addr, data, g_synaptics_read_cnt);
  if (ret != 0)
    {
      printk("[TSP][ERROR] %s() i2c_tsp_sensor_read error : %d\n", __FUNCTION__,
          ret);
    }

  if (data != NULL)
    kfree(data);

  synaptics_set_drive_mode_bit(EN_TOUCH_USE_NOSLEEP_MODE);
}

static void
touchscreen_poweroff(void)
{
  unsigned long touch_handler_flag;

  if (down_interruptible(&sem_touch_onoff))
    {
      printk("[TSP][ERROR] can't get touch_onoff semaphore\n");
      return;
    }
  if (g_touch_onoff_status == 1)
    {
      local_irq_save(touch_handler_flag);
      if (g_enable_touchscreen_handler == 1)
        {
          free_irq(tsp.irq, &tsp);
          g_enable_touchscreen_handler = 0;
        }
      local_irq_restore(touch_handler_flag);
      g_touch_onoff_status = 0;
      gpio_set_value(GPIO_TOUCH_LDO_EN, GPIO_LEVEL_LOW);
      msleep(10);
      dprintk(DCM_INP, "[TSP] touchscreen power off !\n");
    }
  else
    {
      dprintk(DCM_INP, "[TSP] touchscreen power off but invalid flag!\n");
    }
  up(&sem_touch_onoff);

  flush_workqueue(tsp_wq);

  return;
}

static void
touchscreen_poweron(void)
{
  unsigned long touch_handler_flag;

  if (down_interruptible(&sem_touch_onoff))
    {
      printk("[TSP][ERROR] can't get touch_onoff semaphore\n");
      return;
    }
  if (g_touch_onoff_status == 0)
    {
      gpio_set_value(GPIO_TOUCH_LDO_EN, GPIO_LEVEL_HIGH);
      msleep(300);
      local_irq_save(touch_handler_flag);
      if (g_enable_touchscreen_handler == 0)
        {
          if (request_irq(tsp.irq, touchscreen_handler, tsp.irq_type,
              TOUCHSCREEN_NAME, &tsp))
            {
              printk("[TSP][ERROR] Could not allocate touchscreen IRQ!\n");
              input_free_device(tsp.inputdevice);
              tsp.irq = -1;
              up(&sem_touch_handler);
              up(&sem_touch_onoff);
              return;
            }
          g_enable_touchscreen_handler = 1;
        }
      local_irq_restore(touch_handler_flag);
      g_touch_onoff_status = 1;

      synaptics_touchscreen_start_triggering();
      g_sleep_onoff_status = EN_TOUCH_WAKEUP_MODE;

      dprintk(DCM_INP, "[TSP] touchscreen power on !\n");
    }
  else
    {
      dprintk(DCM_INP, "[TSP] touchscreen power on but invalid flag!\n");
    }
  up(&sem_touch_onoff);

  return;
}

static int
synaptics_set_drive_mode_bit(enum EnTouchDriveStatus drive_status)
{
  int data = 0, ret = 0;

  ret = i2c_tsp_sensor_read(g_synaptics_device_control_addr, (u8*) &data, 1);
  if (ret != 0)
    {
      printk("[TSP][ERROR] %s() i2c_tsp_sensor_read error : %d\n", __FUNCTION__,
          ret);
      return -1;
    }

  dprintk(DCM_INP, "%s() data before = 0x%x\n", __FUNCTION__, data);

  if (drive_status == EN_TOUCH_USE_DOZE_MODE)
    {
      data &= 0xfb;
    }
  else if (drive_status == EN_TOUCH_USE_NOSLEEP_MODE)
    {
      data |= 0x04;
    }

  dprintk(DCM_INP, "%s() data after = 0x%x\n", __FUNCTION__, data);

  if (g_synaptics_device_control_addr != 0)
    {
      if ((ret = i2c_tsp_sensor_write_reg(g_synaptics_device_control_addr, data))
          != 0)
        {
          printk("[TSP][ERROR] %s() i2c_tsp_sensor_write_reg error : %d\n",
              __FUNCTION__, ret);
          return -1;
        }
    }
  else
    {
      printk(
          "[TSP][ERROR] %s() g_synaptics_device_control_addr is not setted\n",
          __FUNCTION__);
      return -1;
    }
  return 0;
}

static int
synaptics_write_sleep_bit(enum EnTouchSleepStatus sleep_status)
{
  int data = 0, ret = 0;

  ret = i2c_tsp_sensor_read(g_synaptics_device_control_addr, (u8*) &data, 1);
  if (ret != 0)
    {
      printk("[TSP][ERROR] %s() i2c_tsp_sensor_read error : %d\n", __FUNCTION__,
          ret);
      return -1;
    }

  dprintk(DCM_INP, "%s() data before = 0x%x\n", __FUNCTION__, data);

  if (sleep_status == EN_TOUCH_SLEEP_MODE)
    {
      data &= 0xf8;
      data |= 0x01;
    }
  else if (sleep_status == EN_TOUCH_WAKEUP_MODE)
    {
      data &= 0xfc;
      data |= 0x04;
    }

  dprintk(DCM_INP, "%s() data after = 0x%x\n", __FUNCTION__, data);

  if (g_synaptics_device_control_addr != 0)
    {
      if ((ret = i2c_tsp_sensor_write_reg(g_synaptics_device_control_addr, data))
          != 0)
        {
          printk("[TSP][ERROR] %s() i2c_tsp_sensor_write_reg error : %d\n",
              __FUNCTION__, ret);
          return -1;
        }
    }
  else
    {
      printk(
          "[TSP][ERROR] %s() g_synaptics_device_control_addr is not setted\n",
          __FUNCTION__);
      return -1;
    }
  return 0;
}

void
touchscreen_sleep(void)
{
  unsigned long touch_handler_flag;

  if (down_interruptible(&sem_sleep_onoff))
    {
      printk("[TSP][ERROR] can't get sleep_onoff semaphore\n");
      return;
    }
  if (g_sleep_onoff_status == EN_TOUCH_WAKEUP_MODE)
    {
      local_irq_save(touch_handler_flag);
      if (g_enable_touchscreen_handler == 1)
        {
          free_irq(tsp.irq, &tsp);
          g_enable_touchscreen_handler = 0;
        }
      local_irq_restore(touch_handler_flag);
      g_sleep_onoff_status = EN_TOUCH_SLEEP_MODE;

      if (synaptics_write_sleep_bit(EN_TOUCH_SLEEP_MODE) != 0)
        {
          printk("[TSP][ERROR] touchscreen sleep fail !!!\n");
          udelay(200);
          printk("[TSP][ERROR] try touchscreen sleep one more time.\n");
          if (synaptics_write_sleep_bit(EN_TOUCH_SLEEP_MODE) != 0)
            printk("[TSP][ERROR] touchscreen sleep fail (2) !!!\n");
          else
            dprintk(DCM_INP, "[TSP] touchscreen sleep !\n");
        }
      else
        dprintk(DCM_INP, "[TSP] touchscreen sleep !\n");
    }
  else
    {
      dprintk(DCM_INP, "[TSP] touchscreen sleep but invalid flag!\n");
    }
  up(&sem_sleep_onoff);

  flush_workqueue(tsp_wq);

  return;
}

void
touchscreen_wake_up(void)
{
  unsigned long touch_handler_flag;

  if (down_interruptible(&sem_sleep_onoff))
    {
      printk("[TSP][ERROR] can't get sleep_onoff semaphore\n");
      return;
    }
  if (g_sleep_onoff_status == EN_TOUCH_SLEEP_MODE)
    {
      if (synaptics_write_sleep_bit(EN_TOUCH_WAKEUP_MODE) != 0)
        {
          printk(
              "[TSP][ERROR] touchscreen wake up fail. write sleep bit error !!!\n");
          udelay(200);
          printk("[TSP][ERROR] try touchscreen wake up one more time.\n");
          if (synaptics_write_sleep_bit(EN_TOUCH_WAKEUP_MODE) != 0)
            {
              printk(
                  "[TSP][ERROR] touchscreen wake up fail. write sleep bit error (2) !!!\n");
              up(&sem_sleep_onoff);
              return;
            }
        }
      synaptics_touchscreen_start_triggering();

      local_irq_save(touch_handler_flag);
      if (g_enable_touchscreen_handler == 0)
        {
          if (request_irq(tsp.irq, touchscreen_handler, tsp.irq_type,
              TOUCHSCREEN_NAME, &tsp))
            {
              printk("[TSP][ERROR] Could not allocate touchscreen IRQ!\n");
              input_free_device(tsp.inputdevice);
              tsp.irq = -1;
              up(&sem_touch_handler);
              up(&sem_sleep_onoff);
              return;
            }
          g_enable_touchscreen_handler = 1;
        }
      local_irq_restore(touch_handler_flag);
      g_sleep_onoff_status = EN_TOUCH_WAKEUP_MODE;

      dprintk(DCM_INP, "[TSP] touchscreen wake up !\n");
    }
  else
    {
      dprintk(DCM_INP, "[TSP] touchscreen wake up but invalid flag!\n");
    }
  up(&sem_sleep_onoff);

  return;
}

static void
synaptics_touchscreen_read(struct work_struct *work)
{
  u8 *data = NULL;
  u16 x_position = -1;
  u16 y_position = -1;
  u16 x2_position = -1;
  u16 y2_position = -1;
  int press = 0;
  int press2 = 0;
  int finger_type = 0;
  data = kmalloc(g_synaptics_read_cnt, GFP_KERNEL);
  if (data == NULL)
    {
      printk("[TSP][ERROR] %s() kmalloc fail\n", __FUNCTION__);
      return;
    }

  i2c_tsp_sensor_read(g_synaptics_read_addr, data, g_synaptics_read_cnt);

  press = (data[0x15 - g_synaptics_read_addr] & 0x03);
  x_position = data[0x16 - g_synaptics_read_addr];
  x_position = (x_position << 4) | (data[0x18 - g_synaptics_read_addr] & 0x0f);
  y_position = data[0x17 - g_synaptics_read_addr];
  y_position = (y_position << 4) | (data[0x18 - g_synaptics_read_addr] >> 4);
  press2 = ((data[0x15 - g_synaptics_read_addr] & 0x0C) >> 2);
  x2_position = data[0x1B - g_synaptics_read_addr];
  x2_position = (x2_position << 4)
      | (data[0x1D - g_synaptics_read_addr] & 0x0f);
  y2_position = data[0x1C - g_synaptics_read_addr];
  y2_position = (y2_position << 4) | (data[0x1D - g_synaptics_read_addr] >> 4);

  if ((press == 0) && (press2 == 0))
    {
      finger_type = 0;
    }
  else if ((press == 1) && (press2 == 0))
    {
      finger_type = 1;
      finger_switched = 0;
    }
  else if ((press == 0) && (press2 == 1))
    {
      finger_type = 2;
      finger_switched = 1;
    }
  else if ((press == 1) && (press2 == 1))
    {
      finger_type = 3;
    }
  else if ((press == 2) && (press2 == 2))
    {
      finger_type = 4;
    }
  else if ((press == 0) && (press2 == 2))
    {
      finger_type = 5;
      finger_switched = 1;
    }
  else if ((press == 2) && (press2 == 0))
    {
      finger_type = 6;
      finger_switched = 0;
    }
  else
    {
      finger_type = 7;
    }

  if ((finger_type > 0) && (finger_type < 7))
    {

      if (finger_type == old_finger_type)
        {
          if ((x_position == old_x_position) && (y_position == old_y_position))
            {
              if ((x2_position == old_x2_position)
                  && (y2_position == old_y2_position))
                goto finish_read;
            }
        }

      old_x_position = x_position;
      old_y_position = y_position;
      old_x2_position = x2_position;
      old_y2_position = y2_position;

      if ((finger_type == 1) || (finger_type == 2) || (finger_type == 5)
          || (finger_type == 6))
        {
          if (finger_switched == 0)
            {
              input_report_abs(tsp.inputdevice, ABS_X, x_position);
              input_report_abs(tsp.inputdevice, ABS_Y, y_position);
            }
          else
            {
              input_report_abs(tsp.inputdevice, ABS_X, x2_position);
              input_report_abs(tsp.inputdevice, ABS_Y, y2_position);
            }
          input_report_key(tsp.inputdevice, BTN_TOUCH, 1);
          input_report_abs(tsp.inputdevice, ABS_PRESSURE,
              DEFAULT_PRESSURE_DOWN);
          input_sync(tsp.inputdevice);
          if (finger_switched == 0)
            dprintk(DCM_INP,
                "[TSP][DOWN]type:%d, x=%d, y=%d\n", finger_type, x_position, y_position);
          else if (finger_switched == 1)
            dprintk(DCM_INP,
                "[TSP][DOWN]type:%d, x=%d, y=%d\n", finger_type, x2_position, y2_position);
        }
      else if ((finger_type == 3) || (finger_type == 4))
        {

          dprintk(DCM_INP,
              "[TSP][DOWN][WARNING]type:%d, Multi touch is not supported.\n", finger_type);

        }

    }

  else if (finger_type == 0)
    {
      input_report_key(tsp.inputdevice, BTN_TOUCH, 0);

      if ((old_finger_type == 1) || (old_finger_type == 2)
          || (old_finger_type == 5) || (old_finger_type == 6))
        {
          input_report_abs(tsp.inputdevice, ABS_PRESSURE, DEFAULT_PRESSURE_UP);
          input_sync(tsp.inputdevice);
          if (finger_switched == 0)
            dprintk(DCM_INP,
                "[TSP][UP]type:%d, x=%d, y=%d\n", finger_type, x_position, y_position);
          else if (finger_switched == 1)
            dprintk(DCM_INP,
                "[TSP][UP]type:%d, x=%d, y=%d\n", finger_type, x2_position, y2_position);
        }
      else if ((old_finger_type == 3) || (old_finger_type == 4))
        {
          input_report_abs(tsp.inputdevice, ABS_PRESSURE, DEFAULT_PRESSURE_UP);
          input_sync(tsp.inputdevice);
          dprintk(DCM_INP,
              "[TSP][UP][WARNING]type:%d, Multi touch is not supported. But send release event.\n", finger_type);
        }

      finger_switched = 0;
      old_x_position = -1;
      old_y_position = -1;
      old_x2_position = -1;
      old_y2_position = -1;
    }

  else
    {
      dprintk(DCM_INP, "[TSP][ERROR] Unknown finger_type : %d\n", finger_type);
      dprintk(DCM_INP, "[TSP][ERROR] press : %d, press2 : %dn", press, press2);
    }
  old_finger_type = finger_type;
  if (data != NULL)
    {
      kfree(data);
      data = NULL;
    }
  return;
  finish_read: if (data != NULL)
    {
      kfree(data);
      data = NULL;
    }
  return;

}

static ssize_t
sysfs_touchcontrol_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
  int touch_control_flag;
  sscanf(buf, "%d", &touch_control_flag);

  if (touch_control_flag == 0)
    {
      touchscreen_sleep();

    }
  else if (touch_control_flag == 1)
    {
      touchscreen_wake_up();
    }
  else if (touch_control_flag == 2)
    {
      touchscreen_poweroff();
    }
  else if (touch_control_flag == 3)
    {
      touchscreen_poweron();
    }
  else
    printk("[TSP][ERROR] wrong touch control command\n");
  return count;
}

static ssize_t
sysfs_touchcontrol_show(struct device *dev, struct device_attribute *attr,
    char *buf)
{
  int b_touch_ldo_en = 0;
  b_touch_ldo_en = gpio_get_value(GPIO_TOUCH_LDO_EN);

  printk("[TSP] touch LDO : %s\n", b_touch_ldo_en ? "HIGH" : "LOW");
  printk("[TSP] g_touch_onoff_status = %s\n",
      g_touch_onoff_status ? "ON" : "OFF");
  printk("[TSP] g_sleep_onoff_status = %s\n",
      g_sleep_onoff_status ? "ON" : "OFF");
  printk("[TSP] g_enable_touchscreen_handler = %s\n",
      g_enable_touchscreen_handler ? "ON" : "OFF");

  printk("[TSP] x: %d, y: %d, x2: %d, y2: %d, finger_switched: %d, finger_type: %d\n", old_x_position,old_y_position,old_x2_position,old_y2_position,
  finger_switched, old_finger_type);
  return 0;
}

static DEVICE_ATTR(touchcontrol, S_IRUGO | S_IWUSR, sysfs_touchcontrol_show, sysfs_touchcontrol_store);

static int
find_next_string_in_buf(const char *buf, size_t count)
{
  int ii = 0;
  for (ii = 0; ii < count; ii++)
    {
      if (buf[ii] == ' ')
        return ii;
    }
  return -1;
}

static ssize_t
sysfs_get_register_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
  u8 *pData;
  int start_address = 0, end_address = 0, ret = 0, cnt = 0;
  unsigned int read_size;

  if ((ret = sscanf(buf, "%x", &start_address)) <= 0)
    {
      printk("[TSP][ERROR] %s() input start address and then end address !!\n",
          __FUNCTION__);
      return count;
    }

  cnt = find_next_string_in_buf(buf, count);
  if (cnt > 0)
    {
      ret = sscanf(buf + cnt, "%x", &end_address);
      if (ret <= 0 || end_address == 0 || end_address < start_address)
        {
          read_size = 1;
        }
      else
        {
          read_size = end_address - start_address + 1;
        }
    }
  else
    {
      read_size = 1;
    }

  pData = kmalloc(read_size, GFP_KERNEL);
  ret = i2c_tsp_sensor_read((u8) start_address, pData, read_size);
  if (ret != 0)
    {
      printk("[TSP][ERROR] %s() i2c_tsp_sensor_read error : %d\n", __FUNCTION__,
          ret);
      return count;
    }
  if (read_size == 1)
    printk("[TSP] address : 0x%x, value: 0x%x\n", start_address, pData[0]);
  else
    {
      printk(
          "[TSP] read touch ic register - start address : 0x%x -- end address : 0x%x\n",
          start_address, end_address);
      printk(
          "======================================================================\n");
      for (cnt = 0; cnt < read_size; cnt++)
        {
          printk("0x%02x ", pData[cnt]);
          if ((cnt + 1) % 10 == 0 && cnt != 0)
            printk("\n");
        }
      printk(
          "\n======================================================================\n");
    }
  kfree(pData);
  return count;
}

static ssize_t
sysfs_get_register_show(struct device *dev, struct device_attribute *attr,
    char *buf)
{
  printk("[TSP] synaptics version address : 0x%x\n", g_version_read_addr);
  printk("[TSP] synaptics read address : 0x%x\n", g_synaptics_read_addr);
  printk("[TSP] USAGE : echo [start address] {end address} > get_register\n");
  return 0;
}

static DEVICE_ATTR(get_register, S_IRUGO | S_IWUSR, sysfs_get_register_show, sysfs_get_register_store);

static ssize_t
sysfs_set_register_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
  int address = 0;
  int data = 0, ret = 0, cnt = 0;

  ret = sscanf(buf, "%x", &address);
  if (ret <= 0)
    {
      printk("[TSP][ERROR] %s() input start address and then end address !!\n",
          __FUNCTION__);
      return count;
    }

  cnt = find_next_string_in_buf(buf, count);
  if (cnt < 0)
    {
      printk("[TSP][ERROR] %s() input start address and then end address !!\n",
          __FUNCTION__);
      return count;
    }

  ret = sscanf(buf + cnt, "%x", &data);
  if (ret <= 0)
    {
      printk("[TSP][ERROR] %s() input start address and then end address !!\n",
          __FUNCTION__);
      return count;
    }
  printk("[TSP] try to write : address 0x%x, write value : 0x%x\n", address,
      data);

  ret = i2c_tsp_sensor_write_reg((u8) address, data);
  if (ret != 0)
    {
      printk("[TSP][ERROR] %s() i2c_tsp_sensor_write_reg error : %d\n",
          __FUNCTION__, ret);
      return count;
    }

  data = 0;

  ret = i2c_tsp_sensor_read((u8) address, (u8*) &data, 1);
  if (ret != 0)
    {
      printk("[TSP][ERROR] %s() i2c_tsp_sensor_read error : %d\n", __FUNCTION__,
          ret);
      return count;
    }

  printk("[TSP] read again : address 0x%x, write value : 0x%x\n", address,
      data);

  return count;
}

static ssize_t
sysfs_set_register_show(struct device *dev, struct device_attribute *attr,
    char *buf)
{
  printk("[TSP] USAGE : echo [address] [value] > set_register\n");
  return 0;
}

static DEVICE_ATTR(set_register, S_IRUGO | S_IWUSR, sysfs_set_register_show, sysfs_set_register_store);

static irqreturn_t
touchscreen_handler(int irq, void *dev_id)
{

  if (g_enable_touchscreen_handler == 0)
    return IRQ_HANDLED;

#ifdef CONFIG_CPU_FREQ

  set_dvfs_perf_level(1);
#endif		

  queue_work(tsp_wq, &tsp_work);

  return IRQ_HANDLED;

}

static void
touchscreen_hw_init()
{
  s3c_gpio_cfgpin(GPIO_TOUCH_LDO_EN, S3C_GPIO_OUTPUT);
  s3c_gpio_setpull(GPIO_TOUCH_LDO_EN, S3C_GPIO_PULL_NONE);

  s3c_gpio_setpull(GPIO_TOUCH_SCL, S3C_GPIO_PULL_UP);
  s3c_gpio_setpull(GPIO_TOUCH_SDA, S3C_GPIO_PULL_UP);

  s3c_gpio_cfgpin(GPIO_TOUCH_IRQ, S3C_GPIO_SFN(GPIO_TOUCH_IRQ_AF));
  s3c_gpio_setpull(GPIO_TOUCH_IRQ, S3C_GPIO_PULL_UP);
  set_irq_type(TOUCH_IRQ_EINT, IRQ_TYPE_EDGE_FALLING);
  printk("[TSP] irq %d\n",TOUCH_IRQ_EINT);
  udelay(10);
}

static int __init
touchscreen_probe(struct platform_device *pdev)
{
  int ret;

  struct touch_setup_data setup;

  printk("[TSP] synaptics touchscreen probe - start\n");
  touchscreen_read = synaptics_touchscreen_read;

  g_version_read_addr = SYNAPTICS_VERSION_ADDR;
  g_synaptics_read_addr = SYNAPTICS_READ_ADDR;
  g_synaptics_read_cnt = SYNAPTICS_READ_CNT;
  g_synaptics_device_control_addr = 0x23;
  g_sleep_onoff_status = EN_TOUCH_WAKEUP_MODE;

  sema_init(&sem_touch_onoff, 1);
  sema_init(&sem_touch_handler, 1);
  sema_init(&sem_sleep_onoff, 1);

  touchscreen_hw_init();
  gpio_set_value(GPIO_TOUCH_LDO_EN, GPIO_LEVEL_HIGH);
  g_touch_onoff_status = 1;
  msleep(300);

  memset(&tsp, 0, sizeof(tsp));

  tsp.inputdevice = input_allocate_device();

  if (!tsp.inputdevice)
    {
      printk("[TSP][ERROR] input_allocate_device fail \n");
      return -ENOMEM;
    }

  tsp.irq = TOUCH_IRQ_EINT;
  tsp.irq_type = IRQF_DISABLED;
  tsp.irq_enabled = 1;

  tsp.inputdevice->name = TOUCHSCREEN_NAME;
/*  tsp.inputdevice->evbit[0] = BIT(EV_ABS);
  tsp.inputdevice->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
*/

  tsp.inputdevice->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
  tsp.inputdevice->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
  tsp.inputdevice->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

  input_set_abs_params(tsp.inputdevice, ABS_X, 0, MAX_TOUCH_X_RESOLUTION, 0, 0);
  input_set_abs_params(tsp.inputdevice, ABS_Y, 0, MAX_TOUCH_Y_RESOLUTION, 0, 0);
  input_set_abs_params(tsp.inputdevice, ABS_PRESSURE, 0, 256, 0, 0);

  tsp_wq = create_singlethread_workqueue("tsp_wq");
  INIT_WORK(&tsp_work, touchscreen_read);

  ret = input_register_device(tsp.inputdevice);

  setup.i2c_address = (TSP_SENSOR_ADDRESS);
  setup.i2c_bus = 4;

  ret = touch_add_i2c_device(&setup);

  if (ret != 0)
    {
      printk("[TSP][ERROR] fail touch_add_i2c_device() !\n");
    }

  if (request_irq(tsp.irq, touchscreen_handler, tsp.irq_type, TOUCHSCREEN_NAME,
      &tsp))
    {
      printk("[TSP][ERROR] Could not allocate touchscreen IRQ!\n");
      tsp.irq = -1;
      input_free_device(tsp.inputdevice);
      return -EINVAL;
    }
  g_enable_touchscreen_handler = 1;

  ret = device_create_file(&tsp.inputdevice->dev, &dev_attr_issp);
  ret = device_create_file(&tsp.inputdevice->dev, &dev_attr_touchcontrol);

  ret = device_create_file(&tsp.inputdevice->dev, &dev_attr_get_register);
  ret = device_create_file(&tsp.inputdevice->dev, &dev_attr_set_register);
  synaptics_touchscreen_start_triggering();

  printk("[TSP] success %s() !\n", __FUNCTION__);

  return 0;
}

static int
touchscreen_remove(struct platform_device *pdev)
{
  input_unregister_device(tsp.inputdevice);

  if (g_enable_touchscreen_handler == 1)
    {
      free_irq(tsp.irq, &tsp);
      g_enable_touchscreen_handler = 0;
    }
  return 0;
}

void
touchscreen_poweronoff(int onoff)
{
  dprintk(DCM_INP, "[TSP] %s(%d)!\n", __FUNCTION__, onoff);

  if (onoff)
    {
      touchscreen_wake_up();
    }
  else
    {
      touchscreen_sleep();
    }
  return;
}
EXPORT_SYMBOL(touchscreen_poweronoff);

static int
touchscreen_suspend(struct platform_device *pdev, pm_message_t state)
{

  return 0;
}

static int
touchscreen_resume(struct platform_device *pdev)
{

  return 0;
}

static void
touchscreen_device_release(struct device *dev)
{

}

static struct platform_driver touchscreen_driver =
  { .probe = touchscreen_probe, .remove = touchscreen_remove, .suspend =
      touchscreen_suspend, .resume = touchscreen_resume, .driver =
    { .name = TOUCHSCREEN_NAME, }, };

static struct platform_device touchscreen_device =
  { .name = TOUCHSCREEN_NAME, .id = -1, .dev =
    { .release = touchscreen_device_release, }, };

static int __init
touchscreen_init(void)
{
  int ret;

  ret = platform_device_register(&touchscreen_device);
  if (ret != 0)
    return -ENODEV;

  ret = platform_driver_register(&touchscreen_driver);
  if (ret != 0)
    {
      platform_device_unregister(&touchscreen_device);
      return -ENODEV;
    }

  return 0;
}

static void __exit
touchscreen_exit(void)
{
  platform_driver_unregister(&touchscreen_driver);
  platform_device_unregister(&touchscreen_device);
}

module_init(touchscreen_init);
module_exit(touchscreen_exit);

MODULE_LICENSE("GPL");
