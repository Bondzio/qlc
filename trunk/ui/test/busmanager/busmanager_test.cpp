/*
  Q Light Controller
  busmanager_test.cpp

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

#include <QtTest>
#include <QtGui>

#define private public
#include "busmanager.h"
#undef private

#include "busmanager_test.h"
#include "mastertimer.h"
#include "bus.h"

void BusManager_Test::initTestCase()
{
    Bus::init(this);
}

void BusManager_Test::initial()
{
    QMdiArea parent;

    BusManager* bm = new BusManager(&parent);
    QVERIFY(BusManager::instance() == NULL);
    QVERIFY(qobject_cast<QVBoxLayout*> (bm->layout()) != NULL);
    QVERIFY(bm->m_toolbar != NULL);
    QVERIFY(bm->m_tree != NULL);
    QVERIFY(bm->m_tree->rootIsDecorated() == false);
    QCOMPARE(bm->m_tree->columnCount(), 3);
    QCOMPARE(bm->m_tree->topLevelItemCount(), int(Bus::instance()->count()));
    srand(QDateTime::currentDateTime().toTime_t());
    for (quint32 i = 0; i < Bus::instance()->count(); i++)
    {
        quint32 value;
        QString str;

        Bus::instance()->setValue(i, abs(rand()));
        value = Bus::instance()->value(i);
        str.sprintf("%.2fs", qreal(value) / qreal(MasterTimer::frequency()));

        QTreeWidgetItem* item = bm->m_tree->topLevelItem(i);
        QCOMPARE(item->text(0).toUInt(), i + 1);
        QCOMPARE(item->text(1), Bus::instance()->name(i));
        QCOMPARE(item->text(2), str);
    }

    delete bm;
    QVERIFY(BusManager::instance() == NULL);

    // Use the "official" way to create the manager
    BusManager::createAndShow(&parent);
    bm = BusManager::instance();
    QVERIFY(bm != NULL);

    // Shouldn't create another instance
    BusManager::createAndShow(&parent);
    QCOMPARE(bm, BusManager::instance());

    // Ensure closing the window will also reset the singleton pointer
#ifdef __APPLE__
    QVERIFY(bm->testAttribute(Qt::WA_DeleteOnClose));
    delete bm;
#else
    QMdiSubWindow* sub = qobject_cast<QMdiSubWindow*> (bm->parentWidget());
    QVERIFY(sub != NULL);
    QVERIFY(sub->testAttribute(Qt::WA_DeleteOnClose));
    delete sub;
#endif
    QTest::qWait(1);
    QVERIFY(BusManager::instance() == NULL);
}

void BusManager_Test::itemChanged()
{
    QMdiArea parent;
    BusManager* bm = new BusManager(&parent);

    QTreeWidgetItem* item = bm->m_tree->topLevelItem(0);

    // Reject changes to ID column
    item->setText(0, "Foobar");
    QCOMPARE(item->text(0), QString("1"));

    // Name column
    item->setText(1, "Foobar");
    QCOMPARE(item->text(0), QString("1"));
    QCOMPARE(item->text(1), QString("Foobar"));

    QString str;
    str.sprintf("%.2fs", qreal(Bus::instance()->value(0)) / qreal(MasterTimer::frequency()));

    // Reject non-numeric changes to value column
    item->setText(2, "Foobar");
    QCOMPARE(item->text(0), QString("1"));
    QCOMPARE(item->text(1), QString("Foobar"));
    QCOMPARE(item->text(2), str);

    str.sprintf("%.2fs", qreal(MasterTimer::frequency()) / qreal(MasterTimer::frequency()));

    item->setText(2, str);
    QCOMPARE(item->text(0), QString("1"));
    QCOMPARE(item->text(1), QString("Foobar"));
    QCOMPARE(item->text(2), str);

    delete bm;
}

QTEST_MAIN(BusManager_Test)
