/*
  Q Light Controller
  hotplugmonitor-udev.cpp

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

#include <sys/types.h>
#include <sys/time.h>
#include <libudev.h>
#include <unistd.h>
#include <errno.h>
#include <QDebug>

#include "hotplugmonitor.h"

#define DEVICE_ACTION_ADD    "add"
#define DEVICE_ACTION_REMOVE "remove"
#define UDEV_NETLINK_SOURCE  "udev"
#define USB_SUBSYSTEM        "usb"
#define USB_DEVICE_TYPE      "usb_device"

HotPlugMonitor::HotPlugMonitor(QObject* parent)
    : QThread(parent)
    , m_run(false)
{
}

HotPlugMonitor::~HotPlugMonitor()
{
    if (isRunning() == true)
        stop();
}

void HotPlugMonitor::stop()
{
    m_run = false;
    while (isRunning() == true)
        usleep(10);
}

void HotPlugMonitor::run()
{
    udev* udev_ctx = udev_new();
    Q_ASSERT(udev_ctx != NULL);

    udev_monitor* mon = udev_monitor_new_from_netlink(udev_ctx, UDEV_NETLINK_SOURCE);
    Q_ASSERT(mon != NULL);

    if (udev_monitor_filter_add_match_subsystem_devtype(mon, USB_SUBSYSTEM, USB_DEVICE_TYPE) < 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to add match for USB devices";
        udev_monitor_unref(mon);
        udev_unref(udev_ctx);
        return;
    }

    if (udev_monitor_enable_receiving(mon) < 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to enable udev uevent reception";
        udev_monitor_unref(mon);
        udev_unref(udev_ctx);
        return;
    }

    int fd = udev_monitor_get_fd(mon);
    fd_set readfs;
    FD_ZERO(&readfs);

    m_run = true;
    while (m_run == true)
    {
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_SET(fd, &readfs);
        int retval = select(fd + 1, &readfs, NULL, NULL, &tv);
        if (retval == -1)
        {
            qWarning() << Q_FUNC_INFO << strerror(errno);
            m_run = false;
        }
        else if (retval > 0 && FD_ISSET(fd, &readfs))
        {
            udev_device* dev = udev_monitor_receive_device(mon);
            if (dev != NULL)
            {
                QString action(udev_device_get_action(dev));
                qDebug() << "Device" << action << "action";
                if (action == DEVICE_ACTION_ADD)
                {
                    emit deviceAdded();
                }
                else if (action == DEVICE_ACTION_REMOVE)
                {
                    emit deviceRemoved();
                }

                udev_device_unref(dev);
            }
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "timeout";
        }
    }

    udev_monitor_unref(mon);
    udev_unref(udev_ctx);
}
