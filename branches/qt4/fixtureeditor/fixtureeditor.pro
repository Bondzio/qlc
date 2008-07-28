TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG 		+= qt warn_on
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
win32:LIBS	+= -L../libs/common/release -lqlccommon
unix:LIBS	+= -L../libs/common -lqlccommon

unix:target.path = /usr/bin
INSTALLS 	+= target

# Sources
RESOURCES += fixtureeditor.qrc

HEADERS += aboutbox.h \
           app.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += editcapability.ui \
	 editchannel.ui \
	 editmode.ui \
	 fixtureeditor.ui

SOURCES += aboutbox.cpp \
           app.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp
