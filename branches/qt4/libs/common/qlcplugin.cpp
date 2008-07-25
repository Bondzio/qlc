/*
  Q Light Controller
  qlcplugin.cpp

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

#include "qlcplugin.h"

QLCPlugin::QLCPlugin() : QObject()
{
	m_handle = NULL;
}

QLCPlugin::~QLCPlugin()
{
}

QString QLCPlugin::name()
{
	return m_name;
}

unsigned long QLCPlugin::version()
{
	return m_version;
}

QLCPlugin::Type QLCPlugin::type()
{
	return m_type;
}

void QLCPlugin::setHandle(void* handle)
{
	Q_ASSERT(handle != NULL);
	m_handle = handle;
}

void* QLCPlugin::handle()
{
	return m_handle;
}
