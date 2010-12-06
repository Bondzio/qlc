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

ChaserRunner::ChaserRunner(Doc* doc, QList <Scene*> steps,
                           quint32 holdBusId,
                           Function::Direction direction,
                           Function::RunOrder runOrder)
    : m_doc(doc)
    , m_steps(steps)
    , m_holdBusId(holdBusId)
    , m_direction(direction)
    , m_runOrder(runOrder)
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
    m_tap = false;
    m_channelMap.clear();
}

bool ChaserRunner::write(UniverseArray* universes)
{
    // Nothing to do
    if (m_steps.size() == 0)
        return false;

    if (m_elapsed == 0)
    {
        // First step
        if (m_direction == Function::Forward)
            m_currentStep = 0;
        else
            m_currentStep = m_steps.size() - 1;

        m_elapsed = 1;
        createFadeChannels(universes);
    }
    else if (m_elapsed >= Bus::instance()->value(m_holdBusId) || m_tap == true)
    {
        // Next step
        if (m_direction == Function::Forward)
            m_currentStep++;
        else
            m_currentStep--;

        if (roundCheck() == false)
            return false;

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

    return true;
}

bool ChaserRunner::roundCheck()
{
    if (m_currentStep >= m_steps.size() || m_currentStep < 0)
    {
        // Either end has been reached
        if (m_runOrder == Function::SingleShot)
        {
            // SingleShot has been completed -> stop.
            return false;
        }
        else if (m_runOrder == Function::Loop)
        {
            if (m_direction == Function::Forward)
                m_currentStep = 0;
            else
                m_currentStep = m_steps.size() - 1;
        }
        else // Ping Pong
        {
            // Change running direction, don't run the step at each end twice;
            // thus -1/+1
            if (m_direction == Function::Forward)
            {
                m_currentStep = m_steps.size() - 2;
                m_direction = Function::Backward;
            }
            else
            {
                m_currentStep = 1;
                m_direction = Function::Forward;
            }
        }
    }

    // Let's continue
    return true;
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
