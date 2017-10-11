ifneq ($(KERNELRELEASE),)

obj-m := hello.o

else

KDIR := /lib/modules/4.10.0-32-generic/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.order *.symvers
endif

