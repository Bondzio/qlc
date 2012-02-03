include(../../variables.pri)
include(../platform.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = test

INCLUDEPATH += ../src
LIBS        += -L../src -lhotplugmonitor

CONFIG(udev) {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
}

CONFIG(iokit) {
    LIBS      += -framework IOKit -framework CoreFoundation
}

SOURCES += main.cpp hpmtest.cpp
HEADERS += hpmtest.h
