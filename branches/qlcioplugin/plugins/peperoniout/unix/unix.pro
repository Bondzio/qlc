include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoniout

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
LIBS        += -lusb

HEADERS += peperonidevice.h \
           peperoniout.h

SOURCES += peperonidevice.cpp \
           peperoniout.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

TRANSLATIONS += Peperoni_Output_fi_FI.ts
TRANSLATIONS += Peperoni_Output_de_DE.ts
TRANSLATIONS += Peperoni_Output_es_ES.ts
TRANSLATIONS += Peperoni_Output_fr_FR.ts
TRANSLATIONS += Peperoni_Output_it_IT.ts

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target

# UDEV rule to make Peperoni USB devices readable & writable for users in Linux
udev.path  = /etc/udev/rules.d
udev.files = z65-peperoni.rules
INSTALLS  += udev
