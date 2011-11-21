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

#define private public
#define protected public
#include "chaserrunner_test.h"
#include "mastertimer_stub.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "chaserrunner.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "chaserstep.h"
#include "qlcfile.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#undef protected
#undef private

#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void ChaserRunner_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    m_doc->fixtureDefCache()->load(dir);
}

void ChaserRunner_Test::cleanupTestCase()
{
    delete m_doc;
}

void ChaserRunner_Test::init()
{
    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
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
    m_scene1->setName("S1");
    QVERIFY(m_scene1 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene1->setValue(fxi->id(), i, 255 - i);
    m_doc->addFunction(m_scene1);

    m_scene2 = new Scene(m_doc);
    m_scene2->setName("S2");
    QVERIFY(m_scene2 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene2->setValue(fxi->id(), i, 127 - i);
    m_doc->addFunction(m_scene2);

    m_scene3 = new Scene(m_doc);
    m_scene3->setName("S3");
    QVERIFY(m_scene3 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene3->setValue(fxi->id(), i, 0 + i);
    m_doc->addFunction(m_scene3);
}

void ChaserRunner_Test::cleanup()
{
    m_doc->clearContents();
}

QList <ChaserStep> chaserSteps(const QList <Function*>& funcs)
{
    QList <ChaserStep> steps;
    foreach (Function* f, funcs)
        steps << ChaserStep(f->id());
    return steps;
}

void ChaserRunner_Test::initial()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;

    // Forwards single-shot
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Forward, Function::SingleShot);
    QCOMPARE(cr.m_doc, m_doc);
    QCOMPARE(cr.m_steps, chaserSteps(steps));
    QCOMPARE(cr.m_fadeInSpeed, uint(10));
    QCOMPARE(cr.m_fadeOutSpeed, uint(20));
    QCOMPARE(cr.m_duration, uint(30));
    QCOMPARE(cr.m_direction, Function::Forward);
    QCOMPARE(cr.m_originalDirection, Function::Forward);
    QCOMPARE(cr.m_runOrder, Function::SingleShot);
    QCOMPARE(cr.m_elapsed, quint32(0));
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_currentStep, 0);
    QCOMPARE(cr.m_newCurrent, -1);
    QCOMPARE(cr.m_intensity, qreal(1.0));

    // Backwards loop
    ChaserRunner cr2(m_doc, chaserSteps(steps), 20, 30, 10, Function::Backward, Function::Loop);
    QCOMPARE(cr2.m_doc, m_doc);
    QCOMPARE(cr2.m_steps, chaserSteps(steps));
    QCOMPARE(cr2.m_fadeInSpeed, uint(20));
    QCOMPARE(cr2.m_fadeOutSpeed, uint(30));
    QCOMPARE(cr2.m_duration, uint(10));
    QCOMPARE(cr2.m_direction, Function::Backward);
    QCOMPARE(cr2.m_originalDirection, Function::Backward);
    QCOMPARE(cr2.m_runOrder, Function::Loop);
    QCOMPARE(cr2.m_elapsed, quint32(0));
    QCOMPARE(cr2.m_next, false);
    QCOMPARE(cr2.m_currentStep, 2);
    QCOMPARE(cr2.m_newCurrent, -1);
    QCOMPARE(cr2.m_intensity, qreal(1.0));
}

void ChaserRunner_Test::speed()
{
    ChaserRunner cr(m_doc, QList<ChaserStep>(), 0, 0, 0, Function::Forward, Function::Loop);
    QCOMPARE(cr.m_duration, uint(0));
    cr.setDuration(1000);
    QCOMPARE(cr.m_duration, uint(1000));
}

void ChaserRunner_Test::nextPrevious()
{
    ChaserRunner cr(m_doc, QList <ChaserStep>(), 10, 20, 30, Function::Forward, Function::SingleShot);

    cr.next();
    QCOMPARE(cr.m_next, true);
    QCOMPARE(cr.m_previous, false);

    cr.next();
    QCOMPARE(cr.m_next, true);
    QCOMPARE(cr.m_previous, false);

    cr.previous();
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, true);

    cr.previous();
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, true);

    cr.reset();
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, false);
}

