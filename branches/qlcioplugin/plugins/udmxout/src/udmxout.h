/*
  Q Light Controller
  udmxout.h

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

#ifndef UDMXOUT_H
#define UDMXOUT_H

#include <QStringList>
#include "qlcioplugin.h"

class UDMXDevice;
class UDMXThread;
class QString;

/*****************************************************************************
 * UDMXOut
 *****************************************************************************/

class UDMXOut : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~UDMXOut();

    /** @reimp */
    void init();

    /** @reimp */
    qulonglong uid();

    /** @reimp */
    QString name();

    /** @reimp */
    QString description();

    /*********************************************************************
     * Hot Plugging
     *********************************************************************/
public slots:
    /** @reimp */
    void slotDeviceAdded(uint vid, uint pid);

    /** @reimp */
    void slotDeviceRemoved(uint vid, uint pid);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    QSet <QLCOutputLine*> outputLines();

private:
    /** Attempt to find all uDMX devices */
    void rescanDevices();

    /** Get a UDMXDevice entry by its usbdev struct */
    UDMXDevice* device(struct usb_device* usbdev);

private:
    /** List of available devices */
    QSet <QLCOutputLine*> m_devices;

    /** Writer thread shared among UDMXDevice instances */
    UDMXThread* m_thread;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
