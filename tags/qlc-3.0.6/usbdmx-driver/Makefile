#
# Makefile for the USBDMX-driver for Peperoni's USBDMX interfaces
#
# in case of trouble, please contact driver@lighting-solutions.de
#

# Paths
KERNEL_PATH = /lib/modules/`uname -r`/build
INSTALL_PATH = /lib/modules/`uname -r`/kernel
UDEV_PATH = /etc/udev/rules.d

#############################################################################
# Compiler flags
CC	= gcc
KCFLAGS	= -I$(KERNEL_PATH)/include \
		-O2 -Wall -Wstrict-prototypes -Wno-trigraphs \
		-fno-strict-aliasing -fno-common -fomit-frame-pointer \
		-D__KERNEL__ -DMODULE -Wall -nostdinc -iwithprefix include
obj-m	+= usbdmx.o

#############################################################################
# Targets

all: demo

clean:
	rm -f demo *~
	make -C $(KERNEL_PATH) M=$(PWD) clean

usbdmx.o: usbdmx.c usbdmx.h
	$(CC) $(KCFLAGS) -c -o $*.o $<

modules:
	make -C $(KERNEL_PATH) M=$(PWD) modules

install:
	install -o root -g root -D usbdmx.ko $(DESTDIR)$(INSTALL_PATH)/driver/misc/usbdmx.ko
	install -o root -g root -D z65_usbdmx.rules $(DESTDIR)$(UDEV_PATH)/z65_usbdmx.rules

