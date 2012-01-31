include(../variables.pri)
TEMPLATE = lib
LANGUAGE = C++
TARGET   = hotplugmonitor
CONFIG  += staticlib

unix:!macx:CONFIG += udev

CONFIG(udev) {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
    SOURCES   += hotplugmonitor-udev.cpp
}

HEADERS += hotplugmonitor.h
