obj-m += mod1.o mod2.o  mod3.o mod4.o mod5.o

KDIR = /lib/modules/$(shell uname -r)/build

all: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	 
clean: 
	rm -rf *.o *.ko *.mod.* .c* .t* .mod*.*o* *.symvers *.order