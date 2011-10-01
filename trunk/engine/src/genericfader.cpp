/*
  Q Light Controller
  genericfader.cpp

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

#include <cmath>
#include <QDebug>

#include "universearray.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "doc.h"

GenericFader::GenericFader(Doc* doc)
    : m_intensity(1)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
}

GenericFader::~GenericFader()
{
}

void GenericFader::add(const FadeChannel& ch)
{
    quint32 addr = ch.address(m_doc);

    if (m_channels.contains(addr) == true)
    {
        if (m_channels[addr].current() < ch.current())
            m_channels[addr] = ch;
    }
    else
    {
        m_channels[addr] = ch;
    }
}

void GenericFader::remove(quint32 address)
{
    if (m_channels.contains(address) == true)
        m_channels.remove(address);
}

void GenericFader::remove(const FadeChannel& fc)
{
    remove(fc.address(m_doc));
}

void GenericFader::removeAll()
{
    m_channels.clear();
}

const QHash<quint32,FadeChannel>& GenericFader::channels() const
{
    return m_channels;
}

void GenericFader::write(UniverseArray* ua)
{
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        if (fc.elapsed() >= fc.fadeTime())
        {
            if (fc.group(m_doc) == QLCChannel::Intensity || fc.isReady() == false)
            {
                fc.setReady(true);
                uchar value = uchar(floor((qreal(fc.target()) * intensity()) + 0.5));
                ua->write(fc.address(m_doc), value, fc.group(m_doc));

                // Remove all channels that reach zero
                if (fc.target() == 0 && fc.current() == 0)
                    remove(fc.address(m_doc));
            }
            else
            {
                // After an LTP channel becomes ready, its value is no longer written.
                // Remove it from the fader.
                remove(fc.address(m_doc));
            }
        }
        else
        {
            uchar value = uchar(floor((qreal(fc.current()) * intensity()) + 0.5));
            ua->write(fc.address(m_doc), value, fc.group(m_doc));
            fc.nextStep();
        }
    }
}

void GenericFader::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
}

qreal GenericFader::intensity() const
{
    return m_intensity;
}
