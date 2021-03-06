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
#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

ChaserRunner::ChaserRunner(Doc* doc, QList <Function*> steps,
                           quint32 holdBusId,
                           Function::Direction direction,
                           Function::RunOrder runOrder,
                           qreal intensity,
                           QObject* parent)
    : QObject(parent)
    , m_doc(doc)
    , m_steps(steps)
    , m_holdBusId(holdBusId)
    , m_originalDirection(direction)
    , m_runOrder(runOrder)

    , m_autoStep(true)
    , m_direction(direction)
    , m_elapsed(0)
    , m_next(false)
    , m_previous(false)
    , m_currentStep(0)
    , m_newCurrent(-1)
    , m_intensity(intensity)
{
    reset();
}

ChaserRunner::~ChaserRunner()
{
}

void ChaserRunner::next()
{
    m_next = true;
    m_previous = false;
}

void ChaserRunner::previous()
{
    m_next = false;
    m_previous = true;
}

void ChaserRunner::setCurrentStep(int step)
{
    if (step >= 0 && step < m_steps.size())
    {
        m_newCurrent = step;
        m_next = false;
        m_previous = false;
    }
}

int ChaserRunner::currentStep() const
{
    return m_currentStep;
}

void ChaserRunner::setAutoStep(bool autoStep)
{
    m_autoStep = autoStep;
}

bool ChaserRunner::isAutoStep() const
{
    return m_autoStep;
}

void ChaserRunner::reset()
{
    // Restore original direction since Ping-Pong switches m_direction
    m_direction = m_originalDirection;

    if (m_direction == Function::Backward)
        m_currentStep = m_steps.size() - 1;
    else
        m_currentStep = 0;
    m_elapsed = 0;
    m_next = false;
    m_previous = false;
    m_channelMap.clear();
}

bool ChaserRunner::write(MasterTimer* timer, UniverseArray* universes)
{
    // Nothing to do
    if (m_steps.size() == 0)
        return false;

    if (m_newCurrent != -1)
    {
        // Manually-set current step
        m_currentStep = m_newCurrent;
        m_newCurrent = -1;

        // No need to do roundcheck here, since manually-set steps are
        // always within m_steps limits.

        m_elapsed = 1;
        handleChannelSwitch(timer, universes);
        emit currentStepChanged(m_currentStep);
    }
    else if (m_elapsed == 0)
    {
        // First step
        m_elapsed = 1;
        handleChannelSwitch(timer, universes);
        emit currentStepChanged(m_currentStep);
    }
    else if ((isAutoStep() && m_elapsed >= Bus::instance()->value(m_holdBusId))
             || m_next == true || m_previous == true)
    {
        // Next step
        if (m_direction == Function::Forward)
        {
            // "Previous" for a forwards chaser is -1
            if (m_previous == true)
                m_currentStep--;
            else
                m_currentStep++;
        }
        else
        {
            // "Previous" for a backwards scene is +1
            if (m_previous == true)
                m_currentStep++;
            else
                m_currentStep--;
        }

        if (roundCheck() == false)
            return false;

        m_elapsed = 1;
        m_next = false;
        m_previous = false;

        handleChannelSwitch(timer, universes);
        emit currentStepChanged(m_currentStep);
    }
    else
    {
        // Current step. UINT_MAX is the maximum hold time.
        if (m_elapsed < UINT_MAX)
            m_elapsed++;
    }

    QMutableMapIterator <quint32,FadeChannel> it(m_channelMap);
    while (it.hasNext() == true)
    {
        Scene* scene = qobject_cast<Scene*> (m_steps.at(m_currentStep));
        if (scene == NULL)
            continue;

        quint32 fadeTime = Bus::instance()->value(scene->busID());

        FadeChannel& channel(it.next().value());
        if (channel.current() == channel.target() && channel.group() != QLCChannel::Intensity)
        {
            /* Write the final value to LTP channels only once */
        }
        else
        {
            uchar value = uchar(floor((qreal(channel.calculateCurrent(fadeTime, m_elapsed)) * m_intensity) + 0.5));
            universes->write(channel.address(), value, channel.group());
        }
    }

    return true;
}

void ChaserRunner::postRun(MasterTimer* timer, UniverseArray* universes)
{
    quint32 bus = Bus::defaultFade();
    Q_UNUSED(universes);

    // Nothing to do
    if (m_steps.size() == 0)
        return;

    int step = CLAMP(m_currentStep, 0, m_steps.size() - 1);
    Function* function = m_steps.at(step);
    if (function != NULL)
        bus = function->busID();

    QMapIterator <quint32,FadeChannel> it(m_channelMap);
    while (it.hasNext() == true)
    {
        FadeChannel ch(it.next().value());
        if (ch.group() == QLCChannel::Intensity)
        {
            ch.setStart(ch.current());
            ch.setTarget(0);
            ch.setBus(bus);
            ch.setReady(false);
            timer->fader()->add(ch, false);
        }
    }
}

