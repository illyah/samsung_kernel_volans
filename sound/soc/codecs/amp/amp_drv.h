#ifndef __OMAP3430_AMP_HEADER__
#define __OMAP3430_AMP_HEADER__

#include "amp_i2c.h"

#define AMP_DEV_NAME "amp"

struct class_simple *amp_class;
struct class_device *amp_dev;

/* amp ioctl command label */
#define AMP_INPUT_MODE_CONTROL				0
#define AMP_SPEAKER_VOLUME_CONTROL			1
#define AMP_LEFT_HEADPHONE_VOLUME_CONTROL	2
#define AMP_RIGHT_HEADPHONE_VOLUME_CONTROL	3
#define AMP_OUTPUT_MODE_CONTROL				4
#define AMP_INPUT_GAIN_CONTROL				5
#define AMP_HEADPHONE_VOLUME_CONTROL		6

#endif
