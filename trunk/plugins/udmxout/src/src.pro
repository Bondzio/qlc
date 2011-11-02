include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = udmxout

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
unix:CONFIG      += link_pkgconfig
unix:PKGCONFIG   += libusb

HEADERS += udmxdevice.h \
           udmxout.h

SOURCES += udmxdevice.cpp \
           udmxout.cpp

win32 {
    HEADERS += libusb_dyn.h
    SOURCES += libusb_dyn.c
}

HEADERS += ../../interfaces/qlcoutplugin.h

TRANSLATIONS += uDMX_Output_fi_FI.ts
TRANSLATIONS += uDMX_Output_de_DE.ts
TRANSLATIONS += uDMX_Output_es_ES.ts
TRANSLATIONS += uDMX_Output_fr_FR.ts
TRANSLATIONS += uDMX_Output_it_IT.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target

# UDEV rule to make uDMX USB device readable & writable for users in Linux
unix:!macx {
    udev.path  = /etc/udev/rules.d
    udev.files = z65-anyma-udmx.rules
    INSTALLS  += udev
}
