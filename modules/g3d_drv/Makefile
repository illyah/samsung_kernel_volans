##############################################################################
# COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2011 ALL RIGHTS RESERVED
# AUTHOR		: 
##############################################################################
# VERSION&DATE	: Version 0.1
##############################################################################

ifneq ($(KERNELRELEASE), )

MOD_NAME = s3c-g3d
obj-m := $(MOD_NAME).o

EXTRA_CFLAGS := -I$(PRJROOT)/modules/include 

$(MOD_NAME)-y := s3c_g3d.o

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
