/*
  Q Light Controller
  wing_test.cpp

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

#define protected public
#include "wing.h"
#undef protected

#include "wing_test.h"

WingStub::WingStub(QObject* parent, const QHostAddress& host, const QByteArray& ba)
    : Wing(parent, host, ba)
{
    Q_UNUSED(parent);
    Q_UNUSED(host);
    Q_UNUSED(ba);
}

WingStub::~WingStub()
{
}

QString WingStub::name() const
{
    return QString("WingStub");
}

void WingStub::parseData(const QByteArray& ba)
{
    Q_UNUSED(ba);
}

void Wing_Test::resolveType()
{
    QByteArray ba;
    QCOMPARE(Wing::resolveType(ba), Wing::Unknown);

    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));

    QCOMPARE(Wing::resolveType(ba), Wing::Unknown);

    ba[5] = char(0x01);
    QCOMPARE(Wing::resolveType(ba), Wing::Playback);

    ba[5] = char(0x02);
    QCOMPARE(Wing::resolveType(ba), Wing::Shortcut);

    ba[5] = char(0x03);
    QCOMPARE(Wing::resolveType(ba), Wing::Program);
}

void Wing_Test::resolveFirmware()
{
    QByteArray ba;
    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x00));

    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));

    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x00));

    ba[4] = char(0x01);
    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x01));

    ba[4] = char(0x54);
    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x54));
}

void Wing_Test::isOutputData()
{
    QByteArray ba;
    QCOMPARE(Wing::isOutputData(ba), false);

    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    QCOMPARE(Wing::isOutputData(ba), false);

    ba[0] = 'W';
    ba[1] = 'O';
    ba[2] = 'D';
    ba[3] = 'D';
    QCOMPARE(Wing::isOutputData(ba), true);

    ba[0] = 'V';
    ba[1] = 'O';
    ba[2] = 'D';
    ba[3] = 'D';
    QCOMPARE(Wing::isOutputData(ba), false);

    ba[0] = 'W';
    ba[1] = 'O';
    ba[2] = 'T';
    ba[3] = 'D';
    QCOMPARE(Wing::isOutputData(ba), false);
}

void Wing_Test::initial()
{
    QHostAddress addr("192.168.1.5");
    QByteArray ba;
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x80));
    ba.append(char(0x02));

    WingStub es(this, addr, ba);
    QCOMPARE(es.address(), addr);
    QCOMPARE(es.type(), Wing::Shortcut);
    QCOMPARE(es.firmware(), uchar(0x80));
    QCOMPARE(es.page(), uchar(0));

    // Just for coverage :)
    es.feedBack(0, 1);
}

void Wing_Test::page()
{
    QHostAddress addr;
    QByteArray ba;
    WingStub es(this, addr, ba);

    uchar i;
    for (i = 0; i < 99; i++)
    {
        QCOMPARE(es.page(), i);
        es.nextPage();
    }

    es.nextPage();
    QCOMPARE(es.page(), uchar(0));

    es.previousPage();
    QCOMPARE(es.page(), uchar(99));

    for (i = 99; i > 0; i--)
    {
        QCOMPARE(es.page(), i);
        es.previousPage();
    }

    es.previousPage();
    QCOMPARE(es.page(), uchar(99));
}

void Wing_Test::bcd()
{
    QCOMPARE(Wing::toBCD(uchar(99)), uchar(0x99));
    QCOMPARE(Wing::toBCD(uchar(17)), uchar(0x17));
    QCOMPARE(Wing::toBCD(uchar(0)), uchar(0x00));
    QCOMPARE(Wing::toBCD(uchar(1)), uchar(0x01));
    QCOMPARE(Wing::toBCD(uchar(10)), uchar(0x10));
}

void Wing_Test::cache()
{
    QHostAddress addr;
    QByteArray ba;
    WingStub es(this, addr, ba);
    es.m_values = QByteArray(3, 0);

    es.setCacheValue(0, uchar(255));
    QCOMPARE(es.cacheValue(0), uchar(255));
    QCOMPARE(es.cacheValue(1), uchar(0));

    es.setCacheValue(1, uchar(255));
    QCOMPARE(es.cacheValue(0), uchar(255));
    QCOMPARE(es.cacheValue(1), uchar(255));

    es.setCacheValue(2, uchar(255));
    QCOMPARE(es.cacheValue(0), uchar(255));
    QCOMPARE(es.cacheValue(1), uchar(255));
    QCOMPARE(es.cacheValue(2), uchar(255));

    es.setCacheValue(3, uchar(255));
    QCOMPARE(es.cacheValue(3), uchar(0));
}
