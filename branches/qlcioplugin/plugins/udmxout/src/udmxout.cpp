/*
  Q Light Controller
  udmxout.cpp

  Copyright (c)	Lutz Hillebrand
                Heikki Junnila

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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "libusb_dyn.h"
#else
#include <usb.h>
#endif

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include "udmxdevice.h"
#include "udmxthread.h"
#include "udmxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

UDMXOut::~UDMXOut()
{
    qDebug() << Q_FUNC_INFO;

    delete m_thread;
    m_thread = NULL;
}

void UDMXOut::init()
{
    qDebug() << Q_FUNC_INFO;

    m_thread = NULL;
    usb_init();
}

qulonglong UDMXOut::uid()
{
    qDebug() << Q_FUNC_INFO;
    return 0;
}

QString UDMXOut::name()
{
    qDebug() << Q_FUNC_INFO;
    return QString("uDMX");
}

QString UDMXOut::description()
{
    qDebug() << Q_FUNC_INFO;

    QString info;
    QString gran;

    info += QString("<HTML>");
    info += QString("<HEAD>");
    info += QString("<TITLE>%1</TITLE>").arg(name());
    info += QString("</HEAD>");
    info += QString("<BODY>");

    info += QString("<H3>%1</H3>").arg(name());
    info += QString("<P>");
    info += tr("This plugin provides DMX output support for Anyma uDMX devices.");
    info += QString("<BR><A HREF=\"http://www.anyma.ch\">anyma.ch</A>");
    info += QString("</P>");

    if (m_thread != NULL)
    {
        info += QString("<P>");
        info += QString("<B>%1:</B> %2Hz").arg(tr("DMX Frame Frequency")).arg(m_thread->frequency());
        info += QString("<BR>");
        if (m_thread->granularity() == UDMXThread::Bad)
            gran = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Bad"));
        else if (m_thread->granularity() == UDMXThread::Good)
            gran = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Good"));
        else
            gran = tr("Unknown");
        info += QString("<B>%1:</B> %2").arg(tr("System Timer Accuracy")).arg(gran);
        info += QString("</P>");
    }

    info += QString("</BODY>");
    info += QString("</HTML>");

    return info;
}

/*****************************************************************************
 * Hot Plugging
 *****************************************************************************/

void UDMXOut::slotDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << vid << pid;

    if (vid == UDMXDevice::vid && pid == UDMXDevice::pid)
        rescanDevices();
}

void UDMXOut::slotDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << vid << pid;
    qDebug() << Q_FUNC_INFO;

    if (vid == UDMXDevice::vid && pid == UDMXDevice::pid)
        rescanDevices();
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

QSet <QLCOutputLine*> UDMXOut::outputLines()
{
    return m_devices;
}

void UDMXOut::rescanDevices()
{
    qDebug() << Q_FUNC_INFO;

    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QList <QLCOutputLine*> destroyList(m_devices.toList());

    usb_find_busses();
    usb_find_devices();

    /* Iterate thru all buses */
    for (struct usb_bus* bus = usb_get_busses(); bus != NULL; bus = bus->next)
    {
        /* Iterate thru all devices in each bus */
        for (struct usb_device* dev = bus->devices; dev != NULL; dev = dev->next)
        {
            UDMXDevice* udmx = device(dev);
            if (udmx != NULL)
            {
                /* We already have this device in m_devices and the HW is also present.
                   Remove from the destroy list and continue iterating. */
                destroyList.removeAll(qobject_cast<QLCOutputLine*> (udmx));
                continue;
            }
            else if (UDMXDevice::isUDMXDevice(dev) == true)
            {
                /* This is a new device. Create and append. */
                qulonglong uid = UDMXDevice::extractUid(dev);
                QString name = UDMXDevice::extractName(dev);
                udmx = new UDMXDevice(this, dev, uid, name);
                m_devices << udmx;

                // Add the device to the writer thread (and create the thread if not already there)
                if (m_thread == NULL)
                    m_thread = new UDMXThread(this);
                m_thread->addDevice(udmx);

                emit outputLineAdded(qobject_cast<QLCOutputLine*> (udmx));
            }
        }
    }

    /* Destroy those devices that were no longer found. */
    while (destroyList.isEmpty() == false)
    {
        QLCOutputLine* line = destroyList.takeFirst();
        m_thread->removeDevice(qobject_cast<UDMXDevice*> (line));
        m_devices.remove(line);
        emit outputLineRemoved(qobject_cast<QLCOutputLine*> (line));
        line->deleteLater();
    }

    // Destroy the writer thread if there are no devices present
    if (m_thread->devices().size() == 0)
    {
        delete m_thread;
        m_thread = NULL;
    }
}

UDMXDevice* UDMXOut::device(struct usb_device* usbdev)
{
    qDebug() << Q_FUNC_INFO << (void*) usbdev;

    QSetIterator <QLCOutputLine*> it(m_devices);
    while (it.hasNext() == true)
    {
        UDMXDevice* udmx = qobject_cast<UDMXDevice*> (it.next());
        if (udmx->device() == usbdev)
            return udmx;
    }

    return NULL;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void UDMXOut::configure()
{
    qDebug() << Q_FUNC_INFO;

    int r = QMessageBox::question(NULL, name(),
                                  tr("Do you wish to re-scan your hardware?"),
                                  QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
        rescanDevices();
}

bool UDMXOut::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(udmxout, UDMXOut)
