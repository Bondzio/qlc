TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= dmx4linuxout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

target.path	= /usr/lib/qlc/output
INSTALLS	+= target

# Sources
HEADERS += configuredmx4linuxout.h \
	   dmx4linuxout.h

FORMS += configuredmx4linuxout.ui

SOURCES += configuredmx4linuxout.cpp \
	   dmx4linuxout.cpp
