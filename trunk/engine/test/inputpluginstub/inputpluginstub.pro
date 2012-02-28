include(../../../variables.pri)
include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = inputpluginstub

QT          += xml script
CONFIG      += plugin
INCLUDEPATH += ../../../plugins/interfaces
DEPENDPATH  += ../../../plugins/interfaces

HEADERS += inputpluginstub.h \
           ../../../plugins/interfaces/qlcinplugin.h
SOURCES += inputpluginstub.cpp
