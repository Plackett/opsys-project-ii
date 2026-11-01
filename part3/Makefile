# TODO: Make makefile for this part

obj-m += elevator.o
ccflags-y := -g -Wno-error
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	
elevator.o: src/elevator.c
	gcc -c build/elevator.c

clean: 
	$(MAKE) -C $(KDIR) M=$(PWD) clean
