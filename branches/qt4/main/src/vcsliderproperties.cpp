/*
  Q Light Controller
  vcsliderproperties.cpp

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
#include <QInputDialog>
#include <QRadioButton>
#include <QTreeWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "common/qlccapability.h"
#include "common/qlcchannel.h"

#include "vcsliderproperties.h"
#include "vcslider.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnName 0
#define KColumnType 1
#define KColumnRange 2
#define KColumnID 3

VCSliderProperties::VCSliderProperties(QWidget* parent, VCSlider* slider)
	: QDialog(parent)
{
	Q_ASSERT(slider != NULL);
	m_slider = slider;

	setupUi(this);

	/* Generic page */
	m_nameEdit->setText(m_slider->caption());
	m_modeGroup->setButton(m_slider->sliderMode());
	slotSliderModeClicked(static_cast<int> (m_slider->sliderMode()));
	m_valueDisplayStyleGroup->setButton(static_cast<int>
					    (m_slider->valueDisplayStyle()));
	connect(m_modeBusRadio, SIGNAL(clicked()),
		this, SLOT(slotModeBusClicked()));
	connect(m_modeLevelRadio, SIGNAL(clicked()),
		this, SLOT(slotModeLevelClicked()));
	connect(m_modeSubmasterRadio, SIGNAL(clicked()),
		this, SLOT(slotModeSubmasterClicked()));
	connect(m_valueExactRadio, SIGNAL(clicked()),
		this, SLOT(slotValueExactClicked()));
	connect(m_valuePercentageRadio, SIGNAL(clicked()),
		this, SLOT(slotValuePercentageClicked()));

	/* Bus page */
	fillBusCombo();
	m_busLowLimitSpin->setValue(m_slider->busLowLimit());
	m_busHighLimitSpin->setValue(m_slider->busHighLimit());

	/* Level page */
	m_levelLowLimitSpin->setValue(m_slider->levelLowLimit());
	m_levelHighLimitSpin->setValue(m_slider->levelHighLimit());

	QStringList labels;
	labels << "Name" << "Type" << "Range";
	m_levelList.setHeaderLabels(labels);
/*
	m_levelList->setSorting(KColumnRange);
	m_levelList->setRootIsDecorated(true);
	m_levelList->setResizeMode(QListView::LastColumn);
*/
	levelUpdateFixtures();
	levelUpdateChannelSelections();
}

VCSliderProperties::~VCSliderProperties()
{
}

/*****************************************************************************
 * General page
 *****************************************************************************/

void VCSliderProperties::slotModeBusClicked()
{
	m_nameEdit->setText(Bus::name(m_slider->bus()));
	m_nameEdit->setEnabled(false);
	
	m_busValueRangeGroup->setEnabled(true);
	m_busLabel->setEnabled(true);
	m_busCombo->setEnabled(true);
	
	m_levelValueRangeGroup->setEnabled(false);
	m_levelList->setEnabled(false);
	m_levelAllButton->setEnabled(false);
	m_levelNoneButton->setEnabled(false);
	m_levelInvertButton->setEnabled(false);
	m_levelByGroupButton->setEnabled(false);
}

void VCSliderProperties::slotModeLevelClicked()
{
	m_nameEdit->setEnabled(true);
	
	m_busValueRangeGroup->setEnabled(false);
	m_busLabel->setEnabled(false);
	m_busCombo->setEnabled(false);
	
	m_levelValueRangeGroup->setEnabled(true);
	m_levelList->setEnabled(true);
	m_levelAllButton->setEnabled(true);
	m_levelNoneButton->setEnabled(true);
	m_levelInvertButton->setEnabled(true);
	m_levelByGroupButton->setEnabled(true);
}

void VCSliderProperties::slotModeSubmasterClicked()
{
}

void VCSliderProperties::slotValueExactClicked()
{
}

void VCSliderProperties::slotValuePercentageClicked()
{
}

/*****************************************************************************
 * Bus page
 *****************************************************************************/

void VCSliderProperties::fillBusCombo()
{
	QString s;

	m_busCombo->clear();

	for (t_bus_id i = 0; i < KBusCount; i++)
	{
		s.sprintf("%.2d:", i+1);
		s += Bus::name(i);
		m_busCombo->addItem(s);
	}

	m_busCombo->setCurrentIndex(m_slider->bus());
}

void VCSliderProperties::slotBusComboActivated(int item)
{
	m_nameEdit->setText(Bus::name(item));
}

