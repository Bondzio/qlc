/*
  Q Light Controller
  efxfixture.cpp
  
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

#include <iostream>
#include <math.h>

#include "eventbuffer.h"
#include "efxfixture.h"
#include "function.h"
#include "efx.h"

using namespace std;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXFixture::EFXFixture(EFX* parent, int index, Function::Direction direction)
{
	Q_ASSERT(parent != NULL);

	m_parent = parent;
	m_index = index;
	m_direction = direction;

	m_skipIterator = 0;
	m_skipThreshold = 0;
	m_iterator = 0;
	m_panValue = 0;
	m_tiltValue = 0;

	m_lsbPanChannel = KChannelInvalid;
	m_msbPanChannel = KChannelInvalid;
	m_lsbTiltChannel = KChannelInvalid;
	m_msbTiltChannel = KChannelInvalid;
}

EFXFixture::~EFXFixture()
{
}

void EFXFixture::reset()
{
	m_skipIterator = 0;
	m_iterator = 0;
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void EFXFixture::setLsbPanChannel(t_channel ch)
{
	m_lsbPanChannel = ch;
}

void EFXFixture::setMsbPanChannel(t_channel ch)
{
	m_msbPanChannel = ch;
}

void EFXFixture::setLsbTiltChannel(t_channel ch)
{
	m_lsbTiltChannel = ch;
}

void EFXFixture::setMsbTiltChannel(t_channel ch)
{
	m_msbTiltChannel = ch;
}

void EFXFixture::updateSkipThreshold()
{
	m_skipThreshold = float(m_index) *
		(float(M_PI * 2.0) / float(m_parent->fixtureCount()));
}

bool EFXFixture::isValid()
{
	if (m_msbPanChannel != KChannelInvalid &&
	    m_msbTiltChannel != KChannelInvalid)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFXFixture::nextStep(t_buffer_data* data)
{
	if (m_parent->propagationMode() == EFX::Serial &&
	    m_skipIterator < m_skipThreshold)
	{
		m_skipIterator += m_parent->m_stepSize;
	}
	else
	{
		m_iterator += m_parent->m_stepSize;
	}

	if (m_iterator < (M_PI * 2.0))
	{
		if (m_direction == Function::Forward)
		{
			m_parent->pointFunc(m_parent, m_iterator,
					    &m_panValue, &m_tiltValue);
		}
		else
		{
			m_parent->pointFunc(m_parent, (M_PI * 2.0) - m_iterator,
					    &m_panValue, &m_tiltValue);
		}

		setPoint(data);
	}
	else
	{
		/* Reset iterator, since we've gone a full circle */
		m_iterator = 0;
		
		if (m_parent->m_runOrder == Function::PingPong)
		{
			if (m_direction == Function::Forward)
				m_direction = Function::Backward;
			else
				m_direction = Function::Forward;
		}
		else if (m_parent->m_runOrder == Function::SingleShot)
		{
			/* TODO: SingleShot */
		}
		else
		{
			/* TODO: Loop */
		}
	}
}

void EFXFixture::setPoint(t_buffer_data* data)
{
	/* Write coarse point data to event buffer */
	data[m_index + 0]  = m_msbPanChannel << 8;
	data[m_index + 0] |= static_cast <t_value> (m_panValue);

	data[m_index + 1]  = m_msbTiltChannel << 8;
	data[m_index + 1] |= static_cast <t_value> (m_tiltValue);
	
	/* Write fine point data to event buffer if applicable */
	if (m_lsbPanChannel != KChannelInvalid)
	{
		data[m_index + 2]  = m_lsbPanChannel << 8;
		data[m_index + 2] |= static_cast <t_value> 
			((m_panValue - floor(m_panValue)) * 255.0);
	}

	if (m_lsbTiltChannel != KChannelInvalid)
	{
		data[m_index + 3]  = m_lsbTiltChannel << 8;
		data[m_index + 3] |= static_cast <t_value> 
			((m_tiltValue - floor(m_tiltValue)) * 255.0);
	}
}

