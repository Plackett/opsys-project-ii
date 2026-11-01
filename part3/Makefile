# TODO: Make makefile for this part

obj-m += src/elevator.o
ccflags-y := -g -Wno-error
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean: 
	$(MAKE) -C $(KDIR) M=$(PWD) clean
