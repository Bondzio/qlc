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

#include "chaserrunner_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
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
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Fixture");
    fxi->setAddress(0);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);
}

void ChaserRunner_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void ChaserRunner_Test::initial()
{
    QList <Scene*> steps;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);
    QCOMPARE(cr.m_doc, m_doc);
    QCOMPARE(cr.m_steps, steps);
    QCOMPARE(cr.m_holdBusId, Bus::defaultHold());
    QCOMPARE(cr.m_direction, Function::Forward);
    QCOMPARE(cr.m_runOrder, Function::SingleShot);
    QVERIFY(cr.m_channelMap.isEmpty() == true);
    QCOMPARE(cr.m_elapsed, quint32(0));
    QCOMPARE(cr.m_tap, false);
    QCOMPARE(cr.m_currentStep, 0);
}

void ChaserRunner_Test::tap()
{
    QList <Scene*> steps;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    cr.tap();
    QCOMPARE(cr.m_tap, true);
}

void ChaserRunner_Test::roundCheckSingleShot()
{
    Fixture* fxi = m_doc->fixture(0);
    QVERIFY(fxi != NULL);

    Scene* scene1 = new Scene(m_doc);
    QVERIFY(scene1 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        scene1->setValue(fxi->id(), i, 255);
    m_doc->addFunction(scene1);

    Scene* scene2 = new Scene(m_doc);
    QVERIFY(scene2 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        scene2->setValue(fxi->id(), i, 0);
    m_doc->addFunction(scene2);

    QList <Scene*> steps;
    steps << scene1 << scene2;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    QCOMPARE(cr.m_currentStep, 0);
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
}