bool ChaserRunner::roundCheck()
{
    if (m_currentStep < m_steps.size() && m_currentStep >= 0)
        return true; // In the middle of steps. No need to go any further.

    if (m_runOrder == Function::SingleShot)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                return false; // Forwards SingleShot has been completed.
            else
                m_currentStep = 0; // No wrapping
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                return false; // Backwards SingleShot has been completed.
            else
                m_currentStep = m_steps.size() - 1; // No wrapping
        }
    }
    else if (m_runOrder == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                m_currentStep = 0;
            else
                m_currentStep = m_steps.size() - 1;
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_steps.size() - 1;
            else
                m_currentStep = 0;
        }
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                m_currentStep = 1;
            else
                m_currentStep = m_steps.size() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_steps.size() - 2;
            else
                m_currentStep = 1;
            m_direction = Function::Forward;
        }

        // Make sure we don't go beyond limits.
        m_currentStep = CLAMP(m_currentStep, 0, m_steps.size() - 1);
    }

    // Let's continue
    return true;
}

void ChaserRunner::handleChannelSwitch(MasterTimer* timer, const UniverseArray* universes)
{
    QMap <quint32,FadeChannel> zeroChannels;

    // Create a channel map for the current step and pick a list of channels
    // that can be given back to MasterTimer's GenericFader to be faded back to zero
    m_channelMap = createFadeChannels(universes, zeroChannels);

    // Give to-be-zeroed channels to MasterTimer, but don't overwrite channels
    QMapIterator <quint32,FadeChannel> it(zeroChannels);
    while (it.hasNext() == true)
        timer->fader()->add(it.next().value(), false);
}

QMap <quint32,FadeChannel>
ChaserRunner::createFadeChannels(const UniverseArray* universes,
                                 QMap <quint32,FadeChannel>& zeroChannels) const
{
    QMap <quint32,FadeChannel> map;
    if (m_currentStep >= m_steps.size() || m_currentStep < 0)
        return map;

    Scene* scene = qobject_cast<Scene*> (m_steps.at(m_currentStep));
    Q_ASSERT(scene != NULL);

    // Put all current channels to a map of channels that will be faded to zero.
    // If the same channels are added to the new channel map, they are removed
    // from this zero map.
    zeroChannels = m_channelMap;

    QListIterator <SceneValue> it(scene->values());
    while (it.hasNext() == true)
    {
        SceneValue value(it.next());
        Fixture* fxi = m_doc->fixture(value.fxi);
        if (fxi == NULL || fxi->channel(value.channel) == NULL)
            continue;

        FadeChannel channel;
        channel.setAddress(fxi->universeAddress() + value.channel);
        channel.setGroup(fxi->channel(value.channel)->group());
        channel.setTarget(value.value);

        // Get starting value from universes. For HTP channels it's always 0.
        channel.setStart(uchar(universes->preGMValues()[channel.address()]));

        // Transfer last step's current value to current step's starting value.
        if (m_channelMap.contains(channel.address()) == true)
            channel.setStart(m_channelMap[channel.address()].current());
        channel.setCurrent(channel.start());

        // Append the channel to the channel map
        map[channel.address()] = channel;

        // Remove the channel from a map of to-be-zeroed channels since now it
        // has a new value to fade to.
        zeroChannels.remove(channel.address());
    }

    // All channels that were present in the previous step but are not present
    // in the current step will go through this zeroing process.
    QMutableMapIterator <quint32,FadeChannel> zit(zeroChannels);
    while (zit.hasNext() == true)
    {
        zit.next();
        FadeChannel& channel(zit.value());
        if (channel.current() == 0 || channel.group() != QLCChannel::Intensity)
        {
            // Remove all non-HTP channels and such HTP channels that are
            // already at zero. There's nothing to do for them.
            zit.remove();
        }
        else
        {
            // This HTP channel was present in the previous step, but is absent
            // in the current. It's nicer that we fade it back to zero, rather
            // than just let it drop straight to zero.
            channel.setStart(channel.current());
            channel.setTarget(0);
        }
    }

    return map;
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void ChaserRunner::adjustIntensity(qreal fraction)
{
    m_intensity = CLAMP(fraction, qreal(0.0), qreal(1.0));
}
