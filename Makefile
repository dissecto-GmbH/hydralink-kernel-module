
obj-m += hydralink.o

CFLAGS = -Wall #-Werror
ccflags-y += $(CFLAGS)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: all
	sudo insmod hydralink.ko