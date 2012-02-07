/*
  Q Light Controller
  speeddial.h

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

#ifndef SPEEDDIAL_H
#define SPEEDDIAL_H

#include <QGroupBox>

class SpeedSpinBox;
class QDial;

class SpeedDial : public QGroupBox
{
    Q_OBJECT
    Q_DISABLE_COPY(SpeedDial)

public:
    SpeedDial(QWidget* parent);
    ~SpeedDial();

    void setValue(uint ms);
    uint value() const;

signals:
    void valueChanged(uint ms);

    /*************************************************************************
     * Private
     *************************************************************************/
private:
    /** Calculate the value to add/subtract when a dial has been moved */
    int dialDiff(int value, int previous);

private slots:
    /** Catch dial value changes */
    void slotDialChanged(int value);

    /** Catch spin value changes */
    void slotSpinChanged(int value);

private:
    QDial* m_dial;
    SpeedSpinBox* m_spin;
    int m_previousDialValue;
    bool m_preventSignals;
};

#endif
