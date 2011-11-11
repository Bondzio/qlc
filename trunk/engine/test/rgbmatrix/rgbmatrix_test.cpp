/*
  Q Light Controller
  rgbmatrix_test.cpp

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
#include <QtXml>

#define private public
#include "rgbmatrix_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "fixturegroup.h"
#include "mastertimer.h"
#include "rgbscript.h"
#include "rgbmatrix.h"
#include "fixture.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define INTERNAL_SCRIPTDIR "../../../rgbscripts/"
#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void RGBMatrix_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir fxiDir(INTERNAL_FIXTUREDIR);
    fxiDir.setFilter(QDir::Files);
    fxiDir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(fxiDir) == true);

    const QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Stairville", "LED PAR56");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);

    FixtureGroup* grp = new FixtureGroup(m_doc);
    grp->setName("Test Group");
    m_doc->addFixtureGroup(grp);

    for (int i = 0; i < 4; i++)
    {
        Fixture* fxi = new Fixture(m_doc);
        fxi->setFixtureDefinition(def, mode);
        m_doc->addFixture(fxi);

        grp->assignFixture(fxi->id());
    }

    QDir scrDir(INTERNAL_SCRIPTDIR);
    scrDir.setFilter(QDir::Files);
    scrDir.setNameFilters(QStringList() << QString("*.js"));
    m_scripts = RGBScript::scripts(scrDir);
    QVERIFY(m_scripts.size() > 0);
}

void RGBMatrix_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBMatrix_Test::initial()
{
    RGBMatrix mtx(m_doc);
    QCOMPARE(mtx.type(), Function::RGBMatrix);
    QCOMPARE(mtx.fixtureGroup(), FixtureGroup::invalidId());
    QCOMPARE(mtx.monoColor(), QColor(Qt::red));
    QVERIFY(mtx.m_fader == NULL);
    QCOMPARE(mtx.m_step, 0);
    QCOMPARE(mtx.name(), tr("New RGB Matrix"));
    QCOMPARE(mtx.duration(), uint(500));
    QCOMPARE(mtx.script().fileName(), QString("fullcolumns.js"));
}

void RGBMatrix_Test::group()
{
    RGBMatrix mtx(m_doc);
    mtx.setFixtureGroup(0);
    QCOMPARE(mtx.fixtureGroup(), uint(0));

    mtx.setFixtureGroup(15);
    QCOMPARE(mtx.fixtureGroup(), uint(15));

    mtx.setFixtureGroup(FixtureGroup::invalidId());
    QCOMPARE(mtx.fixtureGroup(), FixtureGroup::invalidId());
}

void RGBMatrix_Test::color()
{
    RGBMatrix mtx(m_doc);
    mtx.setMonoColor(Qt::blue);
    QCOMPARE(mtx.monoColor(), QColor(Qt::blue));

    mtx.setMonoColor(QColor());
    QCOMPARE(mtx.monoColor(), QColor());
}

void RGBMatrix_Test::copy()
{
    RGBMatrix mtx(m_doc);
    mtx.setMonoColor(Qt::magenta);
    mtx.setFixtureGroup(0);
    mtx.setScript(m_scripts.last());

    RGBMatrix* copyMtx = qobject_cast<RGBMatrix*> (mtx.createCopy(m_doc));
    QVERIFY(copyMtx != NULL);
    QCOMPARE(copyMtx->monoColor(), QColor(Qt::magenta));
    QCOMPARE(copyMtx->fixtureGroup(), uint(0));
    QCOMPARE(copyMtx->script(), m_scripts.last());
}

void RGBMatrix_Test::save()
{
    RGBMatrix* mtx = new RGBMatrix(m_doc);
    mtx->setName("Xyzzy");
    mtx->setMonoColor(Qt::magenta);
    mtx->setFixtureGroup(42);
    mtx->setScript(m_scripts.last());
    m_doc->addFunction(mtx);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    QVERIFY(mtx->saveXML(&doc, &root) == true);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Function"));
    QCOMPARE(root.firstChild().toElement().attribute("Type"), QString("RGBMatrix"));
    QCOMPARE(root.firstChild().toElement().attribute("ID"), QString::number(mtx->id()));
    QCOMPARE(root.firstChild().toElement().attribute("Name"), QString("Xyzzy"));

    int speed = 0, dir = 0, run = 0, script = 0, monocolor = 0, grp = 0;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Speed")
            speed++;
        else if (tag.tagName() == "Direction")
            dir++;
        else if (tag.tagName() == "RunOrder")
            run++;
        else if (tag.tagName() == "Script")
            script++;
        else if (tag.tagName() == "MonoColor")
            monocolor++;
        else if (tag.tagName() == "FixtureGroup")
            grp++;
        else
            QFAIL(QString("Unexpected tag: ").arg(tag.tagName()).toUtf8().constData());

        node = node.nextSibling();
    }

    QCOMPARE(speed, 1);
    QCOMPARE(dir, 1);
    QCOMPARE(run, 1);
    QCOMPARE(script, 1);
    QCOMPARE(monocolor, 1);
    QCOMPARE(grp, 1);
}

void RGBMatrix_Test::load()
{
}

QTEST_MAIN(RGBMatrix_Test)
