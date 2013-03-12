ifeq ($(DEBUG),y)
 DEBFLAGS = -O -g -DSNULL_DEBUG
else
 DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I..
ifneq ($(KERNELRELEASE),)
obj-m	:= snull.o
else
KERNELDIR ?= /work/kernel/linux-2.6.35.13
PWD		  := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -rf *.o *~core .depend .*.cmd *.ko *.mod.c .tmp_versions

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
