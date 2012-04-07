/*
  Q Light Controller
  qlcplugincache.h

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

#ifndef QLCPLUGINCACHE_H
#define QLCPLUGINCACHE_H

#include <QObject>
#include <QHash>
#include <QDir>

class QLCIOPlugin;

class QLCPluginCache : public QObject
{
    Q_OBJECT

public:
    QLCPluginCache(QObject* parent);
    ~QLCPluginCache();

    /**
     * Load all plugins from the given directory. Each successfully loaded plugin
     * will be emitted using the ioPluginAdded() signal.
     *
     * @param dir The directory to load from
     */
    void loadIOPlugins(const QDir& dir);

    /**
     * Get a plugin by its UID.
     *
     * @param uid The UID of the plugin to get.
     * @return The plugin whose UID matches $uid or NULL if not found.
     */
    QLCIOPlugin* ioPlugin(qulonglong uid) const;

    /**
     * Get all available IO plugins.
     *
     * @return A hash containing all IO plugins with plugin UID as the key
     */
    QHash <qulonglong,QLCIOPlugin*> ioPlugins() const;

    /**
     * Get the system I/O plugin directory.
     * The location varies greatly between platforms.
     *
     * @return System I/O plugin directory.
     */
    static QDir systemIOPluginDirectory();

private:
    QHash <qulonglong,QLCIOPlugin*> m_ioPlugins;
};

#endif
