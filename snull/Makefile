ifeq($(DEBUG),y)
 DEBFLAGS = -O -g -DSNULL_DEBUG
else
 DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I$(LDDINC)
ifneq($(KERNELRELEASE),)
obj-m	:= snull.o
else
KERNELDIR ?= /wor/kernel/linux-2.6.35.13
PWD		  := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) module
endif

clean rm -rf *.o *~core .depend .*.cmd *.ko *.mod.c .tmp_versions

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend
