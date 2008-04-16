/*
  Q Light Controller
  configurehidinput.cpp
  
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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPushButton>
#include <QString>
#include <QTimer>

#include "configurehidinput.h"
#include "hidinput.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureHIDInput::ConfigureHIDInput(QWidget* parent, HIDInput* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;

	setupUi(this);

	connect(m_refreshButton, SIGNAL(clicked()),
		this, SLOT(slotRefreshClicked()));
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	refreshList();
}

ConfigureHIDInput::~ConfigureHIDInput()
{
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureHIDInput::slotRefreshClicked()
{
	m_plugin->open();
	refreshList();
}

void ConfigureHIDInput::refreshList()
{
	QString s;

	m_list->clear();

	for (unsigned int i = 0; i < m_plugin->m_devices.count(); i++)
	{
		HIDDevice* dev;
		QTreeWidgetItem* item;

		dev = m_plugin->device(i);
		Q_ASSERT(dev != NULL);

		item = new QTreeWidgetItem(m_list);
		item->setText(0, s.setNum(i + 1));
		item->setText(1, dev->name());
	}
}

