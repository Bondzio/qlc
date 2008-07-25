/*
  Q Light Controller
  qlcplugin.h

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

#ifndef QLCPLUGIN_H
#define QLCPLUGIN_H

#include <QObject>
#include "qlctypes.h"

class QLCPlugin;

/** 
 * Type definition for the only C-style exported function for plugins.
 * Define a function called "create" that is of this type and use it to
 * invoke new for your plugin and return the newly-created object, like this:
 *
 * extern "C" [Output|Input]Plugin* create()
 * {
 *         return new MyPlugin;
 * }
 *
 */
typedef QLCPlugin* (*QLCPluginCreateFunction)();

class QLC_DECLSPEC QLCPlugin : public QObject
{
	Q_OBJECT

public:
	/**
	 * Construct a new plugin
	 */
	QLCPlugin();

	/**
	 * Destroy the plugin
	 */
	virtual ~QLCPlugin();
	
	/**
	 * Plugin type
	 */
	enum Type
	{
		Output,
		Input
	};
	
	/**
	 * Open (initialize for operation) the plugin.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 *
	 */
	virtual int open() = 0;

	/**
	 * Close (de-initialize) the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual int close() = 0;

	/**
	 * Invoke a configuration dialog for the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 *
	 * @param parentWidget A parent QWidget for the configuration dialog
	 */
	virtual int configure(QWidget* parentWidget) = 0;

	/**
	 * Provide an information text to be displayed in the plugin manager
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual QString infoText() = 0;

	/*********************************************************************
	 * Standard functions that should not be overwritten
	 *********************************************************************/
	
	/**
	 * Get the plugin's name
	 */
	QString name();

	/**
	 * Get the plugin's version (possibly obsolete)
	 */
	unsigned long version();

	/**
	 * Get the plugin's type
	 */
	Type type();

	/**
	 * Set the plugin's library handle (returned by dlopen())
	 */
	void setHandle(void* handle);

	/**
	 * Get the plugin's library handle
	 */
	void* handle();

protected:
	QString m_name;
	Type m_type;
	unsigned long m_version;
	void* m_handle;
};

#endif
