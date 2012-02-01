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
    HPMPrivate(HotPlugMonitor* hpm);

    kern_return_t extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid);
    void deviceAdded(io_iterator_t iterator);
    void deviceRemoved(io_iterator_t iterator);

public:
    HotPlugMonitor*         m_hpm;

    CFRunLoopRef            loop;
    IONotificationPortRef   notifyPort;
    io_iterator_t           rawAddedIter;
    io_iterator_t           rawRemovedIter;
};

/****************************************************************************
 * Static callback functions for IOKit
 ****************************************************************************/

static void onRawDeviceAdded(void* refCon, io_iterator_t iterator)
{
    qDebug() << Q_FUNC_INFO;

    HPMPrivate* d_ptr = (HPMPrivate*) refCon;
    Q_ASSERT(d_ptr != NULL);
    d_ptr->deviceAdded(iterator);
}

static void onRawDeviceRemoved(void* refCon, io_iterator_t iterator)
{
    qDebug() << Q_FUNC_INFO;

    HPMPrivate* d_ptr = (HPMPrivate*) refCon;
    Q_ASSERT(d_ptr != NULL);
    d_ptr->deviceRemoved(iterator);
}

/****************************************************************************
 * HPMPrivate implementation
 ****************************************************************************/

HPMPrivate::HPMPrivate(HotPlugMonitor* hpm)
    : m_hpm(hpm)
    , loop(NULL)
    , notifyPort(0)
    , rawAddedIter(0)
    , rawRemovedIter(0)
{
    qDebug() << Q_FUNC_INFO;
}

kern_return_t HPMPrivate::extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid)
{
    qDebug() << Q_FUNC_INFO << (void*) usbDevice;

    Q_ASSERT(vid != NULL);
    Q_ASSERT(pid != NULL);

    kern_return_t kr = 0;
    SInt32 score = 0;
    HRESULT result = 0;
    IOCFPlugInInterface** plugInInterface = NULL;
    IOUSBDeviceInterface** dev = NULL;

    // Create an intermediate plug-in
    kr = IOCreatePlugInInterfaceForService(usbDevice,
                                           kIOUSBDeviceUserClientTypeID,
                                           kIOCFPlugInInterfaceID,
                                           &plugInInterface,
                                           &score);
    // Don't need the device object after intermediate plug-in is created
    if (kr != kIOReturnSuccess || plugInInterface == NULL)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a plug-in:" << kr;
        return kr;
    }

    // Now create the device interface
    result = (*plugInInterface)->QueryInterface(plugInInterface,
                                                CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                                                (LPVOID*) &dev);
    // Don't need the intermediate plug-in after device interface is created
    (*plugInInterface)->Release(plugInInterface);
    if (result != 0 || dev == NULL)
    {
        qWarning() << Q_FUNC_INFO << "Couldn't create a device interface:" << (int) result;
        return result;
    }

    kr = (*dev)->GetDeviceVendor(dev, vid);
    if (kr != kIOReturnSuccess)
    {
        qWarning() << Q_FUNC_INFO << "Unable to acquire Vendor ID:" << kr;
        (void) (*dev)->Release(dev);
        return kr;
    }

    kr = (*dev)->GetDeviceProduct(dev, pid);
    if (kr != kIOReturnSuccess)
    {
        qWarning() << Q_FUNC_INFO << "Unable to acquire Product ID:" << kr;
        (void) (*dev)->Release(dev);
        return kr;
    }

    // Release the device
    (void) (*dev)->Release(dev);

    return kIOReturnSuccess;
}

void HPMPrivate::deviceAdded(io_iterator_t iterator)
{
    io_service_t usbDevice;
    kern_return_t kr;

    while ((usbDevice = IOIteratorNext(iterator)) != 0)
    {
        qDebug() << "Device" << (void*) usbDevice << "added";

        UInt16 vid = 0, pid = 0;
        kr = extractVidPid(usbDevice, &vid, &pid);
        if (kr != kIOReturnSuccess)
            continue;
        else
            m_hpm->emitDeviceAdded(vid, pid);

        kr = IOObjectRelease(usbDevice);
    }
}

void HPMPrivate::deviceRemoved(io_iterator_t iterator)
{
    io_service_t usbDevice;
    kern_return_t kr;

    while ((usbDevice = IOIteratorNext(iterator)))
    {
        qDebug() << "Device" << (void*) usbDevice << "removed";

        UInt16 vid = 0, pid = 0;
        kr = extractVidPid(usbDevice, &vid, &pid);
        if (kr != kIOReturnSuccess)
            continue;
        else
            m_hpm->emitDeviceRemoved(vid, pid);

        kr = IOObjectRelease(usbDevice);
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
    qDebug() << Q_FUNC_INFO;
}

HotPlugMonitor::~HotPlugMonitor()
{
    qDebug() << Q_FUNC_INFO;
    delete d_ptr;
    d_ptr = NULL;
}

void HotPlugMonitor::stop()
{
    qDebug() << Q_FUNC_INFO;

    m_run = false;
    CFRunLoopStop(d_ptr->loop);
    while (isRunning() == true)
        usleep(10);
}

void HotPlugMonitor::run()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_run == false);

    // Create an IOMasterPort for accessing IOKit
    mach_port_t masterPort;
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

    // Take an extra reference because IOServiceAddMatchingNotification consumes them
    matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);

/*
    // Use these to get notifications of specific VID/PID combinations
    SInt32 vid = 0x0403;
    SInt32 pid = 0x6001;
    CFDictionarySetValue(matchingDict, CFSTR(kUSBVendorID),
                         CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vid));
    CFDictionarySetValue(matchingDict, CFSTR(kUSBProductID),
                         CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pid));
*/

    d_ptr->loop = CFRunLoopGetCurrent(); // Store the thread's run loop context
    d_ptr->notifyPort = IONotificationPortCreate(masterPort); // New notification port

    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(d_ptr->notifyPort);
    CFRunLoopAddSource(d_ptr->loop, runLoopSource, kCFRunLoopDefaultMode);

    // Listen to device add notifications
    kr = IOServiceAddMatchingNotification(d_ptr->notifyPort,
                                          kIOFirstMatchNotification,
                                          matchingDict,
                                          onRawDeviceAdded,
                                          (void*) d_ptr,
                                          &d_ptr->rawAddedIter);
    if (kr != kIOReturnSuccess)
        qFatal("Unable to add notification for device additions");

    // Iterate over set of matching devices to access already-present devices
    // and to arm the notification.
    onRawDeviceAdded(d_ptr, d_ptr->rawAddedIter);

    // Listen to device removal notifications
    kr = IOServiceAddMatchingNotification(d_ptr->notifyPort,
                                          kIOTerminatedNotification,
                                          matchingDict,
                                          onRawDeviceRemoved,
                                          (void*) d_ptr,
                                          &d_ptr->rawRemovedIter);
    if (kr != kIOReturnSuccess)
        qFatal("Unable to add notification for device termination");

    // Iterate over set of matching devices to release each one and to
    // arm the notification.
    onRawDeviceRemoved(d_ptr, d_ptr->rawRemovedIter);

    // No longer needed
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;

    // Start the run loop inside this thread
    m_run = true;
    qDebug() << Q_FUNC_INFO << "Start run loop";
    CFRunLoopRun();
    qDebug() << Q_FUNC_INFO << "Run loop terminated";
    m_run = false;
}
