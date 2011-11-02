include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = olaout

QT       += core gui
CONFIG   += plugin
QTPLUGIN  =

INCLUDEPATH += ../interfaces

macx: {
    #CONFIG    += link_pkgconfig
    #PKGCONFIG += libola libolaserver
    INCLUDEPATH += /opt/local/include
    LIBS      += -L/opt/local/lib -lolaserver -lola -lolacommon -lprotobuf
} else {
    LIBS      += -L/usr/local/lib -lolaserver -lola -lolacommon -lprotobuf
}

# Forms
FORMS += configureolaout.ui

# Headers
HEADERS += olaout.h \
           olaoutthread.h \
           configureolaout.h \
           qlclogdestination.h

# Source
SOURCES += olaout.cpp \
           olaoutthread.cpp \
           configureolaout.cpp \
           qlclogdestination.cpp

HEADERS += ../interfaces/qlcoutplugin.h

TRANSLATIONS += OLA_Output_fi_FI.ts
TRANSLATIONS += OLA_Output_de_DE.ts
TRANSLATIONS += OLA_Output_es_ES.ts
TRANSLATIONS += OLA_Output_fr_FR.ts
TRANSLATIONS += OLA_Output_it_IT.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
