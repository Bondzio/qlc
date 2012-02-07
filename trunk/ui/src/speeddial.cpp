/*
  Q Light Controller
  speeddial.cpp

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

#include <QLayout>
#include <QDial>

#include "speedspinbox.h"
#include "mastertimer.h"
#include "speeddial.h"

#define LOWLIMIT  0
#define HIGHLIMIT 99
#define THRESHOLD 50

SpeedDial::SpeedDial(QWidget* parent)
    : QGroupBox(parent)
    , m_dial(NULL)
    , m_spin(NULL)
    , m_previousDialValue(0)
    , m_preventSignals(false)
{
    new QVBoxLayout(this);

    m_dial = new QDial(this);
    m_dial->setWrapping(true);
    m_dial->setNotchesVisible(true);
    m_dial->setTracking(true);
    m_dial->setRange(LOWLIMIT, HIGHLIMIT);
    layout()->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialChanged(int)));

    m_spin = new SpeedSpinBox(SpeedSpinBox::Zero, this);
    layout()->addWidget(m_spin);
    connect(m_spin, SIGNAL(valueChanged(int)), this, SLOT(slotSpinChanged(int)));
}

SpeedDial::~SpeedDial()
{
}

void SpeedDial::setValue(uint ms)
{
    m_preventSignals = true;
    m_spin->setValue(ms);
    m_preventSignals = false;
}

uint SpeedDial::value() const
{
    return m_spin->value();
}

/*****************************************************************************
 * Private
 *****************************************************************************/

int SpeedDial::dialDiff(int value, int previous)
{
    int diff = value - previous;
    if (diff < (-THRESHOLD))
        diff = -1;
    else if (diff > THRESHOLD)
        diff = 1;
    return diff;
}

void SpeedDial::slotDialChanged(int value)
{
    int add = dialDiff(value, m_previousDialValue) * MasterTimer::tick();
    m_spin->setValue(m_spin->value() + add);
    m_previousDialValue = value;
}

void SpeedDial::slotSpinChanged(int value)
{
    if (m_preventSignals == false)
        emit valueChanged(uint(value));
}
