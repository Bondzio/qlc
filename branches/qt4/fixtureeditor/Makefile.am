bin_PROGRAMS = qlcfixtureeditor

qlcfixtureeditor_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
qlcfixtureeditor_CXXFLAGS += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
qlcfixtureeditor_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
qlcfixtureeditor_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
qlcfixtureeditor_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"

qlcfixtureeditor_LDADD  = @QT_LIBS@
qlcfixtureeditor_LDADD += $(top_srcdir)/libs/common/libqlccommon.la

noinst_HEADERS = \
	src/aboutbox.h \
	src/app.h \
	src/editchannel.h \
	src/editcapability.h \
	src/editmode.h \
	src/fixtureeditor.h

qlcfixtureeditor_SOURCES = \
	src/ui_editchannel.cpp \
	src/ui_editcapability.cpp \
	src/ui_editmode.cpp \
	src/ui_fixtureeditor.cpp \
	\
	src/aboutbox.cpp \
	src/app.cpp \
	src/editchannel.cpp \
	src/editcapability.cpp \
	src/editmode.cpp \
	src/fixtureeditor.cpp \
	src/main.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_app.cpp \
	src/moc_editchannel.cpp \
	src/moc_editcapability.cpp \
	src/moc_editmode.cpp \
	src/moc_fixtureeditor.cpp

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
ui_%.cpp: %.ui
	$(UIC) $< -o $@
