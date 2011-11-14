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

#include "dmxsource.h"
#include "cue.h"

class UniverseArray;
class MasterTimer;
class Doc;

class CueStack : public QObject, public DMXSource
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    CueStack(Doc* doc);
    ~CueStack();

    Doc* doc() const;

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
    void clear();
    void addCue(const Cue& c);
    QList <Cue> cues() const;

private:
    QList <Cue> m_cues;

    /************************************************************************
     * DMXSource
     ************************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer* timer, UniverseArray* ua);

private:
    GenericFader* m_fader;
};

#endif
