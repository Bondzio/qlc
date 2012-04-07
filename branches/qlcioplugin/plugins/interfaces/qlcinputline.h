/*
  Q Light Controller
  qlcinputline.h

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

#ifndef QLCINPUTLINE_H
#define QLCINPUTLINE_H

#include <QObject>
#include <QString>

class QLCIOPlugin;

class QLCInputLine : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    QLCInputLine(QLCIOPlugin* plugin, qulonglong uid, const QString& name)
        : m_plugin(plugin), m_uid(uid), m_name(name) { /* NOP */ }

    virtual ~QLCInputLine() { /* NOP */ }

    QLCIOPlugin* plugin() const { return m_plugin; }
    qulonglong uid() const { return m_uid; }
    QString name() const { return m_name; }

    virtual QString description() = 0;

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool isOpen() = 0;

    virtual void sendFeedback(quint32 channel, uchar value) = 0;

signals:
    void valueChanged(quint32 channel, uchar value);

private:
    QLCIOPlugin* m_plugin;
    qulonglong m_uid;
    QString m_name;
};

#endif
