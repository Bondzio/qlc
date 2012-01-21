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

#define protected public
#define private public
#include "vcwidget.h"
#include "vcxypad.h"
#undef private
#undef protected

#include "qlcfixturedefcache.h"
#include "vcxypad_test.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcframe.h"
#include "doc.h"
#include "bus.h"

void VCXYPad_Test::initTestCase()
{
    Bus::init(this);
}

void VCXYPad_Test::initial()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
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
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
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
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
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
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCXYPad pad(&w, &doc, &om, &im, &mt);
    QSize size(100, 100);
    pad.resize(size);

    QPoint pt(50, 50);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.m_currentXYPosition, QPoint(50, 50));
    QCOMPARE(pad.m_currentXYPositionChanged, true);

    pt = QPoint(400, 400);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.m_currentXYPosition, QPoint(size.width(), size.height()));
    QCOMPARE(pad.m_currentXYPositionChanged, true);

    pt = QPoint(0, 0);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.m_currentXYPosition, QPoint(0, 0));
    QCOMPARE(pad.m_currentXYPositionChanged, true);

    pt = QPoint(-5, -5);
    pad.setCurrentXYPosition(pt);
    QCOMPARE(pad.m_currentXYPosition, QPoint(0, 0));
    QCOMPARE(pad.m_currentXYPositionChanged, true);
}

QTEST_MAIN(VCXYPad_Test)
