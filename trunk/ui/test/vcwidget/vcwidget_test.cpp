/*
  Q Light Controller
  vcwidget_test.cpp

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
#include "virtualconsole.h"
#include "vcwidget.h"
#undef private
#undef protected

#include "qlcfixturedefcache.h"
#include "vcwidget_test.h"
#include "mastertimer.h"
#include "stubwidget.h"
#include "outputmap.h"
#include "inputmap.h"
#include "doc.h"
#include "bus.h"

void VCWidget_Test::initTestCase()
{
    Bus::init(this);
}

void VCWidget_Test::initial()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.objectName(), QString("VCWidget"));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomFont(), false);
    QCOMPARE(stub.frameStyle(), 0);
    QCOMPARE(stub.inputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(stub.inputChannel(), InputMap::invalidChannel());
    QCOMPARE(stub.canHaveChildren(), false);
}

void VCWidget_Test::bgImage()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(spy.size(), 1);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(spy.size(), 2);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(spy.size(), 3);
}

void VCWidget_Test::bgColor()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(spy.size(), 1);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(spy.size(), 2);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(spy.size(), 3);
}

void VCWidget_Test::fgColor()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setForegroundColor(QColor(Qt::blue));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::blue));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::blue));
    QCOMPARE(spy.size(), 1);
}

void VCWidget_Test::resetBg()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::cyan));
    QCOMPARE(spy.size(), 2);

    stub.resetBackgroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::cyan));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::cyan));
    QCOMPARE(spy.size(), 3);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(spy.size(), 4);

    stub.resetBackgroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::cyan));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::cyan));
    QCOMPARE(spy.size(), 5);
}

void VCWidget_Test::resetFg()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::cyan));
    QCOMPARE(spy.size(), 2);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 3);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(spy.size(), 4);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 5);

    stub.resetBackgroundColor();
    QCOMPARE(spy.size(), 6);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 7);
}

void VCWidget_Test::font()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    QFont font(w.font());
    font.setBold(!font.bold());
    QVERIFY(font != w.font());

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setFont(font);
    QCOMPARE(stub.font(), font);
    QCOMPARE(stub.hasCustomFont(), true);
    QCOMPARE(spy.size(), 1);

    stub.resetFont();
    QCOMPARE(stub.font(), w.font());
    QCOMPARE(stub.hasCustomFont(), false);
    QCOMPARE(spy.size(), 2);
}

void VCWidget_Test::caption()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setCaption("Foobar");
    QCOMPARE(stub.caption(), QString("Foobar"));
    QCOMPARE(spy.size(), 1);
}

void VCWidget_Test::frame()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setFrameStyle(KVCFrameStyleSunken);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(spy.size(), 1);

    stub.setFrameStyle(KVCFrameStyleRaised);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleRaised);
    QCOMPARE(spy.size(), 2);

    stub.resetFrameStyle();
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleNone);
    QCOMPARE(spy.size(), 3);

    QCOMPARE(stub.frameStyleToString(KVCFrameStyleSunken), QString("Sunken"));
    QCOMPARE(stub.frameStyleToString(KVCFrameStyleRaised), QString("Raised"));
    QCOMPARE(stub.frameStyleToString(KVCFrameStyleNone), QString("None"));
    QCOMPARE(stub.frameStyleToString(QFrame::Plain), QString("None"));

    QCOMPARE(stub.stringToFrameStyle("Sunken"), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.stringToFrameStyle("Raised"), (int) KVCFrameStyleRaised);
    QCOMPARE(stub.stringToFrameStyle("None"), (int) KVCFrameStyleNone);
    QCOMPARE(stub.stringToFrameStyle("Foo"), (int) KVCFrameStyleNone);
}

void VCWidget_Test::inputSource()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setInputSource(1, 2);
    QCOMPARE(stub.inputUniverse(), quint32(1));
    QCOMPARE(stub.inputChannel(), quint32(2));

    stub.setInputSource(4, 5);
    QCOMPARE(stub.inputUniverse(), quint32(4));
    QCOMPARE(stub.inputChannel(), quint32(5));

    stub.setInputSource(InputMap::invalidUniverse(), InputMap::invalidChannel());
    QCOMPARE(stub.inputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(stub.inputChannel(), InputMap::invalidChannel());
}

QTEST_MAIN(VCWidget_Test)
