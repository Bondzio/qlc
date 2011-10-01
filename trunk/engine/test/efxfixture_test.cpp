/*
  Q Light Controller - Unit test
  efxfixture_test.cpp

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
#include <QtXml>
#include <QList>

#define protected public
#define private public
#include "mastertimer_stub.h"
#include "efxfixture_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "genericfader.h"
#include "efxfixture.h"
#include "qlcchannel.h"
#include "scene_stub.h"
#include "function.h"
#include "fixture.h"
#include "qlcfile.h"
#include "efx.h"
#include "doc.h"
#undef private
#undef protected

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void EFXFixture_Test::initTestCase()
{
    Bus::init(this);
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void EFXFixture_Test::init()
{
    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "MH-440");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);
}

void EFXFixture_Test::cleanupTestCase()
{
    delete m_doc;
}

void EFXFixture_Test::cleanup()
{
    m_doc->clearContents();
}

void EFXFixture_Test::initial()
{
    EFX e(m_doc);

    EFXFixture ef(&e);
    QVERIFY(ef.fixture() == Fixture::invalidId());
    QVERIFY(ef.direction() == EFX::Forward);
    QVERIFY(ef.serialNumber() == 0);
    QVERIFY(ef.isValid() == false);
    QVERIFY(ef.isReady() == false);

    QVERIFY(ef.m_fixture == Fixture::invalidId());
    QVERIFY(ef.m_direction == EFX::Forward);
    QVERIFY(ef.m_serialNumber == 0);
    QVERIFY(ef.m_runTimeDirection == EFX::Forward);
    QVERIFY(ef.m_ready == false);
    QVERIFY(ef.m_iterator == 0);
    QVERIFY(ef.m_skipIterator == 0);
    QVERIFY(ef.m_skipThreshold == 0);
    QVERIFY(ef.m_panValue == 0);
    QVERIFY(ef.m_tiltValue == 0);

    QVERIFY(ef.m_intensity == 1.0);
    QVERIFY(ef.m_fadeIntensity == uchar(255));
}

void EFXFixture_Test::copyFrom()
{
    EFX e(m_doc);

    EFXFixture ef(&e);
    ef.m_fixture = 15;
    ef.m_direction = EFX::Backward;
    ef.m_serialNumber = 25;
    ef.m_runTimeDirection = EFX::Backward;
    ef.m_ready = true;
    ef.m_iterator = 313.37;
    ef.m_skipIterator = 4.2;
    ef.m_skipThreshold = 6.9;
    ef.m_panValue = 127.15;
    ef.m_tiltValue = 240.99;

    ef.m_intensity = 0.314159;
    ef.m_fadeIntensity = 125;

    EFXFixture copy(&e);
    copy.copyFrom(&ef);
    QVERIFY(copy.m_fixture == 15);
    QVERIFY(copy.m_direction == EFX::Backward);
    QVERIFY(copy.m_serialNumber == 25);
    QVERIFY(copy.m_runTimeDirection == EFX::Backward);
    QVERIFY(copy.m_ready == true);
    QVERIFY(copy.m_iterator == 313.37);
    QVERIFY(copy.m_skipIterator == 4.2);
    QVERIFY(copy.m_skipThreshold == 6.9);
    QVERIFY(copy.m_panValue == 127.15);
    QVERIFY(copy.m_tiltValue == 240.99);

    QVERIFY(copy.m_intensity == 0.314159);
    QVERIFY(copy.m_fadeIntensity == 125);
}

void EFXFixture_Test::publicProperties()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    ef.setFixture(19);
    QVERIFY(ef.fixture() == 19);

    ef.setFixture(Fixture::invalidId());
    QVERIFY(ef.fixture() == Fixture::invalidId());

    ef.setDirection(EFX::Backward);
    QVERIFY(ef.direction() == EFX::Backward);
    QVERIFY(ef.m_runTimeDirection == EFX::Backward);

    ef.setDirection(EFX::Forward);
    QVERIFY(ef.direction() == EFX::Forward);
    QVERIFY(ef.m_runTimeDirection == EFX::Forward);

    ef.setFadeIntensity(69);
    QVERIFY(ef.fadeIntensity() == 69);
}

void EFXFixture_Test::loadSuccess()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("83");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement in = doc.createElement("Intensity");
    QDomText inText = doc.createTextNode("91");
    in.appendChild(inText);
    root.appendChild(in);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == true);
    QVERIFY(ef.fixture() == 83);
    QVERIFY(ef.direction() == EFX::Backward);
    QVERIFY(ef.fadeIntensity() == 91);
}

void EFXFixture_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("EFXFixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("189");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == false);
    QVERIFY(ef.fixture() == Fixture::invalidId());
    QVERIFY(ef.direction() == EFX::Forward);
}

void EFXFixture_Test::loadWrongDirection()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("97");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Phorrwarrd");
    dir.appendChild(dirText);
    root.appendChild(dir);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == true);
    QVERIFY(ef.fixture() == 97);
    QVERIFY(ef.direction() == EFX::Forward);
}

void EFXFixture_Test::loadExtraTag()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("108");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Forward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement foo = doc.createElement("Foobar");
    QDomText fooText = doc.createTextNode("Just testing");
    foo.appendChild(fooText);
    root.appendChild(foo);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == true);
    QVERIFY(ef.fixture() == 108);
    QVERIFY(ef.direction() == EFX::Forward);
}

void EFXFixture_Test::save()
{
    EFX e(m_doc);
    EFXFixture ef(&e);
    ef.setFixture(56);
    ef.setDirection(EFX::Backward);

    QDomDocument doc;
    QDomElement root = doc.createElement("EFX");

    QVERIFY(ef.saveXML(&doc, &root) == true);

    QDomElement tag = root.firstChild().toElement();
    QVERIFY(tag.tagName() == "Fixture");

    tag = tag.firstChild().toElement();
    QVERIFY(tag.tagName() == "ID");
    QVERIFY(tag.text() == "56");

    tag = tag.nextSibling().toElement();
    QVERIFY(tag.tagName() == "Direction");
    QVERIFY(tag.text() == "Backward");
}

void EFXFixture_Test::serialNumber()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    ef.setSerialNumber(15);
    QVERIFY(ef.serialNumber() == 15);
}

void EFXFixture_Test::updateSkipThreshold()
{
    EFX e(m_doc);

    EFXFixture* ef1 = new EFXFixture(&e);
    ef1->setFixture(1);
    ef1->setSerialNumber(0);
    e.addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(&e);
    ef2->setFixture(2);
    ef2->setSerialNumber(1);
    e.addFixture(ef2);

    EFXFixture* ef3 = new EFXFixture(&e);
    ef3->setFixture(3);
    ef3->setSerialNumber(2);
    e.addFixture(ef3);

    EFXFixture* ef4 = new EFXFixture(&e);
    ef4->setFixture(4);
    ef4->setSerialNumber(3);
    e.addFixture(ef4);

    ef1->updateSkipThreshold();
    QVERIFY(ef1->m_skipThreshold == (M_PI * 2.0) * 0);

    ef2->updateSkipThreshold();
    QVERIFY(ef2->m_skipThreshold == (M_PI * 2.0) * 0.25);

    ef3->updateSkipThreshold();
    QVERIFY(ef3->m_skipThreshold == (M_PI * 2.0) * 0.5);

    ef4->updateSkipThreshold();
    QVERIFY(ef4->m_skipThreshold == (M_PI * 2.0) * 0.75);
}

void EFXFixture_Test::isValid()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    QVERIFY(ef.isValid() == false);

    ef.setFixture(0);
    QVERIFY(ef.isValid() == true);
}

void EFXFixture_Test::reset()
{
    EFX e(m_doc);

    EFXFixture* ef1 = new EFXFixture(&e);
    ef1->setFixture(1);
    ef1->setSerialNumber(0);
    e.addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(&e);
    ef2->setFixture(2);
    ef2->setSerialNumber(1);
    e.addFixture(ef2);

    EFXFixture* ef3 = new EFXFixture(&e);
    ef3->setFixture(3);
    ef3->setSerialNumber(2);
    e.addFixture(ef3);

    EFXFixture* ef4 = new EFXFixture(&e);
    ef4->m_fixture = 4;
    ef4->m_direction = EFX::Forward;
    ef4->m_serialNumber = 3;
    ef4->m_runTimeDirection = EFX::Backward;
    ef4->m_ready = true;
    ef4->m_iterator = 313.37;
    ef4->m_skipIterator = 4.2;
    ef4->m_skipThreshold = 6.9;
    ef4->m_panValue = 127.15;
    ef4->m_tiltValue = 240.99;
    e.addFixture(ef4);

    ef1->reset();
    QVERIFY(ef1->m_fixture == 1);
    QVERIFY(ef1->m_direction == EFX::Forward);
    QVERIFY(ef1->m_serialNumber == 0);
    QVERIFY(ef1->m_runTimeDirection == EFX::Forward);
    QVERIFY(ef1->m_ready == false);
    QVERIFY(ef1->m_iterator == 0);
    QVERIFY(ef1->m_skipIterator == 0);
    QVERIFY(ef1->m_skipThreshold == (M_PI * 2.0) * 0);
    QVERIFY(ef1->m_panValue == 0);
    QVERIFY(ef1->m_tiltValue == 0);

    ef2->reset();
    QVERIFY(ef2->m_fixture == 2);
    QVERIFY(ef2->m_direction == EFX::Forward);
    QVERIFY(ef2->m_serialNumber == 1);
    QVERIFY(ef2->m_runTimeDirection == EFX::Forward);
    QVERIFY(ef2->m_ready == false);
    QVERIFY(ef2->m_iterator == 0);
    QVERIFY(ef2->m_skipIterator == 0);
    QVERIFY(ef2->m_skipThreshold == (M_PI * 2.0) * 0.25);
    QVERIFY(ef2->m_panValue == 0);
    QVERIFY(ef2->m_tiltValue == 0);

    ef3->reset();
    QVERIFY(ef3->m_fixture == 3);
    QVERIFY(ef3->m_direction == EFX::Forward);
    QVERIFY(ef3->m_serialNumber == 2);
    QVERIFY(ef3->m_runTimeDirection == EFX::Forward);
    QVERIFY(ef3->m_ready == false);
    QVERIFY(ef3->m_iterator == 0);
    QVERIFY(ef3->m_skipIterator == 0);
    QVERIFY(ef3->m_skipThreshold == (M_PI * 2.0) * 0.5);
    QVERIFY(ef3->m_panValue == 0);
    QVERIFY(ef3->m_tiltValue == 0);

    ef4->reset();
    QVERIFY(ef4->m_fixture == 4);
    QVERIFY(ef4->m_direction == EFX::Forward);
    QVERIFY(ef4->m_serialNumber == 3);
    QVERIFY(ef4->m_runTimeDirection == EFX::Forward);
    QVERIFY(ef4->m_ready == false);
    QVERIFY(ef4->m_iterator == 0);
    QVERIFY(ef4->m_skipIterator == 0);
    QVERIFY(ef4->m_skipThreshold == (M_PI * 2.0) * 0.75);
    QVERIFY(ef4->m_panValue == 0);
    QVERIFY(ef4->m_tiltValue == 0);
}

void EFXFixture_Test::setPoint8bit()
{
    EFX e(m_doc);
    EFXFixture ef(&e);
    ef.setFixture(0);

    ef.m_panValue = 5.4; /* MSB: 5, LSB: 0.4 (102) */
    ef.m_tiltValue = 1.5; /* MSB: 1, LSB: 0.5 (127) */

    UniverseArray array(512 * 4);
    ef.setPoint(&array);
    QVERIFY(array.preGMValues()[0] == (char) 5);
    QVERIFY(array.preGMValues()[1] == (char) 1);
}

