/*
  Q Light Controller
  cue.cpp

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

#include "cue.h"

Cue::Cue(const QString& name)
    : m_name(name)
{
}

Cue::Cue(const Cue& cue)
    : m_name(cue.name())
    , m_values(cue.values())
{
}

Cue::~Cue()
{
}

void Cue::setName(const QString& str)
{
    m_name = str;
}

QString Cue::name() const
{
    return m_name;
}

void Cue::setValue(uint channel, uchar value)
{
    m_values[channel] = value;
}

uchar Cue::value(uint channel) const
{
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

QHash <uint,uchar> Cue::values() const
{
    return m_values;
}
