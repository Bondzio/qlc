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

#ifndef CUE_H
#define CUE_H

#include "cue.h"

Cue::Cue()
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

void Cue::setValue(const SceneValue& value)
{
    if (m_values.contains(value) == false)
        m_values.append(value);
    qSort(m_values.begin(), m_values.end());
}

SceneValue Cue::value(quint32 fxi, quint32 ch)
{
    SceneValue val(fxi, ch);
    int index = m_values.indexOf(val);
    if (index != -1)
        return m_values[index];
    else
        return SceneValue();
}

QList <SceneValue> Cue::values() const
{
    return m_values;
}
