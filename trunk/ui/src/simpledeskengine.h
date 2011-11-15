/*
  Q Light Controller
  simpledeskengine.h

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

#ifndef SIMPLEDESKENGINE_H
#define SIMPLEDESKENGINE_H

#include <QHash>
#include <QList>
#include "dmxsource.h"

class UniverseArray;
class MasterTimer;
class CueStack;
class Doc;

class SimpleDeskEngine : public DMXSource
{
public:
    SimpleDeskEngine(Doc* doc);
    virtual ~SimpleDeskEngine();

    void setValue(uint channel, uchar value);
    uchar value(uint channel) const;

    /** Get current universe contents (as seen by the engine) */
    QHash <uint,uchar> values() const;

    CueStack* cueStack(uint pb);

    /** @reimpl */
    void writeDMX(MasterTimer* timer, UniverseArray* ua);

private:
    Doc* m_doc;
    QHash <uint,uchar> m_values;
    QHash <uint,CueStack*> m_cueStacks;
};

#endif