void EFXFixture_Test::setPoint16bit()
{
    EFX e(m_doc);
    EFXFixture ef(&e);
    ef.setFixture(0);

    ef.m_panValue = 5.4; /* MSB: 5, LSB: 0.4 (102) */
    ef.m_tiltValue = 1.5; /* MSB: 1, LSB: 0.5 (127) */

    UniverseArray array(512 * 4);
    ef.setPoint(&array);
    QVERIFY(array.preGMValues()[0] == (char) 5);
    QVERIFY(array.preGMValues()[1] == (char) 1);
    QVERIFY(array.preGMValues()[2] == (char) 102); /* 255 * 0.4 */
    QVERIFY(array.preGMValues()[3] == (char) 127); /* 255 * 0.5 */
}

void EFXFixture_Test::nextStepLoop()
{
    UniverseArray array(512 * 4);
    MasterTimerStub mts(m_doc, array);

    EFX e(m_doc);
    Bus::instance()->setValue(e.busID(), 50); /* 50 steps */

    EFXFixture* ef = new EFXFixture(&e);
    ef->setFixture(0);
    e.addFixture(ef);

    /* Initialize the EFXFixture so that it can do math */
    ef->setSerialNumber(0);
    QVERIFY(ef->isValid() == true);
    QVERIFY(ef->isReady() == false);
    QVERIFY(ef->m_iterator == 0);

    e.preRun(&mts);

    /* Run two cycles (2 * 50 = 100) and reset the checking iterator in
       the middle to expect correct iterator values. */
    qreal checkIter = 0;
    for (int i = 0; i < 100; i++)
    {
        ef->nextStep(&mts, &array);

        quint32 busValue = Bus::instance()->value(e.busID());
        qreal stepSize = qreal(1) / (qreal(busValue) / qreal(M_PI * 2));
        checkIter += stepSize;

        if (i == 50)
            checkIter = 0;

        QCOMPARE(ef->m_iterator, checkIter);
        QVERIFY(ef->isReady() == false); // Loop is never ready
    }

    e.postRun(&mts, &array);
}

