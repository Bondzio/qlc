/*
  Q Light Controller
  hotplugmonitor-iokit.cpp

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

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <mach/mach_port.h>
#include <QDebug>

#include "hotplugmonitor.h"

/****************************************************************************
 * HPMPrivate declaration
 ****************************************************************************/

class HPMPrivate
{
public:
    HPMPrivate(HotPlugMonitor* parent);

    void extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid);
    void deviceAdded(io_iterator_t iterator);
    void deviceRemoved(io_iterator_t iterator);

public:
    HotPlugMonitor*         hpm;

    CFRunLoopRef            loop;
};

/****************************************************************************
 * Static callback functions for IOKit
 ****************************************************************************/

static void onRawDeviceAdded(void* refCon, io_iterator_t iterator)
{
    HPMPrivate* d_ptr = (HPMPrivate*) refCon;
    Q_ASSERT(d_ptr != NULL);
    d_ptr->deviceAdded(iterator);
}

static void onRawDeviceRemoved(void* refCon, io_iterator_t iterator)
{
    HPMPrivate* d_ptr = (HPMPrivate*) refCon;
    Q_ASSERT(d_ptr != NULL);
    d_ptr->deviceRemoved(iterator);
}

/****************************************************************************
 * HPMPrivate implementation
 ****************************************************************************/

HPMPrivate::HPMPrivate(HotPlugMonitor* parent)
    : hpm(parent)
    , loop(NULL)
{
}

void HPMPrivate::extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid)
{
    Q_ASSERT(vid != NULL);
    Q_ASSERT(pid != NULL);

    CFNumberRef number;

    number = (CFNumberRef) IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kUSBVendorID),
                                                           kCFAllocatorDefault, 0);
    CFNumberGetValue(number, kCFNumberSInt16Type, vid);
    CFRelease(number);

    number = (CFNumberRef) IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kUSBProductID),
                                                           kCFAllocatorDefault, 0);
    CFNumberGetValue(number, kCFNumberSInt16Type, pid);
    CFRelease(number);
}

void HPMPrivate::deviceAdded(io_iterator_t iterator)
{
    io_service_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator)) != 0)
    {
        UInt16 vid = 0, pid = 0;
        extractVidPid(usbDevice, &vid, &pid);
        hpm->emitDeviceAdded(vid, pid);
        IOObjectRelease(usbDevice);
    }
}

void HPMPrivate::deviceRemoved(io_iterator_t iterator)
{
    io_service_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator)))
    {
        UInt16 vid = 0, pid = 0;
        extractVidPid(usbDevice, &vid, &pid);
        hpm->emitDeviceRemoved(vid, pid);
        IOObjectRelease(usbDevice);
    }
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
    delete d_ptr;
    d_ptr = NULL;
}

void HotPlugMonitor::stop()
{
    CFRunLoopStop(d_ptr->loop);
    while (isRunning() == true)
        usleep(10);
}

void HotPlugMonitor::run()
{
    mach_port_t             masterPort = 0;
    IONotificationPortRef   notifyPort = 0;
    io_iterator_t           rawAddedIter = 0;
    io_iterator_t           rawRemovedIter = 0;

    // Create an IOMasterPort for accessing IOKit
    kern_return_t kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (kr || !masterPort)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a master I/O Kit port" << (void*) kr;
        return;
    }

    // Create a new dictionary for matching device classes
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matchingDict)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a USB matching dictionary";
        mach_port_deallocate(mach_task_self(), masterPort);
        return;
    }

    // Take an extra reference because IOServiceAddMatchingNotification consumes one
    matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);

    // Store the thread's run loop context
    d_ptr->loop = CFRunLoopGetCurrent();
    // New notification port
    notifyPort = IONotificationPortCreate(masterPort);

    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notifyPort);
    CFRunLoopAddSource(d_ptr->loop, runLoopSource, kCFRunLoopDefaultMode);

    // Listen to device add notifications
    kr = IOServiceAddMatchingNotification(notifyPort,
                                          kIOFirstMatchNotification,
                                          matchingDict,
                                          onRawDeviceAdded,
                                          (void*) d_ptr,
                                          &rawAddedIter);
    if (kr != kIOReturnSuccess)
        qFatal("Unable to add notification for device additions");

    // Iterate over set of matching devices to access already-present devices
    // and to arm the notification.
    onRawDeviceAdded(d_ptr, rawAddedIter);

    // Listen to device removal notifications
    kr = IOServiceAddMatchingNotification(notifyPort,
                                          kIOTerminatedNotification,
                                          matchingDict,
                                          onRawDeviceRemoved,
                                          (void*) d_ptr,
                                          &rawRemovedIter);
    if (kr != kIOReturnSuccess)
        qFatal("Unable to add notification for device termination");

    // Iterate over set of matching devices to release each one and to
    // arm the notification.
    onRawDeviceRemoved(d_ptr, rawRemovedIter);

    // No longer needed
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;

    // Start the run loop inside this thread. The thread "stops" here.
    CFRunLoopRun();

    // Destroy the notification port when the thread exits
    IONotificationPortDestroy(notifyPort);
    notifyPort = 0;
}
