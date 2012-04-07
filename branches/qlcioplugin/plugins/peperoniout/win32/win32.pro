include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoniout

INCLUDEPATH += peperoni
INCLUDEPATH += ../../interfaces
DEPENDPATH  += peperoni
CONFIG      += plugin

# Headers
HEADERS += peperoni/usbdmx-dynamic.h \
           peperoniout.h \
           peperonidevice.h

# Sources
SOURCES += peperoni/usbdmx-dynamic.cpp \
           peperoniout.cpp \
           peperonidevice.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

TRANSLATIONS += Peperoni_Output_fi_FI.ts
TRANSLATIONS += Peperoni_Output_de_DE.ts
TRANSLATIONS += Peperoni_Output_es_ES.ts
TRANSLATIONS += Peperoni_Output_fr_FR.ts
TRANSLATIONS += Peperoni_Output_it_IT.ts

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
