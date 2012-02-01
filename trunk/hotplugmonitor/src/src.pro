include(../../variables.pri)
TEMPLATE = lib
LANGUAGE = C++
TARGET   = hotplugmonitor
CONFIG  += staticlib

unix:!macx:CONFIG += udev
macx:CONFIG       += iokit

CONFIG(udev) {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
    SOURCES   += hotplugmonitor-udev.cpp
}

CONFIG(iokit) {
    LIBS    += -framework IOKit -framework CoreFoundation
    SOURCES += hotplugmonitor-iokit.cpp
}

HEADERS += hotplugmonitor.h
