/*
  Q Light Controller
  eventbuffer.cpp

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

#include "assert.h"
#include "eventbuffer.h"

#include <qapplication.h>
#include <malloc.h>
#include <string.h>

EventBuffer::EventBuffer(unsigned int eventSize, unsigned int bufferSize) :
	m_ring       ( NULL ),
	m_size       ( bufferSize * eventSize ),
	m_eventSize  ( eventSize ),
	m_filled     ( 0 ),
	m_in         ( 0 ),
	m_out        ( 0 )
{
	m_ring = new t_buffer_data[m_size];
	
	for (unsigned int i = 0; i < m_size; i++)
		m_ring[i] = 0;

#ifndef WIN32
	pthread_mutex_init(&m_mutex, 0);
	pthread_cond_init(&m_nonEmpty, 0);
	pthread_cond_init(&m_nonFull, 0);
#else
	m_nonEmpty = true;
	m_nonFull = true;
#endif
}

EventBuffer::~EventBuffer()
{
	delete [] m_ring;

#ifndef WIN32
	pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_nonEmpty);
	pthread_cond_destroy(&m_nonFull);
#endif
}


void EventBuffer::put(t_buffer_data* ev)
{
#ifndef WIN32
	pthread_mutex_lock(&m_mutex);
	if (m_filled == m_size)
		pthread_cond_wait(&m_nonFull, &m_mutex);
#else
	m_mutex.lock();
	while (m_filled == m_size)
	{
		m_mutex.unlock();
		m_mutex.lock();
	}
#endif
	
	assert(m_filled < m_size);
	memcpy(m_ring + m_in, ev, m_eventSize * sizeof(t_buffer_data));
	m_in = (m_in + m_eventSize) % m_size;
	m_filled += m_eventSize;

#ifndef WIN32
	pthread_cond_signal(&m_nonEmpty);
	pthread_mutex_unlock(&m_mutex);
#else
	m_mutex.unlock();
#endif
}


int EventBuffer::get(t_buffer_data* event)
{
#ifndef WIN32
	pthread_mutex_lock(&m_mutex);
	if (m_filled == 0)
	{
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}
#else
	m_mutex.lock();
	if (m_filled == 0)
	{
		m_mutex.unlock();
		return -1;
	}
#endif
	
	assert(m_filled > 0);
	memcpy(event, m_ring + m_out, m_eventSize * sizeof(t_buffer_data));
	m_out = (m_out + m_eventSize) % m_size;
	m_filled -= m_eventSize;
	
#ifndef WIN32
	pthread_cond_signal(&m_nonFull);
	pthread_mutex_unlock(&m_mutex);
#else
	m_mutex.unlock();
#endif	
	return 0;
}


void EventBuffer::purge()
{
#ifndef WIN32
	pthread_mutex_lock(&m_mutex);
#else
	m_mutex.lock();
#endif
	
	m_filled = 0;
	m_in = 0;
	m_out = 0;
	
#ifndef WIN32
	pthread_mutex_unlock(&m_mutex);
#else
	m_mutex.unlock();
#endif
}
