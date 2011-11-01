/*
  Q Light Controller
  vccuelist_test.cpp

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QMdiArea>
#include <QtTest>

#define protected public
#define private public
#include "virtualconsole.h"
#include "genericfader.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "vccuelist.h"
#include "vcwidget.h"
#include "vcframe.h"

#include "qlcfixturedefcache.h"
#include "qlcinputsource.h"
#include "vccuelist_test.h"
#include "outputmap.h"
#include "inputmap.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#undef private
#undef protected

void VCCueList_Test::initTestCase()
{
    m_doc = NULL;
    m_area = NULL;
}

void VCCueList_Test::init()
{
    m_doc = new Doc(this);
    m_area = new QMdiArea;
    VirtualConsole::createAndShow(m_area, m_doc);
}

void VCCueList_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_area;
    delete m_doc;
}

void VCCueList_Test::initial()
{
    QWidget w;

    VCCueList cl(&w, m_doc);
    QCOMPARE(cl.objectName(), QString("VCCueList"));
    QCOMPARE(cl.frameStyle(), QFrame::Panel | QFrame::Sunken);
    QCOMPARE(cl.caption(), tr("Cue list"));
    QCOMPARE(cl.size(), QSize(200, 200));
    QVERIFY(cl.m_runner == NULL);
    QVERIFY(cl.m_list != NULL);
    QCOMPARE(cl.m_list->isEnabled(), false);
    QCOMPARE(cl.m_list->topLevelItemCount(), 0);
    QCOMPARE(cl.m_list->selectionMode(), QAbstractItemView::SingleSelection);
    QCOMPARE(cl.m_list->rootIsDecorated(), false);

    QCOMPARE(cl.m_nextLatestValue, quint32(0));
    QCOMPARE(cl.m_previousLatestValue, quint32(0));
    QCOMPARE(cl.m_stopLatestValue, quint32(0));

    QCOMPARE(cl.m_nextKeySequence, QKeySequence());
    QCOMPARE(cl.m_previousKeySequence, QKeySequence());
    QCOMPARE(cl.m_stopKeySequence, QKeySequence());

    QVERIFY(cl.inputSource(VCCueList::nextInputSourceId).isValid() == false);
    QVERIFY(cl.inputSource(VCCueList::previousInputSourceId).isValid() == false);
    QVERIFY(cl.inputSource(VCCueList::stopInputSourceId).isValid() == false);
}

void VCCueList_Test::appendClear()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    VCCueList cl(&w, m_doc);

    cl.append(INT_MAX - 1);
    QCOMPARE(cl.m_list->topLevelItemCount(), 0);

    cl.append(s1->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 1);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0), QString::number(1));
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2), QString::number(s1->id()));

    cl.append(s2->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 2);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0), QString::number(1));
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2), QString::number(s1->id()));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(0), QString::number(2));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(2), QString::number(s2->id()));

    // Same step can exist multiple times
    cl.append(s2->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 3);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0), QString::number(1));
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2), QString::number(s1->id()));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(0), QString::number(2));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(2), QString::number(s2->id()));
    QCOMPARE(cl.m_list->topLevelItem(2)->text(0), QString::number(3));
    QCOMPARE(cl.m_list->topLevelItem(2)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(2), QString::number(s2->id()));

    cl.clear();
    QCOMPARE(cl.m_list->topLevelItemCount(), 0);
    QVERIFY(m_doc->function(s1->id()) == s1); // Function must not be deleted
    QVERIFY(m_doc->function(s2->id()) == s2); // Function must not be deleted
}

void VCCueList_Test::functionRemoved()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    VCCueList cl(&w, m_doc);

    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s1->id());
    cl.append(s2->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 4);

    m_doc->deleteFunction(s1->id());
    s1 = NULL;
    QCOMPARE(cl.m_list->topLevelItemCount(), 2);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0), QString::number(2));
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2), QString::number(s2->id()));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(0), QString::number(4));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(2), QString::number(s2->id()));

    m_doc->deleteFunction(s3->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 2);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0), QString::number(2));
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2), QString::number(s2->id()));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(0), QString::number(4));
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(2), QString::number(s2->id()));

    m_doc->deleteFunction(s2->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 0);
}

void VCCueList_Test::functionChanged()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    VCCueList cl(&w, m_doc);

    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s1->id());
    cl.append(s2->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 4);

    QSignalSpy spy(s1, SIGNAL(changed(quint32)));
    s1->setName("There can only be one");
    QCOMPARE(spy.size(), 1);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(3)->text(1), s2->name());

    s2->setName("Says who?");
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(3)->text(1), s2->name());

    s3->setName("Pssht. Third's the charm.");
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(3)->text(1), s2->name());
}

void VCCueList_Test::keySequences()
{
    QWidget w;

    VCCueList cl(&w, m_doc);
    cl.setNextKeySequence(QKeySequence(QKeySequence::Copy));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(cl.previousKeySequence(), QKeySequence());
    QCOMPARE(cl.stopKeySequence(), QKeySequence());

    cl.setPreviousKeySequence(QKeySequence(QKeySequence::Cut));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(cl.previousKeySequence(), QKeySequence(QKeySequence::Cut));
    QCOMPARE(cl.stopKeySequence(), QKeySequence());

    cl.setStopKeySequence(QKeySequence(QKeySequence::Undo));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(cl.previousKeySequence(), QKeySequence(QKeySequence::Cut));
    QCOMPARE(cl.stopKeySequence(), QKeySequence(QKeySequence::Undo));
}

void VCCueList_Test::copy()
{
    QWidget w;

    // Input sources are tested by VCWidget tests. No point testing here.

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    VCFrame parent(&w, m_doc);

    VCCueList cl(&parent, m_doc);
    cl.setCaption("Wheeee");
    cl.setNextKeySequence(QKeySequence(QKeySequence::Copy));
    cl.setPreviousKeySequence(QKeySequence(QKeySequence::Cut));
    cl.setStopKeySequence(QKeySequence(QKeySequence::Paste));
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());
    QCOMPARE(cl.m_list->topLevelItemCount(), 5);

    VCCueList* cl2 = qobject_cast<VCCueList*> (cl.createCopy(&parent));
    QVERIFY(cl2 != NULL);
    QCOMPARE(cl2->caption(), QString("Wheeee"));
    QCOMPARE(cl2->nextKeySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(cl2->previousKeySequence(), QKeySequence(QKeySequence::Cut));
    QCOMPARE(cl2->stopKeySequence(), QKeySequence(QKeySequence::Paste));
    QCOMPARE(cl2->m_list->topLevelItemCount(), 5);
    QCOMPARE(cl2->m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl2->m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl2->m_list->topLevelItem(2)->text(1), s1->name());
    QCOMPARE(cl2->m_list->topLevelItem(3)->text(1), s2->name());
    QCOMPARE(cl2->m_list->topLevelItem(4)->text(1), s3->name());

    VCCueList cl3(&parent, m_doc);
    cl3.copyFrom(NULL);
    QCOMPARE(cl3.m_list->topLevelItemCount(), 0);
    QCOMPARE(cl3.caption(), tr("Cue list"));

    cl.copyFrom(&cl3);
    QCOMPARE(cl.caption(), cl3.caption());
    QCOMPARE(cl.m_list->topLevelItemCount(), 0);
    QCOMPARE(cl.nextKeySequence(), QKeySequence());
    QCOMPARE(cl.previousKeySequence(), QKeySequence());
    QCOMPARE(cl.stopKeySequence(), QKeySequence());

    delete cl2;
}

void VCCueList_Test::modeChange()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    VCFrame parent(&w, m_doc);
    VCCueList cl(&parent, m_doc);
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());

    m_doc->setMode(Doc::Operate);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 1);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList[0], &cl);
    QVERIFY(cl.m_runner == NULL);
    QVERIFY(cl.m_list->isEnabled() == true);

    cl.createRunner();

    m_doc->setMode(Doc::Design);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
    QVERIFY(cl.m_runner == NULL);
    QVERIFY(cl.m_list->isEnabled() == false);
}

void VCCueList_Test::loadXML()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    Chaser* c4 = new Chaser(m_doc);
    c4->setName("The defiant one");
    m_doc->addFunction(c4);

    QDomDocument xmldoc;

    QDomElement root = xmldoc.createElement("CueList");
    root.setAttribute("Caption", "Test list");
    xmldoc.appendChild(root);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement appearance = xmldoc.createElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    QDomElement font = xmldoc.createElement("Font");
    QDomText fontText = xmldoc.createTextNode(f.toString());
    font.appendChild(fontText);
    appearance.appendChild(font);
    root.appendChild(appearance);

    QDomElement next = xmldoc.createElement("Next");
    QDomElement nextInput = xmldoc.createElement("Input");
    nextInput.setAttribute("Universe", "0");
    nextInput.setAttribute("Channel", "1");
    next.appendChild(nextInput);
    QDomElement nextKey = xmldoc.createElement("Key");
    QDomText nextKeyText = xmldoc.createTextNode(QKeySequence(QKeySequence::Undo).toString());
    nextKey.appendChild(nextKeyText);
    next.appendChild(nextKey);
    QDomElement nextFoo = xmldoc.createElement("Foo");
    next.appendChild(nextFoo);
    root.appendChild(next);

    QDomElement previous = xmldoc.createElement("Previous");
    QDomElement previousInput = xmldoc.createElement("Input");
    previousInput.setAttribute("Universe", "2");
    previousInput.setAttribute("Channel", "3");
    previous.appendChild(previousInput);
    QDomElement previousKey = xmldoc.createElement("Key");
    QDomText previousKeyText = xmldoc.createTextNode(QKeySequence(QKeySequence::Paste).toString());
    previousKey.appendChild(previousKeyText);
    previous.appendChild(previousKey);
    QDomElement previousFoo = xmldoc.createElement("Foo");
    previous.appendChild(previousFoo);
    root.appendChild(previous);

    QDomElement stop = xmldoc.createElement("Stop");
    QDomElement stopInput = xmldoc.createElement("Input");
    stopInput.setAttribute("Universe", "4");
    stopInput.setAttribute("Channel", "5");
    stop.appendChild(stopInput);
    QDomElement stopKey = xmldoc.createElement("Key");
    QDomText stopKeyText = xmldoc.createTextNode(QKeySequence(QKeySequence::Cut).toString());
    stopKey.appendChild(stopKeyText);
    stop.appendChild(stopKey);
    QDomElement stopFoo = xmldoc.createElement("Foo");
    stop.appendChild(stopFoo);
    root.appendChild(stop);

    QDomElement f1 = xmldoc.createElement("Function");
    QDomText f1Text = xmldoc.createTextNode(QString::number(s1->id()));
    f1.appendChild(f1Text);
    root.appendChild(f1);

    QDomElement f2 = xmldoc.createElement("Function");
    QDomText f2Text = xmldoc.createTextNode(QString::number(s2->id()));
    f2.appendChild(f2Text);
    root.appendChild(f2);

    QDomElement f3 = xmldoc.createElement("Function");
    QDomText f3Text = xmldoc.createTextNode(QString::number(s3->id()));
    f3.appendChild(f3Text);
    root.appendChild(f3);

    QDomElement f4 = xmldoc.createElement("Function");
    QDomText f4Text = xmldoc.createTextNode(QString::number(c4->id()));
    f4.appendChild(f4Text);
    root.appendChild(f4);

    QDomElement f5 = xmldoc.createElement("Function");
    QDomText f5Text = xmldoc.createTextNode(QString::number(INT_MAX - 1));
    f5.appendChild(f5Text);
    root.appendChild(f5);

    QDomElement foo = xmldoc.createElement("Foobar");
    root.appendChild(foo);

    VCCueList cl(&w, m_doc);
    QVERIFY(cl.loadXML(&root) == true);
    QCOMPARE(cl.m_list->topLevelItemCount(), 4);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0).toInt(), 1);
    QCOMPARE(cl.m_list->topLevelItem(1)->text(0).toInt(), 2);
    QCOMPARE(cl.m_list->topLevelItem(2)->text(0).toInt(), 3);
    QCOMPARE(cl.m_list->topLevelItem(3)->text(0).toInt(), 4);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(1), s3->name());
    QCOMPARE(cl.m_list->topLevelItem(3)->text(1), c4->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2).toUInt(), s1->id());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(2).toUInt(), s2->id());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(2).toUInt(), s3->id());
    QCOMPARE(cl.m_list->topLevelItem(3)->text(2).toUInt(), c4->id());
    QCOMPARE(cl.inputSource(VCCueList::nextInputSourceId), QLCInputSource(0, 1));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(QKeySequence::Undo));
    QCOMPARE(cl.inputSource(VCCueList::previousInputSourceId), QLCInputSource(2, 3));
    QCOMPARE(cl.previousKeySequence(), QKeySequence(QKeySequence::Paste));
    QCOMPARE(cl.inputSource(VCCueList::stopInputSourceId), QLCInputSource(4, 5));
    QCOMPARE(cl.stopKeySequence(), QKeySequence(QKeySequence::Cut));

    QCOMPARE(cl.pos(), QPoint(3, 4));
    QCOMPARE(cl.size(), QSize(42, 69));
    QCOMPARE(cl.font(), f);

    cl.postLoad();
    QCOMPARE(cl.m_list->topLevelItemCount(), 3);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(0).toInt(), 1);
    QCOMPARE(cl.m_list->topLevelItem(1)->text(0).toInt(), 2);
    QCOMPARE(cl.m_list->topLevelItem(2)->text(0).toInt(), 3);
    QCOMPARE(cl.m_list->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(1), s3->name());
    QCOMPARE(cl.m_list->topLevelItem(0)->text(2).toUInt(), s1->id());
    QCOMPARE(cl.m_list->topLevelItem(1)->text(2).toUInt(), s2->id());
    QCOMPARE(cl.m_list->topLevelItem(2)->text(2).toUInt(), s3->id());

    root.setTagName("CueLits");
    QVERIFY(cl.loadXML(&root) == false);
}

void VCCueList_Test::saveXML()
{
    QWidget w;

    VCCueList cl(&w, m_doc);
    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());
    cl.setCaption("Testing");
    cl.setInputSource(QLCInputSource(0, 1), VCCueList::nextInputSourceId);
    cl.setInputSource(QLCInputSource(2, 3), VCCueList::previousInputSourceId);
    cl.setInputSource(QLCInputSource(4, 5), VCCueList::stopInputSourceId);
    cl.setNextKeySequence(QKeySequence(QKeySequence::Copy));
    cl.setPreviousKeySequence(QKeySequence(QKeySequence::Cut));
    cl.setStopKeySequence(QKeySequence(QKeySequence::Paste));

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);

    int function = 0, next = 0, nextKey = 0, nextInput = 0, previous = 0, previousKey = 0,
        previousInput = 0, stop = 0, stopKey = 0, stopInput = 0, wstate = 0, appearance = 0;

    QVERIFY(cl.saveXML(&xmldoc, &root) == true);
    QDomElement clroot = root.firstChild().toElement();
    QCOMPARE(clroot.tagName(), QString("CueList"));
    QCOMPARE(clroot.attribute("Caption"), QString("Testing"));

    QDomNode node = clroot.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Function")
        {
            function++;
            QVERIFY(tag.text() == QString::number(s1->id()) ||
                    tag.text() == QString::number(s2->id()) ||
                    tag.text() == QString::number(s3->id()));
        }
        else if (tag.tagName() == "Next")
        {
            next++;
            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Key")
                {
                    nextKey++;
                    QCOMPARE(subtag.text(), QKeySequence(QKeySequence::Copy).toString());
                }
                else if (subtag.tagName() == "Input")
                {
                    nextInput++;
                    // Handled by VCWidget tests, just check that the node is there
                }
                else
                {
                    QFAIL(QString("Unexpected tag: %1").arg(subtag.tagName()).toUtf8().constData());
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "Previous")
        {
            previous++;
            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Key")
                {
                    previousKey++;
                    QCOMPARE(subtag.text(), QKeySequence(QKeySequence::Cut).toString());
                }
                else if (subtag.tagName() == "Input")
                {
                    previousInput++;
                    // Handled by VCWidget tests, just check that the node is there
                }
                else
                {
                    QFAIL(QString("Unexpected tag: %1").arg(subtag.tagName()).toUtf8().constData());
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "Stop")
        {
            stop++;
            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Key")
                {
                    stopKey++;
                    QCOMPARE(subtag.text(), QKeySequence(QKeySequence::Paste).toString());
                }
                else if (subtag.tagName() == "Input")
                {
                    stopInput++;
                    // Handled by VCWidget tests, just check that the node is there
                }
                else
                {
                    QFAIL(QString("Unexpected tag: %1").arg(subtag.tagName()).toUtf8().constData());
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "WindowState")
        {
            // Handled by VCWidget tests, just check that the node is there
            wstate++;
        }
        else if (tag.tagName() == "Appearance")
        {
            // Handled by VCWidget tests, just check that the node is there
            appearance++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(function, 3);
    QCOMPARE(next, 1);
    QCOMPARE(nextKey, 1);
    QCOMPARE(nextInput, 1);
    QCOMPARE(previous, 1);
    QCOMPARE(previousKey, 1);
    QCOMPARE(previousInput, 1);
    QCOMPARE(stop, 1);
    QCOMPARE(stopKey, 1);
    QCOMPARE(stopInput, 1);
    QCOMPARE(wstate, 1);
    QCOMPARE(appearance, 1);
}

void VCCueList_Test::nextPrevious()
{
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    s1->setValue(fxi->id(), 0, 255);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    s2->setValue(fxi->id(), 0, 127);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    s3->setValue(fxi->id(), 0, 64);
    m_doc->addFunction(s3);

    Scene* s4 = new Scene(m_doc);
    s4->setName("The fourth one");
    s4->setValue(fxi->id(), 0, 32);
    m_doc->addFunction(s4);

    VCCueList cl(&w, m_doc);
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());
    cl.append(s4->id());

    // Not in operate mode, check for crashes
    cl.slotNextCue();
    cl.slotPreviousCue();
    cl.slotItemActivated(cl.m_list->topLevelItem(2));
    QVERIFY(cl.m_runner == NULL);

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // Create runner with a next action -> first item should be activated
    cl.slotNextCue();
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);

    cl.slotNextCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s1); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s2); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s2);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s2);

    cl.slotNextCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s2); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s3); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);

    cl.slotPreviousCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s3); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s2); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s2);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s2);

    cl.slotPreviousCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s2); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s1); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);

    // Wrap around to the last cue
    cl.slotPreviousCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s1); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s4); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s4);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s4);

    // Wrap around to the next cue
    cl.slotNextCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s4); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s1); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
}

void VCCueList_Test::manualActivation()
{
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    s1->setValue(fxi->id(), 0, 255);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    s2->setValue(fxi->id(), 0, 127);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    s3->setValue(fxi->id(), 0, 64);
    m_doc->addFunction(s3);

    Scene* s4 = new Scene(m_doc);
    s4->setName("The fourth one");
    s4->setValue(fxi->id(), 0, 32);
    m_doc->addFunction(s4);

    VCCueList cl(&w, m_doc);
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());
    cl.append(s4->id());

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    QVERIFY(cl.m_runner == NULL);
    cl.slotItemActivated(cl.m_list->topLevelItem(2));
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);

    // Same item
    cl.slotItemActivated(cl.m_list->topLevelItem(2));
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s3);

    // Another item
    cl.slotItemActivated(cl.m_list->topLevelItem(0));
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2); // DMX sources are run after functions, so
    QCOMPARE(timer->m_functionList[0], s3); // the function will be removed in the next
    QCOMPARE(timer->m_functionList[1], s1); // round.
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);

    // Crash check
    cl.slotItemActivated(NULL);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 1);
    QCOMPARE(timer->m_functionList[0], s1);
}


void VCCueList_Test::keyboardNextPrevious()
{
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    s1->setValue(fxi->id(), 0, 255);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    s2->setValue(fxi->id(), 0, 127);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    s3->setValue(fxi->id(), 0, 64);
    m_doc->addFunction(s3);

    Scene* s4 = new Scene(m_doc);
    s4->setName("The fourth one");
    s4->setValue(fxi->id(), 0, 32);
    m_doc->addFunction(s4);

    VCCueList cl(&w, m_doc);
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());
    cl.append(s4->id());
    cl.setNextKeySequence(QKeySequence(QKeySequence::Copy));
    cl.setPreviousKeySequence(QKeySequence(QKeySequence::Cut));
    cl.setStopKeySequence(QKeySequence(QKeySequence::Undo));

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // Next keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::Copy));
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 0);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), 0);

    // Next keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::Copy));
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 1);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), 1);

    // Unrecognized keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::SelectAll));
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 1);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), 1);

    // Previous keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::Cut));
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 0);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), 0);

    // Previous keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::Cut));
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 3);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), 3);

    // Next keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::Copy));
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 0);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), 0);

    // Stop
    cl.slotKeyPressed(QKeySequence(QKeySequence::Undo));
    timer->timerTick();
    QVERIFY(cl.m_runner == NULL);
    QCOMPARE(cl.m_list->indexOfTopLevelItem(cl.m_list->currentItem()), -1);
}

void VCCueList_Test::input()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    Scene* s4 = new Scene(m_doc);
    s4->setName("The fourth one");
    m_doc->addFunction(s4);

    VCCueList cl(&w, m_doc);
    cl.append(s1->id());
    cl.append(s2->id());
    cl.append(s3->id());
    cl.append(s4->id());
    cl.setInputSource(QLCInputSource(0, 1), VCCueList::nextInputSourceId);
    cl.setInputSource(QLCInputSource(2, 3), VCCueList::previousInputSourceId);
    cl.setInputSource(QLCInputSource(4, 5), VCCueList::stopInputSourceId);

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // Runner creation thru "next" input
    cl.slotInputValueChanged(5, 3, 255);
    QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(2, 15, 255);
    QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(0, 1, 255);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 0);

    cl.slotInputValueChanged(0, 1, 0);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 0);

    cl.slotInputValueChanged(0, 1, 255);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 1);

    // Runner creation thru previous input
    m_doc->setMode(Doc::Design);
    QVERIFY(cl.m_runner == NULL);
    m_doc->setMode(Doc::Operate);

    cl.slotInputValueChanged(0, 3, 255);
    QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(2, 1, 255);
    QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(2, 3, 255);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 3);

    cl.slotInputValueChanged(2, 3, 0);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 3);

    cl.slotInputValueChanged(2, 3, 255);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QCOMPARE(cl.m_runner->currentStep(), 2);

    cl.slotInputValueChanged(4, 5, 255);
    QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(4, 5, 0);
    QVERIFY(cl.m_runner == NULL);
    timer->timerTick();
    QVERIFY(cl.m_runner == NULL);
}

QTEST_MAIN(VCCueList_Test)
