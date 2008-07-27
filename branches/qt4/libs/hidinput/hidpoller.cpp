/*
  Q Light Controller
  hidpoller.cpp

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

#include "hidpoller.h"
#include "hidinput.h"
#include "poll.h"

#define KPollTimeout 5000

/*****************************************************************************
 * Initialization
 *****************************************************************************/

HIDPoller::HIDPoller(HIDInput* parent) : QThread(parent)
{
	Q_ASSERT(parent != NULL);

	m_running = true;
	start();
}

HIDPoller::~HIDPoller()
{
	m_devices.clear();
	stop();
}

/*****************************************************************************
 * Polled devices
 *****************************************************************************/

bool HIDPoller::addDevice(HIDDevice* device)
{
	Q_ASSERT(device != NULL);

	if (m_devices.contains(device) == true)
		return false;

	if (device->open() == true)
	{
		m_devices.append(device);
		m_changed = true;
	}

	return true;
}

bool HIDPoller::removeDevice(HIDDevice* device)
{
	Q_ASSERT(device != NULL);

	if (m_devices.contains(device) == false)
		return false;

	device->close();
	m_devices.removeAll(device);
	m_changed = true;

	return true;
}

/*****************************************************************************
 * Poller thread
 *****************************************************************************/

void HIDPoller::stop()
{
	m_running = false;
	wait();
}

void HIDPoller::run()
{
	struct pollfd* fds = NULL;
	HIDDevice* device;
	int r;

	while (m_running == true)
	{
		/* If the list of polled devices has changed, reload all
		   devices into the array of pollfd's */
		if (m_changed == true)
		{
			delete [] fds;
			fds = new struct pollfd[m_devices.count()];
			memset(fds, 0, sizeof(struct pollfd) * m_devices.count());
			
			for (int i = 0; i < m_devices.count(); i++)
			{
				device = m_devices.at(i);
				Q_ASSERT(device != NULL);
				device->open();

				fds[i].fd = device->handle();
				fds[i].events = POLLIN;
			}

			m_changed = false;
		}

		r = poll(fds, m_devices.count(), KPollTimeout);
		if (r == 0)
		{
			/* Plain timeout, continue polling */
			continue;
		}
		else if (r < 0)
		{
			/* Error occurred */
			perror("poll: ");
			continue;
		}
		else
		{
			/* One or more polled descriptors produced an event */
		}
	}
}
