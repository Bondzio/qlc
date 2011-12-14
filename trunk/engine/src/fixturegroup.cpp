/*
  Q Light Controller
  fixturegroup.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>

#include "fixturegroup.h"
#include "qlcpoint.h"
#include "fixture.h"
#include "doc.h"

#define KXMLQLCFixtureGroupID "ID"
#define KXMLQLCFixtureGroupHead "Head"
#define KXMLQLCFixtureGroupSize "Size"
#define KXMLQLCFixtureGroupName "Name"
#define KXMLQLCFixtureGroupDisplayStyle "DisplayStyle"

/****************************************************************************
 * GroupHead
 ****************************************************************************/

GroupHead::GroupHead(quint32 aFxi, int aHead)
    : fxi(aFxi)
    , head(aHead)
{
}

GroupHead::GroupHead(const GroupHead& another)
    : fxi(another.fxi)
    , head(another.head)
{
}

GroupHead::~GroupHead()
{
}

bool GroupHead::isValid() const
{
    if (fxi != Fixture::invalidId() && head >= 0)
        return true;
    else
        return false;
}

bool GroupHead::operator==(const GroupHead& another) const
{
    if (fxi == another.fxi && head == another.head)
        return true;
    else
        return false;
}

/****************************************************************************
 * Initialization
 ****************************************************************************/

FixtureGroup::FixtureGroup(Doc* parent)
    : QObject(parent)
    , m_id(FixtureGroup::invalidId())
    , m_displayStyle(DisplayIcon | DisplayAddress | DisplayHead)
{
    Q_ASSERT(parent != NULL);

    // Listen to fixture removals
    connect(parent, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));
}

FixtureGroup::~FixtureGroup()
{
}

void FixtureGroup::copyFrom(const FixtureGroup* grp)
{
    // Don't copy ID
    m_name = grp->name();
    m_size = grp->size();
    m_heads = grp->headHash();
    m_displayStyle = grp->displayStyle();
}

/****************************************************************************
 * ID
 ****************************************************************************/

void FixtureGroup::setId(quint32 id)
{
    m_id = id;
}

quint32 FixtureGroup::id() const
{
    return m_id;
}

quint32 FixtureGroup::invalidId()
{
    return UINT_MAX;
}

/****************************************************************************
 * Information
 ****************************************************************************/

void FixtureGroup::setName(const QString& name)
{
    m_name = name;
}

QString FixtureGroup::name() const
{
    return m_name;
}

void FixtureGroup::setDisplayStyle(int s)
{
    m_displayStyle = s;
}

int FixtureGroup::displayStyle() const
{
    return m_displayStyle;
}

