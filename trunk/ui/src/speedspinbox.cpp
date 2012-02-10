/*
  Q Light Controller
  speedspinbox.cpp

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

#include <QApplication>
#include <QPalette>

#include "speedspinbox.h"
#include "mastertimer.h"

#define INFINITY_STRING QChar(0x221E)

SpeedSpinBox::SpeedSpinBox(SpeedSpinBox::LowLimit l, QWidget* parent)
    : QSpinBox(parent)
    , m_lowLimit(l)
{
    setSingleStep(SPEEDSPINBOX_SINGLESTEP);

    switch (m_lowLimit)
    {
        case SpeedSpinBox::Infinite:
            setRange(SPEEDSPINBOX_INFINITE_VALUE, INT_MAX);
            setValue(0);
            slotValueChanged(0);
            break;

        default:
            setRange(0, INT_MAX);
            setValue(0);
            slotValueChanged(0);
            break;
    }

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

SpeedSpinBox::~SpeedSpinBox()
{
}

QString SpeedSpinBox::textFromValue(int value) const
{
    if (value == SPEEDSPINBOX_INFINITE_VALUE)
        return INFINITY_STRING;
    else
        return speedText(value);
}

void SpeedSpinBox::slotValueChanged(int value)
{
    if (value == SPEEDSPINBOX_INFINITE_VALUE)
        setToolTip(tr("Infinite"));
    else
        setToolTip(tr("Seconds"));
}

int SpeedSpinBox::valueFromText(const QString& text) const
{
    if (text == INFINITY_STRING)
        return SPEEDSPINBOX_INFINITE_VALUE;
    else
        return text.toDouble() * 1000;
}

QValidator::State SpeedSpinBox::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);
    bool ok = false;
    input.toDouble(&ok);
    if (ok == true || input.isEmpty() == false)
        return QValidator::Acceptable;
    else
        return QValidator::Invalid;
}

QString SpeedSpinBox::speedText(uint ms)
{
    if (ms == SPEEDSPINBOX_INFINITE_VALUE)
        return INFINITY_STRING;
    else
        return QLocale::system().toString(qreal(ms) / qreal(1000), 'f', 2);
}

