/*
  Q Light Controller
  inputmap.cpp
  
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

#include <QMessageBox>
#include <QStringList>
#include <iostream>
#include <QList>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "inputmap.h"
#include "app.h"

using namespace std;

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap()
{
}

InputMap::~InputMap()
{
	QLCInPlugin* inputPlugin;
	while ((inputPlugin = m_plugins.takeFirst()) != NULL)
		delete inputPlugin;
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList InputMap::pluginNames()
{
	QListIterator <QLCInPlugin*> it(m_plugins);
	QStringList list;

	while (it.hasNext() == true)
		list.append(it.next()->name());
	
	return list;
}

int InputMap::pluginInputs(const QString& pluginName)
{
	/* TODO */
	return 0;
}

void InputMap::configurePlugin(const QString& pluginName)
{
	QLCInPlugin* inputPlugin = plugin(pluginName);
	if (inputPlugin == NULL)
		QMessageBox::warning(_app,
				     "Unable to configure plugin",
				     pluginName + QString(" not found!"));
	else
		inputPlugin->configure(_app);
}

QString InputMap::pluginStatus(const QString& pluginName)
{
	QLCInPlugin* inputPlugin;
	QString info;

	if (pluginName != QString::null)
		inputPlugin = plugin(pluginName);
	else
		inputPlugin = NULL;
	
	if (inputPlugin == NULL)
	{
		/* Overall plugin info */

		// HTML header
		info += QString("<HTML>");
		info += QString("<HEAD>");
		info += QString("<TITLE>Input mapping status</TITLE>");
		info += QString("</HEAD>");
		info += QString("<BODY>");

		// Mapping status title
		info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
		info += QString("<TR>");
		info += QString("<TD BGCOLOR=\"");
		//info += _app->colorGroup().highlight().name();
		info += QString("\">");
		info += QString("<FONT COLOR=\"");
		//info += _app->colorGroup().highlightedText().name();
		info += QString("\" SIZE=\"5\">");
		info += QString("Input mapping status");
		info += QString("</FONT>");
		info += QString("</TD>");
		info += QString("</TR>");
		info += QString("</TABLE>");

		info += QString("TODO...");
	}
	else
	{
		/* Plugin-specific info */
		info = inputPlugin->infoText();
	}

	return info;
}

bool InputMap::appendPlugin(QLCInPlugin* inputPlugin)
{
	Q_ASSERT(inputPlugin != NULL);

	if (plugin(inputPlugin->name()) == NULL)
	{
		cout << "Found input plugin: "
		     << inputPlugin->name().toStdString()
		     << endl;
		m_plugins.append(inputPlugin);
		return true;
	}
	else
	{
		cout << "Input plugin: "
		     << inputPlugin->name().toStdString()
		     << " is already loaded." << endl;
		return false;
	}
}

QLCInPlugin* InputMap::plugin(const QString& name)
{
	QListIterator <QLCInPlugin*> it(m_plugins);

	while (it.hasNext() == true)
	{
		QLCInPlugin* plugin = it.next();
		if (plugin->name() == name)
			return plugin;
	}
	
	return NULL;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults(const QString& path)
{
}

void InputMap::saveDefaults(const QString& path)
{
}
