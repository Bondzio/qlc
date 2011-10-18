/*
  Q Light Controller
  vcproperties_test.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QtTest>

#define protected public
#define private public
#include "vcwidgetproperties.h"
#include "vcproperties.h"
#include "qlcfixturedefcache.h"
#include "vcproperties_test.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#undef private
#undef protected

void VCProperties_Test::init()
{
    m_doc = new Doc(this);
}

void VCProperties_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void VCProperties_Test::initial()
{
    VCProperties p;
    QVERIFY(p.m_contents == NULL);
    QVERIFY(p.contents() == NULL);

    QCOMPARE(p.width(), 640);
    QCOMPARE(p.height(), 480);

    QCOMPARE(p.m_gridEnabled, true);
    QCOMPARE(p.m_gridX, 10);
    QCOMPARE(p.m_gridY, 10);

    QCOMPARE(p.m_keyRepeatOff, true);
    QCOMPARE(p.m_grabKeyboard, true);

    QCOMPARE(p.m_gmChannelMode, UniverseArray::GMIntensity);
    QCOMPARE(p.m_gmValueMode, UniverseArray::GMReduce);
    QCOMPARE(p.m_gmInputUniverse, InputMap::invalidUniverse());
    QCOMPARE(p.m_gmInputChannel, InputMap::invalidChannel());

    QCOMPARE(p.m_blackoutInputUniverse, InputMap::invalidUniverse());
    QCOMPARE(p.m_blackoutInputChannel, InputMap::invalidChannel());

    QCOMPARE(p.m_gmVisible, true);
}

void VCProperties_Test::copy()
{
    VCProperties p;
    p.m_contents = (VCFrame*) 0xDEADBEEF;
    p.m_width = 1;
    p.m_height = 2;
    p.m_gridEnabled = false;
    p.m_gridX = 3;
    p.m_gridY = 4;
    p.m_keyRepeatOff = false;
    p.m_grabKeyboard = false;
    p.m_gmChannelMode = UniverseArray::GMAllChannels;
    p.m_gmValueMode = UniverseArray::GMLimit;
    p.m_gmInputUniverse = 5;
    p.m_gmInputChannel = 6;

    p.m_blackoutInputUniverse = 7;
    p.m_blackoutInputChannel = 8;

    p.m_gmVisible = false;

    VCProperties p2(p);
    QCOMPARE(p2.m_contents, p.m_contents);
    QCOMPARE(p2.width(), p.width());
    QCOMPARE(p2.height(), p.height());
    QCOMPARE(p2.m_gridEnabled, p.m_gridEnabled);
    QCOMPARE(p2.m_gridX, p.m_gridX);
    QCOMPARE(p2.m_gridY, p.m_gridY);
    QCOMPARE(p2.m_keyRepeatOff, p.m_keyRepeatOff);
    QCOMPARE(p2.m_grabKeyboard, p.m_grabKeyboard);
    QCOMPARE(p2.m_gmChannelMode, p.m_gmChannelMode);
    QCOMPARE(p2.m_gmValueMode, p.m_gmValueMode);
    QCOMPARE(p2.m_gmInputUniverse, p.m_gmInputUniverse);
    QCOMPARE(p2.m_gmInputChannel, p.m_gmInputChannel);
    QCOMPARE(p2.m_blackoutInputUniverse, p.m_blackoutInputUniverse);
    QCOMPARE(p2.m_blackoutInputChannel, p.m_blackoutInputChannel);
    QCOMPARE(p2.m_gmVisible, p.m_gmVisible);

    VCProperties p3 = p;
    QCOMPARE(p3.m_contents, p.m_contents);
    QCOMPARE(p3.width(), p.width());
    QCOMPARE(p3.height(), p.height());
    QCOMPARE(p3.m_gridEnabled, p.m_gridEnabled);
    QCOMPARE(p3.m_gridX, p.m_gridX);
    QCOMPARE(p3.m_gridY, p.m_gridY);
    QCOMPARE(p3.m_keyRepeatOff, p.m_keyRepeatOff);
    QCOMPARE(p3.m_grabKeyboard, p.m_grabKeyboard);
    QCOMPARE(p3.m_gmChannelMode, p.m_gmChannelMode);
    QCOMPARE(p3.m_gmValueMode, p.m_gmValueMode);
    QCOMPARE(p3.m_gmInputUniverse, p.m_gmInputUniverse);
    QCOMPARE(p3.m_gmInputChannel, p.m_gmInputChannel);
    QCOMPARE(p3.m_blackoutInputUniverse, p.m_blackoutInputUniverse);
    QCOMPARE(p3.m_blackoutInputChannel, p.m_blackoutInputChannel);
    QCOMPARE(p3.m_gmVisible, p.m_gmVisible);
}

void VCProperties_Test::reset()
{
    QWidget w;

    VCProperties p;
    p.resetContents(&w, m_doc);
    QVERIFY(p.m_contents != NULL);
    QVERIFY(qobject_cast<VCWidget*> (p.m_contents) != NULL);

    p.resetContents(&w, m_doc);
    QVERIFY(p.m_contents != NULL);
    QVERIFY(qobject_cast<VCWidget*> (p.m_contents) != NULL);
}

void VCProperties_Test::loadXML()
{
    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("VirtualConsole");
    xmldoc.appendChild(root);

    QDomElement prop = xmldoc.createElement("Properties");
    root.appendChild(prop);

    QDomElement frame = xmldoc.createElement("Frame");
    root.appendChild(frame);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QWidget w;

    VCProperties p;
    p.resetContents(&w, m_doc);
    QVERIFY(p.loadXML(root) == true);

    root.setTagName("VirtualCosnole");
    QVERIFY(p.loadXML(root) == false);
}

void VCProperties_Test::loadPropertiesHappy()
{
    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Properties");
    xmldoc.appendChild(root);

    // Grid
    QDomElement grid = xmldoc.createElement("Grid");
    grid.setAttribute("Enabled", "True");
    grid.setAttribute("XResolution", "1");
    grid.setAttribute("YResolution", "2");
    root.appendChild(grid);

    // Keyboard
    QDomElement kb = xmldoc.createElement("Keyboard");
    kb.setAttribute("Grab", "True");
    kb.setAttribute("RepeatOff", "True");
    root.appendChild(kb);

    // Blackout
    QDomElement bo = xmldoc.createElement("Blackout");
    root.appendChild(bo);

    QDomElement boInput = xmldoc.createElement("Input");
    boInput.setAttribute("Universe", "6");
    boInput.setAttribute("Channel", "1");
    bo.appendChild(boInput);

    // Grand Master
    QDomElement gm = xmldoc.createElement("GrandMaster");
    gm.setAttribute("ChannelMode", "All");
    gm.setAttribute("ValueMode", "Limit");
    root.appendChild(gm);

    QDomElement gmInput = xmldoc.createElement("Input");
    gmInput.setAttribute("Universe", "2");
    gmInput.setAttribute("Channel", "15");
    gm.appendChild(gmInput);

    // Default Fade (left here to test sanity)
    QDomElement fade = xmldoc.createElement("DefaultSlider");
    fade.setAttribute("Role", "Fade");
    fade.setAttribute("High", "76");
    fade.setAttribute("Low", "3");
    fade.setAttribute("Visible", "True");
    root.appendChild(fade);

    QDomElement fadeInput = xmldoc.createElement("Input");
    fadeInput.setAttribute("Universe", "2");
    fadeInput.setAttribute("Channel", "24");
    fade.appendChild(fadeInput);

    // Default Hold (left here to test sanity)
    QDomElement hold = xmldoc.createElement("DefaultSlider");
    hold.setAttribute("Role", "Hold");
    hold.setAttribute("High", "12");
    hold.setAttribute("Low", "7");
    hold.setAttribute("Visible", "True");
    root.appendChild(hold);

    QDomElement holdInput = xmldoc.createElement("Input");
    holdInput.setAttribute("Universe", "4");
    holdInput.setAttribute("Channel", "42");
    hold.appendChild(holdInput);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QWidget w;

    VCProperties p;
    p.resetContents(&w, m_doc);
    QVERIFY(p.loadProperties(root) == true);
    QCOMPARE(p.isGridEnabled(), true);
    QCOMPARE(p.gridX(), 1);
    QCOMPARE(p.gridY(), 2);

    QCOMPARE(p.isGrabKeyboard(), true);
    QCOMPARE(p.isKeyRepeatOff(), true);

    QCOMPARE(p.isGMVisible(), false);

    QCOMPARE(p.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p.grandMasterInputUniverse(), quint32(2));
    QCOMPARE(p.grandMasterInputChannel(), quint32(15));

    QCOMPARE(p.blackoutInputUniverse(), quint32(6));
    QCOMPARE(p.blackoutInputChannel(), quint32(1));

    // Load with new (post 3.2.0) GM visibility tag
    gm.setAttribute("Visible", "True");
    p.resetContents(&w, m_doc);
    QVERIFY(p.loadProperties(root) == true);
    QCOMPARE(p.isGridEnabled(), true);
    QCOMPARE(p.gridX(), 1);
    QCOMPARE(p.gridY(), 2);

    QCOMPARE(p.isGrabKeyboard(), true);
    QCOMPARE(p.isKeyRepeatOff(), true);

    QCOMPARE(p.isGMVisible(), true);

    QCOMPARE(p.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p.grandMasterInputUniverse(), quint32(2));
    QCOMPARE(p.grandMasterInputChannel(), quint32(15));

    QCOMPARE(p.blackoutInputUniverse(), quint32(6));
    QCOMPARE(p.blackoutInputChannel(), quint32(1));

    // Load with invalid root
    root.setTagName("Porperties");
    QVERIFY(p.loadXML(root) == false);
}

void VCProperties_Test::loadPropertiesSad()
{
    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Properties");
    xmldoc.appendChild(root);

    // Grid
    QDomElement grid = xmldoc.createElement("Grid");
    grid.setAttribute("Enabled", "False");
    grid.setAttribute("XResolution", "1");
    grid.setAttribute("YResolution", "2");
    root.appendChild(grid);

    // Keyboard
    QDomElement kb = xmldoc.createElement("Keyboard");
    kb.setAttribute("Grab", "False");
    kb.setAttribute("RepeatOff", "False");
    root.appendChild(kb);

    // Blackout
    QDomElement bo = xmldoc.createElement("Blackout");
    root.appendChild(bo);

    QDomElement boInput = xmldoc.createElement("Input");
    bo.appendChild(boInput);

    // Grand Master
    QDomElement gm = xmldoc.createElement("GrandMaster");
    gm.setAttribute("ChannelMode", "All");
    gm.setAttribute("ValueMode", "Limit");
    root.appendChild(gm);

    QDomElement gmInput = xmldoc.createElement("Input");
    gm.appendChild(gmInput);

    // Default Fade
    QDomElement fade = xmldoc.createElement("DefaultSlider");
    fade.setAttribute("Role", "Fade");
    fade.setAttribute("Visible", "False");
    root.appendChild(fade);

    QDomElement fadeInput = xmldoc.createElement("Input");
    fade.appendChild(fadeInput);

    // Default Hold
    QDomElement hold = xmldoc.createElement("DefaultSlider");
    hold.setAttribute("Role", "Hold");
    hold.setAttribute("Visible", "False");
    root.appendChild(hold);

    QDomElement holdInput = xmldoc.createElement("Input");
    hold.appendChild(holdInput);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QWidget w;

    VCProperties p;
    p.resetContents(&w, m_doc);
    QVERIFY(p.loadProperties(root) == true);
    QCOMPARE(p.isGridEnabled(), false);
    QCOMPARE(p.gridX(), 1);
    QCOMPARE(p.gridY(), 2);

    QCOMPARE(p.isGrabKeyboard(), false);
    QCOMPARE(p.isKeyRepeatOff(), false);

    QCOMPARE(p.isGMVisible(), false);

    QCOMPARE(p.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p.grandMasterInputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(p.grandMasterInputChannel(), InputMap::invalidChannel());

    QCOMPARE(p.blackoutInputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(p.blackoutInputChannel(), InputMap::invalidChannel());

    root.setTagName("Porperties");
    QVERIFY(p.loadProperties(root) == false);
    QVERIFY(p.loadXML(root) == false);
}

void VCProperties_Test::loadXMLInput()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Input");
    root.setAttribute("Universe", "3");
    root.setAttribute("Channel", "78");
    doc.appendChild(root);

    quint32 universe = 0;
    quint32 channel = 0;

    QVERIFY(VCProperties::loadXMLInput(root, &universe, &channel) == true);
    QCOMPARE(universe, quint32(3));
    QCOMPARE(channel, quint32(78));

    universe = channel = 0;
    root.setTagName("Inputt");
    QVERIFY(VCProperties::loadXMLInput(root, &universe, &channel) == false);
    QCOMPARE(universe, quint32(0));
    QCOMPARE(channel, quint32(0));

    universe = channel = 0;
    root.setTagName("Input");
    root.removeAttribute("Universe");
    root.removeAttribute("Channel");
    QVERIFY(VCProperties::loadXMLInput(root, &universe, &channel) == false);
    QCOMPARE(universe, InputMap::invalidUniverse());
    QCOMPARE(channel, InputMap::invalidChannel());
}

void VCProperties_Test::saveXMLHappy()
{
    QWidget w;

    VCProperties p;
    p.resetContents(&w, m_doc);

    p.m_gridEnabled = true;
    p.m_gridX = 11;
    p.m_gridY = 22;

    p.m_grabKeyboard = true;
    p.m_keyRepeatOff = true;

    p.m_gmVisible = true;

    p.m_gmChannelMode = UniverseArray::GMAllChannels;
    p.m_gmValueMode = UniverseArray::GMLimit;
    p.m_gmInputUniverse = 3;
    p.m_gmInputChannel = 42;

    p.m_blackoutInputUniverse = 1;
    p.m_blackoutInputChannel = 3;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);
    QVERIFY(p.saveXML(&xmldoc, &root) == true);

    VCProperties p2;
    p2.resetContents(&w, m_doc);
    QVERIFY(p2.loadXML(root.firstChild().toElement()) == true);
    QCOMPARE(p2.isGridEnabled(), true);
    QCOMPARE(p2.gridX(), 11);
    QCOMPARE(p2.gridY(), 22);

    QCOMPARE(p2.isGrabKeyboard(), true);
    QCOMPARE(p2.isKeyRepeatOff(), true);

    QCOMPARE(p2.isGMVisible(), true);

    QCOMPARE(p2.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p2.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p2.grandMasterInputUniverse(), quint32(3));
    QCOMPARE(p2.grandMasterInputChannel(), quint32(42));

    QCOMPARE(p2.blackoutInputUniverse(), quint32(1));
    QCOMPARE(p2.blackoutInputChannel(), quint32(3));
}

void VCProperties_Test::saveXMLSad()
{
    QWidget w;

    VCProperties p;
    p.resetContents(&w, m_doc);

    p.m_gridEnabled = false;
    p.m_gridX = 11;
    p.m_gridY = 22;

    p.m_grabKeyboard = false;
    p.m_keyRepeatOff = false;

    p.m_gmVisible = false;

    p.m_gmChannelMode = UniverseArray::GMAllChannels;
    p.m_gmValueMode = UniverseArray::GMLimit;
    p.m_gmInputUniverse = InputMap::invalidUniverse();
    p.m_gmInputChannel = InputMap::invalidChannel();

    p.m_blackoutInputUniverse = InputMap::invalidUniverse();
    p.m_blackoutInputChannel = InputMap::invalidChannel();

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);
    QVERIFY(p.saveXML(&xmldoc, &root) == true);

    VCProperties p2;
    p2.resetContents(&w, m_doc);
    QVERIFY(p2.loadXML(root.firstChild().toElement()) == true);
    QCOMPARE(p2.isGridEnabled(), false);
    QCOMPARE(p2.gridX(), 11);
    QCOMPARE(p2.gridY(), 22);

    QCOMPARE(p2.isGrabKeyboard(), false);
    QCOMPARE(p2.isKeyRepeatOff(), false);

    QCOMPARE(p2.isGMVisible(), false);

    QCOMPARE(p2.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p2.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p2.grandMasterInputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(p2.grandMasterInputChannel(), InputMap::invalidChannel());

    QCOMPARE(p2.blackoutInputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(p2.blackoutInputChannel(), InputMap::invalidChannel());
}

QTEST_MAIN(VCProperties_Test)
