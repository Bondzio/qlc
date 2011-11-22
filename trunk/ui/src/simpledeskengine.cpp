/*
  Q Light Controller
  simpledeskengine.cpp

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
#include <QVariant>
#include <QDebug>

#include "simpledeskengine.h"
#include "universearray.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "outputmap.h"
#include "cuestack.h"
#include "doc.h"

#define PROP_ID "id"

/****************************************************************************
 * Initialization
 ****************************************************************************/

SimpleDeskEngine::SimpleDeskEngine(Doc* doc)
    : QObject(doc)
{
    Q_ASSERT(doc != NULL);
    doc->masterTimer()->registerDMXSource(this);
}

SimpleDeskEngine::~SimpleDeskEngine()
{
    doc()->masterTimer()->unregisterDMXSource(this);

    m_mutex.lock();
    foreach (CueStack* cs, m_cueStacks.values())
        delete cs;
    m_cueStacks.clear();
    m_mutex.unlock();
}

Doc* SimpleDeskEngine::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/****************************************************************************
 * Universe Values
 ****************************************************************************/

void SimpleDeskEngine::setValue(uint channel, uchar value)
{
    // Use FadeChannel's reverse-lookup to dig up the channel's group
    FadeChannel fc;
    fc.setChannel(channel);
    QLCChannel::Group group = fc.group(doc());

    if (value == 0 && group == QLCChannel::Intensity)
        m_values.remove(channel);
    else
        m_values[channel] = value;
}

uchar SimpleDeskEngine::value(uint channel) const
{
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

QHash <uint,uchar> SimpleDeskEngine::values() const
{
    return m_values;
}

/****************************************************************************
 * Cue Stacks
 ****************************************************************************/

CueStack* SimpleDeskEngine::cueStack(uint stack)
{
    if (m_cueStacks.contains(stack) == false)
    {
        m_cueStacks[stack] = createCueStack();
        m_cueStacks[stack]->setProperty(PROP_ID, stack);
    }

    return m_cueStacks[stack];
}

CueStack* SimpleDeskEngine::createCueStack()
{
    CueStack* cs = new CueStack(doc());
    Q_ASSERT(cs != NULL);
    connect(cs, SIGNAL(currentCueChanged(int)), this, SLOT(slotCurrentCueChanged(int)));
    connect(cs, SIGNAL(started()), this, SLOT(slotCueStackStarted()));
    connect(cs, SIGNAL(stopped()), this, SLOT(slotCueStackStopped()));
    return cs;
}

void SimpleDeskEngine::replaceCueStack(uint stack, CueStack* cs)
{
    Q_ASSERT(cs != NULL);

    if (m_cueStacks.contains(stack) == true)
        delete m_cueStacks[stack];
    m_cueStacks[stack] = cs;
}

void SimpleDeskEngine::slotCurrentCueChanged(int index)
{
    Q_ASSERT(sender() != NULL);
    uint stack = sender()->property(PROP_ID).toUInt();
    emit currentCueChanged(stack, index);
}

void SimpleDeskEngine::slotCueStackStarted()
{
    Q_ASSERT(sender() != NULL);
    uint stack = sender()->property(PROP_ID).toUInt();
    emit cueStackStarted(stack);
}

void SimpleDeskEngine::slotCueStackStopped()
{
    Q_ASSERT(sender() != NULL);
    uint stack = sender()->property(PROP_ID).toUInt();
    emit cueStackStopped(stack);
}

/************************************************************************
 * Save & Load
 ************************************************************************/

bool SimpleDeskEngine::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCSimpleDeskEngine)
    {
        qWarning() << Q_FUNC_INFO << "Simple Desk Engine node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCCueStack)
        {
            uint id = 0;
            CueStack* cs = createCueStack();
            if (cs->loadXML(tag, id) == true)
                replaceCueStack(id, cs);
            else
                delete cs;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Simple Desk Engine tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool SimpleDeskEngine::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCSimpleDeskEngine);
    wksp_root->appendChild(root);

    QHashIterator <uint,CueStack*> it(m_cueStacks);
    while (it.hasNext() == true)
    {
        it.next();
        it.value()->saveXML(doc, &root, it.key());
    }

    return true;
}

/****************************************************************************
 * DMXSource
 ****************************************************************************/

void SimpleDeskEngine::writeDMX(MasterTimer* timer, UniverseArray* ua)
{
    QHashIterator <uint,uchar> it(m_values);
    while (it.hasNext() == true)
    {
        it.next();

        Fixture* fxi = doc()->fixture(doc()->fixtureForAddress(it.key()));
        if (fxi == NULL || fxi->isDimmer() == true)
        {
            ua->write(it.key(), it.value(), QLCChannel::Intensity);
        }
        else
        {
            uint ch = it.key() - fxi->universeAddress();
            QLCChannel::Group grp = QLCChannel::NoGroup;
            if (ch < fxi->channels())
                grp = fxi->channel(ch)->group();
            else
                grp = QLCChannel::Intensity;
            ua->write(it.key(), it.value(), grp);
        }
    }

    m_mutex.lock();
    foreach (CueStack* cueStack, m_cueStacks)
    {
        if (cueStack->isRunning() == true)
        {
            if (cueStack->isStarted() == false)
                cueStack->preRun();

            cueStack->write(ua);
        }
        else
        {
            if (cueStack->isStarted() == true)
                cueStack->postRun(timer);
        }
    }
    m_mutex.unlock();
}