void ChaserRunner_Test::autoStep()
{
    ChaserRunner cr(m_doc, QList <ChaserStep>(), 10, 20, 30, Function::Forward, Function::SingleShot);

    QCOMPARE(cr.isAutoStep(), true);
    cr.setAutoStep(true);
    QCOMPARE(cr.m_autoStep, true);
    QCOMPARE(cr.isAutoStep(), true);
    cr.setAutoStep(false);
    QCOMPARE(cr.m_autoStep, false);
    QCOMPARE(cr.isAutoStep(), false);
    cr.setAutoStep(false);
    QCOMPARE(cr.m_autoStep, false);
    QCOMPARE(cr.isAutoStep(), false);
}

void ChaserRunner_Test::roundCheckSingleShotForward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Forward, Function::SingleShot);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 4; // Over list.size
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -1; // Under list.size
    QVERIFY(cr.roundCheck() == false);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
}

void ChaserRunner_Test::roundCheckSingleShotBackward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Backward, Function::SingleShot);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 3; // Over list.size
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -1; // Under list.size
    QVERIFY(cr.roundCheck() == false);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
}

void ChaserRunner_Test::roundCheckLoopForward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Forward, Function::Loop);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    // Loops around back to index 0
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    // Loops around to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
}

void ChaserRunner_Test::roundCheckLoopBackward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Backward, Function::Loop);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    // Loops around back to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    // Loops around to index 0
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
}

void ChaserRunner_Test::roundCheckPingPongForward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3 << m_scene1;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Forward, Function::PingPong);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Forward);
}

void ChaserRunner_Test::roundCheckPingPongBackward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3 << m_scene1;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Backward, Function::PingPong);

    QCOMPARE(cr.currentStep(), 3);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.reset();
    QCOMPARE(cr.currentStep(), 3);
    QCOMPARE(cr.m_direction, Function::Backward);
}

void ChaserRunner_Test::writeNoSteps()
{
    QList <Function*> steps;
    ChaserRunner cr(m_doc, chaserSteps(steps), 10, 20, 30, Function::Forward, Function::Loop);
    UniverseArray ua(512);
    MasterTimerStub timer(m_doc, ua);

    QVERIFY(cr.write(&timer, &ua) == false);
}

void ChaserRunner_Test::writeForwardLoopZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Forward, Function::Loop);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
}

void ChaserRunner_Test::writeBackwardLoopZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Backward, Function::Loop);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
}

void ChaserRunner_Test::writeForwardSingleShotZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Forward, Function::SingleShot);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == false);
    cr.postRun(&timer, NULL);
    timer.timerTick();
    QVERIFY(m_scene3->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeBackwardSingleShotZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Backward, Function::SingleShot);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == false);
    cr.postRun(&timer, NULL);
    timer.timerTick();
    QVERIFY(m_scene1->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeForwardPingPongZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Forward, Function::PingPong);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QVERIFY(m_scene1->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QVERIFY(m_scene1->stopped() == true);
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
}

void ChaserRunner_Test::writeBackwardPingPongZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Backward, Function::PingPong);
    MasterTimer timer(m_doc);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
}

void ChaserRunner_Test::writeForwardLoopFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    uint dur = MasterTimer::tick() * 5;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, dur, Function::Forward, Function::Loop);
    MasterTimer timer(m_doc);

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }
}

void ChaserRunner_Test::writeBackwardLoopFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    uint dur = MasterTimer::tick() * 5;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, dur, Function::Backward, Function::Loop);
    MasterTimer timer(m_doc);

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }
}

