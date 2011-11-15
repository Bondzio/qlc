/*
  Q Light Controller
  cuestack.cpp

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

#include "qlcmacros.h"
#include "cuestack.h"
#include "cue.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

CueStack::CueStack()
    : m_fadeInSpeed(0)
    , m_fadeOutSpeed(0)
    , m_duration(1000)
    , m_currentIndex(-1)
{
}

CueStack::CueStack(const CueStack& cs)
    : m_fadeInSpeed(cs.fadeInSpeed())
    , m_fadeOutSpeed(cs.fadeOutSpeed())
    , m_duration(cs.duration())
    , m_cues(cs.cues())
    , m_currentIndex(cs.currentIndex())
{
}

CueStack::~CueStack()
{
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void CueStack::setFadeInSpeed(uint ms)
{
    m_fadeInSpeed = ms;
}

uint CueStack::fadeInSpeed() const
{
    return m_fadeInSpeed;
}

void CueStack::setFadeOutSpeed(uint ms)
{
    m_fadeOutSpeed = ms;
}

uint CueStack::fadeOutSpeed() const
{
    return m_fadeOutSpeed;
}

void CueStack::setDuration(uint ms)
{
    m_duration = ms;
}

uint CueStack::duration() const
{
    return m_duration;
}

/****************************************************************************
 * Cues
 ****************************************************************************/

void CueStack::addCue(const Cue& c)
{
    m_cues.append(c);
}

void CueStack::removeCue(int index)
{
    if (index >= 0 && index < m_cues.size())
        m_cues.removeAt(index);
}

QList <Cue> CueStack::cues() const
{
    return m_cues;
}

void CueStack::setCurrentIndex(int index)
{
    if (m_cues.size() > 0)
        m_currentIndex = CLAMP(index, 0, m_cues.size());
    else
        m_currentIndex = -1;
}

int CueStack::currentIndex() const
{
    return m_currentIndex;
}
