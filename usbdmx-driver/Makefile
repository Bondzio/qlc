#
# Makefile for the USBDMX-driver for Peperoni's USBDMX interfaces
#
# in case of trouble, please contact driver@lighting-solutions.de
#

# path to kernel sources
KERNEL_PATH = /lib/modules/`uname -r`/build

# path to udev rules
UDEV_PATH = /etc/udev/rules.d

CC	= gcc
KCFLAGS	= -I$(KERNEL_PATH)/include \
		-O2 -Wall -Wstrict-prototypes -Wno-trigraphs \
		-fno-strict-aliasing -fno-common -fomit-frame-pointer \
		-D__KERNEL__ -DMODULE -Wall -nostdinc -iwithprefix include
obj-m	+= usbdmx.o

#############################################################################
# targets

all: demo

clean:
	rm -f demo *~
	make -C $(KERNEL_PATH) M=$(PWD) clean

usbdmx.o: usbdmx.c usbdmx.h
	$(CC) $(KCFLAGS) -c -o $*.o $<

modules:
	make -C $(KERNEL_PATH) M=$(PWD) modules

install:
	install -o root -g root -D usbdmx.ko $(DESTDIR)$(KERNEL_PATH)/kernel/driver/misc/usbdmx.ko
	install -o root -g root -D z65_usbdmx.rules $(DESTDIR)$(UDEV_PATH)/z65_usbdmx.rules