void VCSliderProperties::slotBusLowLimitSpinChanged(int value)
{
	/* Don't allow the low limit to get higher than the high limit */
	if (value >= m_busHighLimitSpin->value())
		m_busHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotBusHighLimitSpinChanged(int value)
{
	/* Don't allow the high limit to get lower than the low limit */
	if (value <= m_busLowLimitSpin->value())
		m_busLowLimitSpin->setValue(value - 1);
}

/*****************************************************************************
 * Level page
 *****************************************************************************/

void VCSliderProperties::levelUpdateFixtures()
{
	t_fixture_id id = 0;
	Fixture* fxi = NULL;

	for (id = 0; id < KFixtureArraySize; id++)
	{
		fxi = _app->doc()->fixture(id);
		if (fxi == NULL)
			continue;

		levelUpdateFixtureNode(id);
	}
}

void VCSliderProperties::levelUpdateFixtureNode(t_fixture_id id)
{
	QTreeWidgetItem* item;
	Fixture* fxi;
	QString str;

	fxi = _app->doc()->fixture(id);
	Q_ASSERT(fxi != NULL);

	item = levelFixtureNode(id);
	if (item == NULL)
	{
		item = new QTreeWidgetItem(m_levelList);
		item->setText(KColumnName, fxi->name());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	}

	item->setText(KColumnType, fxi->type());
	item->setText(KColumnID, str.setNum(id));
	
	levelUpdateChannels(item, fxi);
}

QTreeWidgetItem* VCSliderProperties::levelFixtureNode(t_fixture_id id)
{
	QTreeWidgetItemIterator it(m_levelList);
	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == id)
			return *it;
		++it;
	}

	return NULL;
}

void VCSliderProperties::levelUpdateChannels(QTreeWidgetItem* parent,
					     Fixture* fxi)
{
	t_channel channels = 0;
	t_channel ch = 0;

	Q_ASSERT(parent != NULL);
	Q_ASSERT(fxi != NULL);

	channels = fxi->channels();
	for (ch = 0; ch < channels; ch++)
		levelUpdateChannelNode(parent, fxi, ch);
}

void VCSliderProperties::levelUpdateChannelNode(QTreeWidgetItem* parent,
						Fixture* fxi, t_channel ch)
{
	QTreeWidgetItem* item;
	QLCChannel* channel;
	QString str;

	Q_ASSERT(parent != NULL);

	channel = fxi->channel(ch);
	Q_ASSERT(channel != NULL);

	item = levelChannelNode(parent, ch);
	if (item == NULL)
	{
		item = new QTreeWidgetItem(parent);
		item->setText(KColumnName, channel->name());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	}

	item->setText(KColumnType, channel->group());
	item->setText(KColumnID, str.setNum(ch));

	levelUpdateCapabilities(item, channel);
}

QTreeWidgetItem* VCSliderProperties::levelChannelNode(QTreeWidgetItem* parent,
						      t_channel ch)
{
	Q_ASSERT(parent != NULL);

	QTreeWidgetItemIterator it(parent);
	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == ch)
			return *it;
		++it;
	}

	return NULL;
}

void VCSliderProperties::levelUpdateCapabilities(QTreeWidgetItem* parent,
						 QLCChannel* channel)
{
	Q_ASSERT(parent != NULL);
	Q_ASSERT(channel != NULL);

	QListIterator <QLCCapability*> it(*channel->capabilities());
	while (it.hasNext() == true)
		levelUpdateCapabilityNode(parent, it.next());
}

void VCSliderProperties::levelUpdateCapabilityNode(QTreeWidgetItem* parent,
						   QLCCapability* cap)
{
	QTreeWidgetItem* item;
	QString str;

	Q_ASSERT(parent != NULL);
	Q_ASSERT(cap != NULL);

	item = new QTreeWidgetItem(parent);
	item->setText(KColumnName, cap->name());
	item->setText(KColumnRange, str.sprintf("%.3d - %.3d",
						cap->min(), cap->max()));
}

void VCSliderProperties::levelUpdateChannelSelections()
{
	/* Check all items that are present in the slider's list of
	   controlled channels. We don't need to set other items off, 
	   because this function is run only during init when everything
	   is off. */
	QListIterator <int> it(m_slider->m_levelChannels);
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* fxi_node;
		QTreeWidgetItem* ch_node;

		t_fixture_id fxi_id = KNoID;
		t_channel ch = 0;

		VCSlider::splitCombinedValue(it.next(), &fxi_id, &ch);

		fxi_node = levelFixtureNode(fxi_id);
		Q_ASSERT(fxi_node != NULL);

		ch_node = levelChannelNode(fxi_node, ch);
		Q_ASSERT(ch_node != NULL);

		ch_node->setOn(true);
	}
}

void VCSliderProperties::levelSelectChannelsByGroup(QString group)
{
	QTreeWidgetItem* fxiNode = NULL;
	QTreeWidgetItem* chNode = NULL;

	QTreeWidgetItemIterator fxit(m_levelList);
	while (*fxit != NULL)
	{
		fxiNode = *fxit;
		++fxit;

		QTreeWidgetItemIterator chit(fxiNode);
		while (*chit != NULL)
		{
			chNode = *chit;
			++chit;

			if (chNode->text(KColumnType) == group)
				chNode->setOn(true);
			else
				chNode->setOn(false);
		}
	}
}

