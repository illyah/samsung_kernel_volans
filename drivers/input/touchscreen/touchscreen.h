#ifndef __TOUCHSCREEN_H__
#define __TOUCHSCREEN_H__

#include <mach/volans.h>
#include "../../../kernel/kheart-beat/khb_main.h"

#define TSP_SENSOR_ADDRESS		0x20

struct touchscreen_t {
	struct input_dev * inputdevice;
	struct timer_list ts_timer;
	int touched;
	int irq;
	int irq_type;
	int irq_enabled;
	struct ts_device *dev;
	spinlock_t lock;
};


struct touch_setup_data {
	int            i2c_bus;
	unsigned short i2c_address;
};

#endif	//__TOUCHSCREEN_H__

