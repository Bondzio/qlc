/*
  Q Light Controller
  enttecdmxusb.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifndef ENTTECDMXUSBOUT_H
#define ENTTECDMXUSBOUT_H

#include "qlcoutplugin.h"

class EnttecDMXUSBWidget;

class EnttecDMXUSBOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** @reimp */
    virtual ~EnttecDMXUSBOut();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** Find out what kinds of widgets there are currently connected */
    bool rescanWidgets();

    /** Get currently connected widgets (input & output) */
    QList <EnttecDMXUSBWidget*> widgets() const;

    /************************************************************************
     * Outputs
     ************************************************************************/
public:
    /** @reimp */
    void openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe);

private:
    QList <EnttecDMXUSBWidget*> m_outputs;

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    void openInput(quint32 input);

    /** @reimp */
    void closeInput(quint32 input);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); }

private:
    QList <EnttecDMXUSBWidget*> m_inputs;

    /********************************************************************
     * Configuration
     ********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
