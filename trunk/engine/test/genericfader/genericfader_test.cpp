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
#include "doc.h"

#define private public
#include "genericfader.h"
#undef private

void GenericFader_Test::initTestCase()
{
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
    QCOMPARE(fader.m_channels[fc].target(), uchar(63));

    fc.setCurrent(63);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(63));
}

void GenericFader_Test::writeZeroFade()
{
    UniverseArray ua(512);
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(0);
    fc.setChannel(0);
    fc.setStart(0);
    fc.setTarget(255);
    fc.setFadeTime(0);

    fader.add(fc);
    QCOMPARE(ua.preGMValues()[10], (char) 0);
    fader.write(&ua);
    QCOMPARE(ua.preGMValues()[10], (char) 255);
}

void GenericFader_Test::writeLoop()
{
    UniverseArray ua(512);
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(0);
    fc.setChannel(0);
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader.add(fc);

    QCOMPARE(ua.preGMValues()[10], (char) 0);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua.zeroIntensityChannels();
        fader.write(&ua);

        int actual = uchar(ua.preGMValues()[10]);
        expected += 5;
        QCOMPARE(actual, expected);
    }
}

void GenericFader_Test::adjustIntensity()
{
    UniverseArray ua(512);
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(0);
    fc.setChannel(0);
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader.add(fc);

    qreal intensity = 0.5;
    fader.adjustIntensity(intensity);
    QCOMPARE(fader.intensity(), intensity);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua.zeroIntensityChannels();
        fader.write(&ua);

        int actual = uchar(ua.preGMValues()[10]);
        expected += 5;
        int expectedWithIntensity = floor((qreal(expected) * intensity) + 0.5);
        QVERIFY(actual == expectedWithIntensity);
    }
}

QTEST_APPLESS_MAIN(GenericFader_Test)
