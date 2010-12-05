/*
  Q Light Controller
  chaserrunner.cpp

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

#include <QDebug>

#include "universearray.h"
#include "chaserrunner.h"
#include "fadechannel.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

ChaserRunner::ChaserRunner(Doc* doc, QList <Scene*> steps)
    : m_doc(doc)
    , m_steps(steps)
    , m_elapsed(0)
    , m_tap(false)
    , m_currentStep(0)
{
}

ChaserRunner::~ChaserRunner()
{
}

void ChaserRunner::tap()
{
    m_tap = true;
}

void ChaserRunner::reset()
{
    m_currentStep = 0;
    m_elapsed = 0;
    m_channelMap.clear();
}

void ChaserRunner::write(UniverseArray* universes, quint32 hold)
{
    // Nothing to do
    if (m_steps.size() == 0)
        return;

    if (m_elapsed == 0)
    {
        // First step
        m_currentStep = 0;
        m_elapsed = 1;

        createFadeChannels(universes);
    }
    else if (m_elapsed >= hold || m_tap == true)
    {
        // Next step
        m_currentStep++;
        if (m_currentStep >= m_steps.size())
            m_currentStep = 0;
        m_elapsed = 1;
        m_tap = false;

        createFadeChannels(universes, true);
    }
    else
    {
        // Current step
        m_elapsed++;
    }

    QMutableMapIterator <quint32,FadeChannel> it(m_channelMap);
    while (it.hasNext() == true)
    {
        Scene* scene = m_steps.at(m_currentStep);
        Q_ASSERT(scene != NULL);
        quint32 fadeTime = Bus::instance()->value(scene->busID());

        FadeChannel& channel(it.next().value());
        if (channel.current() == channel.target() && channel.group() != QLCChannel::Intensity)
        {
            /* Write the final value to LTP channels only once */
        }
        else
        {
            universes->write(channel.address(),
                             channel.calculateCurrent(fadeTime, m_elapsed),
                             channel.group());
        }
    }
}

void ChaserRunner::createFadeChannels(UniverseArray* universes, bool handover)
{
    Scene* scene = m_steps.at(m_currentStep);
    Q_ASSERT(scene != NULL);

    QMap <quint32,FadeChannel> channelMap;

    QListIterator <SceneValue> it(scene->values());
    while (it.hasNext() == true)
    {
        SceneValue value(it.next());
        Fixture* fxi = m_doc->fixture(value.fxi);
        Q_ASSERT(fxi != NULL);

        FadeChannel channel;
        channel.setAddress(fxi->universeAddress() + value.channel);
        channel.setGroup(fxi->channel(value.channel)->group());
        channel.setTarget(value.value);

        channel.setStart(uchar(universes->preGMValues()[channel.address()]));
        if (handover == true && m_channelMap.contains(channel.address()) == true)
            channel.setStart(m_channelMap[channel.address()].current());
        channel.setCurrent(channel.start());
        channelMap[channel.address()] = channel;
    }

    m_channelMap = channelMap;
}
