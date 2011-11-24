/*
  Q Light Controller - Unit test
  cuestack_test.cpp

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

#include "cuestack_test.h"

#define private public
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "cuestack.h"
#include "fixture.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void CueStack_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void CueStack_Test::cleanupTestCase()
{
    delete m_doc;
}

void CueStack_Test::init()
{
    m_doc->clearContents();
}

void CueStack_Test::initial()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.fadeInSpeed(), uint(0));
    QCOMPARE(cs.fadeOutSpeed(), uint(0));
    QCOMPARE(cs.duration(), uint(UINT_MAX));
    QCOMPARE(cs.cues().size(), 0);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.intensity(), qreal(1.0));
    QCOMPARE(cs.isRunning(), false);
    QCOMPARE(cs.isStarted(), false);
    QCOMPARE(cs.m_fader, (GenericFader*) NULL);
    QCOMPARE(cs.m_elapsed, uint(0));
    QCOMPARE(cs.m_previous, false);
    QCOMPARE(cs.m_next, false);
    QCOMPARE(cs.doc(), m_doc);
}

void CueStack_Test::speeds()
{
    CueStack cs(m_doc);

    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);
    QCOMPARE(cs.fadeInSpeed(), uint(100));
    QCOMPARE(cs.fadeOutSpeed(), uint(200));
    QCOMPARE(cs.duration(), uint(300));
}

void CueStack_Test::appendCue()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.cues().size(), 0);

    cs.appendCue(Cue("One"));
    QCOMPARE(cs.cues().size(), 1);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));

    cs.appendCue(Cue("Two"));
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));

    cs.appendCue(Cue("Three"));
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));

    cs.appendCue(Cue("Four"));
    QCOMPARE(cs.cues().size(), 4);
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));
    QCOMPARE(cs.cues().at(3).name(), QString("Four"));
}

void CueStack_Test::currentIndex()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.appendCue(Cue("Four"));
    cs.appendCue(Cue("Five"));

    cs.setCurrentIndex(-1);
    QCOMPARE(cs.currentIndex(), -1);

    cs.setCurrentIndex(-2);
    QCOMPARE(cs.currentIndex(), -1);

    cs.setCurrentIndex(1);
    QCOMPARE(cs.currentIndex(), 1);

    cs.setCurrentIndex(2);
    QCOMPARE(cs.currentIndex(), 2);

    cs.setCurrentIndex(4);
    QCOMPARE(cs.currentIndex(), 4);

    cs.setCurrentIndex(3);
    QCOMPARE(cs.currentIndex(), 3);

    cs.setCurrentIndex(5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.setCurrentIndex(-1);
    QCOMPARE(cs.currentIndex(), -1);

    cs.nextCue();
    QCOMPARE(cs.m_previous, false);
    QCOMPARE(cs.m_next, true);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);

    cs.nextCue();
    QCOMPARE(cs.m_previous, false);
    QCOMPARE(cs.m_next, true);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);

    cs.m_running = false;
    cs.m_next = false;

    cs.previousCue();
    QCOMPARE(cs.m_previous, true);
    QCOMPARE(cs.m_next, false);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);

    cs.previousCue();
    QCOMPARE(cs.m_previous, true);
    QCOMPARE(cs.m_next, false);
    QCOMPARE(cs.currentIndex(), -1);
    QCOMPARE(cs.isRunning(), true);
    QCOMPARE(cs.isStarted(), false);
}

void CueStack_Test::removeCue()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.appendCue(Cue("Four"));
    cs.appendCue(Cue("Five"));
    QCOMPARE(cs.cues().size(), 5);
    cs.setCurrentIndex(4);

    cs.removeCue(-1);
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.removeCue(5);
    QCOMPARE(cs.cues().size(), 5);
    QCOMPARE(cs.currentIndex(), 4);

    cs.removeCue(3);
    QCOMPARE(cs.cues().size(), 4);
    QCOMPARE(cs.currentIndex(), 3); // currentIndex-- because a cue before it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("One"));
    QCOMPARE(cs.cues().at(1).name(), QString("Two"));
    QCOMPARE(cs.cues().at(2).name(), QString("Three"));
    QCOMPARE(cs.cues().at(3).name(), QString("Five"));

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.currentIndex(), 2); // currentIndex-- because a cue before it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Three"));
    QCOMPARE(cs.cues().at(2).name(), QString("Five"));

    cs.setCurrentIndex(0);

    cs.removeCue(2);
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.currentIndex(), 0); // no change because a cue AFTER it was removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Three"));

    cs.removeCue(2);
    QCOMPARE(cs.cues().size(), 2);
    QCOMPARE(cs.currentIndex(), 0); // no change because nothing was removed
    QCOMPARE(cs.cues().at(0).name(), QString("Two"));
    QCOMPARE(cs.cues().at(1).name(), QString("Three"));

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 1);
    QCOMPARE(cs.currentIndex(), 0); // currentIndex was removed -> next cue becomes current
    QCOMPARE(cs.cues().at(0).name(), QString("Three"));

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 0);

    cs.removeCue(0);
    QCOMPARE(cs.cues().size(), 0);
}

void CueStack_Test::load()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("CueStack");
    doc.appendChild(root);

    uint id = 0;
    CueStack cs(m_doc);
    QCOMPARE(cs.loadXML(root, id), false);

    root.setAttribute("ID", 15);
    QCOMPARE(cs.loadXML(root, id), true);
    QCOMPARE(id, uint(15));
    QCOMPARE(cs.cues().size(), 0);

    QDomElement speed = doc.createElement("Speed");
    speed.setAttribute("FadeIn", 100);
    speed.setAttribute("FadeOut", 200);
    speed.setAttribute("Duration", 300);
    root.appendChild(speed);

    QDomElement cue = doc.createElement("Cue");
    cue.setAttribute("Name", "First");
    root.appendChild(cue);

    cue = doc.createElement("Cue");
    cue.setAttribute("Name", "Second");
    root.appendChild(cue);

    cue = doc.createElement("Cue");
    cue.setAttribute("Name", "Third");
    root.appendChild(cue);

    // Extra garbage
    QDomElement foo = doc.createElement("Foo");
    root.appendChild(foo);

    id = 0;
    QCOMPARE(cs.loadXML(root, id), true);
    QCOMPARE(id, uint(15));
    QCOMPARE(cs.cues().size(), 3);
    QCOMPARE(cs.cues().at(0).name(), QString("First"));
    QCOMPARE(cs.cues().at(1).name(), QString("Second"));
    QCOMPARE(cs.cues().at(2).name(), QString("Third"));

    QCOMPARE(cs.loadXML(foo, id), false);
}

void CueStack_Test::save()
{
    CueStack cs(m_doc);
    cs.appendCue(Cue("One"));
    cs.appendCue(Cue("Two"));
    cs.appendCue(Cue("Three"));
    cs.setFadeInSpeed(200);
    cs.setFadeOutSpeed(300);
    cs.setDuration(400);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    doc.appendChild(root);

    QCOMPARE(cs.saveXML(&doc, &root, 42), true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("CueStack"));

    int cue = 0, speed = 0;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Speed")
        {
            speed++;
            QCOMPARE(tag.attribute("FadeIn"), QString("200"));
            QCOMPARE(tag.attribute("FadeOut"), QString("300"));
            QCOMPARE(tag.attribute("Duration"), QString("400"));
        }
        else if (tag.tagName() == "Cue")
        {
            // The contents of a Cue tag are tested in Cue tests
            cue++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(cue, 3);
    QCOMPARE(speed, 1);
}

void CueStack_Test::preRun()
{
    CueStack cs(m_doc);
    QVERIFY(cs.m_fader == NULL);
    QCOMPARE(cs.isStarted(), false);

    cs.m_elapsed = 500;
    QSignalSpy spy(&cs, SIGNAL(started()));
    cs.preRun();
    QVERIFY(cs.m_fader != NULL);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(cs.m_elapsed, uint(0));
    QCOMPARE(cs.m_fader->intensity(), qreal(1.0));
    QCOMPARE(cs.isStarted(), true);
}

void CueStack_Test::startStop()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.isRunning(), false);
    cs.start();
    QCOMPARE(cs.isRunning(), true);
    cs.stop();
    QCOMPARE(cs.isRunning(), false);
    cs.start();
    QCOMPARE(cs.isRunning(), true);
}

void CueStack_Test::intensity()
{
    CueStack cs(m_doc);

    cs.adjustIntensity(0.5);
    QCOMPARE(cs.intensity(), qreal(0.5));

    cs.preRun();
    QCOMPARE(cs.intensity(), qreal(0.5));
    QCOMPARE(cs.m_fader->intensity(), qreal(0.5));
}

void CueStack_Test::nextPrevious()
{
    CueStack cs(m_doc);
    QCOMPARE(cs.previous(), -1);
    QCOMPARE(cs.next(), -1);

    cs.appendCue(Cue("One"));
    QCOMPARE(cs.previous(), 0);
    QCOMPARE(cs.next(), 0);

    cs.appendCue(Cue("Two"));
    QCOMPARE(cs.previous(), 1);
    QCOMPARE(cs.next(), 0);
    QCOMPARE(cs.next(), 1);
    QCOMPARE(cs.next(), 0);
    QCOMPARE(cs.previous(), 1);
    QCOMPARE(cs.previous(), 0);

    cs.appendCue(Cue("Three"));
    QCOMPARE(cs.next(), 1);
    QCOMPARE(cs.next(), 2);
    QCOMPARE(cs.next(), 0);
    QCOMPARE(cs.previous(), 2);
    QCOMPARE(cs.previous(), 1);
    QCOMPARE(cs.previous(), 0);
}

void CueStack_Test::insertStartValue()
{
    UniverseArray ua(512);
    CueStack cs(m_doc);
    cs.preRun();

    FadeChannel fc;
    fc.setChannel(0);
    fc.setStart(0);
    fc.setTarget(255);
    fc.setCurrent(127);

    cs.m_fader->add(fc);

    fc.setTarget(64);
    cs.insertStartValue(fc, &ua);
    QCOMPARE(fc.start(), uchar(127));
    QCOMPARE(fc.current(), uchar(127));

    cs.m_fader->remove(fc);

    // HTP channel in universes
    ua.write(0, 192);
    cs.insertStartValue(fc, &ua);
    QCOMPARE(fc.start(), uchar(0));
    QCOMPARE(fc.current(), uchar(0));

    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Scanner");
    fxi->setAddress(0);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    // LTP channel (Pan) in universes
    ua.write(0, 192);
    cs.insertStartValue(fc, &ua);
    QCOMPARE(fc.start(), uchar(192));
    QCOMPARE(fc.current(), uchar(192));
}

void CueStack_Test::switchCue()
{
    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Scanner");
    fxi->setAddress(10);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    UniverseArray ua(512);
    CueStack cs(m_doc);
    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);

    Cue cue;
    cue.setName("One");
    cue.setValue(0, 255);
    cue.setValue(1, 255);
    cue.setValue(500, 255);
    cue.setValue(10, 255); // LTP
    cue.setValue(11, 255); // LTP
    cs.appendCue(cue);

    cue = Cue();

    cue.setName("Two");
    cue.setValue(500, 255);
    cue.setValue(3, 255);
    cue.setValue(4, 255);
    cue.setValue(11, 255); // LTP
    cs.appendCue(cue);

    cs.preRun();

    // Do nothing with invalid cue indices
    cs.switchCue(-1, &ua);
    QCOMPARE(cs.m_fader->channels().size(), 0);
    cs.switchCue(3, &ua);
    QCOMPARE(cs.m_fader->channels().size(), 0);

    // Switch to cue one
    cs.switchCue(0, &ua);
    QCOMPARE(cs.m_fader->channels().size(), 5);

    FadeChannel fc;
    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(10);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(10));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(11);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(11));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(500);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(500));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(3);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), QLCChannel::invalid());
    fc.setChannel(4);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), QLCChannel::invalid());

    fc.setChannel(0);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(1);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(10);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(11);
    cs.m_fader->m_channels[fc].setCurrent(127);
    fc.setChannel(500);
    cs.m_fader->m_channels[fc].setCurrent(127);

    // Switch to cue two
    cs.switchCue(1, &ua);
    QCOMPARE(cs.m_fader->channels().size(), 6);

    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());

    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());

    fc.setChannel(10); // LTP channel no longer in fader
    QCOMPARE(cs.m_fader->channels()[fc].channel(), QLCChannel::invalid());

    fc.setChannel(11); // LTP channel also in the next cue
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(11));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(500);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(500));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(3);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(3));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    fc.setChannel(4);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(4));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeInSpeed());

    // Stop
    cs.switchCue(-1, &ua);
    QCOMPARE(cs.m_fader->channels().size(), 5);

    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());

    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());

    fc.setChannel(500);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(127));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(500));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());

    fc.setChannel(3);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(3));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());

    fc.setChannel(4);
    QCOMPARE(cs.m_fader->channels()[fc].start(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].current(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(4));
    QCOMPARE(cs.m_fader->channels()[fc].fadeTime(), cs.fadeOutSpeed());
}

void CueStack_Test::postRun()
{
    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Scanner");
    fxi->setAddress(10);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    MasterTimer mt(m_doc);
    UniverseArray ua(512);
    CueStack cs(m_doc);
    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);

    Cue cue;
    cue.setName("One");
    cue.setValue(0, 255);
    cue.setValue(1, 255);
    cue.setValue(500, 255);
    cue.setValue(10, 255); // LTP
    cue.setValue(11, 255); // LTP
    cs.appendCue(cue);

    cue = Cue();

    cue.setName("Two");
    cue.setValue(500, 255);
    cue.setValue(3, 255);
    cue.setValue(4, 255);
    cue.setValue(11, 255); // LTP
    cs.appendCue(cue);

    cs.preRun();

    // Switch to cue one
    cs.switchCue(0, &ua);
    QCOMPARE(cs.m_fader->channels().size(), 5);

    QSignalSpy cueSpy(&cs, SIGNAL(currentCueChanged(int)));
    QSignalSpy stopSpy(&cs, SIGNAL(stopped()));

    cs.postRun(&mt);
    QCOMPARE(cs.m_fader, (GenericFader*) NULL);
    QCOMPARE(cs.m_currentIndex, -1);
    QCOMPARE(cueSpy.size(), 1);
    QCOMPARE(cueSpy.at(0).size(), 1);
    QCOMPARE(cueSpy.at(0).at(0).toInt(), -1);
    QCOMPARE(stopSpy.size(), 1);

    // Only HTP channels go to MasterTimer's GenericFader
    QCOMPARE(mt.m_fader->channels().size(), 3);
    FadeChannel fc;
    fc.setChannel(0);
    QCOMPARE(mt.m_fader->channels().contains(fc), true);
    fc.setChannel(1);
    QCOMPARE(mt.m_fader->channels().contains(fc), true);
    fc.setChannel(500);
    QCOMPARE(mt.m_fader->channels().contains(fc), true);
}

void CueStack_Test::write()
{
    UniverseArray ua(512);

    CueStack cs(m_doc);
    cs.setFadeInSpeed(100);
    cs.setFadeOutSpeed(200);
    cs.setDuration(300);

    Cue cue("One");
    cue.setValue(0, 255);
    cs.appendCue(cue);

    cue = Cue("Two");
    cue.setValue(1, 255);
    cs.appendCue(cue);

    cs.preRun();
    QVERIFY(cs.m_fader != NULL);

    cs.write(&ua);
    QCOMPARE(cs.currentIndex(), -1);

    cs.start();
    cs.write(&ua);
    QCOMPARE(cs.currentIndex(), -1);

    cs.nextCue();
    QCOMPARE(cs.currentIndex(), -1);
    cs.write(&ua);
    QCOMPARE(cs.currentIndex(), 0);
    QCOMPARE(cs.m_fader->channels().size(), 1);
    FadeChannel fc;
    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));

    cs.previousCue();
    QCOMPARE(cs.currentIndex(), 0);
    cs.write(&ua);
    QCOMPARE(cs.currentIndex(), 1);
    fc.setChannel(0);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(0));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(0));
    fc.setChannel(1);
    QCOMPARE(cs.m_fader->channels()[fc].channel(), uint(1));
    QCOMPARE(cs.m_fader->channels()[fc].target(), uchar(255));
}

QTEST_APPLESS_MAIN(CueStack_Test)