void ChaserRunner_Test::writeForwardSingleShotFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    uint dur = MasterTimer::tick() * 5;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, dur, Function::Forward, Function::SingleShot);
    MasterTimer timer(m_doc);

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    QVERIFY(cr.write(&timer, NULL) == false);
    timer.timerTick();

    cr.postRun(&timer, NULL);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeBackwardSingleShotFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    uint dur = MasterTimer::tick() * 5;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, dur, Function::Backward, Function::SingleShot);
    MasterTimer timer(m_doc);

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    QVERIFY(cr.write(&timer, NULL) == false);
    timer.timerTick();

    cr.postRun(&timer, NULL);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 0);
}

void ChaserRunner_Test::writeForwardPingPongFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    uint dur = MasterTimer::tick() * 5;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, dur, Function::Forward, Function::PingPong);
    MasterTimer timer(m_doc);

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }
}

void ChaserRunner_Test::writeBackwardPingPongFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    uint dur = MasterTimer::tick() * 5;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, dur, Function::Backward, Function::PingPong);
    MasterTimer timer(m_doc);

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 3
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    // Step 2
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    // Step 1
    for (uint i = 0; i < dur; i += MasterTimer::tick())
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }
}

void ChaserRunner_Test::writeNoAutoStep()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Forward, Function::Loop);
    cr.setAutoStep(false);
    MasterTimer timer(m_doc);

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    cr.next();

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    cr.next();

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    cr.next();

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }

    cr.previous();

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene3);
    }

    cr.previous();

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene2);
    }

    cr.previous();

    for (int i = 0; i < 10; i++)
    {
        QVERIFY(cr.write(&timer, NULL) == true);
        timer.timerTick();
        QCOMPARE(timer.m_functionList.size(), 1);
        QCOMPARE(timer.m_functionList[0], m_scene1);
    }
}

void ChaserRunner_Test::adjustIntensity()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, chaserSteps(steps), 0, 0, 0, Function::Forward, Function::Loop);
    MasterTimer timer(m_doc);

    cr.adjustIntensity(0.5);

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
    QCOMPARE(m_scene1->intensity(), qreal(0.5));
    QCOMPARE(m_scene2->intensity(), qreal(1.0));
    QCOMPARE(m_scene3->intensity(), qreal(1.0));

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);
    QCOMPARE(m_scene1->intensity(), qreal(1.0));
    QCOMPARE(m_scene2->intensity(), qreal(0.5));
    QCOMPARE(m_scene3->intensity(), qreal(1.0));

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
    QCOMPARE(m_scene1->intensity(), qreal(1.0));
    QCOMPARE(m_scene2->intensity(), qreal(1.0));
    QCOMPARE(m_scene3->intensity(), qreal(0.5));

    cr.adjustIntensity(0.7);
    QCOMPARE(m_scene1->intensity(), qreal(1.0));
    QCOMPARE(m_scene2->intensity(), qreal(1.0));
    QCOMPARE(m_scene3->intensity(), qreal(0.7));

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene1);
    QCOMPARE(m_scene1->intensity(), qreal(0.7));
    QCOMPARE(m_scene2->intensity(), qreal(1.0));
    QCOMPARE(m_scene3->intensity(), qreal(1.0));

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene2);
    QCOMPARE(m_scene1->intensity(), qreal(1.0));
    QCOMPARE(m_scene2->intensity(), qreal(0.7));
    QCOMPARE(m_scene3->intensity(), qreal(1.0));

    cr.adjustIntensity(1.5);
    QCOMPARE(m_scene1->intensity(), qreal(1.0));
    QCOMPARE(m_scene2->intensity(), qreal(1.0));
    QCOMPARE(m_scene3->intensity(), qreal(1.0));

    QVERIFY(cr.write(&timer, NULL) == true);
    timer.timerTick();
    QCOMPARE(timer.m_functionList.size(), 1);
    QCOMPARE(timer.m_functionList[0], m_scene3);
    QCOMPARE(m_scene1->intensity(), qreal(1.0));
    QCOMPARE(m_scene2->intensity(), qreal(1.0));
    QCOMPARE(m_scene3->intensity(), qreal(1.0));
}

QTEST_APPLESS_MAIN(ChaserRunner_Test)
