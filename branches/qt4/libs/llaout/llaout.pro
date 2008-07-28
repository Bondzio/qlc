TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= llaout

CONFIG		+= link_pkgconfig
PKGCONFIG	+= liblla
INCLUDEPATH 	+= . ../../libs/

target.path	= /usr/lib/qlc
INSTALLS	+= target

# Source
HEADERS += configurellaout.h \
	   llaout.h

FORMS += configurellaout.ui

SOURCES += configurellaout.cpp \
	   llaout.cpp
