##############################################################################
# COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2011 ALL RIGHTS RESERVED
# AUTHOR		: KyoungHOON Kim (khoonk)
##############################################################################
# VERSION&DATE	: Version 1.00	2006/06/02 (khoonk)
##############################################################################

ifeq ($(CONFIG_X86_32),y)
obj-m := vtouch.o vkeypad.o

else
ifneq ($(KERNELRELEASE), )

MOD_NAME = dcm
obj-m := $(MOD_NAME).o

EXTRA_CFLAGS := -I$(PRJROOT)/modules/include	\
				-I../../../android-2.6.28/drivers/txsr/Inc	

ifeq ("$(BOARD_NAME)","volans")
$(MOD_NAME)-y := dcm_module.o \
				dcm_usb.o \
				dcm_parameter.o \
				dcm_led.o \
#				dcm_earphone.o \
#				dcm_keypad.o \
#				dcm_touchscreen.o \
#				dcm_thread.o \

endif

else

KDIR := ../../../android-2.6.28

all:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.o *.ko *.mod.c *~ .*.cmd

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install

endif
endif
