/*
  Q Light Controller
  udmxthread.cpp

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

#include <QSettings>
#include <QDebug>
#include <QTime>
#include <cmath>

#include "udmxthread.h"
#include "udmxdevice.h"
#include "qlcmacros.h"

#define SETTINGS_FREQUENCY "udmx/frequency"

/****************************************************************************
 * Initialization
 ****************************************************************************/

UDMXThread::UDMXThread(QObject* parent)
    : QThread(parent)
    , m_granularity(Unknown)
    , m_frequency(30)
    , m_running(false)
{
    QSettings settings;
    QVariant var = settings.value(SETTINGS_FREQUENCY);
    if (var.isValid() == true)
        m_frequency = var.toDouble();
}

UDMXThread::~UDMXThread()
{
    stop();
}

/****************************************************************************
 * Devices
 ****************************************************************************/

void UDMXThread::addDevice(UDMXDevice* device)
{
    m_mutex.lock();
    if (device != NULL && m_devices.contains(device) == false)
        m_devices << device;
    m_mutex.unlock();
}

void UDMXThread::removeDevice(UDMXDevice* device)
{
    m_mutex.lock();
    if (m_devices.contains(device) == true)
        m_devices.remove(device);
    m_mutex.unlock();
}

QSet <UDMXDevice*> UDMXThread::devices() const
{
    return m_devices;
}

/****************************************************************************
 * Thread
 ****************************************************************************/

UDMXThread::TimerGranularity UDMXThread::granularity() const
{
    return m_granularity;
}

double UDMXThread::frequency() const
{
    return m_frequency;
}

void UDMXThread::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void UDMXThread::run()
{
    // One "official" DMX frame can take (1s/44Hz) = 23ms
    int frameTime = (int) floor(((double)1000 / m_frequency) + (double)0.5);

    // Wait for device to settle in case the device was opened just recently
    // Also measure, whether timer granularity is OK
    QTime time;
    time.start();
    usleep(1000);
    if (time.elapsed() > 3)
        m_granularity = Bad;
    else
        m_granularity = Good;

    m_running = true;
    while (m_running == true)
    {
        time.restart();

        // Write all devices' data inside mutex lock to prevent device removal
        // in the middle of a write operation.
        m_mutex.lock();
        foreach (UDMXDevice* device, m_devices)
            device->writeHW();
        m_mutex.unlock();

        // Sleep for the remainder of the DMX frame time
        if (m_granularity == Good)
            while (time.elapsed() < frameTime) { usleep(1000); }
        else
            while (time.elapsed() < frameTime) { /* Busy sleep */ }
    }
}
