/*
  Q Light Controller
  vcxypad_test.cpp

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

#include <QFrame>
#include <QtTest>
#include <QtXml>

#define protected public
#define private public
#include "mastertimer.h"
#include "vcwidget.h"
#include "vcxypad.h"
#undef private
#undef protected

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "vcxypad_test.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcframe.h"
#include "qlcfile.h"
#include "doc.h"
#include "bus.h"

#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void VCXYPad_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void VCXYPad_Test::initial()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCXYPad pad(&w, &doc, &om, &im, &mt);
    QCOMPARE(pad.objectName(), QString("VCXYPad"));
    QCOMPARE(pad.caption(), QString("XY Pad"));
    QCOMPARE(pad.frameStyle(), QFrame::Panel | QFrame::Sunken);
    QCOMPARE(pad.size(), QSize(120, 120));
    QVERIFY(pad.m_xyPosPixmap.isNull() == false);
    QCOMPARE(pad.m_currentXYPosition, QPoint(60, 60));
    QCOMPARE(pad.m_fixtures.size(), 0);
}

void VCXYPad_Test::fixtures()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCXYPad pad(&w, &doc, &om, &im, &mt);

    VCXYPadFixture xyf1(&doc);
    xyf1.setFixture(1);

    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 1);
    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 1);

    VCXYPadFixture xyf2(&doc);
    xyf2.setFixture(2);

    pad.appendFixture(xyf2);
    QCOMPARE(pad.m_fixtures.size(), 2);
    pad.appendFixture(xyf2);
    QCOMPARE(pad.m_fixtures.size(), 2);
    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 2);

    pad.removeFixture(3);
    QCOMPARE(pad.m_fixtures.size(), 2);

    pad.removeFixture(1);
    QCOMPARE(pad.m_fixtures.size(), 1);
    QCOMPARE(pad.m_fixtures[0].fixture(), quint32(2));

    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 2);

    pad.clearFixtures();
    QCOMPARE(pad.m_fixtures.size(), 0);

    // Invalid fixture
    VCXYPadFixture xyf3(&doc);
    pad.appendFixture(xyf3);
    QCOMPARE(pad.m_fixtures.size(), 0);
}

void VCXYPad_Test::copy()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCFrame parent(&w, &doc, &om, &im, &mt);
    VCXYPad pad(&parent, &doc, &om, &im, &mt);
    pad.setCaption("Dingdong");
    QSize size(80, 80);
    QPoint pt(50, 30);
    pad.setCurrentXYPosition(pt);

    VCXYPadFixture xyf1(&doc);
    xyf1.setFixture(1);
    pad.appendFixture(xyf1);

    VCXYPadFixture xyf2(&doc);
    xyf2.setFixture(2);
    pad.appendFixture(xyf2);

    VCXYPadFixture xyf3(&doc);
    xyf3.setFixture(3);
    pad.appendFixture(xyf3);

    VCXYPad* copy = qobject_cast<VCXYPad*> (pad.createCopy(&parent));
    QVERIFY(copy != NULL);
    QCOMPARE(copy->m_fixtures.size(), 3);
    QVERIFY(copy->m_fixtures[0] == xyf1);
    QVERIFY(copy->m_fixtures[1] == xyf2);
    QVERIFY(copy->m_fixtures[2] == xyf3);

    QVERIFY(&copy->m_fixtures[0] != &xyf1);
    QVERIFY(&copy->m_fixtures[1] != &xyf2);
    QVERIFY(&copy->m_fixtures[2] != &xyf3);

    QCOMPARE(copy->currentXYPosition(), pt);
    QCOMPARE(copy->size(), pad.size());
    QCOMPARE(copy->caption(), QString("Dingdong"));
}

void VCXYPad_Test::setPos()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCXYPad pad(&w, &doc, &om, &im, &mt);
    QSize size(100, 100);
    pad.resize(size);

    QPoint pt(50, 50);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.currentXYPosition(), QPoint(50, 50));
    QCOMPARE(pad.m_currentXYPositionChanged, true);

    pt = QPoint(400, 400);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.currentXYPosition(), QPoint(size.width(), size.height()));
    QCOMPARE(pad.m_currentXYPositionChanged, true);

    pt = QPoint(0, 0);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.currentXYPosition(), QPoint(0, 0));
    QCOMPARE(pad.m_currentXYPositionChanged, true);

    pt = QPoint(-5, -5);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.currentXYPosition(), QPoint(0, 0));
    QCOMPARE(pad.m_currentXYPositionChanged, true);
}

void VCXYPad_Test::loadXML()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("XYPad");
    xmldoc.appendChild(root);

    QDomElement pos = xmldoc.createElement("Position");
    pos.setAttribute("X", "10");
    pos.setAttribute("Y", "20");
    root.appendChild(pos);

    QDomElement fxi = xmldoc.createElement("Fixture");
    fxi.setAttribute("ID", "69");
    root.appendChild(fxi);

    QDomElement x = xmldoc.createElement("Axis");
    x.setAttribute("ID", "X");
    x.setAttribute("LowLimit", "0.1");
    x.setAttribute("HighLimit", "0.5");
    x.setAttribute("Reverse", "True");
    fxi.appendChild(x);

    QDomElement y = xmldoc.createElement("Axis");
    y.setAttribute("ID", "Y");
    y.setAttribute("LowLimit", "0.2");
    y.setAttribute("HighLimit", "0.6");
    y.setAttribute("Reverse", "True");
    fxi.appendChild(y);

    QDomElement fxi2 = xmldoc.createElement("Fixture");
    fxi2.setAttribute("ID", "50");
    root.appendChild(fxi2);

    QDomElement x2 = xmldoc.createElement("Axis");
    x2.setAttribute("ID", "X");
    x2.setAttribute("LowLimit", "0.0");
    x2.setAttribute("HighLimit", "1.0");
    x2.setAttribute("Reverse", "False");
    fxi2.appendChild(x2);

    QDomElement y2 = xmldoc.createElement("Axis");
    y2.setAttribute("ID", "Y");
    y2.setAttribute("LowLimit", "0.0");
    y2.setAttribute("HighLimit", "1.0");
    y2.setAttribute("Reverse", "False");
    fxi2.appendChild(y2);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement appearance = xmldoc.createElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    QDomElement font = xmldoc.createElement("Font");
    QDomText fontText = xmldoc.createTextNode(f.toString());
    font.appendChild(fontText);
    appearance.appendChild(font);
    root.appendChild(appearance);

    QDomElement foobar = xmldoc.createElement("Foobar");
    root.appendChild(foobar);

    VCXYPad pad(&w, &doc, &om, &im, &mt);
    QVERIFY(pad.loadXML(&root) == true);
    QCOMPARE(pad.m_fixtures.size(), 2);
    QCOMPARE(pad.currentXYPosition(), QPoint(10, 20));
    QCOMPARE(pad.pos(), QPoint(3, 4));
    QCOMPARE(pad.size(), QSize(42, 69));

    VCXYPadFixture fixture(&doc);
    fixture.setFixture(69);
    QVERIFY(pad.m_fixtures.contains(fixture) == true);
    fixture.setFixture(50);
    QVERIFY(pad.m_fixtures.contains(fixture) == true);

    root.setTagName("YXPad");
    QVERIFY(pad.loadXML(&root) == false);
}

void VCXYPad_Test::saveXML()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCXYPad pad(&w, &doc, &om, &im, &mt);
    pad.setCaption("MyPad");
    pad.resize(QSize(150, 200));
    pad.move(QPoint(10, 20));
    pad.setCurrentXYPosition(QPoint(23, 45));

    VCXYPadFixture fixture1(&doc);
    fixture1.setFixture(11);
    pad.appendFixture(fixture1);

    VCXYPadFixture fixture2(&doc);
    fixture2.setFixture(22);
    pad.appendFixture(fixture2);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int fixture = 0, position = 0, wstate = 0, appearance = 0;

    QVERIFY(pad.saveXML(&xmldoc, &root) == true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("XYPad"));
    QCOMPARE(node.toElement().attribute("Caption"), QString("MyPad"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Fixture")
        {
            fixture++;
            QVERIFY(tag.attribute("ID") == QString("11") ||
                    tag.attribute("ID") == QString("22"));
            QCOMPARE(tag.childNodes().count(), 2);
        }
        else if (tag.tagName() == "Position")
        {
            position++;
            QCOMPARE(tag.attribute("X"), QString("23"));
            QCOMPARE(tag.attribute("Y"), QString("45"));
        }
        else if (tag.tagName() == "WindowState")
        {
            wstate++;
        }
        else if (tag.tagName() == "Appearance")
        {
            appearance++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(fixture, 2);
    QCOMPARE(position, 1);
    QCOMPARE(wstate, 1);
    QCOMPARE(appearance, 1);
}

void VCXYPad_Test::modeChange()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    UniverseArray ua(512);
    QWidget w;

    Fixture* fxi = new Fixture(&doc);
    const QLCFixtureDef* def = m_cache.fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    doc.addFixture(fxi);

    VCXYPad pad(&w, &doc, &om, &im, &mt);
    pad.resize(QSize(200, 200));

    VCXYPadFixture xy(&doc);
    xy.setFixture(fxi->id());
    pad.appendFixture(xy);
    QCOMPARE(pad.fixtures().size(), 1);
    QCOMPARE(pad.fixtures()[0].m_xMSB, QLCChannel::invalid());
    QCOMPARE(pad.fixtures()[0].m_xLSB, QLCChannel::invalid());

    doc.setMode(Doc::Operate);
    QVERIFY(pad.fixtures()[0].m_xMSB != QLCChannel::invalid());
    QVERIFY(pad.fixtures()[0].m_yMSB != QLCChannel::invalid());
    QCOMPARE(mt.m_dmxSourceList.size(), 1);
    QCOMPARE(mt.m_dmxSourceList[0], &pad);

    pad.setCurrentXYPosition(QPoint(200, 200));
    pad.writeDMX(&mt, &ua);
    QCOMPARE(ua.preGMValues()[0], char(255));
    QCOMPARE(ua.preGMValues()[1], char(255));

    pad.setCurrentXYPosition(QPoint(100, 50));
    pad.writeDMX(&mt, &ua);
    QCOMPARE(ua.preGMValues()[0], char(128));
    QCOMPARE(ua.preGMValues()[1], char(64));

    doc.setMode(Doc::Design);
    QCOMPARE(pad.fixtures()[0].m_xMSB, QLCChannel::invalid());
    QCOMPARE(pad.fixtures()[0].m_yMSB, QLCChannel::invalid());
    QCOMPARE(mt.m_dmxSourceList.size(), 0);
}

void VCXYPad_Test::paint()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    UniverseArray ua(512);
    QWidget w;

    // Just a crash test, no point in checking individual pixels
    VCXYPad pad(&w, &doc, &om, &im, &mt);
    pad.resize(QSize(200, 200));
    QPaintEvent ev(QRect(pad.rect()));
    pad.paintEvent(&ev);
}

void VCXYPad_Test::mouse()
{
    Doc doc(this, m_cache);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    UniverseArray ua(512);
    QWidget w;

    // Just a crash test, no point in checking individual pixels
    VCXYPad pad(&w, &doc, &om, &im, &mt);
    pad.resize(QSize(200, 200));
    doc.setMode(Doc::Operate);

    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(100, 100), Qt::LeftButton, 0, 0);
    pad.mousePressEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(100, 100));
    QCOMPARE(pad.cursor().shape(), Qt::CrossCursor);
    QCOMPARE(pad.hasMouseTracking(), true);

    ev = QMouseEvent(QEvent::MouseMove, QPoint(50, 50), Qt::NoButton, 0, 0);
    pad.mouseMoveEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(50, 50));
    QCOMPARE(pad.cursor().shape(), Qt::CrossCursor);
    QCOMPARE(pad.hasMouseTracking(), true);

    ev = QMouseEvent(QEvent::MouseMove, QPoint(250, 250), Qt::NoButton, 0, 0);
    pad.mouseMoveEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(200, 200));
    QCOMPARE(pad.cursor().shape(), Qt::CrossCursor);
    QCOMPARE(pad.hasMouseTracking(), true);

    ev = QMouseEvent(QEvent::MouseMove, QPoint(-5, -5), Qt::NoButton, 0, 0);
    pad.mouseMoveEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(0, 0));
    QCOMPARE(pad.cursor().shape(), Qt::CrossCursor);
    QCOMPARE(pad.hasMouseTracking(), true);

    ev = QMouseEvent(QEvent::MouseButtonRelease, QPoint(150, 150), Qt::LeftButton, 0, 0);
    pad.mouseReleaseEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(0, 0));
    QCOMPARE(pad.cursor().shape(), Qt::ArrowCursor);
    QCOMPARE(pad.hasMouseTracking(), false);

    ev = QMouseEvent(QEvent::MouseButtonPress, QPoint(250, 250), Qt::LeftButton, 0, 0);
    pad.mousePressEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(200, 200));
    QCOMPARE(pad.cursor().shape(), Qt::CrossCursor);
    QCOMPARE(pad.hasMouseTracking(), true);

    ev = QMouseEvent(QEvent::MouseButtonPress, QPoint(-5, -5), Qt::LeftButton, 0, 0);
    pad.mousePressEvent(&ev);
    QCOMPARE(pad.currentXYPosition(), QPoint(0, 0));
    QCOMPARE(pad.cursor().shape(), Qt::CrossCursor);
    QCOMPARE(pad.hasMouseTracking(), true);
}

QTEST_MAIN(VCXYPad_Test)
