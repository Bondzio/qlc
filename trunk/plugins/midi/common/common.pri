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
