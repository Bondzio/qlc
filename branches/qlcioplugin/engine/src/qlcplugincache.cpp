/*
  Q Light Controller
  qlcplugincache.cpp

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

#include <QPluginLoader>
#include <QDebug>

#include "hotplugmonitor.h"
#include "qlcplugincache.h"
#include "qlcioplugin.h"
#include "qlcconfig.h"
#include "qlcfile.h"

QLCPluginCache::QLCPluginCache(QObject* parent)
    : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;
}

QLCPluginCache::~QLCPluginCache()
{
    qDebug() << Q_FUNC_INFO;
}

void QLCPluginCache::loadIOPlugins(const QDir& dir)
{
    qDebug() << Q_FUNC_INFO << dir;

    /* Check that we can access the directory */
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Loop thru all files in the directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        /* Attempt to load a plugin from the path */
        QString fileName(it.next());
        QString path = dir.absoluteFilePath(fileName);
        QPluginLoader loader(path, this);
        QLCIOPlugin* ptr = qobject_cast<QLCIOPlugin*> (loader.instance());
        if (ptr != NULL)
        {
            /* Check for duplicates */
            if (ioPlugin(ptr->uid()) == NULL)
            {
                /* New plugin. Append and init. */
                qDebug() << "Loaded I/O plugin" << ptr->name() << "from" << fileName;
                ptr->init();

                m_ioPlugins[ptr->uid()] = ptr;
                HotPlugMonitor::connectListener(ptr);

                //! @todo Plugin translation
                //QLCi18n::loadTranslation(p->name().replace(" ", "_"));
            }
            else
            {
                /* Duplicate plugin. Unload it. */
                qWarning() << Q_FUNC_INFO << "Discarded duplicate I/O plugin"
                           << fileName;
                loader.unload();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << fileName
                       << "doesn't contain a QLC input plugin:"
                       << loader.errorString();
            loader.unload();
        }
    }
}

QLCIOPlugin* QLCPluginCache::ioPlugin(qulonglong uid) const
{
    qDebug() << Q_FUNC_INFO << uid;

    if (m_ioPlugins.contains(uid) == true)
        return m_ioPlugins[uid];
    else
        return NULL;
}

QHash <qulonglong,QLCIOPlugin*> QLCPluginCache::ioPlugins() const
{
    return m_ioPlugins;
}

QDir QLCPluginCache::systemIOPluginDirectory()
{
    qDebug() << Q_FUNC_INFO;

    QDir dir;
#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(PLUGINDIR));
#else
    dir.setPath(PLUGINDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));

    return dir;
}
