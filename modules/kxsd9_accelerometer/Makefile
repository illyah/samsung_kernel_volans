##############################################################################
# COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2011 ALL RIGHTS RESERVED
# AUTHOR		: Varun Mahajan
##############################################################################
# VERSION&DATE	: Version 0.1
##############################################################################

ifneq ($(KERNELRELEASE), )

MOD_NAME = KXSD9_driver
obj-m := $(MOD_NAME).o

EXTRA_CFLAGS := -I$(PRJROOT)/modules/include

$(MOD_NAME)-y := KXSD9_dev.o \
				KXSD9_i2c_drv.o \
				KXSD9_main.o 
else

ifndef KDIR
	KDIR :=../../../android-2.6.28
endif 

all:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.o *.ko *.mod.c *~ .*.cmd

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install

endif
