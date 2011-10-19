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

#define SINGLESTEP int(MasterTimer::tick())
#define DEFAULT (0 - SINGLESTEP)
#define INFINITE (DEFAULT - SINGLESTEP)

SpeedSpinBox::SpeedSpinBox(SpeedSpinBox::LowLimit l, QWidget* parent)
    : QSpinBox(parent)
    , m_lowLimit(l)
{
    setSingleStep(SINGLESTEP);

    switch (m_lowLimit)
    {
        case SpeedSpinBox::Infinite:
            setRange(INFINITE, INT_MAX);
            setValue(DEFAULT);
            slotValueChanged(DEFAULT);
            break;
        case SpeedSpinBox::Default:
            setRange(DEFAULT, INT_MAX);
            setValue(DEFAULT);
            slotValueChanged(DEFAULT);
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
    if (value == INFINITE)
        return QString("Infinite");
    else if (value == DEFAULT)
        return QString("Default");
    else
        return locale().toString(qreal(value) / qreal(1000), 'f', 2);
}

void SpeedSpinBox::slotValueChanged(int value)
{
    if (value == INFINITE)
    {
        QPalette pal(palette());
        pal.setColor(QPalette::Base, QColor(255, 127, 127));
        setPalette(pal);
        setToolTip(tr("Infinite"));
    }
    else if (value == DEFAULT)
    {
        QPalette pal(palette());
        pal.setColor(QPalette::Base, QColor(127, 255, 127));
        setPalette(pal);
        setToolTip(tr("Use default value"));
    }
    else
    {
        setPalette(QApplication::palette());
        setToolTip(tr("Seconds.MilliSeconds"));
    }
}

int SpeedSpinBox::valueFromText(const QString& text) const
{
    double value = text.toDouble();
    return value * 1000;
}

QValidator::State SpeedSpinBox::validate(QString& input, int& pos) const
{
    bool ok = false;
    input.toDouble(&ok);
    if (ok == true || input.isEmpty() == false)
        return QValidator::Acceptable;
    else
        return QValidator::Invalid;
}
