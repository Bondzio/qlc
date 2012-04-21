include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoniout

QT          += core gui
CONFIG      += plugin
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb
INCLUDEPATH += ../../interfaces

HEADERS += peperonidevice.h \
           peperoniout.h

SOURCES += peperonidevice.cpp \
           peperoniout.cpp

HEADERS += ../../interfaces/qlcioplugin.h

TRANSLATIONS += Peperoni_Output_fi_FI.ts
TRANSLATIONS += Peperoni_Output_de_DE.ts
TRANSLATIONS += Peperoni_Output_es_ES.ts
TRANSLATIONS += Peperoni_Output_fr_FR.ts
TRANSLATIONS += Peperoni_Output_it_IT.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

# UDEV rule to make Peperoni USB devices readable & writable for users in Linux
udev.path  = /etc/udev/rules.d
udev.files = z65-peperoni.rules
!macx:INSTALLS  += udev
