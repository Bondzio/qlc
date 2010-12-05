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

FadeChannel::FadeChannel()
    : m_address(0)
    , m_group(QLCChannel::NoGroup)
    , m_start(0)
    , m_target(0)
    , m_current(0)
{
}

FadeChannel::~FadeChannel()
{
}

void FadeChannel::setAddress(quint32 addr)
{
    m_address = addr;
}

quint32 FadeChannel::address() const
{
    return m_address;
}

void FadeChannel::setGroup(QLCChannel::Group grp)
{
    m_group = grp;
}

QLCChannel::Group FadeChannel::group() const
{
    return m_group;
}

void FadeChannel::setStart(qint32 value)
{
    m_start = value;
}

qint32 FadeChannel::start() const
{
    return m_start;
}

void FadeChannel::setTarget(qint32 value)
{
    m_target = value;
}

qint32 FadeChannel::target() const
{
    return m_target;
}

void FadeChannel::setCurrent(qint32 value)
{
    m_current = value;
}

qint32 FadeChannel::current() const
{
    return m_current;
}

uchar FadeChannel::calculateCurrent(quint32 fadeTime, quint32 elapsedTime)
{
    // If all time has been consumed, return the target value
    if (elapsedTime >= fadeTime)
    {
        m_current = m_target;
        return m_current;
    }

    // Time scale is basically a percentage (0.0 - 1.0) of remaining time.
    qreal timeScale = qreal(elapsedTime) / qreal(fadeTime);

    m_current = m_target - m_start;
    m_current = qint32(qreal(m_current) * timeScale);
    m_current += m_start;

    return uchar(m_current);
}
