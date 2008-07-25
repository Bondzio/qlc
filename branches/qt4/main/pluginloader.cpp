/*
  Q Light Controller
  pluginloader.cpp

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

#include <QStringList>
#include <QSettings>
#include <iostream>
#include <QDir>

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "common/qlcplugin.h"
#include "common/qlcinplugin.h"
#include "common/qlcoutplugin.h"

#include "pluginloader.h"
#include "dmxmap.h"
#include "inputmap.h"

using namespace std;

void PluginLoader::load(DMXMap* outputMap, InputMap* inputMap)
{
	QSettings s;
	
	QString pluginPath = s.value("directories/plugins").toString();
	QDir pluginDir(pluginPath, "*.so", QDir::Name, QDir::Files);

	Q_ASSERT(outputMap != NULL);
	Q_ASSERT(inputMap != NULL);

	/* Check that we can access the directory */
	if (pluginDir.exists() == false)
	{
		cout << pluginPath.toStdString() << " doesn't exist" << endl;
		return;
	}
	else if (pluginDir.isReadable() == false)
	{
		cout << pluginPath.toStdString()
		     << " is not accessible"
		     << endl;
		return;
	}

	/* Loop thru all files in the directory */
	QStringList dirlist(pluginDir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		create(pluginPath + QDir::separator() + *it,
		       outputMap, inputMap);
	}
}

#ifndef WIN32
void PluginLoader::create(const QString& path,
			  DMXMap* outputMap, InputMap* inputMap)
{
	QLCPluginCreateFunction create;

	void* pluginHandle = NULL;
	QLCPlugin* plugin = NULL;

	/* Load the (presumed) shared object into memory. Don't resolve
	   symbols until they're needed */
	pluginHandle = ::dlopen((const char*) path.toAscii(), RTLD_LAZY);
	if (pluginHandle == NULL)
	{
		cout << "Unable to open " << path.toStdString()
		     << " with dlopen(): " << dlerror()
		     << endl;
		return;
	}
	
	/* Attempt to resolve "create" symbol from the shared object */
	create = (QLCPluginCreateFunction) ::dlsym(pluginHandle, "create");
	if (create == NULL)
	{
		::dlclose(pluginHandle);
		cout << "Unable to resolve symbols for "
		     << path.toStdString() << ": " << ::dlerror()
		     << endl;
		return;
	}

	/* Attempt to use the "create" symbol to create a Plugin instance */
	plugin = create();
	Q_ASSERT(plugin != NULL);
	
	/* We accept only output plugins here */
	if (plugin->type() == QLCPlugin::Output)
	{
		plugin->setHandle(pluginHandle);

		if (outputMap->appendPlugin(
			    static_cast<QLCOutPlugin*> (plugin)) == false)
		{
			/* Plugin already exists, delete it */
			delete plugin;
		}
	}
	else if (plugin->type() == QLCPlugin::Input)
	{
		plugin->setHandle(pluginHandle);

		if (inputMap->appendPlugin(
			    static_cast<QLCInPlugin*> (plugin)) == false)
		{
			/* Plugin already exists, delete it */
			delete plugin;
		}
	}
	else
	{
		cout << QString("Unknown plugin type: %1")
			.arg(plugin->type()).toStdString() << endl;
		delete plugin;
		::dlclose(pluginHandle);
	}
}
#else
void PluginLoader::create(const QString& path,
			  DMXMap* outputMap, InputMap* inputMap)
{
}
#endif
