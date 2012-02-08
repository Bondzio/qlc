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

class SpeedDial;
class QGroupBox;
class QLineEdit;

class SpeedDialWidget : public QWidget
{
    Q_OBJECT

public:
    SpeedDialWidget(QWidget* parent,
                    Qt::WindowFlags flags = (Qt::WindowFlags)
                        (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Window &
                            (~Qt::WindowCloseButtonHint)));
    ~SpeedDialWidget();

    /************************************************************************
     * Speed settings
     ************************************************************************/
public:
    void setFadeInEnabled(bool set);
    void setFadeInSpeed(uint ms);
    uint fadeIn() const;

    void setFadeOutEnabled(bool set);
    void setFadeOutSpeed(uint ms);
    uint fadeOut() const;

    void setDurationEnabled(bool set);
    void setDuration(uint ms);
    uint duration() const;

signals:
    void fadeInChanged(uint ms);
    void fadeOutChanged(uint ms);
    void durationChanged(uint ms);

private:
    SpeedDial* m_fadeIn;
    SpeedDial* m_fadeOut;
    SpeedDial* m_duration;

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
    QGroupBox* m_optionalTextGroup;
    QLineEdit* m_optionalTextEdit;
};

#endif
