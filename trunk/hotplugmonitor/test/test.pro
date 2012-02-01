include(../../variables.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = test

INCLUDEPATH += ../src
LIBS        += -L../src -lhotplugmonitor
LIBS        += -framework IOKit -framework CoreFoundation

SOURCES += main.cpp
