/*
  Q Light Controller
  hotplugmonitor-win32.cpp

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

// Let's assume we have at least W2K (http://msdn.microsoft.com/en-us/library/Aa383745)
#define _WIN32_WINNT 0x05000000
#define _WIN32_WINDOWS 0x05000000
#define WINVER 0x05000000
#include <Windows.h>
#include <Dbt.h>

#include <QMessageBox>
#include <QWidget>

#include "hotplugmonitor.h"
#include "hpmprivate-win32.h"

#define DBCC_NAME_RAWDEVICE_USB_GUID "a5dcbf10-6530-11d2-901f-00c04fb951ed"
#define DBCC_NAME_SEPARATOR "#"
#define DBCC_NAME_VID "VID_"
#define DBCC_NAME_PID "PID_"
#define DBCC_NAME_VIDPID_SEPARATOR "&"

const GUID HPMPrivate::USBClassGUID =
    { 0x25dbce51, 0x6c8f, 0x4a72, { 0x8a, 0x6d, 0xb5, 0x4c, 0x2b, 0x4f, 0xc8, 0x35 } };

/****************************************************************************
 * HPMPrivate implementation
 ****************************************************************************/

HPMPrivate::HPMPrivate(HotPlugMonitor* parent)
    : QWidget(0) // This class has to be a widget to receive winEvent() events
    , hpm(parent)
    , hDeviceNotify(NULL)
{
}

HPMPrivate::~HPMPrivate()
{
}

void HPMPrivate::registerNotification()
{
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;

    ZeroMemory(&notificationFilter, sizeof(notificationFilter));
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notificationFilter.dbcc_classguid = USBClassGUID;

    hDeviceNotify = RegisterDeviceNotification(winId(),
                                               &notificationFilter,
                                               DEVICE_NOTIFY_WINDOW_HANDLE |
                                               DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

    if (hDeviceNotify == NULL)
        qWarning() << Q_FUNC_INFO << "Unable to register device notification.";
}

void HPMPrivate::unregisterNotification()
{
    if (hDeviceNotify == NULL)
        return;
    else if (UnregisterDeviceNotification(hDeviceNotify) == FALSE)
        qWarning() << Q_FUNC_INFO << "Unable to unregister device notification.";
    hDeviceNotify = NULL;
}

bool HPMPrivate::extractVidPid(const QString& deviceId, uint* vid, uint* pid)
{
    QStringList parts = deviceId.toUpper().split(DBCC_NAME_SEPARATOR);
    for (int i = 0; i < parts.size(); i++)
    {
        if (parts[i].startsWith(DBCC_NAME_VID) == true)
        {
            QStringList vidpid = parts[i].split(DBCC_NAME_VIDPID_SEPARATOR);
            if (vidpid.size() != 2)
                return false;

            QString v = vidpid[0].remove(DBCC_NAME_VID);
            QString p = vidpid[1].remove(DBCC_NAME_PID);

            *vid = v.toUInt(0, 16);
            *pid = p.toUInt(0, 16);

            return true;
        }
    }

    return false;
}

bool HPMPrivate::winEvent(MSG* message, long* RESULT)
{
    UINT msg = message->message;
    WPARAM wParam = message->wParam;
    LPARAM lParam = message->lParam;

    if (msg == WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_HDR hdr = (PDEV_BROADCAST_HDR) lParam;
        if (wParam == DBT_DEVICEARRIVAL)
        {
            Q_ASSERT(hdr != NULL);

            if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE dev = (PDEV_BROADCAST_DEVICEINTERFACE) hdr;
                QString name(QString::fromWCharArray(dev->dbcc_name));
                if (name.contains(DBCC_NAME_RAWDEVICE_USB_GUID) == true)
                {
                    qDebug() << name;
                    uint vid = 0, pid = 0;
                    if (extractVidPid(name, &vid, &pid) == true)
                        hpm->emitDeviceAdded(vid, pid);
                }
            }
        }
        else if (wParam == DBT_DEVICEREMOVECOMPLETE)
        {
            Q_ASSERT(hdr != NULL);

            if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE dev = (PDEV_BROADCAST_DEVICEINTERFACE) hdr;
                QString name(QString::fromWCharArray(dev->dbcc_name));
                if (name.contains(DBCC_NAME_RAWDEVICE_USB_GUID) == true)
                {
                    uint vid = 0, pid = 0;
                    if (extractVidPid(name, &vid, &pid) == true)
                        hpm->emitDeviceRemoved(vid, pid);
                }
            }
        }
    }

    return false;
}

/****************************************************************************
 * HotPlugMonitor
 ****************************************************************************/

HotPlugMonitor::HotPlugMonitor(QObject* parent)
    : QThread(parent)
    , d_ptr(new HPMPrivate(this))
    , m_run(false)
{
}

HotPlugMonitor::~HotPlugMonitor()
{
    stop();
    delete d_ptr;
    d_ptr = NULL;
}

void HotPlugMonitor::stop()
{
    d_ptr->unregisterNotification();
}

void HotPlugMonitor::start(QThread::Priority priority)
{
    // Don't call QThread::start(priority) because the thread is not needed
    d_ptr->registerNotification();
}

void HotPlugMonitor::run()
{
    /* NOP */
}
