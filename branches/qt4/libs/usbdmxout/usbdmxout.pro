TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= usbdmx
INCLUDEPATH	+= . ../

target.path	= /usr/lib/qlc
INSTALLS	+= target

# Sources
HEADERS += configureusbdmxout.h \
	   usbdmxout.h

FORMS += configureusbdmxout.ui

SOURCES += configureusbdmxout.cpp \
	   usbdmxout.cpp
