/*
  Q Light Controller
  addfixture_test.cpp

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
#include <QList>

#include "qlcfixturedefcache.h"
#include "fixture.h"
#include "doc.h"
#include "addfixture_test.h"
#define protected public
#include "addfixture.h"
#undef protected

void AddFixture_Test::findAddress()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);
    QList <Fixture*> fixtures;

    /* All addresses are available (except for fixtures taking more than
       one complete universe). */
    QVERIFY(AddFixture::findAddress(15, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(0, fixtures, 4) == QLCChannel::invalid());
    QVERIFY(AddFixture::findAddress(512, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(513, fixtures, 4) == QLCChannel::invalid());

    Fixture* f1 = new Fixture(&doc);
    f1->setChannels(15);
    f1->setAddress(10);
    fixtures << f1;

    /* There's a fixture taking 15 channels (10-24) */
    QVERIFY(AddFixture::findAddress(10, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 25);

    Fixture* f2 = new Fixture(&doc);
    f2->setChannels(15);
    f2->setAddress(10);
    fixtures << f2;

    /* Now there are two fixtures at the same address, with all channels
       overlapping. */
    QVERIFY(AddFixture::findAddress(10, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 25);

    /* Now only some channels overlap (f2: 0-14, f1: 10-24) */
    f2->setAddress(0);
    QVERIFY(AddFixture::findAddress(1, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(10, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 25);

    Fixture* f3 = new Fixture(&doc);
    f3->setChannels(5);
    f3->setAddress(30);
    fixtures << f3;

    /* Next free slot for max 5 channels is between 25 and 30 */
    QVERIFY(AddFixture::findAddress(1, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(5, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(6, fixtures, 4) == 35);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 35);

    /* Next free slot is found only from the next universe */
    QVERIFY(AddFixture::findAddress(500, fixtures, 4) == 512);

    while (fixtures.isEmpty() == false)
        delete fixtures.takeFirst();
}

QTEST_MAIN(AddFixture_Test)
