/*
  Q Light Controller
  udmxthread.h

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

#ifndef UDMXTHREAD_H
#define UDMXTHREAD_H

#include <QThread>
#include <QMutex>
#include <QSet>

class UDMXDevice;

class UDMXThread : public QThread
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    UDMXThread(QObject* parent);
    virtual ~UDMXThread();

    /************************************************************************
     * Devices
     ************************************************************************/
public:
    void addDevice(UDMXDevice* device);
    void removeDevice(UDMXDevice* device);

    QSet <UDMXDevice*> devices() const;

private:
    QSet <UDMXDevice*> m_devices;

    /************************************************************************
     * Thread
     ************************************************************************/
public:
    enum TimerGranularity { Unknown, Good, Bad };

    TimerGranularity granularity() const;
    double frequency() const;

    /** Stop the thread */
    void stop();

private:
    /** Worker thread */
    void run();

private:
    TimerGranularity m_granularity;
    double m_frequency;
    bool m_running;
    QMutex m_mutex;
};

#endif
