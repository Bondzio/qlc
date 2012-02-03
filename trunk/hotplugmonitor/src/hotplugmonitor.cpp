/*
  Q Light Controller
  hotplugmonitor.cpp

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

#include <QDebug>

#include "hotplugmonitor.h"

#ifdef WIN32
#   include "hpmprivate-win32.h"
#elif __APPLE__
#   include "hpmprivate-iokit.h"
#else
#   include "hpmprivate-udev.h"
#endif

HotPlugMonitor::HotPlugMonitor(QObject* parent)
    : QObject(parent)
    , d_ptr(new HPMPrivate(this))
{
}

HotPlugMonitor::~HotPlugMonitor()
{
    stop();
    delete d_ptr;
    d_ptr = NULL;
}

void HotPlugMonitor::start()
{
    d_ptr->start();
}

void HotPlugMonitor::stop()
{
    d_ptr->stop();
}

void HotPlugMonitor::emitDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << vid << pid;
    emit deviceAdded(vid, pid);
}

void HotPlugMonitor::emitDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << vid << pid;
    emit deviceRemoved(vid, pid);
}
