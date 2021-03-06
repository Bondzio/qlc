include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = enttecwing

INCLUDEPATH   += ../../interfaces
CONFIG        += plugin
QT            += network
win32:DEFINES += QLC_EXPORT
QTPLUGIN       =

win32 {
    # Qt Libraries
    qtnetwork.path = $$INSTALLROOT/$$LIBSDIR
    CONFIG(release, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetwork4.dll
    CONFIG(debug, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetworkd4.dll
    INSTALLS    += qtnetwork
}

# Input
HEADERS += enttecwing.h \
           playbackwing.h \
           shortcutwing.h \
           programwing.h \
           wing.h

SOURCES += enttecwing.cpp \
           playbackwing.cpp \
           shortcutwing.cpp \
           programwing.cpp \
           wing.cpp

HEADERS += ../../interfaces/qlcioplugin.h

TRANSLATIONS += ENTTEC_Wing_fi_FI.ts
TRANSLATIONS += ENTTEC_Wing_de_DE.ts
TRANSLATIONS += ENTTEC_Wing_es_ES.ts
TRANSLATIONS += ENTTEC_Wing_fr_FR.ts
TRANSLATIONS += ENTTEC_Wing_it_IT.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
