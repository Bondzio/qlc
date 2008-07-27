TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= hidinput

INCLUDEPATH	+= . ../../libs/
CONFIG		+= qt warn_on release build_all

target.path	= /usr/lib/qlc
INSTALLS	+= target

# Input
HEADERS += configurehidinput.h \
           hiddevice.h \
           hideventdevice.h \
           hidinput.h \
           hidpoller.h

FORMS += configurehidinput.ui

SOURCES += configurehidinput.cpp \
           hiddevice.cpp \
           hideventdevice.cpp \
           hidinput.cpp \
           hidpoller.cpp
