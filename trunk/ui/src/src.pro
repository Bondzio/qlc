include(../../variables.pri)
include(../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcui

CONFIG += qt
QT     += core xml gui

# Engine
INCLUDEPATH     += ../../engine/src
DEPENDPATH      += ../../engine/src
QMAKE_LIBDIR    += ../../engine/src
LIBS            += -lqlcengine

# X
unix:!macx:LIBS += -lX11

# Types
INCLUDEPATH += ../../plugins/interfaces

# Resources
RESOURCES    += qlcui.qrc

# Sources
HEADERS += aboutbox.h \
           addfixture.h \
           addvcbuttonmatrix.h \
           addvcslidermatrix.h \
           app.h \
           apputil.h \
           assignhotkey.h \
           busmanager.h \
           chasereditor.h \
           collectioneditor.h \
           consolechannel.h \
           docbrowser.h \
           efxeditor.h \
           efxpreviewarea.h \
           fixtureconsole.h \
           fixturemanager.h \
           fixtureselection.h \
           functionmanager.h \
           functionselection.h \
           functionwizard.h \
           grandmasterslider.h \
           inputchanneleditor.h \
           inputprofileeditor.h \
           inputmanager.h \
           inputpatcheditor.h \
           monitor.h \
           monitorfixture.h \
           monitorlayout.h \
           outputmanager.h \
           outputpatcheditor.h \
           sceneeditor.h \
           selectinputchannel.h \
           vcbutton.h \
           vcbuttonproperties.h \
           vccuelist.h \
           vccuelistproperties.h \
           vcdockarea.h \
           vcdockslider.h \
           vcframe.h \
           vcframeproperties.h \
           vclabel.h \
           vcproperties.h \
           vcpropertieseditor.h \
           vcslider.h \
           vcsliderproperties.h \
           vcsoloframe.h \
           vcwidget.h \
           vcwidgetproperties.h \
           vcxypad.h \
           vcxypadarea.h \
           vcxypadfixture.h \
           vcxypadfixtureeditor.h \
           vcxypadproperties.h \
           virtualconsole.h

FORMS += aboutbox.ui \
         addfixture.ui \
         addvcbuttonmatrix.ui \
         addvcslidermatrix.ui \
         assignhotkey.ui \
         chasereditor.ui \
         collectioneditor.ui \
         efxeditor.ui \
         fixtureselection.ui \
         functionselection.ui \
         functionwizard.ui \
         inputchanneleditor.ui \
         inputprofileeditor.ui \
         inputpatcheditor.ui \
         outputpatcheditor.ui \
         sceneeditor.ui \
         selectinputchannel.ui \
         vcbuttonproperties.ui \
         vccuelistproperties.ui \
         vcframeproperties.ui \
         vcproperties.ui \
         vcsliderproperties.ui \
         vcxypadfixtureeditor.ui \
         vcxypadproperties.ui

SOURCES += aboutbox.cpp \
           addfixture.cpp \
           addvcbuttonmatrix.cpp \
           addvcslidermatrix.cpp \
           app.cpp \
           apputil.cpp \
           assignhotkey.cpp \
           busmanager.cpp \
           chasereditor.cpp \
           collectioneditor.cpp \
           consolechannel.cpp \
           docbrowser.cpp \
           efxeditor.cpp \
           efxpreviewarea.cpp \
           fixtureconsole.cpp \
           fixturemanager.cpp \
           fixtureselection.cpp \
           functionmanager.cpp \
           functionselection.cpp \
           functionwizard.cpp \
           grandmasterslider.cpp \
           inputchanneleditor.cpp \
           inputprofileeditor.cpp \
           inputmanager.cpp \
           inputpatcheditor.cpp \
           monitor.cpp \
           monitorfixture.cpp \
           monitorlayout.cpp \
           outputmanager.cpp \
           outputpatcheditor.cpp \
           sceneeditor.cpp \
           selectinputchannel.cpp \
           vcbutton.cpp \
           vcbuttonproperties.cpp \
           vccuelist.cpp \
           vccuelistproperties.cpp \
           vcdockarea.cpp \
           vcdockslider.cpp \
           vcframe.cpp \
           vcframeproperties.cpp \
           vclabel.cpp \
           vcproperties.cpp \
           vcpropertieseditor.cpp \
           vcslider.cpp \
           vcsliderproperties.cpp \
           vcsoloframe.cpp \
           vcwidget.cpp \
           vcwidgetproperties.cpp \
           vcxypad.cpp \
           vcxypadarea.cpp \
           vcxypadfixture.cpp \
           vcxypadfixtureeditor.cpp \
           vcxypadproperties.cpp \
           virtualconsole.cpp

# Internationalization
PRO_FILE      = src.pro
TRANSLATIONS += qlc_fi_FI.ts
TRANSLATIONS += qlc_fr_FR.ts
TRANSLATIONS += qlc_es_ES.ts
TRANSLATIONS += qlc_de_DE.ts
TRANSLATIONS += qlc_it_IT.ts
include(../../i18n.pri)

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target
