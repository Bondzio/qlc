/*
  Q Light Controller
  dmxmapeditor.cpp
  
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
#include <QStringList>
#include <QAction>
#include <QMenu>

#include "common/qlcoutplugin.h"

#include "dmxpatcheditor.h"
#include "dmxmapeditor.h"
#include "dmxmap.h"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnOutput   2

DMXMapEditor::DMXMapEditor(QWidget* parent, DMXMap* dmxMap) : QDialog(parent)
{
	QTreeWidgetItem* item;
	DMXPatch* dmxPatch;
	QString str;

	Q_ASSERT(dmxMap != NULL);
	m_dmxMap = dmxMap;

	setupUi(this);

	m_pluginList = m_dmxMap->pluginNames();

	/* Clear the mapping list first */
	m_listView->clear();

	for (int i = 0; i < KUniverseCount; i++)
	{
		dmxPatch = m_dmxMap->patch(i);
		Q_ASSERT(dmxPatch != NULL);

		item = new QTreeWidgetItem(m_listView);
		item->setText(KColumnUniverse, str.setNum(i + 1));
		item->setText(KColumnPlugin, dmxPatch->plugin->name());
		item->setText(KColumnOutput, str.setNum(dmxPatch->output + 1));
	}

	connect(m_listView, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(slotListViewContextMenuRequested(const QPoint&)));
}

DMXMapEditor::~DMXMapEditor()
{
}

/*****************************************************************************
 *
 *****************************************************************************/

void DMXMapEditor::slotEditMappingButtonClicked()
{
	QTreeWidgetItem* item;
	int universe;
	QString str;
	int output;

	item = m_listView->currentItem();
	if (item == NULL)
		return;

	universe = item->text(KColumnUniverse).toInt() - 1;
	str = item->text(KColumnPlugin);
	output = item->text(KColumnOutput).remove("Output").toInt() - 1;

	DMXPatchEditor dpe(this, m_dmxMap, universe, str, output);
	if (dpe.exec() == QDialog::Accepted)
	{
		m_dmxMap->setPatch(universe, dpe.pluginName(), dpe.output());
		item->setText(KColumnUniverse, str.setNum(universe + 1));
		item->setText(KColumnPlugin, dpe.pluginName());
		item->setText(KColumnOutput, str.setNum(dpe.output() + 1));
	}
}

void DMXMapEditor::slotListViewContextMenuRequested(const QPoint& point)
{
	QStringList::Iterator it;
	QTreeWidgetItem* item;
	QAction* action;
	QString name;
	int universe;
	int outputs;
	QString str;

	item = m_listView->currentItem();
	if (item == NULL)
		return;

	QMenu pluginMenu(this);
	pluginMenu.setTitle(str.sprintf("Route universe %1 thru...")
			    .arg(item->text(KColumnUniverse)));

	for (it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
	{
		name = *it;

		outputs = m_dmxMap->pluginOutputs(name);
		if (outputs >= 0)
		{
			/* Put the plugin's outputs into a sub menu and
			   insert that submenu to the top level menu */
			QMenu outputMenu(&pluginMenu);
			outputMenu.setTitle(name);
			pluginMenu.addMenu(&outputMenu);

			for (int i = 0; i < outputs && i < 100; i++)
			{
				action = outputMenu.addAction(
					str.sprintf("Output %1").arg(i + 1));
				action->setData(name);
			}
		}
	}

	/* Execute menu and check, whether something was selected */
	action = pluginMenu.exec(point);
	if (action == NULL)
		return;

	/* Extract selected information from the action and patch them */
	outputs = action->text().remove("Output").toInt() - 1;
	name = action->data().toString();
	m_dmxMap->setPatch(universe, name, outputs);
	
	/* Update the current tree widget item */
	item->setText(KColumnUniverse, str.setNum(universe + 1));
	item->setText(KColumnPlugin, name);
	item->setText(KColumnOutput, str.setNum(outputs + 1));
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void DMXMapEditor::accept()
{
	QTreeWidgetItemIterator it(m_listView);
	QString pluginName;
	int universe;
	int output;

	while (*it != NULL)
	{
		universe = (*it)->text(KColumnUniverse).toInt() - 1;
		pluginName = (*it)->text(KColumnPlugin);
		output = (*it)->text(KColumnOutput).toInt() - 1;

		m_dmxMap->setPatch(universe, pluginName, output);

		++it;
	}

	QDialog::accept();
}
