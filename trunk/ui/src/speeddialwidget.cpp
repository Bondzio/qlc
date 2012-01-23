/*
  Q Light Controller
  speeddialwidget.cpp

  Copyright (C) Heikki Junnila

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

#include <QSettings>
#include <QDebug>

#include "speeddialwidget.h"
#include "speedspinbox.h"
#include "mastertimer.h"
#include "apputil.h"

#define SETTINGS_GEOMETRY "speeddialwidget/geometry"

SpeedDialWidget::SpeedDialWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_fadeInPrev(0)
    , m_fadeOutPrev(0)
    , m_durationPrev(0)
{
    setupUi(this);

    /* Spin boxes */
    new QHBoxLayout(m_fadeInContainer);
    m_fadeInSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeInContainer);
    m_fadeInContainer->layout()->addWidget(m_fadeInSpin);
    m_fadeInContainer->layout()->setMargin(0);
    connect(m_fadeInSpin, SIGNAL(valueChanged(int)), this, SLOT(slotFadeInSpinChanged(int)));

    new QHBoxLayout(m_fadeOutContainer);
    m_fadeOutSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeOutContainer);
    m_fadeOutContainer->layout()->addWidget(m_fadeOutSpin);
    m_fadeOutContainer->layout()->setMargin(0);
    connect(m_fadeOutSpin, SIGNAL(valueChanged(int)), this, SLOT(slotFadeOutSpinChanged(int)));

    new QHBoxLayout(m_durationContainer);
    m_durationSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_durationContainer);
    m_durationContainer->layout()->addWidget(m_durationSpin);
    m_durationContainer->layout()->setMargin(0);
    connect(m_durationSpin, SIGNAL(valueChanged(int)), this, SLOT(slotDurationSpinChanged(int)));

    /* Optional text */
    m_optionalTextGroup->setVisible(false);
    connect(m_optionalTextEdit, SIGNAL(textEdited(const QString&)),
            this, SIGNAL(optionalTextEdited(const QString&)));

    /* Position */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        this->restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

SpeedDialWidget::~SpeedDialWidget()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/****************************************************************************
 * Speed settings
 ****************************************************************************/

void SpeedDialWidget::setFadeInSpeed(uint ms)
{
    m_fadeInSpin->setValue(ms);
}

uint SpeedDialWidget::fadeIn() const
{
    return m_fadeInSpin->value();
}

void SpeedDialWidget::setFadeOutSpeed(uint ms)
{
    m_fadeOutSpin->setValue(ms);
}

uint SpeedDialWidget::fadeOut() const
{
    return m_fadeOutSpin->value();
}

void SpeedDialWidget::setDuration(uint ms)
{
    m_durationSpin->setValue(ms);
}

uint SpeedDialWidget::duration() const
{
    return m_durationSpin->value();
}

/************************************************************************
 * Optional text
 ************************************************************************/

void SpeedDialWidget::setOptionalTextTitle(const QString& title)
{
    m_optionalTextGroup->setTitle(title);
    m_optionalTextGroup->setVisible(!title.isEmpty());
}

QString SpeedDialWidget::optionalTextTitle() const
{
    return m_optionalTextGroup->title();
}

void SpeedDialWidget::setOptionalText(const QString& text)
{
    m_optionalTextEdit->setText(text);
}

QString SpeedDialWidget::optionalText() const
{
    return m_optionalTextEdit->text();
}

/****************************************************************************
 * Private
 ****************************************************************************/

int SpeedDialWidget::dialDiff(int value, int previous)
{
    int diff = value - previous;
    if (diff < (-50))
        diff = -1;
    else if (diff > 50)
        diff = 1;
    return diff;
}

void SpeedDialWidget::slotFadeInDialChanged(int value)
{
    int add = dialDiff(value, m_fadeInPrev) * MasterTimer::tick();
    m_fadeInSpin->setValue(m_fadeInSpin->value() + add);
    m_fadeInPrev = value;
}

void SpeedDialWidget::slotFadeOutDialChanged(int value)
{
    int add = dialDiff(value, m_fadeOutPrev) * MasterTimer::tick();
    m_fadeOutSpin->setValue(m_fadeOutSpin->value() + add);
    m_fadeOutPrev = value;
}

void SpeedDialWidget::slotDurationDialChanged(int value)
{
    int add = dialDiff(value, m_durationPrev) * MasterTimer::tick();
    m_durationSpin->setValue(m_durationSpin->value() + add);
    m_durationPrev = value;
}

void SpeedDialWidget::slotFadeInSpinChanged(int value)
{
    emit fadeInChanged(uint(value));
}

void SpeedDialWidget::slotFadeOutSpinChanged(int value)
{
    emit fadeOutChanged(uint(value));
}

void SpeedDialWidget::slotDurationSpinChanged(int value)
{
    emit durationChanged(uint(value));
}

