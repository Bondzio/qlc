/*
  Q Light Controller
  speedspinbox.h

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

#ifndef SPEEDSPINBOX_H
#define SPEEDSPINBOX_H

#include <QDoubleSpinBox>
#include <QValidator>
#include "function.h"

#define SPEEDSPINBOX_SINGLESTEP int(MasterTimer::tick())
#define SPEEDSPINBOX_INFINITE_VALUE Function::infiniteSpeed()

class SpeedSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    enum LowLimit { Infinite, Zero };

    SpeedSpinBox(LowLimit l, QWidget* parent = 0);
    ~SpeedSpinBox();

    QValidator::State validate(QString& input, int& pos) const;

    /** Convert $ms milliseconds to QString(seconds.milliseconds) */
    static QString speedText(uint ms);

protected:
    QString textFromValue(int value) const;
    int valueFromText(const QString& text) const;

private slots:
    void slotValueChanged(int value);

private:
    LowLimit m_lowLimit;
};

#endif
