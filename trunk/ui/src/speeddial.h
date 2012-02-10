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
#include <QSpinBox>

class QToolButton;
class QFocusEvent;
class QCheckBox;
class QTimer;
class QDial;

class FocusSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    FocusSpinBox(QWidget* parent = 0);

signals:
    void focusGained();

protected:
    void focusInEvent(QFocusEvent* event);
};

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
    void setSpinValues(int ms);
    int spinValues() const;

    /** Calculate the value to add/subtract when a dial has been moved */
    int dialDiff(int value, int previous, int step);

private slots:
    void slotPlusMinus();
    void slotPlusMinusTimeout();
    void slotDialChanged(int value);
    void slotHoursChanged();
    void slotMinutesChanged();
    void slotSecondsChanged();
    void slotMSChanged();
    void slotInfiniteChecked(bool state);
    void slotSpinFocusGained();

private:
    QTimer* m_timer;
    QDial* m_dial;
    QToolButton* m_plus;
    QToolButton* m_minus;
    FocusSpinBox* m_hrs;
    FocusSpinBox* m_min;
    FocusSpinBox* m_sec;
    FocusSpinBox* m_ms;
    QCheckBox* m_infiniteCheck;
    FocusSpinBox* m_focus;

    int m_previousDialValue;
    bool m_preventSignals;
    int m_value;
};

#endif
