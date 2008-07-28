TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= usbdmxout

INCLUDEPATH	+= . ../../libs/
CONFIG		+= plugin warn_on release build_all

target.path	= /usr/lib/qlc/output
INSTALLS	+= target

# Sources
HEADERS += configureusbdmxout.h \
	   usbdmxout.h

FORMS += configureusbdmxout.ui

SOURCES += configureusbdmxout.cpp \
	   usbdmxout.cpp