void VCSliderProperties::slotLevelLowSpinChanged(int value)
{
	if (value >= m_levelHighLimitSpin->value())
		m_levelHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotLevelHighSpinChanged(int value)
{
	if (value <= m_levelLowLimitSpin->value())
		m_levelLowLimitSpin->setValue(value - 1);
}

void VCSliderProperties::slotLevelCapabilityButtonClicked()
{
	QTreeWidgetItem* item;
	QStringList list;

	item = m_levelList->currentItem();
	if (item == NULL || item->depth() != 2)
		return;

	list = item->text(KColumnRange).split("-");
	Q_ASSERT(list.size() == 2);

	m_levelLowLimitSpin->setValue(list[0].toInt());
	m_levelHighLimitSpin->setValue(list[1].toInt());
}

void VCSliderProperties::slotLevelListClicked(QTreeWidgetItem* item)
{
	if (item == NULL)
		return;

	/* Enable the capability button if a capability has been selected */
	if (item->depth() == 2)
		m_levelCapabilityButton->setEnabled(true);
	else
		m_levelCapabilityButton->setEnabled(false);
}

void VCSliderProperties::slotLevelAllClicked()
{
	/* Set all fixture items selected, their children should get selected
	   as well because the fixture items are Controller items. */
	QTreeWidgetItemIterator it(m_levelList);
	while (it.hasNext() == true)
		it.next()->setOn(true);
}

void VCSliderProperties::slotLevelNoneClicked()
{
	/* Set all fixture items unselected, their children should get unselected
	   as well because the fixture items are Controller items. */
	QTreeWidgetItemIterator it(m_levelList);
	while (it.hasNext() == true)
		it.next()->setOn(false);
}

void VCSliderProperties::slotLevelInvertClicked()
{
	QCheckListItem* fxi_item = NULL;
	QCheckListItem* ch_item = NULL;

	/* Go thru only channel items. Fixture items get (partially) selected 
	   according to their children's state */
	QListViewItemIterator fxit(m_levelList);
	while (*fxit != NULL)
	{
		QListViewItemIterator chit(*fxit);
		while (*chit != NULL)
		{
			*chit->setOn(!*chit->isOn());
			++chit;
		}

		++fxit;
	}
}

void VCSliderProperties::slotLevelByGroupButtonClicked()
{
	bool ok = false;
	QString group;

	group = QInputDialog::getItem("Select channels by group",
				      "Select a channel group",
				      QLCChannel::groupList(), 0,
				      false, &ok, _app);

	if (ok == true)
		levelSelectChannelsByGroup(group);
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void VCSliderProperties::storeLevelChannels()
{
	QCheckListItem* fxi_item = NULL;
	QCheckListItem* ch_item = NULL;
	t_fixture_id fxi_id = KNoID;
	t_channel ch_num = 0;
	
	m_slider->clearLevelChannels();

	for (fxi_item = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     fxi_item != NULL;
	     fxi_item = static_cast<QCheckListItem*> (fxi_item->nextSibling()))
	{
		fxi_id = fxi_item->text(KColumnID).toInt();

		for (ch_item = static_cast<QCheckListItem*> (fxi_item->firstChild());
		     ch_item != NULL;
		     ch_item = static_cast<QCheckListItem*> (ch_item->nextSibling()))
		{
			if (ch_item->isOn() == true)
			{
				ch_num = ch_item->text(KColumnID).toInt();
				m_slider->addLevelChannel(fxi_id, ch_num);
			}
		}
	}
}

void VCSliderProperties::accept()
{
	/* Bus page */
	m_slider->setBus(m_busCombo->currentItem());
	m_slider->setBusLowLimit(m_busLowLimitSpin->value());
	m_slider->setBusHighLimit(m_busHighLimitSpin->value());

	/* Level page */
	m_slider->setLevelLowLimit(m_levelLowLimitSpin->value());
	m_slider->setLevelHighLimit(m_levelHighLimitSpin->value());
	storeLevelChannels();

	/* Set general page stuff last so that name & mode don't get
	   overridden by bus/value/submaster setters */
	m_slider->setCaption(m_nameEdit->text());
	m_slider->setSliderMode(static_cast<VCSlider::SliderMode>
				(m_modeGroup->selectedId()));
	m_slider->setValueDisplayStyle(static_cast<VCSlider::ValueDisplayStyle>
			       (m_valueDisplayStyleGroup->selectedId()));

	UI_VCSliderProperties::accept();
}
