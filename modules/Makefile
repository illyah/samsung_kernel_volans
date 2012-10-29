export BOARD_NAME := volans
export PRJROOT := /home/user/v/
export CROSS_COMPILE := arm-none-linux-gnueabi-

ifndef PRJROOT
    $(error Cannot locate project environment)
endif

.PHONY: clean install

subdir-y += rb_dump
subdir-y += vtmdrv
subdir-y += fm_si4709
#subdir-y += dcm
subdir-y += kxsd9_accelerometer 
subdir-y += g3d_drv 
subdir-y += microusbic 
subdir-y += kxsd9

SUBDIRS := $(subdir-y)

DO_INSTALL_MODS = y

all: $(SUBDIRS)

$(SUBDIRS)::
	@$(MAKE) -C $@

install: $(SUBDIRS)
ifeq ($(strip $(DO_INSTALL_MODS)),y)
	@for i in `echo $(SUBDIRS)`; do \
		$(MAKE) --no-print-directory -C $$i $@ > /dev/null 2>&1; \
	done
endif
	@$(MAKE) --no-print-directory -C $(KDIR) modules_install

clean:
	@for i in `echo $(SUBDIRS)`; do \
		$(MAKE) -C $$i $@; \
	done
