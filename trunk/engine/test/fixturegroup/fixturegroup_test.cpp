/*
  Q Light Controller - Unit test
  fixturegroup_test.cpp

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

#include "fixturegroup_test.h"
#include "fixturegroup.h"
#include "doc.h"

void FixtureGroup_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void FixtureGroup_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void FixtureGroup_Test::init()
{
    m_doc->clearContents();
}

void FixtureGroup_Test::id()
{
    FixtureGroup grp(m_doc);
    QCOMPARE(grp.id(), FixtureGroup::invalidId());
    grp.setId(69);
    QCOMPARE(grp.id(), quint32(69));
    grp.setId(42);
    QCOMPARE(grp.id(), quint32(42));
}

void FixtureGroup_Test::name()
{
    FixtureGroup grp(m_doc);
    QCOMPARE(grp.name(), QString());
    grp.setName("Esko Mörkö");
    QCOMPARE(grp.name(), QString("Esko Mörkö"));
    grp.setName("Pertti Pasanen");
    QCOMPARE(grp.name(), QString("Pertti Pasanen"));
}

void FixtureGroup_Test::size()
{
    FixtureGroup grp(m_doc);
    QCOMPARE(grp.size(), QSize());
    grp.setSize(QSize(10, 10));
    QCOMPARE(grp.size(), QSize(10, 10));
    grp.setSize(QSize(20, 30));
    QCOMPARE(grp.size(), QSize(20, 30));
}

void FixtureGroup_Test::assignFixtureNoSize()
{
    QLCPoint pt;
    FixtureGroup grp(m_doc);
    QCOMPARE(grp.fixtureList().size(), 0);

    grp.assignFixture(0);
    QCOMPARE(grp.fixtureList().size(), 1);
    QCOMPARE(grp.size(), QSize(1, 1));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixtureHash()[pt] == quint32(0));

    // Same fixture can't be at two places
    grp.assignFixture(0, QLCPoint(100, 100));
    QCOMPARE(grp.fixtureList().size(), 1);
    QCOMPARE(grp.size(), QSize(1, 1));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixtureHash()[pt] == quint32(0));

    grp.assignFixture(1);
    QCOMPARE(grp.fixtureList().size(), 2);
    QCOMPARE(grp.size(), QSize(1, 1));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixtureHash()[pt] == quint32(0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixtureHash()[pt] == quint32(1));

    grp.assignFixture(2);
    QCOMPARE(grp.fixtureList().size(), 3);
    QCOMPARE(grp.size(), QSize(1, 1));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixtureHash()[pt] == quint32(0));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixtureHash()[pt] == quint32(1));
    pt = QLCPoint(0, 2);
    QVERIFY(grp.fixtureHash()[pt] == quint32(2));
}

void FixtureGroup_Test::assignFixture4x2()
{
    QLCPoint pt;
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 2));
    QCOMPARE(grp.fixtureList().size(), 0);

    grp.assignFixture(0);
    QCOMPARE(grp.fixtureList().size(), 1);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));

    grp.assignFixture(1);
    QCOMPARE(grp.fixtureList().size(), 2);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));

    grp.assignFixture(2);
    QCOMPARE(grp.fixtureList().size(), 3);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));

    grp.assignFixture(3);
    QCOMPARE(grp.fixtureList().size(), 4);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));

    grp.assignFixture(4);
    QCOMPARE(grp.fixtureList().size(), 5);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixture(pt) == quint32(4));

    grp.assignFixture(5);
    QCOMPARE(grp.fixtureList().size(), 6);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixture(pt) == quint32(4));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.fixture(pt) == quint32(5));

    grp.assignFixture(6);
    QCOMPARE(grp.fixtureList().size(), 7);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixture(pt) == quint32(4));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.fixture(pt) == quint32(5));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.fixture(pt) == quint32(6));

    grp.assignFixture(7);
    QCOMPARE(grp.fixtureList().size(), 8);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixture(pt) == quint32(4));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.fixture(pt) == quint32(5));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.fixture(pt) == quint32(6));
    pt = QLCPoint(3, 1);
    QVERIFY(grp.fixture(pt) == quint32(7));

    // Now beyond size(); should continue to make a third row of 4 columns
    grp.assignFixture(8);
    QCOMPARE(grp.fixtureList().size(), 9);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixture(pt) == quint32(4));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.fixture(pt) == quint32(5));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.fixture(pt) == quint32(6));
    pt = QLCPoint(3, 1);
    QVERIFY(grp.fixture(pt) == quint32(7));
    pt = QLCPoint(0, 2);
    QVERIFY(grp.fixture(pt) == quint32(8));

    grp.assignFixture(9);
    QCOMPARE(grp.fixtureList().size(), 10);
    QCOMPARE(grp.size(), QSize(4, 2));
    pt = QLCPoint(0, 0);
    QVERIFY(grp.fixture(pt) == quint32(0));
    pt = QLCPoint(1, 0);
    QVERIFY(grp.fixture(pt) == quint32(1));
    pt = QLCPoint(2, 0);
    QVERIFY(grp.fixture(pt) == quint32(2));
    pt = QLCPoint(3, 0);
    QVERIFY(grp.fixture(pt) == quint32(3));
    pt = QLCPoint(0, 1);
    QVERIFY(grp.fixture(pt) == quint32(4));
    pt = QLCPoint(1, 1);
    QVERIFY(grp.fixture(pt) == quint32(5));
    pt = QLCPoint(2, 1);
    QVERIFY(grp.fixture(pt) == quint32(6));
    pt = QLCPoint(3, 1);
    QVERIFY(grp.fixture(pt) == quint32(7));
    pt = QLCPoint(0, 2);
    QVERIFY(grp.fixture(pt) == quint32(8));
    pt = QLCPoint(1, 2);
    QVERIFY(grp.fixture(pt) == quint32(9));

    // Going waaay beyond size should be possible
    pt = QLCPoint(1024, 2048);
    grp.assignFixture(10, pt);
    QVERIFY(grp.fixtureHash().contains(pt) == true);
    QCOMPARE(grp.fixture(pt), quint32(10));
    QCOMPARE(grp.size(), QSize(4, 2));
}

void FixtureGroup_Test::resignFixture()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
        grp.assignFixture(id);
    QCOMPARE(grp.fixtureList().size(), 16);

    // Remove a fixture
    grp.resignFixture(13);
    QCOMPARE(grp.fixtureList().size(), 15);
    QVERIFY(grp.fixtureList().contains(13) == false);
    QVERIFY(grp.fixtureHash().contains(QLCPoint(1, 3)) == false);

    // Remove a nonexistent fixture
    grp.resignFixture(42);
    QCOMPARE(grp.fixtureList().size(), 15);
    QVERIFY(grp.fixtureList().contains(42) == false);
    QVERIFY(grp.fixtureHash().contains(QLCPoint(1, 3)) == false);

    // Test that the gap is again filled
    grp.assignFixture(42);
    QCOMPARE(grp.fixtureList().size(), 16);
    QVERIFY(grp.fixtureList().contains(42) == true);
    QVERIFY(grp.fixtureHash().contains(QLCPoint(1, 3)) == true);
    QCOMPARE(grp.fixtureHash()[QLCPoint(1, 3)], quint32(42));
}

void FixtureGroup_Test::fixtureRemoved()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
        grp.assignFixture(id);
    QCOMPARE(grp.fixtureList().size(), 16);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(5);
    m_doc->addFixture(fxi, 10);
    QCOMPARE(fxi->id(), quint32(10));

    // FixtureGroup should listen to Doc's fixtureRemoved() signal
    m_doc->deleteFixture(10);
    QCOMPARE(grp.fixtureList().size(), 15);
    QVERIFY(grp.fixtureHash().contains(QLCPoint(2, 2)) == false);

    fxi = new Fixture(m_doc);
    fxi->setChannels(5);
    m_doc->addFixture(fxi, 69);
    QCOMPARE(fxi->id(), quint32(69));

    // Uninteresting fixture removed (not part of group)
    m_doc->deleteFixture(69);
    QCOMPARE(grp.fixtureList().size(), 15);
}

void FixtureGroup_Test::swap()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
        grp.assignFixture(id);
    QCOMPARE(grp.fixtureList().size(), 16);

    QLCPoint pt1(0, 0);
    QLCPoint pt2(2, 1);
    QVERIFY(grp.fixtureHash().contains(pt1) == true);
    QVERIFY(grp.fixtureHash().contains(pt2) == true);
    QCOMPARE(grp.fixtureHash()[pt1], quint32(0));
    QCOMPARE(grp.fixtureHash()[pt2], quint32(6));

    // Switch places with two fixtures
    grp.swap(pt1, pt2);
    QVERIFY(grp.fixtureHash().contains(pt1) == true);
    QVERIFY(grp.fixtureHash().contains(pt2) == true);
    QCOMPARE(grp.fixtureHash()[pt1], quint32(6));
    QCOMPARE(grp.fixtureHash()[pt2], quint32(0));

    // Switch places with a fixture and an empty point
    pt2 = QLCPoint(500, 500);
    grp.swap(pt1, pt2);
    QVERIFY(grp.fixtureHash().contains(pt1) == false);
    QVERIFY(grp.fixtureHash().contains(pt2) == true);
    QCOMPARE(grp.fixtureHash()[pt2], quint32(6));

    // ...and back again
    grp.swap(pt1, pt2);
    QVERIFY(grp.fixtureHash().contains(pt1) == true);
    QVERIFY(grp.fixtureHash().contains(pt2) == false);
    QCOMPARE(grp.fixtureHash()[pt1], quint32(6));
}

void FixtureGroup_Test::copy()
{
    FixtureGroup grp1(m_doc);
    grp1.setSize(QSize(4, 4));
    grp1.setName("Pertti Pasanen");
    grp1.setId(99);
    for (quint32 id = 0; id < 16; id++)
        grp1.assignFixture(id);
    QCOMPARE(grp1.fixtureList().size(), 16);

    FixtureGroup grp2(m_doc);
    grp2.copyFrom(&grp1);
    QCOMPARE(grp2.size(), QSize(4, 4));
    QCOMPARE(grp2.name(), QString("Pertti Pasanen"));
    QVERIFY(grp2.id() != quint32(99)); // ID must not be copied
    QCOMPARE(grp2.fixtureList().size(), 16);
    for (quint32 id = 0; id < 16; id++)
        QVERIFY(grp2.fixtureList().contains(id) == true);
}

void FixtureGroup_Test::infoText()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 4));
    for (quint32 id = 0; id < 16; id++)
    {
        Fixture* fxi = new Fixture(m_doc);
        m_doc->addFixture(fxi);
        grp.assignFixture(fxi->id());
    }

    grp.resignFixture(0);

    QVERIFY(grp.infoText().isEmpty() == false);
}

void FixtureGroup_Test::load()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 5));
    grp.setName("Pertti Pasanen");
    grp.setId(99);
    for (quint32 id = 0; id < 32; id++)
        grp.assignFixture(id);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(grp.saveXML(&doc, &root) == true);

    QVERIFY(FixtureGroup::loader(&root, m_doc) == false);

    QDomElement tag = root.firstChild().toElement();
    QVERIFY(FixtureGroup::loader(&tag, m_doc) == true);
    QCOMPARE(m_doc->fixtureGroups().size(), 1);
    FixtureGroup* grp2 = m_doc->fixtureGroup(99);
    QVERIFY(grp2 != NULL);
    QCOMPARE(grp2->size(), QSize(4, 5));
    QCOMPARE(grp2->name(), QString("Pertti Pasanen"));
    QCOMPARE(grp2->id(), quint32(99));
    QCOMPARE(grp2->fixtureHash(), grp.fixtureHash());
}

void FixtureGroup_Test::save()
{
    FixtureGroup grp(m_doc);
    grp.setSize(QSize(4, 5));
    grp.setName("Pertti Pasanen");
    grp.setId(99);
    for (quint32 id = 0; id < 32; id++)
        grp.assignFixture(id);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");

    QVERIFY(grp.saveXML(&doc, &root) == true);
    QDomElement tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("FixtureGroup"));
    QCOMPARE(tag.attribute("ID"), QString("99"));

    int size = 0, name = 0, fixture = 0;

    QDomNode node = tag.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Size")
        {
            QCOMPARE(tag.attribute("X").toInt(), 4);
            QCOMPARE(tag.attribute("Y").toInt(), 5);
            size++;
        }
        else if (tag.tagName() == "Name")
        {
            QCOMPARE(tag.text(), QString("Pertti Pasanen"));
            name++;
        }
        else if (tag.tagName() == "Fixture")
        {
            quint32 id = tag.text().toUInt();
            QLCPoint pt(tag.attribute("X").toInt(), tag.attribute("Y").toInt());
            QCOMPARE(grp.fixture(pt), id);
            fixture++;
        }
        else
        {
            QFAIL(QString("Unexpected tag in FixtureGroup: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(size, 1);
    QCOMPARE(name, 1);
    QCOMPARE(fixture, 32);
}

QTEST_APPLESS_MAIN(FixtureGroup_Test)
