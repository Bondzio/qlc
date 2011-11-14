/*
  Q Light Controller
  simpledeskengine.cpp

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

#include "simpledeskengine.h"
#include "universearray.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "doc.h"

SimpleDeskEngine::SimpleDeskEngine(Doc* doc)
    : m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    m_doc->masterTimer()->registerDMXSource(this);
}

SimpleDeskEngine::~SimpleDeskEngine()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
}

void SimpleDeskEngine::setValue(uint channel, uchar value)
{
    if (value == 0 && m_values.contains(channel) == true)
        m_values.remove(channel);
    else
        m_values[channel] = value;
}

uchar SimpleDeskEngine::value(uint channel) const
{
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

void SimpleDeskEngine::writeDMX(MasterTimer* timer, UniverseArray* ua)
{
    QHashIterator <uint,uchar> it(m_values);
    while (it.hasNext() == true)
    {
        it.next();

        Fixture* fxi = m_doc->fixture(m_doc->fixtureForAddress(it.key()));
        if (fxi == NULL || fxi->isDimmer() == true)
        {
            ua->write(it.key(), it.value(), QLCChannel::Intensity);
        }
        else
        {
            uint ch = it.key() - fxi->universeAddress();
            QLCChannel::Group grp = QLCChannel::NoGroup;
            if (ch < fxi->channels())
                grp = fxi->channel(ch)->group();
            else
                grp = QLCChannel::Intensity;
            ua->write(it.key(), it.value(), grp);
        }
    }
}
