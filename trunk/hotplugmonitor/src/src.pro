include(../../variables.pri)
include(../platform.pri)
TEMPLATE = lib
LANGUAGE = C++
TARGET   = hotplugmonitor
CONFIG  += staticlib

CONFIG(udev) {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
    SOURCES   += hotplugmonitor-udev.cpp
}

CONFIG(iokit) {
    SOURCES += hotplugmonitor-iokit.cpp
}

HEADERS += hotplugmonitor.h
