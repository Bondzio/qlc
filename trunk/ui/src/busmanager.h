/*
  Q Light Controller
  busmanager.h

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

#ifndef BUSMANAGER_H
#define BUSMANAGER_H

#include <QWidget>
#include "app.h"

class QTreeWidgetItem;
class QTreeWidget;
class QToolBar;

class BusManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BusManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the BusManager singleton instance. Can be NULL. */
    static BusManager* instance();

    /** Create a BusManager with parent. Fails if s_instance is not NULL. */
    static void createAndShow(QWidget* parent);

    /** Normal public destructor */
    ~BusManager();

private:
    /** Private constructor to prevent multiple instances. */
    BusManager(QWidget* parent, Qt::WindowFlags f = 0);

private:
    static BusManager* s_instance;

    /*********************************************************************
     * Toolbar
     *********************************************************************/
private slots:
    void slotEditClicked();

private:
    QToolBar* m_toolbar;

    /*********************************************************************
     * Tree widget
     *********************************************************************/
private:
    void fillTree();

private slots:
    void slotItemChanged(QTreeWidgetItem* item, int column);
    void slotBusValueChanged(quint32 bus, quint32 value);

private:
    QTreeWidget* m_tree;
};

#endif
