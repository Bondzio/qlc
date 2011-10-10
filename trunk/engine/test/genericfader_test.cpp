/*
  Q Light Controller - Unit test
  genericfader_test.cpp

  Copyright (c) Heikki Junnila

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

#include "genericfader_test.h"
#include "universearray.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "bus.h"
#include "doc.h"

#define private public
#include "genericfader.h"
#undef private

void GenericFader_Test::initTestCase()
{
    Bus::init(this);
    m_doc = new Doc(this);
}

void GenericFader_Test::init()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(4);
    fxi->setAddress(10);
    m_doc->addFixture(fxi);
}

void GenericFader_Test::cleanup()
{
    m_doc->clearContents();
}

void GenericFader_Test::addRemove()
{
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(0);
    fc.setChannel(0);

    FadeChannel wrong;
    fc.setFixture(0);

    QCOMPARE(fader.m_channels.count(), 0);
    QVERIFY(fader.m_channels.contains(fc) == false);

    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);
    QCOMPARE(fader.m_channels.count(), 1);

    fader.remove(wrong);
    QVERIFY(fader.m_channels.contains(fc) == true);
    QCOMPARE(fader.m_channels.count(), 1);

    fader.remove(fc);
    QVERIFY(fader.m_channels.contains(fc) == false);
    QCOMPARE(fader.m_channels.count(), 0);

    fc.setChannel(0);
    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);

    fc.setChannel(1);
    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);

    fc.setChannel(2);
    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);
    QCOMPARE(fader.m_channels.count(), 3);

    fader.removeAll();
    QCOMPARE(fader.m_channels.count(), 0);

    fc.setFixture(0);
    fc.setChannel(0);
    fc.setTarget(127);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(127));

    fc.setTarget(63);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(127));

    fc.setCurrent(63);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(63));
}

void GenericFader_Test::write()
{
    UniverseArray ua(512);
    GenericFader fader(m_doc);

    Bus::instance()->setValue(Bus::defaultFade(), 255);

    FadeChannel fc;
    fc.setFixture(0);
    fc.setChannel(0);
    fc.setStart(255);
    fc.setTarget(0);
    fc.setCurrent(255);
    fc.setBus(Bus::defaultFade());
    fader.add(fc);

    for (int i = 255; i >= 0; i--)
    {
        ua.zeroIntensityChannels();
        fader.write(&ua);
        QCOMPARE(uchar(ua.preGMValues()[10]), uchar(i));
    }
}
