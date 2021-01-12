obj-m := heartmonkmod.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) INSTALL_MOD_DIR=kernel/drivers/misc -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o .depend .*.cmd *.ko *.mod.c *- core
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
