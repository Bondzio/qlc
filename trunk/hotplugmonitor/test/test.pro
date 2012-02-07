include(../../variables.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = test

INCLUDEPATH += ../src
LIBS        += -L../src -lhotplugmonitor

SOURCES += main.cpp hpmtest.cpp
HEADERS += hpmtest.h
