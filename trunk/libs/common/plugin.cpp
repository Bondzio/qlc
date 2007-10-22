/*
  Q Light Controller
  plugin.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#include "plugin.h"

Plugin::Plugin(t_plugin_id id) : QObject()
{
	m_id = id;
	m_handle = NULL;
}

Plugin::~Plugin()
{
}

QString Plugin::name()
{
	return m_name;
}

t_plugin_id Plugin::id()
{
	return m_id;
}

unsigned long Plugin::version()
{
	return m_version;
}

Plugin::PluginType Plugin::type()
{
	return m_type;
}

void Plugin::setHandle(void* handle)
{
	m_handle = handle;
}

void* Plugin::handle()
{
	return m_handle;
}

void Plugin::setEventReceiver(QObject* parent)
{
	m_eventReceiver = parent;
}

QObject* Plugin::eventReceiver() const
{
	return m_eventReceiver;
}
	
