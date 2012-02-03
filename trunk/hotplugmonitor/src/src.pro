include(../../variables.pri)
include(../platform.pri)
TEMPLATE = lib
LANGUAGE = C++
TARGET   = hotplugmonitor
CONFIG  += staticlib

CONFIG(udev) {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
    SOURCES   += hpmprivate-udev.cpp
    HEADERS   += hpmprivate-udev.h
}

CONFIG(iokit) {
    SOURCES   += hpmprivate-iokit.cpp
    HEADERS   += hpmprivate-iokit.h
}

CONFIG(win32) {
    SOURCES += hpmprivate-win32.cpp
    HEADERS += hpmprivate-win32.h
}

SOURCES += hotplugmonitor.cpp
HEADERS += hotplugmonitor.h
