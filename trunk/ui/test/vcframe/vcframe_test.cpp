/*
  Q Light Controller
  vcframe_test.cpp

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

#include <QMdiArea>
#include <QFrame>
#include <QtTest>
#include <QMenu>
#include <QSet>

#define protected public
#include "virtualconsole.h"
#include "vcframe.h"
#undef protected

#include "qlcfixturedefcache.h"
#include "vcframe_test.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#include "bus.h"

void VCFrame_Test::initTestCase()
{
    Bus::init(this);
}

void VCFrame_Test::initial()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCFrame frame(&w, &doc, &om, &im, &mt);
    QCOMPARE(frame.objectName(), QString("VCFrame"));
    QCOMPARE(frame.frameStyle(), QFrame::Panel | QFrame::Sunken);
}

void VCFrame_Test::copy()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCFrame parent(&w, &doc, &om, &im, &mt);
    VCFrame frame(&parent, &doc, &om, &im, &mt);
    VCWidget* frame2 = frame.createCopy(&parent);
    QVERIFY(frame2 != NULL && frame2 != &frame);
    QCOMPARE(frame2->objectName(), QString("VCFrame"));
    QCOMPARE(frame2->parentWidget(), &parent);

    QVERIFY(frame.copyFrom(NULL) == false);
}

void VCFrame_Test::loadXML()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Frame");
    xmldoc.appendChild(root);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement allowChildren = xmldoc.createElement("AllowChildren");
    QDomText allowChildrenText = xmldoc.createTextNode("False");
    allowChildren.appendChild(allowChildrenText);
    root.appendChild(allowChildren);

    QDomElement allowResize = xmldoc.createElement("AllowResize");
    QDomText allowResizeText = xmldoc.createTextNode("False");
    allowResize.appendChild(allowResizeText);
    root.appendChild(allowResize);

    QDomElement frame = xmldoc.createElement("Frame");
    root.appendChild(frame);

    QDomElement label = xmldoc.createElement("Label");
    root.appendChild(label);

    QDomElement button = xmldoc.createElement("Button");
    root.appendChild(button);

    QDomElement xypad = xmldoc.createElement("XYPad");
    root.appendChild(xypad);

    QDomElement slider = xmldoc.createElement("Slider");
    root.appendChild(slider);

    QDomElement soloframe = xmldoc.createElement("SoloFrame");
    root.appendChild(soloframe);

    QDomElement cuelist = xmldoc.createElement("CueList");
    root.appendChild(cuelist);

    QDomElement foobar = xmldoc.createElement("Foobar");
    root.appendChild(foobar);

    VCFrame parent(&w, &doc, &om, &im, &mt);
    QVERIFY(parent.loadXML(&root) == true);
    parent.postLoad();
    QCOMPARE(parent.geometry().width(), 42);
    QCOMPARE(parent.geometry().height(), 69);
    QCOMPARE(parent.geometry().x(), 3);
    QCOMPARE(parent.geometry().y(), 4);
    QVERIFY(parent.allowChildren() == false);
    QVERIFY(parent.allowResize() == false);

    QSet <QString> childSet;
    QCOMPARE(parent.children().size(), 7);
    foreach (QObject* child, parent.children())
        childSet << child->metaObject()->className();
    QVERIFY(childSet.contains("VCFrame"));
    QVERIFY(childSet.contains("VCLabel"));
    QVERIFY(childSet.contains("VCButton"));
    QVERIFY(childSet.contains("VCXYPad"));
    QVERIFY(childSet.contains("VCSlider"));
    QVERIFY(childSet.contains("VCSoloFrame"));
    QVERIFY(childSet.contains("VCCueList"));

    root.setTagName("Farme");
    QVERIFY(parent.loadXML(&root) == false);
}

void VCFrame_Test::saveXML()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCFrame parent(&w, &doc, &om, &im, &mt);
    parent.setCaption("Parent");
    VCFrame child(&parent, &doc, &om, &im, &mt);
    child.setCaption("Child");
    VCFrame grandChild(&child, &doc, &om, &im, &mt);
    grandChild.setCaption("Grandchild");
    child.setAllowChildren(false);
    child.setAllowResize(false);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);

    QVERIFY(parent.saveXML(&xmldoc, &root) == true);

    QDomNode node = root.firstChild();
    QVERIFY(node.nextSibling().isNull() == true);
    QCOMPARE(node.toElement().tagName(), QString("Frame"));
    QVERIFY(node.firstChild().isNull() == false);

    QDomNode subFrame;
    int appearance = 0, windowstate = 0, frame = 0, allowChildren = 0, allowResize = 0;

    // Parent
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == QString("Appearance"))
        {
            appearance++;
        }
        else if (tag.tagName() == QString("WindowState"))
        {
            windowstate++;
        }
        else if (tag.tagName() == QString("Frame"))
        {
            frame++;
            QVERIFY(subFrame.isNull() == true);
            subFrame = node;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 1);
    QCOMPARE(windowstate, 0);
    QCOMPARE(frame, 1);
    QVERIFY(subFrame.isNull() == false);

    // Child
    node = subFrame.firstChild();
    subFrame = QDomNode(); // Don't use .clear() since that would clear the whole tree
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == QString("Appearance"))
        {
            appearance++;
        }
        else if (tag.tagName() == QString("WindowState"))
        {
            windowstate++;
        }
        else if (tag.tagName() == QString("AllowChildren"))
        {
            allowChildren++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("AllowResize"))
        {
            allowResize++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("Frame"))
        {
            frame++;
            QVERIFY(subFrame.isNull() == true);
            subFrame = node;
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 2);
    QCOMPARE(windowstate, 1);
    QCOMPARE(allowChildren, 1);
    QCOMPARE(allowResize, 1);
    QCOMPARE(frame, 2);
    QVERIFY(subFrame.isNull() == false);

    // Grandchild
    node = subFrame.firstChild();
    subFrame = QDomNode(); // Don't use .clear() since that would clear the whole tree
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == QString("Appearance"))
        {
            appearance++;
        }
        else if (tag.tagName() == QString("WindowState"))
        {
            windowstate++;
        }
        else if (tag.tagName() == QString("AllowChildren"))
        {
            allowChildren++;
            QCOMPARE(tag.text(), QString("True"));
        }
        else if (tag.tagName() == QString("AllowResize"))
        {
            allowResize++;
            QCOMPARE(tag.text(), QString("True"));
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 3);
    QCOMPARE(windowstate, 2);
    QCOMPARE(frame, 2);
    QVERIFY(subFrame.isNull() == true);
}

void VCFrame_Test::customMenu()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QMdiArea w;

    VCFrame frame(&w, &doc, &om, &im, &mt);
    QVERIFY(frame.customMenu(NULL) == NULL);

    QMenu menu;
    VirtualConsole::resetContents(&w, &doc, &om, &im, &mt);
    VirtualConsole::createAndShow(&w, &doc, &om, &im, &mt);
    QMenu* customMenu = frame.customMenu(&menu);
    QVERIFY(customMenu != NULL);
    QCOMPARE(customMenu->title(), tr("Add"));
    delete customMenu;
    delete VirtualConsole::instance();
}

void VCFrame_Test::handleWidgetSelection()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QMdiArea w;

    VCFrame frame(&w, &doc, &om, &im, &mt);
    QVERIFY(frame.isBottomFrame() == true);

    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton,
                   Qt::LeftButton, Qt::NoModifier);
    frame.handleWidgetSelection(&ev);

    VirtualConsole::resetContents(&w, &doc, &om, &im, &mt);
    VirtualConsole::createAndShow(&w, &doc, &om, &im, &mt);
    VirtualConsole::instance()->hide();

    // Select bottom frame
    frame.handleWidgetSelection(&ev);
    QCOMPARE(VirtualConsole::instance()->selectedWidgets().size(), 0);

    // Select a child frame
    VCFrame child(&frame, &doc, &om, &im, &mt);
    QVERIFY(child.isBottomFrame() == false);
    child.handleWidgetSelection(&ev);
    QCOMPARE(VirtualConsole::instance()->selectedWidgets().size(), 1);

    // Again select bottom frame
    frame.handleWidgetSelection(&ev);
    QCOMPARE(VirtualConsole::instance()->selectedWidgets().size(), 0);

    delete VirtualConsole::instance();
}

void VCFrame_Test::mouseMoveEvent()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QMdiArea w;

    // Well, there isn't much that can be checked here... Actual moving happens in VCWidget.
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton,
                   Qt::LeftButton, Qt::NoModifier);

    VCFrame frame(&w, &doc, &om, &im, &mt);
    QVERIFY(frame.isBottomFrame() == true);
    frame.move(QPoint(0, 0));
    frame.mouseMoveEvent(&ev);
    QCOMPARE(frame.pos(), QPoint(0, 0));

    VCFrame child(&frame, &doc, &om, &im, &mt);
    QVERIFY(child.isBottomFrame() == false);
    child.mouseMoveEvent(&ev);
    QCOMPARE(child.pos(), QPoint(0, 0));
}

QTEST_MAIN(VCFrame_Test)
