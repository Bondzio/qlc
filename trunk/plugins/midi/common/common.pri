INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../common
DEPENDPATH  += ../common
HEADERS     += ../../interfaces/qlcoutplugin.h

HEADERS += ../common/mididevice.h \
           ../common/midiinputdevice.h \
           ../common/midioutputdevice.h \
           ../common/midiplugin.h \
           ../common/midiprotocol.h \
           ../common/midienumerator.h

SOURCES += ../common/mididevice.cpp \
           ../common/midiinputdevice.cpp \
           ../common/midioutputdevice.cpp \
           ../common/midiplugin.cpp \
           ../common/midiprotocol.cpp

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
