/*
  Q Light Controller
  udmxdevice.cpp

  Copyright (c) Lutz Hillebrand
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

#include <QSettings>
#include <QDebug>
#include <QTime>
#include <cmath>

#include "udmxdevice.h"
#include "qlcmacros.h"

#define UDMX_SET_CHANNEL_RANGE 0x0002 /* Command to set n channel values */

quint16 UDMXDevice::vid = 0x16C0; /* VOTI */
quint16 UDMXDevice::pid = 0x05DC; /* Obdev's free shared PID */

/****************************************************************************
 * Initialization
 ****************************************************************************/

UDMXDevice::UDMXDevice(QLCIOPlugin* plugin, struct usb_device* device,
                       qulonglong uid, const QString& name)
    : QLCOutputLine(plugin, uid, name)
    , m_device(device)
    , m_handle(NULL)
    , m_universe(QByteArray(512, 0))
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(device != NULL);
}

UDMXDevice::~UDMXDevice()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

/****************************************************************************
 * Device information
 ****************************************************************************/

bool UDMXDevice::isUDMXDevice(const struct usb_device* device)
{
    if (device == NULL)
        return false;

    if ((device->descriptor.idVendor == UDMXDevice::vid) &&
        (device->descriptor.idProduct == UDMXDevice::pid))
    {
        return true;
    }
    else
    {
        return false;
    }
}

QString UDMXDevice::extractName(struct usb_device* device)
{
    qDebug() << Q_FUNC_INFO << (void*) device;

    QString unknown = tr("Unknown");

    Q_ASSERT(device != NULL);

    usb_dev_handle* handle = usb_open(device);
    if (handle == NULL)
        return unknown;

    /* Extract the name */
    char buf[256] = { 0 };
    int len = usb_get_string_simple(handle, device->descriptor.iProduct,
                                    buf, sizeof(buf));
    QString name;
    if (len > 0)
        name = QString(buf);
    else
        name = unknown;

    usb_close(handle);
    handle = NULL;

    return name;
}

qulonglong UDMXDevice::extractUid(struct usb_device* device)
{
    qDebug() << Q_FUNC_INFO << (void*) device;
    Q_UNUSED(device);
    return 0;
}

QString UDMXDevice::description()
{
    qDebug() << Q_FUNC_INFO;

    QString info;
    if (m_device != NULL && m_handle != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>%1</B>").arg(tr("Unknown device"));
        info += QString("<P>");
        info += tr("Cannot connect to USB device.");
        info += QString("</P>");
    }

    return info;
}

/****************************************************************************
 * Device access
 ****************************************************************************/

void UDMXDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (m_device != NULL && m_handle == NULL)
        m_handle = usb_open(m_device);
}

void UDMXDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    if (m_device != NULL && m_handle != NULL)
        usb_close(m_handle);
    m_handle = NULL;
}

bool UDMXDevice::isOpen()
{
    if (m_handle != NULL)
        return true;
    else
        return false;
}

const struct usb_device* UDMXDevice::device() const
{
    return m_device;
}

const usb_dev_handle* UDMXDevice::handle() const
{
    return m_handle;
}

void UDMXDevice::writeUniverse(const QByteArray& universe)
{
    m_universe.replace(0, MIN(universe.size(), m_universe.size()), universe);
}

void UDMXDevice::writeHW()
{
    int r = usb_control_msg(m_handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                            UDMX_SET_CHANNEL_RANGE,   /* Command */
                            m_universe.size(),        /* Number of channels to set */
                            0,                        /* Starting index */
                            (char*)m_universe.data(), /* Values to set */
                            m_universe.size(),        /* Size of values */
                            500);                     /* Timeout 0.5s */
    if (r < 0)
        qWarning() << "uDMX: unable to write universe:" << usb_strerror();
}