QString FixtureGroup::infoText() const
{
    QString info;

    QString title("<TR><TD CLASS='hilite' COLSPAN='3'>%1: %2</TD></TR>");
    QString subTitle("<TR><TD CLASS='subhi' COLSPAN='3'>%1</TD></TR>");
    QString genInfo("<TR><TD CLASS='emphasis' COLSPAN='2'>%1</TD><TD>%2</TD></TR>");

    info += "<TABLE COLS='3' WIDTH='100%'>";
    info += title.arg(tr("Fixture Group")).arg(name());
    info += genInfo.arg(tr("Fixture count")).arg(headList().size());
    info += "</TABLE>";

    if (size().isValid() == true)
    {
        info += QString("<TABLE ROWS='%1' COLS='%2' WIDTH='100%' BORDER='0'>").arg(size().height() + 1).arg(size().width() + 1);
        QString subTitle("<TR><TD CLASS='subhi' COLSPAN='%1'>%2</TD></TR>");
        info += subTitle.arg(size().width() + 1).arg(tr("Grid Placement"));

        // Horizontal header
        info += "<TR>";
        info += "<TD></TD>"; // Empty corner
        for (int x = 0; x < size().width(); x++)
        {
            info += QString("<TD CLASS='emphasis' ALIGN='center' VALIGN='center'>%1</TD>").arg(x + 1);
        }
        info += "</TR>";

        for (int y = 0; y < size().height(); y++)
        {
            info += "<TR>";
            info += QString("<TD CLASS='emphasis' ALIGN='center' VALIGN='center'>%1</TD>").arg(y + 1);
            for (int x = 0; x < size().width(); x++)
            {
                QLCPoint pt(x, y);
                if (m_heads.contains(pt) == true)
                {
                    GroupHead head(m_heads[pt]);
                    Fixture* fxi = doc()->fixture(head.fxi);
                    Q_ASSERT(fxi != NULL);

                    info += "<TD CLASS='tiny' ALIGN='center'>";
                    if (displayStyle() & DisplayIcon)
                        info += "<IMG SRC='qrc:/fixture.png'/>";
                    if (displayStyle() & DisplayName || displayStyle() & DisplayAddress ||
                        displayStyle() & DisplayUniverse)
                        info += "<BR/>";
                    if (displayStyle() & DisplayName)
                        info += QString("%1").arg(fxi->name());
                    if (displayStyle() & DisplayName)
                        info += "<BR/>";
                    if (displayStyle() & DisplayAddress)
                        info += QString("A:%1 ").arg(fxi->address() + 1);
                    if (displayStyle() & DisplayUniverse)
                        info += QString("U:%1").arg(fxi->universe() + 1);
                    if (displayStyle() & DisplayHead)
                        info += QString("H:%1").arg(head.head + 1);
                    info += "</TD>";
                }
                else
                {
                    info += "<TD class='tiny' ALIGN='center' VALIGN='center'>";
                    info += "<IMG SRC='qrc:/empty.png'/><BR/>";
                    info += "---";
                    info += "</TD>";
                }
            }
            info += "</TR>";
        }

        info += "</TABLE>";
    }

    return info;
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void FixtureGroup::assignHead(const QLCPoint& pt, const GroupHead& head)
{
    if (m_heads.values().contains(head) == true)
        return;

    if (size().isValid() == false)
        setSize(QSize(1, 1));

    if (pt.isNull() == false)
    {
        m_heads[pt] = head;
    }
    else
    {
        bool assigned = false;
        int y = 0;
        int x = 0;
        int xmax = size().width();
        int ymax = size().height();

        while (assigned == false)
        {
            for (; y < ymax; y++)
            {
                for (x = 0; x < xmax; x++)
                {
                    QLCPoint tmp(x, y);
                    if (m_heads.contains(tmp) == false)
                    {
                        m_heads[tmp] = head;
                        assigned = true;
                        break;
                    }
                }

                if (assigned == true)
                    break;
            }

            ymax++;
        }
    }
}

void FixtureGroup::assignFixture(quint32 id, const QLCPoint& pt)
{
    assignHead(pt, GroupHead(id, 0));
}

void FixtureGroup::resignFixture(quint32 id)
{
    foreach (QLCPoint pt, m_heads.keys())
    {
        if (m_heads[pt].fxi == id)
            m_heads.remove(pt);
    }
}

void FixtureGroup::swap(const QLCPoint& a, const QLCPoint& b)
{
    GroupHead ah;
    GroupHead bh;

    if (m_heads.contains(a) == true)
        ah = m_heads[a];
    if (m_heads.contains(b) == true)
        bh = m_heads[b];

    if (ah.isValid() == true)
        m_heads[b] = ah;
    else
        m_heads.remove(b);

    if (bh.isValid() == true)
        m_heads[a] = bh;
    else
        m_heads.remove(a);
}

GroupHead FixtureGroup::head(const QLCPoint& pt) const
{
    if (m_heads.contains(pt) == true)
        return m_heads[pt];
    else
        return GroupHead();
}

QList <GroupHead> FixtureGroup::headList() const
{
    return m_heads.values();
}

QHash <QLCPoint,GroupHead> FixtureGroup::headHash() const
{
    return m_heads;
}

QList <quint32> FixtureGroup::fixtureList() const
{
    QList <quint32> list;
    foreach (GroupHead head, headList())
        list << head.fxi;
    return list;
}

Doc* FixtureGroup::doc() const
{
    return qobject_cast<Doc*> (parent());
}

void FixtureGroup::slotFixtureRemoved(quint32 id)
{
    // Remove the fixture from group records since it's no longer there
    resignFixture(id);
}

/****************************************************************************
 * Size
 ****************************************************************************/

void FixtureGroup::setSize(const QSize& sz)
{
    m_size = sz;
}

QSize FixtureGroup::size() const
{
    return m_size;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool FixtureGroup::loader(const QDomElement& root, Doc* doc)
{
    bool result = false;

    FixtureGroup* grp = new FixtureGroup(doc);
    Q_ASSERT(grp != NULL);

    if (grp->loadXML(root) == true)
    {
        doc->addFixtureGroup(grp, grp->id());
        result = true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "FixtureGroup" << grp->name() << "cannot be loaded.";
        delete grp;
        result = false;
    }

    return result;
}

bool FixtureGroup::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFixtureGroup)
    {
        qWarning() << Q_FUNC_INFO << "Fixture group node not found";
        return false;
    }

    bool ok = false;
    quint32 id = root.attribute(KXMLQLCFixtureGroupID).toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid FixtureGroup ID:" << root.attribute(KXMLQLCFixtureGroupID);
        return false;
    }

    // Assign the ID to myself
    m_id = id;

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCFixtureGroupHead)
        {
            bool ok = false;
            int x = tag.attribute("X").toInt(&ok);
            if (ok == false)
                continue;

            int y = tag.attribute("Y").toInt(&ok);
            if (ok == false)
                continue;

            quint32 id = tag.attribute("Fixture").toUInt(&ok);
            if (ok == false)
                continue;

            int head = tag.text().toInt(&ok);
            if (ok == false)
                continue;

            m_heads[QLCPoint(x, y)] = GroupHead(id, head);
        }
        else if (tag.tagName() == KXMLQLCFixtureGroupSize)
        {
            bool ok = false;
            int x = tag.attribute("X").toInt(&ok);
            if (ok == false)
                continue;
            int y = tag.attribute("Y").toInt(&ok);
            if (ok == false)
                continue;

            m_size = QSize(x, y);
        }
        else if (tag.tagName() == KXMLQLCFixtureGroupName)
        {
            m_name = tag.text();
        }
        else if (tag.tagName() == KXMLQLCFixtureGroupDisplayStyle)
        {
            if (tag.text().isEmpty() == false)
                m_displayStyle = tag.text().toInt();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown fixture group tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool FixtureGroup::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);

    /* Fixture Group entry */
    root = doc->createElement(KXMLQLCFixtureGroup);
    root.setAttribute(KXMLQLCFixtureGroupID, this->id());
    wksp_root->appendChild(root);

    /* Name */
    tag = doc->createElement(KXMLQLCFixtureGroupName);
    text = doc->createTextNode(name());
    tag.appendChild(text);
    root.appendChild(tag);

    /* Matrix size */
    tag = doc->createElement(KXMLQLCFixtureGroupSize);
    tag.setAttribute("X", size().width());
    tag.setAttribute("Y", size().height());
    root.appendChild(tag);

    /* Display style */
    tag = doc->createElement(KXMLQLCFixtureGroupDisplayStyle);
    text = doc->createTextNode(QString::number(displayStyle()));
    tag.appendChild(text);
    root.appendChild(tag);

    /* Fixture heads */
    QHashIterator <QLCPoint,GroupHead> it(m_heads);
    while (it.hasNext() == true)
    {
        it.next();
        tag = doc->createElement(KXMLQLCFixtureGroupHead);
        tag.setAttribute("X", it.key().x());
        tag.setAttribute("Y", it.key().y());
        tag.setAttribute("Fixture", it.value().fxi);
        text = doc->createTextNode(QString::number(it.value().head));
        tag.appendChild(text);
        root.appendChild(tag);
    }

    return true;
}
