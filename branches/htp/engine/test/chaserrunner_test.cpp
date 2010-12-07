/*
  Q Light Controller - Unit test
  chaserrunner_test.cpp

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
#include <QMap>

#include "chaserrunner_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "fadechannel.h"
#include "qlcfile.h"
#include "fixture.h"
#include "scene.h"
#include "bus.h"
#include "doc.h"

#define private public
#include "chaserrunner.h"
#undef private

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void ChaserRunner_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void ChaserRunner_Test::init()
{
    m_doc = new Doc(this, m_cache);

    const QLCFixtureDef* def = m_cache.fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    QVERIFY(fxi != NULL);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Fixture");
    fxi->setAddress(0);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    m_scene1 = new Scene(m_doc);
    QVERIFY(m_scene1 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene1->setValue(fxi->id(), i, 255 - i);
    m_doc->addFunction(m_scene1);

    m_scene2 = new Scene(m_doc);
    QVERIFY(m_scene2 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene2->setValue(fxi->id(), i, 127 - i);
    m_doc->addFunction(m_scene2);

    m_scene3 = new Scene(m_doc);
    QVERIFY(m_scene3 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene3->setValue(fxi->id(), i, 0 + i);
    m_doc->addFunction(m_scene3);
}

void ChaserRunner_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void ChaserRunner_Test::initial()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);
    QCOMPARE(cr.m_doc, m_doc);
    QCOMPARE(cr.m_steps, steps);
    QCOMPARE(cr.m_holdBusId, Bus::defaultHold());
    QCOMPARE(cr.m_direction, Function::Forward);
    QCOMPARE(cr.m_originalDirection, Function::Forward);
    QCOMPARE(cr.m_runOrder, Function::SingleShot);
    QVERIFY(cr.m_channelMap.isEmpty() == true);
    QCOMPARE(cr.m_elapsed, quint32(0));
    QCOMPARE(cr.m_tap, false);
    QCOMPARE(cr.m_currentStep, 0);

    ChaserRunner cr2(m_doc, steps, Bus::defaultFade(), Function::Backward,
                    Function::Loop);
    QCOMPARE(cr2.m_doc, m_doc);
    QCOMPARE(cr2.m_steps, steps);
    QCOMPARE(cr2.m_holdBusId, Bus::defaultFade());
    QCOMPARE(cr2.m_direction, Function::Backward);
    QCOMPARE(cr2.m_originalDirection, Function::Backward);
    QCOMPARE(cr2.m_runOrder, Function::Loop);
    QVERIFY(cr2.m_channelMap.isEmpty() == true);
    QCOMPARE(cr2.m_elapsed, quint32(0));
    QCOMPARE(cr2.m_tap, false);
    QCOMPARE(cr2.m_currentStep, 2);
}

void ChaserRunner_Test::tap()
{
    QList <Scene*> steps;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    cr.tap();
    QCOMPARE(cr.m_tap, true);

    cr.reset();
    QCOMPARE(cr.m_tap, false);
}

void ChaserRunner_Test::roundCheckSingleShotForward()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    QCOMPARE(cr.m_currentStep, 0);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == false);

    cr.reset();
    QCOMPARE(cr.m_currentStep, 0);
}

void ChaserRunner_Test::roundCheckSingleShotBackward()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::SingleShot);

    QCOMPARE(cr.m_currentStep, 2);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -2;
    QVERIFY(cr.roundCheck() == false);

    cr.reset();
    QCOMPARE(cr.m_currentStep, 2);
}

void ChaserRunner_Test::roundCheckLoopForward()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);

    QCOMPARE(cr.m_currentStep, 0);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 2);

    // Loops around back to index 0
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 0);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 2);

    cr.reset();
    QCOMPARE(cr.m_currentStep, 0);
}

void ChaserRunner_Test::roundCheckLoopBackward()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::Loop);

    QCOMPARE(cr.m_currentStep, 2);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 0);

    // Loops around back to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 2);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 0);

    cr.reset();
    QCOMPARE(cr.m_currentStep, 2);
}

void ChaserRunner_Test::roundCheckPingPongForward()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::PingPong);

    QCOMPARE(cr.m_currentStep, 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.reset();
    QCOMPARE(cr.m_currentStep, 0);
    QCOMPARE(cr.m_direction, Function::Forward);
}

void ChaserRunner_Test::roundCheckPingPongBackward()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::PingPong);

    QCOMPARE(cr.m_currentStep, 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_currentStep, 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.reset();
    QCOMPARE(cr.m_currentStep, 2);
    QCOMPARE(cr.m_direction, Function::Backward);
}

void ChaserRunner_Test::createFadeChannels()
{
    QList <Scene*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    UniverseArray ua(512);
    QMap <quint32,FadeChannel> map;
    FadeChannel ch;

    // No handover
    QCOMPARE(cr.m_currentStep, 0);
    map = cr.createFadeChannels(&ua, false);

    QVERIFY(map.contains(0) == true);
    ch = map[0];
    QCOMPARE(ch.address(), quint32(0));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(255));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(1) == true);
    ch = map[1];
    QCOMPARE(ch.address(), quint32(1));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(254));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(2) == true);
    ch = map[2];
    QCOMPARE(ch.address(), quint32(2));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(253));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(3) == true);
    ch = map[3];
    QCOMPARE(ch.address(), quint32(3));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(252));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(4) == true);
    ch = map[4];
    QCOMPARE(ch.address(), quint32(4));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(251));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(5) == true);
    ch = map[5];
    QCOMPARE(ch.address(), quint32(5));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(250));
    QCOMPARE(ch.current(), uchar(0));

    // Handover with previous values
    map[0].setCurrent(map[0].target());
    map[1].setCurrent(map[1].target());
    map[2].setCurrent(map[2].target());
    map[3].setCurrent(map[3].target());
    map[4].setCurrent(map[4].target());
    map[5].setCurrent(map[5].target());
    cr.m_channelMap = map;
    cr.m_currentStep = 1;
    map = cr.createFadeChannels(&ua, true);

    QVERIFY(map.contains(0) == true);
    ch = map[0];
    QCOMPARE(ch.address(), quint32(0));
    QCOMPARE(ch.start(), uchar(255));
    QCOMPARE(ch.target(), uchar(127));
    QCOMPARE(ch.current(), uchar(255));

    QVERIFY(map.contains(1) == true);
    ch = map[1];
    QCOMPARE(ch.address(), quint32(1));
    QCOMPARE(ch.start(), uchar(254));
    QCOMPARE(ch.target(), uchar(126));
    QCOMPARE(ch.current(), uchar(254));

    QVERIFY(map.contains(2) == true);
    ch = map[2];
    QCOMPARE(ch.address(), quint32(2));
    QCOMPARE(ch.start(), uchar(253));
    QCOMPARE(ch.target(), uchar(125));
    QCOMPARE(ch.current(), uchar(253));

    QVERIFY(map.contains(3) == true);
    ch = map[3];
    QCOMPARE(ch.address(), quint32(3));
    QCOMPARE(ch.start(), uchar(252));
    QCOMPARE(ch.target(), uchar(124));
    QCOMPARE(ch.current(), uchar(252));

    QVERIFY(map.contains(4) == true);
    ch = map[4];
    QCOMPARE(ch.address(), quint32(4));
    QCOMPARE(ch.start(), uchar(251));
    QCOMPARE(ch.target(), uchar(123));
    QCOMPARE(ch.current(), uchar(251));

    QVERIFY(map.contains(5) == true);
    ch = map[5];
    QCOMPARE(ch.address(), quint32(5));
    QCOMPARE(ch.start(), uchar(250));
    QCOMPARE(ch.target(), uchar(122));
    QCOMPARE(ch.current(), uchar(250));

    // Handover attempt without previous values -> runner takes them from UA
    ua.write(0, 1, QLCChannel::Intensity);
    ua.write(1, 2, QLCChannel::Intensity);
    ua.write(2, 3, QLCChannel::Intensity);
    ua.write(3, 4, QLCChannel::Intensity);
    ua.write(4, 5, QLCChannel::Intensity);
    ua.write(5, 6, QLCChannel::Intensity);
    cr.m_channelMap.clear();
    cr.m_currentStep = 2;
    map = cr.createFadeChannels(&ua, true);

    QVERIFY(map.contains(0) == true);
    ch = map[0];
    QCOMPARE(ch.address(), quint32(0));
    QCOMPARE(ch.start(), uchar(1));
    QCOMPARE(ch.target(), uchar(0));
    QCOMPARE(ch.current(), uchar(1));

    QVERIFY(map.contains(1) == true);
    ch = map[1];
    QCOMPARE(ch.address(), quint32(1));
    QCOMPARE(ch.start(), uchar(2));
    QCOMPARE(ch.target(), uchar(1));
    QCOMPARE(ch.current(), uchar(2));

    QVERIFY(map.contains(2) == true);
    ch = map[2];
    QCOMPARE(ch.address(), quint32(2));
    QCOMPARE(ch.start(), uchar(3));
    QCOMPARE(ch.target(), uchar(2));
    QCOMPARE(ch.current(), uchar(3));

    QVERIFY(map.contains(3) == true);
    ch = map[3];
    QCOMPARE(ch.address(), quint32(3));
    QCOMPARE(ch.start(), uchar(4));
    QCOMPARE(ch.target(), uchar(3));
    QCOMPARE(ch.current(), uchar(4));

    QVERIFY(map.contains(4) == true);
    ch = map[4];
    QCOMPARE(ch.address(), quint32(4));
    QCOMPARE(ch.start(), uchar(5));
    QCOMPARE(ch.target(), uchar(4));
    QCOMPARE(ch.current(), uchar(5));

    QVERIFY(map.contains(5) == true);
    ch = map[5];
    QCOMPARE(ch.address(), quint32(5));
    QCOMPARE(ch.start(), uchar(6));
    QCOMPARE(ch.target(), uchar(5));
    QCOMPARE(ch.current(), uchar(6));

    cr.m_currentStep = 3;
    map = cr.createFadeChannels(&ua, false);
    QVERIFY(map.isEmpty() == true);
}
