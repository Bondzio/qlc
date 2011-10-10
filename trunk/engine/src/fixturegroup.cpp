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
#define KXMLQLCFixtureGroupFixture "Fixture"
#define KXMLQLCFixtureGroupSize "Size"
#define KXMLQLCFixtureGroupName "Name"

/****************************************************************************
 * Initialization
 ****************************************************************************/

FixtureGroup::FixtureGroup(Doc* parent)
    : QObject(parent)
    , m_id(FixtureGroup::invalidId())
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
    m_fixtures = grp->fixtureHash();
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

QString FixtureGroup::infoText() const
{
    QString info;

    QString title("<TR><TD CLASS='hilite' COLSPAN='3'>%1: %2</TD></TR>");
    QString subTitle("<TR><TD CLASS='subhi' COLSPAN='3'>%1</TD></TR>");
    QString genInfo("<TR><TD CLASS='emphasis' COLSPAN='2'>%1</TD><TD>%2</TD></TR>");

    info += "<TABLE COLS='3' WIDTH='100%'>";
    info += title.arg(tr("Fixture Group")).arg(name());
    info += genInfo.arg(tr("Fixture count")).arg(fixtureList().size());
    info += subTitle.arg(tr("Grid Placement"));
    if (size().isValid() == true)
    {
        info += genInfo.arg(tr("Horizontal size")).arg(size().width());
        info += genInfo.arg(tr("Vertical size")).arg(size().height());

    }
    info += "</TABLE>";

    if (size().isValid() == true)
    {
        info += QString("<TABLE ROWS='%1' COLS='%2' WIDTH='100%' BORDER='1'>").arg(size().height() + 1).arg(size().width() + 1);

        // Horizontal header
        info += "<TR>";
        info += "<TD></TD>"; // Empty corner
        for (int x = 0; x < size().width(); x++)
        {
            info += QString("<TD CLASS='emphasis'>%1</TD>").arg(x + 1);
        }
        info += "</TR>";

        for (int y = 0; y < size().height(); y++)
        {
            info += "<TR>";
            info += QString("<TD CLASS='emphasis'>%1</TD>").arg(y + 1);
            for (int x = 0; x < size().width(); x++)
            {
                QLCPoint pt(x, y);
                if (m_fixtures.contains(pt) == true)
                {
                    Fixture* fxi = doc()->fixture(m_fixtures[pt]);
                    Q_ASSERT(fxi != NULL);
                    info += "<TD CLASS='tiny'>";
                    info += fxi->name();
                    info += "</TD>";
                }
                else
                {
                    info += "<TD class='tiny'>";
                    info += tr("---");
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

void FixtureGroup::assignFixture(quint32 id, const QLCPoint& pt)
{
    if (m_fixtures.values().contains(id) == true)
        return;

    if (size().isValid() == false)
        setSize(QSize(1, 1));

    if (pt.isNull() == false)
    {
        m_fixtures[pt] = id;
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
                    if (m_fixtures.contains(tmp) == false)
                    {
                        m_fixtures[tmp] = id;
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

void FixtureGroup::resignFixture(quint32 id)
{
    foreach (QLCPoint pt, m_fixtures.keys())
    {
        if (m_fixtures[pt] == id)
            m_fixtures.remove(pt);
    }
}

void FixtureGroup::swap(const QLCPoint& a, const QLCPoint& b)
{
    quint32 aId = Fixture::invalidId();
    quint32 bId = Fixture::invalidId();;

    if (m_fixtures.contains(a) == true)
        aId = m_fixtures[a];
    if (m_fixtures.contains(b) == true)
        bId = m_fixtures[b];

    if (aId != Fixture::invalidId())
        m_fixtures[b] = aId;
    else
        m_fixtures.remove(b);

    if (bId != Fixture::invalidId())
        m_fixtures[a] = bId;
    else
        m_fixtures.remove(a);
}

quint32 FixtureGroup::fixture(const QLCPoint& pt) const
{
    if (m_fixtures.contains(pt) == true)
        return m_fixtures[pt];
    else
        return Fixture::invalidId();
}

QList <quint32> FixtureGroup::fixtureList() const
{
    return m_fixtures.values();
}

QHash <QLCPoint,quint32> FixtureGroup::fixtureHash() const
{
    return m_fixtures;
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

bool FixtureGroup::loader(const QDomElement* root, Doc* doc)
{
    bool result = false;

    FixtureGroup* grp = new FixtureGroup(doc);
    Q_ASSERT(grp != NULL);

    if (grp->loadXML(root) == true)
    {
        doc->addFixtureGroup(grp, grp->id());
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "FixtureGroup" << grp->name() << "cannot be loaded.";
        delete grp;
    }

    return result;
}

bool FixtureGroup::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFixtureGroup)
    {
        qWarning() << Q_FUNC_INFO << "Fixture group node not found";
        return false;
    }

    bool ok = false;
    quint32 id = root->attribute(KXMLQLCFixtureGroupID).toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid FixtureGroup ID:" << root->attribute(KXMLQLCFixtureGroupID);
        return false;
    }

    // Assign the ID to myself
    m_id = id;

    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCFixtureGroupFixture)
        {
            bool ok = false;
            int x = tag.attribute("X").toInt(&ok);
            if (ok == false)
                continue;

            int y = tag.attribute("Y").toInt(&ok);
            if (ok == false)
                continue;

            quint32 id = tag.text().toUInt(&ok);
            if (ok == false)
                continue;

            m_fixtures[QLCPoint(x, y)] = id;
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

    /* Matrix */
    tag = doc->createElement(KXMLQLCFixtureGroupSize);
    tag.setAttribute("X", size().width());
    tag.setAttribute("Y", size().height());
    root.appendChild(tag);

    /* Fixtures */
    QHashIterator <QLCPoint,quint32> it(m_fixtures);
    while (it.hasNext() == true)
    {
        it.next();
        tag = doc->createElement(KXMLQLCFixtureGroupFixture);
        tag.setAttribute("X", QString::number(it.key().x()));
        tag.setAttribute("Y", QString::number(it.key().y()));
        text = doc->createTextNode(QString::number(it.value()));
        tag.appendChild(text);
        root.appendChild(tag);
    }

    return true;
}
