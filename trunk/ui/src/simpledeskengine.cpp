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

#include <QVariant>
#include <QDebug>

#include "simpledeskengine.h"
#include "universearray.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "cuestack.h"
#include "doc.h"

#define PROP_ID "id"

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

void SimpleDeskEngine::setValue(uint channel, uchar value)
{
    if (value == 0 && m_values.contains(channel) == true)
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

CueStack* SimpleDeskEngine::cueStack(uint stack)
{
    if (m_cueStacks.contains(stack) == false)
    {
        m_cueStacks[stack] = new CueStack(doc());
        m_cueStacks[stack]->setProperty(PROP_ID, stack);
        connect(m_cueStacks[stack], SIGNAL(currentCueChanged(int)),
                this, SLOT(slotCurrentCueChanged(int)));
        connect(m_cueStacks[stack], SIGNAL(started()), this, SLOT(slotCueStackStarted()));
        connect(m_cueStacks[stack], SIGNAL(stopped()), this, SLOT(slotCueStackStopped()));
    }

    return m_cueStacks[stack];
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
