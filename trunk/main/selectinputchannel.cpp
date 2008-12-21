/*
  Q Light Controller
  selectinputchannel.cpp

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

#include <common/qlcinputchannel.h>
#include <common/qlcinputdevice.h>

#include "selectinputchannel.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

#define KColumnName     0
#define KColumnUniverse 1
#define KColumnChannel  2

/****************************************************************************
 * Initialization
 ****************************************************************************/

SelectInputChannel::SelectInputChannel(QWidget* parent) : QDialog(parent)
{
	m_universe = KInputUniverseInvalid;
	m_channel = KInputChannelInvalid;

	setupUi(this);
	fillTree();
}

SelectInputChannel::~SelectInputChannel()
{
}

void SelectInputChannel::accept()
{
	QTreeWidgetItem* item;

	/* Extract data from the selected item */
	item = m_tree->currentItem();
	if (item != NULL)
	{
		m_universe = item->text(KColumnUniverse).toInt();
		m_channel = item->text(KColumnChannel).toInt();
	}

	QDialog::accept();
}

/****************************************************************************
 * Tree widget
 ****************************************************************************/

void SelectInputChannel::fillTree()
{
	QLCInputChannel* channel;
	QTreeWidgetItem* uniItem;
	QTreeWidgetItem* chItem;
	QLCInputDevice* device;
	t_input_universe uni;
	InputPatch* patch;

	for (uni = 0; uni < _app->inputMap()->universes(); uni++)
	{
		/* Get the patch associated to the current universe */
		patch = _app->inputMap()->patch(uni);
		Q_ASSERT(patch != NULL);

		/* Make an item for each universe */
		uniItem = new QTreeWidgetItem(m_tree);
		updateUniverseItem(uniItem, uni, patch);

		if (patch->plugin() != NULL && patch->input() != KInputInvalid)
		{
			/* Add a manual option to each patched universe */
			chItem = new QTreeWidgetItem(uniItem);
			updateChannelItem(chItem, uni, NULL);
		}

		/* Add known channels from device (if any) */
		device = patch->device();
		if (device != NULL)
		{
			QMapIterator <t_input_channel, QLCInputChannel*>
				it(device->channels());
			while (it.hasNext() == true)
			{
				channel = it.next().value();
				Q_ASSERT(channel != NULL);

				chItem = new QTreeWidgetItem(uniItem);
				updateChannelItem(chItem, uni, channel);
			}
		}
	}

	/* Listen to item changed signals so that we can catch user's
	   manual input for <...> nodes. Connect AFTER filling the tree
	   so all the initial item->setText()'s won't get caught here. */
	connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void SelectInputChannel::updateChannelItem(QTreeWidgetItem* item,
					   t_input_universe universe,
					   QLCInputChannel* channel)
{
	Q_ASSERT(item != NULL);

	/* Add a manual option to each universe */
	item->setText(KColumnUniverse, QString("%1").arg(universe));
	if (channel == NULL)
	{
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setText(KColumnName,
		   tr("<Double click here to enter channel number manually>"));
		item->setText(KColumnChannel,
			      QString("%1").arg(KInputChannelInvalid));
	}
	else
	{
		item->setText(KColumnName, QString("%1: %2")
						.arg(channel->channel() + 1)
						.arg(channel->name()));
		item->setText(KColumnChannel, QString("%1")
						.arg(channel->channel()));

		/* Display nice icons to indicate channel type */
		if (channel->type() == QLCInputChannel::Slider)
			item->setIcon(KColumnName, QIcon(":/slider.png"));
		else if (channel->type() == QLCInputChannel::Knob)
			item->setIcon(KColumnName, QIcon(":/knob.png"));
		else if (channel->type() == QLCInputChannel::Button)
			item->setIcon(KColumnName, QIcon(":/button.png"));
	}
}

void SelectInputChannel::updateUniverseItem(QTreeWidgetItem* item,
					    t_input_universe universe,
					    InputPatch* patch)
{
	QString name;

	Q_ASSERT(item != NULL);

	if (patch == NULL || patch->plugin() == NULL)
	{
		/* The current universe doesn't have anything assigned to it */
		name = QString("%1: %2").arg(universe + 1).arg(KInputNone);
	}
	else
	{
		/* The current universe has something assigned to it. Check,
		   whether it has an input device. */
		if (patch->device() != NULL)
		{
			name = QString("%1: %2").arg(universe + 1)
						.arg(patch->deviceName());
		}
		else
		{
			name = QString("%1: %2 / %3").arg(universe + 1)
						     .arg(patch->pluginName())
						     .arg(patch->inputName());
		}
	}

	item->setText(KColumnName, name);
	item->setText(KColumnUniverse, QString("%1").arg(universe));
	item->setText(KColumnChannel, QString("%1").arg(KInputChannelInvalid));
}

void SelectInputChannel::slotItemChanged(QTreeWidgetItem* item, int column)
{
	t_input_channel channel;

	Q_ASSERT(item != NULL);
	if (column != KColumnName)
		return;

	/* Extract only numbers from the input data */
	channel = item->text(KColumnName).toInt();

	/* Put the entered channel number also to the channel column */
	item->setText(KColumnChannel, QString("%1").arg(channel - 1));
}
