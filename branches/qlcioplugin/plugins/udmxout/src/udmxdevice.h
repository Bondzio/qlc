/*
  Q Light Controller
  udmxdevice.h

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

#ifndef UDMXDEVICE_H
#define UDMXDEVICE_H

#include <QThread>
#include "qlcoutputline.h"

struct usb_dev_handle;
struct usb_device;
class QString;

class UDMXDevice : public QLCOutputLine
{
    Q_OBJECT

public:
    static quint16 vid;
    static quint16 pid;

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    UDMXDevice(QLCIOPlugin* plugin, struct usb_device* device,
               qulonglong uid, const QString& name);
    virtual ~UDMXDevice();

    /********************************************************************
     * Device information
     ********************************************************************/
public:
    /** Find out, whether the given USB device is a uDMX device */
    static bool isUDMXDevice(const struct usb_device* device);

    /** Extract name from the given USB device */
    static QString extractName(struct usb_device* device);

    /** Extract uid from the given USB device */
    static qulonglong extractUid(struct usb_device* device);

    /** @reimp */
    QString description();

    /********************************************************************
     * Device access
     ********************************************************************/
public:
    /** @reimp */
    void open();

    /** @reimp */
    void close();

    /** @reimp */
    bool isOpen();

    /** @reimp */
    void writeUniverse(const QByteArray& universe);

    /** Called by UDMXThread to write m_universe contents to HW */
    void writeHW();

    const struct usb_device* device() const;
    const usb_dev_handle* handle() const;

private:
    struct usb_device* m_device;
    usb_dev_handle* m_handle;
    QByteArray m_universe;
};

#endif
