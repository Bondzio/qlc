/*
  Q Light Controller
  enttecdmxusb.cpp

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

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "enttecdmxusbconfig.h"
#include "enttecdmxusbwidget.h"
#include "enttecdmxusbprorx.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbout.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOut::~EnttecDMXUSBOut()
{
    while (m_outputs.isEmpty() == false)
        delete m_outputs.takeFirst();

    while (m_inputs.isEmpty() == false)
        delete m_inputs.takeFirst();
}

void EnttecDMXUSBOut::init()
{
    /* Search for new widgets */
    rescanWidgets();
}

QString EnttecDMXUSBOut::name()
{
    return QString("Enttec DMX USB Output");
}

bool EnttecDMXUSBOut::rescanWidgets()
{
    while (m_outputs.isEmpty() == false)
        delete m_outputs.takeFirst();
    while (m_inputs.isEmpty() == false)
        delete m_inputs.takeFirst();

    QList <EnttecDMXUSBWidget*> widgets(QLCFTDI::widgets());

    foreach (EnttecDMXUSBWidget* widget, widgets)
    {
        if (widget->type() == EnttecDMXUSBWidget::ProRX)
        {
            m_inputs << widget;
            EnttecDMXUSBProRX* prorx = (EnttecDMXUSBProRX*) widget;
            connect(prorx, SIGNAL(valueChanged(quint32,quint32,uchar)),
                    this, SIGNAL(valueChanged(quint32,quint32,uchar)));
        }
        else
        {
            m_outputs << widget;
        }
    }

    emit configurationChanged();
    return true;
}

QList <EnttecDMXUSBWidget*> EnttecDMXUSBOut::widgets() const
{
    QList <EnttecDMXUSBWidget*> widgets;
    widgets << m_outputs;
    widgets << m_inputs;
    return widgets;
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

void EnttecDMXUSBOut::openOutput(quint32 output)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->open();
}

void EnttecDMXUSBOut::closeOutput(quint32 output)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->close();
}

QStringList EnttecDMXUSBOut::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <EnttecDMXUSBWidget*> it(m_outputs);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
    return list;
}

QString EnttecDMXUSBOut::outputInfo(quint32 output)
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    if (output == QLCIOPlugin::invalidLine())
    {
        str += QString("<H3>%1</H3>").arg(name());
        if (m_outputs.size() == 0)
        {
            str += QString("<B>%1</B>").arg(tr("No devices available."));
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
        }

        str += QString("<P>");
        str += tr("This plugin provides DMX output support for");
        str += QString(" DMXKing USB DMX512-A, Enttec DMX USB Pro, "
                       "Enttec Open DMX USB, FTDI USB COM485 Plus1 ");
        str += tr("and compatible devices.");
        str += QString("</P>");
    }
    else if (output < quint32(m_outputs.size()))
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("</P>");
        QString add = m_outputs[output]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void EnttecDMXUSBOut::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->writeUniverse(universe);
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

void EnttecDMXUSBOut::openInput(quint32 input)
{
    if (input < quint32(m_inputs.size()))
        m_inputs.at(input)->open();
}

void EnttecDMXUSBOut::closeInput(quint32 input)
{
    if (input < quint32(m_inputs.size()))
        m_inputs.at(input)->close();
}

QStringList EnttecDMXUSBOut::inputs()
{
    QStringList list;
    int i = 1;

    QListIterator <EnttecDMXUSBWidget*> it(m_inputs);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
    return list;
}

QString EnttecDMXUSBOut::inputInfo(quint32 input)
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    if (input == QLCIOPlugin::invalidLine())
    {
        str += QString("<H3>%1</H3>").arg(name());
        if (m_inputs.size() == 0)
        {
            str += QString("<B>%1</B>").arg(tr("No devices available."));
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
        }

        str += QString("<P>");
        str += tr("This plugin provides DMX output support for");
        str += QString(" DMXKing USB DMX512-A, Enttec DMX USB Pro, "
                       "Enttec Open DMX USB, FTDI USB COM485 Plus1 ");
        str += tr("and compatible devices.");
        str += QString("</P>");
    }
    else if (input < quint32(m_inputs.size()))
    {
        str += QString("<H3>%1</H3>").arg(inputs()[input]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("</P>");
        QString add = m_inputs[input]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void EnttecDMXUSBOut::configure()
{
    qDebug() << Q_FUNC_INFO;
    EnttecDMXUSBConfig config(this);
    config.exec();
    rescanWidgets();
}

bool EnttecDMXUSBOut::canConfigure()
{
    return true;
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(enttecdmxusbout, EnttecDMXUSBOut)
