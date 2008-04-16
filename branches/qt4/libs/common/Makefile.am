lib_LTLIBRARIES = libqlccommon.la

libqlccommon_la_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
libqlccommon_la_CXXFLAGS += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
libqlccommon_la_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
libqlccommon_la_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
libqlccommon_la_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"
libqlccommon_la_CXXFLAGS += -DVERSION=\"$(VERSION)\"

libqlccommon_la_LIBS  = @QT_LIBS@

# Legacy classes
# capability.h
# configitem.h
# deviceclass.h
# ledbar.h
# logicalchannel.h
# qlcimagepreview.h
# settings.h
# qlcworkspace.h
# capability.cpp
# deviceclass.cpp
# ledbar.cpp
# logicalchannel.cpp
# qlcimagepreview.cpp
# qlcworkspace.cpp
# settings.cpp
# moc_ledbar.cpp
# moc_settings.cpp
# moc_qlcimagepreview.cpp
# moc_qlcworkspace.cpp

noinst_HEADERS = \
	qlccapability.h \
	qlcchannel.h \
	qlcdocbrowser.h \
	qlcfile.h \
	qlcfixturedef.h \
	qlcfixturemode.h \
	qlcinplugin.h \
	qlcoutplugin.h \
	qlcphysical.h \
	qlcplugin.h \
	qlctypes.h

libqlccommon_la_SOURCES = \
	qlccapability.cpp \
	qlcchannel.cpp \
	qlcdocbrowser.cpp \
	qlcfile.cpp \
	qlcfixturedef.cpp \
	qlcfixturemode.cpp \
	qlcinplugin.cpp \
	qlcoutplugin.cpp \
	qlcphysical.cpp \
	qlcplugin.cpp \
	\
	moc_qlcdocbrowser.cpp \
	moc_qlcinplugin.cpp \
	moc_qlcoutplugin.cpp \
	moc_qlcplugin.cpp

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
