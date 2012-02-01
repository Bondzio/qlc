/*
  Q Light Controller
  hotplugmonitor.h

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

#ifndef HOTPLUGMONITOR_H
#define HOTPLUGMONITOR_H

#include <QThread>

class HPMPrivate;

/**
 * HotPlugMonitor monitors for USB subsystem hotplug events and emits either
 * deviceAdded() or deviceRemoved() signal, depending on which event has occurred.
 * This info can then be used by plugins to see if they need to update their
 * own device lists.
 *
 * @todo The signals should contain an ID that is usable by plugins so that
 *       every plugin doesn't need to do a complete rescan.
 */
class HotPlugMonitor : public QThread
{
    Q_OBJECT

    friend class HPMPrivate;

public:
    HotPlugMonitor(QObject* parent = 0);
    ~HotPlugMonitor();

    void stop();

signals:
    void deviceAdded();
    void deviceRemoved();

private:
    void run();
    void emitDeviceAdded(uint vid, uint pid);
    void emitDeviceRemoved(uint vid, uint pid);

private:
    HPMPrivate* d_ptr;
    bool m_run;
};

#endif