void EFXFixture_Test::nextStepSingleShot()
{
    UniverseArray array(512 * 4);
    MasterTimerStub mts(m_doc, array);

    EFX e(m_doc);
    Bus::instance()->setValue(e.busID(), 50); /* 50 steps */
    e.setRunOrder(EFX::SingleShot);

    EFXFixture* ef = new EFXFixture(&e);
    ef->setFixture(0);
    e.addFixture(ef);

    /* Initialize the EFXFixture so that it can do math */
    ef->setSerialNumber(0);
    QVERIFY(ef->isValid() == true);
    QVERIFY(ef->isReady() == false);
    QVERIFY(ef->m_iterator == 0);

    e.preRun(&mts);

    ef->reset();

    /* Run one cycle (50 steps) */
    qreal checkIter = 0;
    for (int i = 0; i < 50; i++)
    {
        ef->nextStep(&mts, &array);

        quint32 busValue = Bus::instance()->value(e.busID());
        qreal stepSize = qreal(1) / (qreal(busValue) / qreal(M_PI * 2));
        checkIter += stepSize;

        QVERIFY(ef->m_iterator == checkIter);
        QVERIFY(ef->isReady() == false);
    }

    ef->nextStep(&mts, &array);

    /* Single-shot EFX should now be ready */
    QVERIFY(ef->isReady() == true);

    e.postRun(&mts, &array);
}

