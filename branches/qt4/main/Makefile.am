bin_PROGRAMS = qlc

qlc_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
qlc_CXXFLAGS +=	-DFIXTURES=\"$(datadir)/$(FIXTURES)\"
qlc_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
qlc_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
qlc_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"
qlc_CXXFLAGS += -DVERSION=\"$(VERSION)\"

qlc_LDADD  = @QT_LIBS@
qlc_LDADD += $(top_srcdir)/libs/common/libqlccommon.la

UICS = \
	src/ui_addfixture.cpp \
	src/ui_assignhotkey.cpp \
	src/ui_busproperties.cpp \
	src/ui_chasereditor.cpp \
	src/ui_consolechannel.cpp \
	src/ui_dmxaddresstool.cpp \
	src/ui_dmxmapeditor.cpp \
	src/ui_dmxpatcheditor.cpp \
	src/ui_efxeditor.cpp \
	src/ui_fixturelist.cpp \
	src/ui_fixtureproperties.cpp \
	src/ui_functioncollectioneditor.cpp \
	src/ui_functionselection.cpp \
	src/ui_sceneeditor.cpp \
	src/ui_vcbuttonproperties.cpp \
	src/ui_vcdockslider.cpp \
	src/ui_vcframeproperties.cpp \
	src/ui_vcsliderproperties.cpp \
	src/ui_vcxypadproperties.cpp \
	src/ui_virtualconsoleproperties.cpp

MOCS = \
	src/moc_aboutbox.cpp \
	src/moc_addfixture.cpp \
	src/moc_app.cpp \
	src/moc_assignhotkey.cpp \
	src/moc_bus.cpp \
	src/moc_busproperties.cpp \
	src/moc_chaser.cpp \
	src/moc_chasereditor.cpp \
	src/moc_consolechannel.cpp \
	src/moc_dmxaddresstool.cpp \
	src/moc_dmxmap.cpp \
	src/moc_dmxmapeditor.cpp \
	src/moc_dmxpatcheditor.cpp \
	src/moc_doc.cpp \
	src/moc_dummyinplugin.cpp \
	src/moc_dummyoutplugin.cpp \
	src/moc_efx.cpp \
	src/moc_efxeditor.cpp \
	src/moc_eventbuffer.cpp \
	src/moc_fixture.cpp \
	src/moc_fixtureconsole.cpp \
	src/moc_fixturelist.cpp \
	src/moc_fixturemanager.cpp \
	src/moc_fixtureproperties.cpp \
	src/moc_functioncollection.cpp \
	src/moc_functioncollectioneditor.cpp \
	src/moc_functionconsumer.cpp \
	src/moc_function.cpp \
	src/moc_functionmanager.cpp \
	src/moc_functionselection.cpp \
	src/moc_generatorarea.cpp \
	src/moc_inputmap.cpp \
	src/moc_keybind.cpp \
	src/moc_monitor.cpp \
	src/moc_pluginmanager.cpp \
	src/moc_scene.cpp \
	src/moc_sceneeditor.cpp \
	src/moc_sliderkeybind.cpp \
	src/moc_vcbutton.cpp \
	src/moc_vcbuttonproperties.cpp \
	src/moc_vcdockarea.cpp \
	src/moc_vcdockslider.cpp \
	src/moc_vcframe.cpp \
	src/moc_vcframeproperties.cpp \
	src/moc_vclabel.cpp \
	src/moc_vcslider.cpp \
	src/moc_vcsliderproperties.cpp \
	src/moc_vcwidget.cpp \
	src/moc_vcxypad.cpp \
	src/moc_vcxypadproperties.cpp \
	src/moc_virtualconsole.cpp \
	src/moc_virtualconsoleproperties.cpp \
	src/moc_xychannelunit.cpp

qlc_SOURCES = \
	$(UICS) \
	\
	src/aboutbox.cpp \
	src/addfixture.cpp \
	src/app.cpp \
	src/assignhotkey.cpp \
	src/bus.cpp \
	src/busproperties.cpp \
	src/chaser.cpp \
	src/chasereditor.cpp \
	src/consolechannel.cpp \
	src/dmxaddresstool.cpp \
	src/dmxmap.cpp \
	src/dmxmapeditor.cpp \
	src/dmxpatcheditor.cpp \
	src/doc.cpp \
	src/dummyinplugin.cpp \
	src/dummyoutplugin.cpp \
	src/efx.cpp \
	src/efxeditor.cpp \
	src/eventbuffer.cpp \
	src/fixture.cpp \
	src/fixtureconsole.cpp \
	src/fixturelist.cpp \
	src/fixturemanager.cpp \
	src/fixtureproperties.cpp \
	src/functioncollection.cpp \
	src/functioncollectioneditor.cpp \
	src/functionconsumer.cpp \
	src/function.cpp \
	src/functionmanager.cpp \
	src/functionselection.cpp \
	src/generatorarea.cpp \
	src/inputmap.cpp \
	src/keybind.cpp \
	src/main.cpp \
	src/monitor.cpp \
	src/pluginloader.cpp \
	src/pluginmanager.cpp \
	src/scene.cpp \
	src/sceneeditor.cpp \
	src/sliderkeybind.cpp \
	src/vcbutton.cpp \
	src/vcbuttonproperties.cpp \
	src/vcdockarea.cpp \
	src/vcdockslider.cpp \
	src/vcframe.cpp \
	src/vcframeproperties.cpp \
	src/vclabel.cpp \
	src/vcslider.cpp \
	src/vcsliderproperties.cpp \
	src/vcwidget.cpp \
	src/vcxypad.cpp \
	src/vcxypadproperties.cpp \
	src/virtualconsole.cpp \
	src/virtualconsoleproperties.cpp \
	src/xychannelunit.cpp \
	\
	$(MOCS)

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UICS
ui_%.cpp: %.ui
	$(UIC) $< -o $@
