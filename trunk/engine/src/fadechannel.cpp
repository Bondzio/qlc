/*
  Q Light Controller
  fadechannel.cpp

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

#include "fadechannel.h"
#include "qlcchannel.h"
#include "fixture.h"
#include "bus.h"

FadeChannel::FadeChannel()
    : m_fixture(Fixture::invalidId())
    , m_channel(QLCChannel::invalid())
    , m_start(0)
    , m_target(0)
    , m_current(0)
    , m_ready(false)
    , m_bus(Bus::invalid())
    , m_fixedTime(0)
    , m_elapsed(0)
    , m_removeWhenTargetReached(false)
{
}

FadeChannel::FadeChannel(const FadeChannel& ch)
    : m_fixture(ch.m_fixture)
    , m_channel(ch.m_channel)
    , m_start(ch.m_start)
    , m_target(ch.m_target)
    , m_current(ch.m_current)
    , m_ready(ch.m_ready)
    , m_bus(ch.m_bus)
    , m_fixedTime(ch.m_fixedTime)
    , m_elapsed(ch.m_elapsed)
    , m_removeWhenTargetReached(ch.m_removeWhenTargetReached)
{
}

FadeChannel::~FadeChannel()
{
}

bool FadeChannel::operator==(const FadeChannel& ch) const
{
    return (m_fixture == ch.m_fixture && m_channel == ch.m_channel);
}

void FadeChannel::setFixture(quint32 id)
{
    m_fixture = id;
}

quint32 FadeChannel::fixture() const
{
    return m_fixture;
}

void FadeChannel::setChannel(quint32 num)
{
    m_channel = num;
}

quint32 FadeChannel::channel() const
{
    return m_channel;
}

quint32 FadeChannel::address(const Doc* doc) const
{
    Fixture* fxi = doc->fixture(fixture());
    if (fxi == NULL)
        return QLCChannel::invalid();
    return (fxi->universeAddress() + channel());
}

QLCChannel::Group FadeChannel::group(const Doc* doc) const
{
    Fixture* fxi = doc->fixture(fixture());
    if (fxi == NULL)
        return QLCChannel::Intensity;
    const QLCChannel* ch = fxi->channel(channel());
    if (ch == NULL)
        return QLCChannel::Intensity;
    return ch->group();
}

void FadeChannel::setStart(uchar value)
{
    m_start = value;
}

uchar FadeChannel::start() const
{
    return m_start;
}

void FadeChannel::setTarget(uchar value)
{
    m_target = value;
}

uchar FadeChannel::target() const
{
    return m_target;
}

void FadeChannel::setCurrent(uchar value)
{
    m_current = value;
}

uchar FadeChannel::current() const
{
    return m_current;
}

void FadeChannel::setReady(bool rdy)
{
    m_ready = rdy;
}

bool FadeChannel::isReady() const
{
    return m_ready;
}

void FadeChannel::setBus(quint32 busId)
{
    m_bus = busId;
}

quint32 FadeChannel::bus() const
{
    return m_bus;
}

void FadeChannel::setFixedTime(quint32 ticks)
{
    m_fixedTime = ticks;
}

quint32 FadeChannel::fixedTime() const
{
    return m_fixedTime;
}

void FadeChannel::setElapsed(quint32 time)
{
    m_elapsed = time;
}

quint32 FadeChannel::elapsed() const
{
    return m_elapsed;
}

quint32 FadeChannel::fadeTime() const
{
    if (bus() != Bus::invalid())
        return Bus::instance()->value(m_bus);
    else
        return m_fixedTime;
}

uchar FadeChannel::nextStep()
{
    if (elapsed() < UINT_MAX)
        setElapsed(elapsed() + 1);
    return calculateCurrent(fadeTime(), elapsed());
}

uchar FadeChannel::calculateCurrent(quint32 fadeTime, quint32 elapsedTime)
{
    // Return the target value if all time has been consumed or the channel
    // has been marked ready.
    if (elapsedTime >= fadeTime || m_ready == true)
    {
        m_current = m_target;
        return m_current;
    }

    // Time scale is basically a percentage (0.0 - 1.0) of remaining time.
    // Add 1.0 to both to get correct scale (fadeTime==1 means two steps)
    qreal timeScale = qreal(elapsedTime + 1.0) / qreal(fadeTime + 1.0);

    m_current = m_target - m_start;
    m_current = qint32(qreal(m_current) * timeScale);
    m_current += m_start;

    return static_cast<uchar>(m_current);
}

uint qHash(const FadeChannel& key)
{
    uint hash = key.fixture() << 16;
    hash = hash | (key.channel() & 0xFFFF);
    hash = hash & (~0U);
    return hash;
}
