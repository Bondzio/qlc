include(../../../variables.pri)
include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = outputpluginstub

QT          += xml script
CONFIG      += plugin
INCLUDEPATH += ../../../plugins/interfaces
DEPENDPATH  += ../../../plugins/interfaces

HEADERS += outputpluginstub.h \
           ../../../plugins/interfaces/qlcoutplugin.h
SOURCES += outputpluginstub.cpp
