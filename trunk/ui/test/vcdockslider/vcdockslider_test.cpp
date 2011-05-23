/*
  Q Light Controller
  vcdockslider_test.cpp

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

#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include <QtTest>

#define protected public
#define private public
#include "virtualconsole.h"
#include "vcdockslider.h"
#undef private
#undef protected

#include "qlcfixturedefcache.h"
#include "vcdockslider_test.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "doc.h"
#include "bus.h"

void VCDockSlider_Test::initTestCase()
{
    Bus::init(this);

    VCProperties prop;
    prop.setFadeLimits(0, 10);
    prop.setHoldLimits(0, 10);
    VirtualConsole::s_properties = prop;
}

void VCDockSlider_Test::initial()
{
    InputMap im(this, 4);
    QWidget w;

    Bus::instance()->setValue(Bus::defaultFade(), 42);
    Bus::instance()->setName(Bus::defaultFade(), "Foo");

    VCDockSlider s(&w, &im, Bus::defaultFade());
    QCOMPARE(s.m_bus, Bus::defaultFade());
    QCOMPARE(s.m_inputMap, &im);

    QVERIFY(s.m_valueLabel != NULL);

    QVERIFY(s.m_slider != NULL);
    QCOMPARE(s.m_slider->value(), 42);
    QCOMPARE(s.m_slider->minimum(), 0);
    QCOMPARE(s.m_slider->maximum(), int(MasterTimer::frequency() * 10));

    QVERIFY(s.m_tapButton != NULL);
    QCOMPARE(s.m_tapButton->text(), QString("Foo"));

    Bus::instance()->setValue(Bus::defaultFade(), 123);
    QCOMPARE(s.m_slider->value(), 123);

    Bus::instance()->setName(Bus::defaultFade(), "Bar");
    QCOMPARE(s.m_tapButton->text(), QString("Bar"));

    Bus::instance()->setName(Bus::defaultFade(), QString());
    QCOMPARE(s.m_tapButton->text(), tr("Bus %1").arg(Bus::defaultFade() + 1).replace(" ", "\n"));
}

void VCDockSlider_Test::refreshProperties()
{
    InputMap im(this, 4);
    QWidget w;

    Bus::instance()->setValue(Bus::defaultFade(), 0);
    Bus::instance()->setName(Bus::defaultFade(), "Fade");
    Bus::instance()->setValue(Bus::defaultHold(), 0);
    Bus::instance()->setName(Bus::defaultHold(), "Fade");

    VCDockSlider f(&w, &im, Bus::defaultFade());
    VCDockSlider h(&w, &im, Bus::defaultHold());

    VCProperties prop;
    prop.setFadeLimits(0, 20);
    prop.setHoldLimits(5, 15);
    VirtualConsole::s_properties = prop;

    f.refreshProperties();
    QCOMPARE(f.m_slider->minimum(), 0);
    QCOMPARE(f.m_slider->maximum(), int(MasterTimer::frequency() * 20));

    h.refreshProperties();
    QCOMPARE(h.m_slider->minimum(), int(MasterTimer::frequency() * 5));
    QCOMPARE(h.m_slider->maximum(), int(MasterTimer::frequency() * 15));
}

void VCDockSlider_Test::tap()
{
    InputMap im(this, 4);
    QWidget w;

    const qreal tick = qreal(1) / qreal(MasterTimer::frequency());
    quint32 waitTicks = quint32(qreal(MasterTimer::frequency()) * 0.5);
    quint32 waitMs = quint32(qreal(25) * tick * qreal(1000));

    Bus::instance()->setValue(Bus::defaultFade(), 0);

    VCDockSlider f(&w, &im, Bus::defaultFade());
    QTest::qWait(waitMs);
    f.slotTapButtonClicked();
    QVERIFY(Bus::instance()->value(Bus::defaultFade()) > 0);

    waitTicks = waitTicks * 3;
    waitMs = waitMs * 3;
    QTest::qWait(waitMs);
    f.slotTapButtonClicked();
    QVERIFY(Bus::instance()->value(Bus::defaultFade()) > 0);
}

void VCDockSlider_Test::sliderValue()
{
    InputMap im(this, 4);
    QWidget w;

    VirtualConsole::s_properties.setFadeInputSource(0, 1);
    VirtualConsole::s_properties.setHoldInputSource(2, 3);

    VirtualConsole::s_properties.setFadeLimits(0, 10);
    VirtualConsole::s_properties.setHoldLimits(0, 10);

    VCDockSlider f(&w, &im, Bus::defaultFade());
    VCDockSlider h(&w, &im, Bus::defaultHold());

    f.m_slider->setValue(MasterTimer::frequency());
    QCOMPARE(Bus::instance()->value(Bus::defaultFade()), MasterTimer::frequency());
    //! @todo Test InputMap feedback (mock objects don't work on OSX...?)

    h.m_slider->setValue(MasterTimer::frequency() * 2);
    QCOMPARE(Bus::instance()->value(Bus::defaultHold()), MasterTimer::frequency() * 2);
    //! @todo Test InputMap feedback (mock objects don't work on OSX...?)
}

void VCDockSlider_Test::input()
{
    InputMap im(this, 4);
    QWidget w;

    VirtualConsole::s_properties.setFadeInputSource(InputMap::invalidUniverse(),
                                                    InputMap::invalidChannel());
    VirtualConsole::s_properties.setFadeLimits(0, 10);
    VirtualConsole::s_properties.setHoldInputSource(InputMap::invalidUniverse(),
                                                    InputMap::invalidChannel());
    VirtualConsole::s_properties.setHoldLimits(0, 10);

    VCDockSlider f(&w, &im, Bus::defaultFade());
    f.m_slider->setValue(0);
    f.m_slider->setInvertedAppearance(false);
    VCDockSlider h(&w, &im, Bus::defaultHold());
    h.m_slider->setValue(0);
    h.m_slider->setInvertedAppearance(false);

    h.slotInputValueChanged(0, 0, 255);
    QCOMPARE(f.m_slider->value(), 0);
    h.slotInputValueChanged(0, 0, 255);
    QCOMPARE(h.m_slider->value(), 0);

    VirtualConsole::s_properties.setFadeInputSource(0, 1);
    VirtualConsole::s_properties.setHoldInputSource(2, 3);

    f.slotInputValueChanged(0, 1, 255);
    QCOMPARE(f.m_slider->value(), f.m_slider->maximum());
    h.slotInputValueChanged(2, 3, 255);
    QCOMPARE(h.m_slider->value(), h.m_slider->maximum());

    f.m_slider->setInvertedAppearance(true);
    h.m_slider->setInvertedAppearance(true);

    f.slotInputValueChanged(0, 1, 0);
    QCOMPARE(f.m_slider->value(), f.m_slider->maximum());
    h.slotInputValueChanged(2, 3, 0);
    QCOMPARE(h.m_slider->value(), h.m_slider->maximum());
}

QTEST_MAIN(VCDockSlider_Test)
