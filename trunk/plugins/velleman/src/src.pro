include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = velleman
CONFIG  += plugin
win32:DEFINES += QLC_EXPORT

INCLUDEPATH += ../../interfaces

win32: {
    # K8062D is a proprietary interface by Velleman and would therefore taint the
    # 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
    # the package from http://www.box.net/shared/2l0b2tk8e1 and
    # extract its contents under K8062DDIR below to compile this plugin.
    K8062DDIR    = C:/K8062D
    LIBS        += -L$$K8062DDIR -lK8062D
} else {
    SOURCES += velleman_mock.cpp
}

HEADERS += velleman.h
SOURCES += velleman.cpp
HEADERS += ../../interfaces/qlcioplugin.h

TRANSLATIONS += Velleman_fi_FI.ts
TRANSLATIONS += Velleman_de_DE.ts
TRANSLATIONS += Velleman_es_ES.ts
TRANSLATIONS += Velleman_fr_FR.ts
TRANSLATIONS += Velleman_it_IT.ts

# Installation only on Windows; Unix targets are built only for unit testing.
target.path = $$INSTALLROOT/$$PLUGINDIR
win32:INSTALLS   += target
