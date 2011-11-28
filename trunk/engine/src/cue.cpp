/*
  Q Light Controller
  cue.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "cue.h"

Cue::Cue(const QString& name)
    : m_name(name)
{
}

Cue::Cue(const QHash <uint,uchar> values)
    : m_name(QString())
    , m_values(values)
{
}

Cue::Cue(const Cue& cue)
    : m_name(cue.name())
    , m_values(cue.values())
{
}

Cue::~Cue()
{
}

void Cue::setName(const QString& str)
{
    m_name = str;
}

QString Cue::name() const
{
    return m_name;
}

void Cue::setValue(uint channel, uchar value)
{
    m_values[channel] = value;
}

void Cue::unsetValue(uint channel)
{
    if (m_values.contains(channel) == true)
        m_values.remove(channel);
}

uchar Cue::value(uint channel) const
{
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

QHash <uint,uchar> Cue::values() const
{
    return m_values;
}

bool Cue::loadXML(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;

    if (root.tagName() != KXMLQLCCue)
    {
        qWarning() << Q_FUNC_INFO << "Cue node not found";
        return false;
    }

    setName(root.attribute(KXMLQLCCueName));

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCCueValue)
        {
            QString ch = tag.attribute(KXMLQLCCueValueChannel);
            QString val = tag.text();
            if (ch.isEmpty() == false && val.isEmpty() == false)
                setValue(ch.toUInt(), uchar(val.toUInt()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Cue tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool Cue::saveXML(QDomDocument* doc, QDomElement* stack_root) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    Q_ASSERT(stack_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCCue);
    root.setAttribute(KXMLQLCCueName, name());
    stack_root->appendChild(root);

    QHashIterator <uint,uchar> it(values());
    while (it.hasNext() == true)
    {
        it.next();
        QDomElement e = doc->createElement(KXMLQLCCueValue);
        e.setAttribute(KXMLQLCCueValueChannel, it.key());
        QDomText t = doc->createTextNode(QString::number(it.value()));
        e.appendChild(t);
        root.appendChild(e);
    }

    return true;
}