void EFXFixture_Test::start()
{
    UniverseArray array(512 * 4);
    MasterTimerStub mts(m_doc, array);

    EFX e(m_doc);
    EFXFixture* ef = new EFXFixture(&e);
    ef->setFixture(0);
    e.addFixture(ef);

    Fixture* fxi = m_doc->fixture(0);
    QVERIFY(fxi != NULL);

    e.preRun(&mts);

    // Fade intensity == 0, no need to do fade-in
    ef->setFadeIntensity(0);
    ef->start(&mts, &array);
    QCOMPARE(e.m_fader->m_channels.size(), 0);
    ef->m_started = false;

    // Fade intensity > 0, need to do fade-in
    ef->setFadeIntensity(1);
    ef->start(&mts, &array);
    QCOMPARE(e.m_fader->m_channels.size(), 1);
    QVERIFY(e.m_fader->m_channels.contains(fxi->masterIntensityChannel()) == true);

    e.postRun(&mts, &array);
}

void EFXFixture_Test::stop()
{
    UniverseArray array(512 * 4);
    MasterTimerStub mts(m_doc, array);

    EFX e(m_doc);
    EFXFixture* ef = new EFXFixture(&e);
    ef->setFixture(0);
    e.addFixture(ef);

    Fixture* fxi = m_doc->fixture(0);
    QVERIFY(fxi != NULL);

    e.preRun(&mts);

    // Not started yet
    ef->stop(&mts, &array);
    QCOMPARE(e.m_fader->m_channels.size(), 0);
    QCOMPARE(mts.fader()->m_channels.size(), 0);

    // Start
    ef->start(&mts, &array);
    QCOMPARE(e.m_fader->m_channels.size(), 1);
    QVERIFY(e.m_fader->m_channels.contains(fxi->masterIntensityChannel()) == true);

    // Then stop
    ef->stop(&mts, &array);
    QCOMPARE(e.m_fader->m_channels.size(), 0);

    // FadeChannels are handed over to MasterTimer's GenericFader
    QCOMPARE(mts.fader()->m_channels.size(), 1);
    QVERIFY(e.m_fader->m_channels.contains(fxi->masterIntensityChannel()) == false);
    QVERIFY(mts.m_fader->m_channels.contains(fxi->masterIntensityChannel()) == true);

    e.postRun(&mts, &array);
}
