/*
  Q Light Controller
  cuestack.h

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

#ifndef CUESTACK_H
#define CUESTACK_H

#include <QObject>
#include <QList>

#include "cue.h"

class CueStack
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    CueStack();
    CueStack(const CueStack& cs);
    ~CueStack();

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    void setFadeInSpeed(uint ms);
    uint fadeInSpeed() const;

    void setFadeOutSpeed(uint ms);
    uint fadeOutSpeed() const;

    void setDuration(uint ms);
    uint duration() const;

private:
    uint m_fadeInSpeed;
    uint m_fadeOutSpeed;
    uint m_duration;

    /************************************************************************
     * Cues
     ************************************************************************/
public:
    void addCue(const Cue& c);
    void removeCue(int index);

    QList <Cue> cues() const;

    void setCurrentIndex(int index);
    int currentIndex() const;

private:
    QList <Cue> m_cues;
    int m_currentIndex;
};

#endif
