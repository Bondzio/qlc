include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiplugin

CONFIG      += plugin
INCLUDEPATH += ../common
LIBS        += -framework CoreMIDI -framework CoreFoundation

include(../common/common.pri)

HEADERS += coremidiinputdevice.h \
           coremidioutputdevice.h

SOURCES += coremidiinputdevice.cpp \
           coremidioutputdevice.cpp \
           coremidienumerator.cpp

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
