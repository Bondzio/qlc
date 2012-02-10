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

#include <QFocusEvent>
#include <QToolButton>
#include <QCheckBox>
#include <QLayout>
#include <QTimer>
#include <QDebug>
#include <QDial>

#include "speedspinbox.h"
#include "mastertimer.h"
#include "speeddial.h"
#include "qlcmacros.h"
#include "function.h"

#define MS_PER_SECOND (1000)
#define MS_PER_MINUTE (60 * MS_PER_SECOND)
#define MS_PER_HOUR   (60 * MS_PER_MINUTE)

#define THRESHOLD 10
#define HRS_MAX   (596 - 1) // INT_MAX can hold 596h 31m 23s 647ms
#define MIN_MAX   59
#define SEC_MAX   59
#define MS_MAX    999
#define MS_DIV    10

/****************************************************************************
 * FocusSpinBox
 ****************************************************************************/

FocusSpinBox::FocusSpinBox(QWidget* parent)
    : QSpinBox(parent)
{
}

void FocusSpinBox::focusInEvent(QFocusEvent* event)
{
    if (event->gotFocus() == true)
        emit focusGained();
}

/****************************************************************************
 * SpeedDial
 ****************************************************************************/

SpeedDial::SpeedDial(QWidget* parent)
    : QGroupBox(parent)
    , m_timer(new QTimer(this))
    , m_dial(NULL)
    , m_hrs(NULL)
    , m_min(NULL)
    , m_sec(NULL)
    , m_ms(NULL)
    , m_focus(NULL)
    , m_previousDialValue(0)
    , m_preventSignals(false)
    , m_value(0)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    vbox->addLayout(hbox);

    m_minus = new QToolButton(this);
    m_minus->setIconSize(QSize(32, 32));
    m_minus->setIcon(QIcon(":/edit_remove.png"));
    hbox->addWidget(m_minus);
    hbox->setAlignment(m_minus, Qt::AlignVCenter | Qt::AlignLeft);
    connect(m_minus, SIGNAL(pressed()), this, SLOT(slotPlusMinus()));
    connect(m_minus, SIGNAL(released()), this, SLOT(slotPlusMinus()));

    m_dial = new QDial(this);
    m_dial->setWrapping(true);
    m_dial->setNotchesVisible(true);
    m_dial->setTracking(true);
    hbox->addWidget(m_dial);
    vbox->setAlignment(m_dial, Qt::AlignHCenter);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialChanged(int)));

    m_plus = new QToolButton(this);
    m_plus->setIconSize(QSize(32, 32));
    m_plus->setIcon(QIcon(":/edit_add.png"));
    hbox->addWidget(m_plus);
    hbox->setAlignment(m_plus, Qt::AlignVCenter | Qt::AlignRight);
    connect(m_plus, SIGNAL(pressed()), this, SLOT(slotPlusMinus()));
    connect(m_plus, SIGNAL(released()), this, SLOT(slotPlusMinus()));

    hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    vbox->addLayout(hbox);

    m_hrs = new FocusSpinBox(this);
    m_hrs->setRange(0, HRS_MAX);
    m_hrs->setSuffix("h");
    m_hrs->setButtonSymbols(QSpinBox::NoButtons);
    m_hrs->setFixedSize(QSize(40, 30));
    hbox->addWidget(m_hrs);
    connect(m_hrs, SIGNAL(valueChanged(int)), this, SLOT(slotHoursChanged()));
    connect(m_hrs, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_min = new FocusSpinBox(this);
    m_min->setRange(0, MIN_MAX);
    m_min->setSuffix("m");
    m_min->setButtonSymbols(QSpinBox::NoButtons);
    m_min->setFixedSize(QSize(40, 30));
    hbox->addWidget(m_min);
    connect(m_min, SIGNAL(valueChanged(int)), this, SLOT(slotMinutesChanged()));
    connect(m_min, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_sec = new FocusSpinBox(this);
    m_sec->setRange(0, SEC_MAX);
    m_sec->setSuffix("s");
    m_sec->setButtonSymbols(QSpinBox::NoButtons);
    m_sec->setFixedSize(QSize(40, 30));
    hbox->addWidget(m_sec);
    connect(m_sec, SIGNAL(valueChanged(int)), this, SLOT(slotSecondsChanged()));
    connect(m_sec, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_ms = new FocusSpinBox(this);
    m_ms->setRange(0, MS_MAX / MS_DIV);
    m_ms->setSingleStep(MasterTimer::tick() / MS_DIV);
    m_ms->setPrefix(".");
    m_ms->setButtonSymbols(QSpinBox::NoButtons);
    m_ms->setFixedSize(QSize(40, 30));
    hbox->addWidget(m_ms);
    connect(m_ms, SIGNAL(valueChanged(int)), this, SLOT(slotMSChanged()));
    connect(m_ms, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_infiniteCheck = new QCheckBox(this);
    m_infiniteCheck->setText(tr("Infinite"));
    vbox->addWidget(m_infiniteCheck);
    connect(m_infiniteCheck, SIGNAL(toggled(bool)), this, SLOT(slotInfiniteChecked(bool)));

    m_focus = m_ms;
    m_dial->setRange(m_focus->minimum(), m_focus->maximum());
    m_dial->setSingleStep(m_focus->singleStep());

    m_timer->setInterval(25);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotPlusMinusTimeout()));
}

SpeedDial::~SpeedDial()
{
}

void SpeedDial::setValue(int ms)
{
    m_preventSignals = true;
    m_value = ms;
    setSpinValues(ms);

    if (ms == Function::infiniteSpeed())
        m_infiniteCheck->setChecked(true);
    else
        m_infiniteCheck->setChecked(false);

    m_preventSignals = false;
}

int SpeedDial::value() const
{
    return m_value;
}

/*****************************************************************************
 * Private
 *****************************************************************************/

void SpeedDial::setSpinValues(int ms)
{
    if (ms == Function::infiniteSpeed())
    {
        m_hrs->setValue(m_hrs->minimum());
        m_min->setValue(m_min->minimum());
        m_sec->setValue(m_sec->minimum());
        m_ms->setValue(m_ms->minimum());
    }
    else
    {
        ms = CLAMP(ms, 0, INT_MAX);

        m_hrs->setValue(ms / MS_PER_HOUR);
        ms -= (m_hrs->value() * MS_PER_HOUR);

        m_min->setValue(ms / MS_PER_MINUTE);
        ms -= (m_min->value() * MS_PER_MINUTE);

        m_sec->setValue(ms / MS_PER_SECOND);
        ms -= (m_sec->value() * MS_PER_SECOND);

        m_ms->setValue(ms / MS_DIV);
    }
}

int SpeedDial::spinValues() const
{
    int value = 0;

    if (m_infiniteCheck->isChecked() == false)
    {
        value += m_hrs->value() * MS_PER_HOUR;
        value += m_min->value() * MS_PER_MINUTE;
        value += m_sec->value() * MS_PER_SECOND;
        value += m_ms->value() * MS_DIV;
    }
    else
    {
        value = Function::infiniteSpeed();
    }

    return CLAMP(value, 0, INT_MAX);
}

int SpeedDial::dialDiff(int value, int previous, int step)
{
    int diff = value - previous;
    if (diff > THRESHOLD)
        diff = -step;
    else if (diff < (-THRESHOLD))
        diff = step;
    return diff;
}

void SpeedDial::slotPlusMinus()
{
    if (m_minus->isDown() == true || m_plus->isDown() == true)
    {
        slotPlusMinusTimeout();
        m_timer->start();
    }
    else
    {
        m_timer->stop();
    }
}

void SpeedDial::slotPlusMinusTimeout()
{
    if (m_minus->isDown() == true)
    {
        if (m_dial->value() == m_dial->minimum())
            m_dial->setValue(m_dial->maximum()); // Wrap around
        else
            m_dial->setValue(m_dial->value() - m_dial->singleStep()); // Normal increment
    }
    else if (m_plus->isDown() == true)
    {
        if (m_dial->value() == m_dial->maximum())
            m_dial->setValue(m_dial->minimum()); // Wrap around
        else
            m_dial->setValue(m_dial->value() + m_dial->singleStep()); // Normal increment
    }
}

void SpeedDial::slotDialChanged(int value)
{
    Q_ASSERT(m_focus != NULL);

    int newValue = dialDiff(value, m_previousDialValue, m_dial->singleStep()) + m_focus->value();
    if (newValue > m_focus->maximum())
    {
        // Incremented value is above m_focus->maximum(). Spill the overflow to the
        // bigger number (unless already incrementing hours).
        if (m_focus == m_ms)
            m_value += (m_ms->singleStep() * MS_DIV);
        else if (m_focus == m_sec)
            m_value += MS_PER_SECOND;
        else if (m_focus == m_min)
            m_value += MS_PER_MINUTE;

        m_value = CLAMP(m_value, 0, INT_MAX);
        setSpinValues(m_value);
    }
    else if (newValue < m_focus->minimum())
    {
        newValue = m_value;
        // Decremented value is below m_focus->minimum(). Spill the underflow to the
        // smaller number (unless already decrementing milliseconds).
        if (m_focus == m_ms)
            newValue -= (m_ms->singleStep() * MS_DIV);
        else if (m_focus == m_sec)
            newValue -= MS_PER_SECOND;
        else if (m_focus == m_min)
            newValue -= MS_PER_MINUTE;

        if (newValue >= 0)
        {
            m_value = newValue;
            m_value = CLAMP(m_value, 0, INT_MAX);
            setSpinValues(m_value);
        }
    }
    else
    {
        // Normal value increment/decrement.
        m_value = newValue;
        m_value = CLAMP(m_value, 0, INT_MAX);
        m_focus->setValue(m_value);
    }

    // Store the current value so it can be compared on the next pass to determine the
    // dial's direction of rotation.
    m_previousDialValue = value;
}

void SpeedDial::slotHoursChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotMinutesChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotSecondsChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotMSChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotInfiniteChecked(bool state)
{
    m_minus->setEnabled(!state);
    m_dial->setEnabled(!state);
    m_plus->setEnabled(!state);
    m_hrs->setEnabled(!state);
    m_min->setEnabled(!state);
    m_sec->setEnabled(!state);
    m_ms->setEnabled(!state);

    if (state == true)
    {
        m_value = Function::infiniteSpeed();
        if (m_preventSignals == false)
            emit valueChanged(Function::infiniteSpeed());
    }
    else
    {
        m_value = 0;
        if (m_preventSignals == false)
            emit valueChanged(0);
    }
}

void SpeedDial::slotSpinFocusGained()
{
    m_focus = qobject_cast <FocusSpinBox*> (QObject::sender());
    Q_ASSERT(m_focus != NULL);
    m_dial->setRange(m_focus->minimum(), m_focus->maximum());
    m_dial->setSingleStep(m_focus->singleStep());
}
