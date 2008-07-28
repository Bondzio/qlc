TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= dmx4linuxout

INCLUDEPATH	+= . ../../libs/
CONFIG		+= qt warn_on release build_all
LIBS		+= -ldmx4l

target.path	= /usr/lib/qlc
INSTALLS	+= target

# Sources
HEADERS += configuredmx4linuxout.h \
	   dmx4linuxout.h

FORMS += configuredmx4linuxout.ui

SOURCES += configuredmx4linuxout.cpp \
	   dmx4linuxout.cpp
