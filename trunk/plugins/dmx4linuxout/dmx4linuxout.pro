include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = dmx4linuxout

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target

TRANSLATIONS += DMX4Linux_Output_fi_FI.ts
TRANSLATIONS += DMX4Linux_Output_de_DE.ts
TRANSLATIONS += DMX4Linux_Output_es_ES.ts
TRANSLATIONS += DMX4Linux_Output_fr_FR.ts
TRANSLATIONS += DMX4Linux_Output_it_IT.ts

HEADERS += dmx4linuxout.h
SOURCES += dmx4linuxout.cpp
HEADERS += ../interfaces/qlcoutplugin.h
