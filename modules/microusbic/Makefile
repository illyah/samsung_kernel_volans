ifneq ($(KERNELRELEASE), )

MOD_NAME = microusbic
obj-m := $(MOD_NAME).o

EXTRA_CFLAGS := -I$(PRJROOT)/modules/include

$(MOD_NAME)-y := freescale_drv.o \
				 freescale_i2c.o \

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
