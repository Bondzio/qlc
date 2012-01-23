/*
  Q Light Controller
  speeddialwidget.h

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

#ifndef SPEEDDIALWIDGET_H
#define SPEEDDIALWIDGET_H

#include <QWidget>
#include "ui_speeddialwidget.h"

class SpeedSpinBox;

class SpeedDialWidget : public QWidget, public Ui_SpeedDialWidget
{
    Q_OBJECT

public:
    SpeedDialWidget(QWidget* parent, Qt::WindowFlags flags = 0);
    ~SpeedDialWidget();

    /************************************************************************
     * Speed settings
     ************************************************************************/
public:
    void setFadeInSpeed(uint ms);
    uint fadeIn() const;

    void setFadeOutSpeed(uint ms);
    uint fadeOut() const;

    void setDuration(uint ms);
    uint duration() const;

signals:
    void fadeInChanged(uint ms);
    void fadeOutChanged(uint ms);
    void durationChanged(uint ms);

    /************************************************************************
     * Optional text
     ************************************************************************/
public:
    void setOptionalTextTitle(const QString& title);
    QString optionalTextTitle() const;

    void setOptionalText(const QString& text);
    QString optionalText() const;

signals:
    void optionalTextEdited(const QString& text);

private:
    QString m_optionalTextTitle;
    QString m_optionalText;

    /************************************************************************
     * Private
     ************************************************************************/
private:
    /** Calculate the value to add/subtract when a dial has been moved */
    int dialDiff(int value, int previous);

private slots:
    void slotFadeInDialChanged(int value);
    void slotFadeOutDialChanged(int value);
    void slotDurationDialChanged(int value);

    void slotFadeInSpinChanged(int value);
    void slotFadeOutSpinChanged(int value);
    void slotDurationSpinChanged(int value);

private:
    SpeedSpinBox* m_fadeInSpin;
    SpeedSpinBox* m_fadeOutSpin;
    SpeedSpinBox* m_durationSpin;
    int m_fadeInPrev;
    int m_fadeOutPrev;
    int m_durationPrev;
};

#endif
