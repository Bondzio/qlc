/*
  Q Light Controller
  consolechannel.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Versio 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QToolButton>
#include <iostream>
#include <QSlider>
#include <QLabel>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "common/qlcchannel.h"
#include "common/qlccapability.h"

#include "app.h"
#include "doc.h"
#include "scene.h"
#include "dmxmap.h"
#include "fixture.h"
#include "consolechannel.h"

extern App* _app;

using namespace std;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConsoleChannel::ConsoleChannel(QWidget* parent, t_fixture_id fixtureID,
			       t_channel channel) : QGroupBox(parent)
{
	Q_ASSERT(fixtureID != KNoID);
	m_fixtureID = fixtureID;

	Q_ASSERT(channel != KChannelInvalid);
	m_channel = channel;

	m_value = 0;
	m_menu = NULL;

	init();
	update();
}

ConsoleChannel::~ConsoleChannel()
{
}

void ConsoleChannel::init()
{
	Fixture* fixture;
	QLCChannel* ch;
	QString num;

	this->setCheckable(true);

	new QVBoxLayout(this);
	layout()->setContentsMargins(1, 15, 1, 1);

	m_presetButton = new QToolButton(this);
	layout()->addWidget(m_presetButton);
	m_presetButton->setSizePolicy(QSizePolicy::Preferred,
				      QSizePolicy::Preferred);

	m_valueLabel = new QLabel(this);
	layout()->addWidget(m_valueLabel);
	m_valueLabel->setAlignment(Qt::AlignCenter);

	m_valueSlider = new QSlider(this);
	layout()->addWidget(m_valueSlider);
	m_valueSlider->setInvertedAppearance(true);
	m_valueSlider->setRange(0, 255);
	m_valueSlider->setSizePolicy(QSizePolicy::Preferred,
				     QSizePolicy::Expanding);

	m_numberLabel = new QLabel(this);
	layout()->addWidget(m_numberLabel);
	m_numberLabel->setAlignment(Qt::AlignCenter);

	// Check that we have an actual fixture
	fixture = _app->doc()->fixture(m_fixtureID);
	Q_ASSERT(fixture != NULL);
	
	// Check that the given channel is valid
	Q_ASSERT(m_channel < fixture->channels());
	
	// Generic fixtures don't have channel objects
	ch = fixture->channel(m_channel);
	if (ch != NULL)
		this->setToolTip(ch->name());
	else
		this->setToolTip(tr("Level"));

	// Set channel label
	num.sprintf("%d", m_channel + 1);
	m_numberLabel->setText(num);
	
	connect(m_valueSlider, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChange(int)));

	connect(m_valueSlider, SIGNAL(sliderPressed()),
		this, SLOT(slotSetFocus()));

	initMenu();
}

void ConsoleChannel::slotSetFocus()
{
	t_value value = 0;

	Fixture* fixture = _app->doc()->fixture(m_fixtureID);
	Q_ASSERT(fixture != NULL);

	// In case someone else has set the value for this channel, animate
	// the slider to the correct position
	value = _app->dmxMap()->getValue(fixture->universeAddress() +
					 m_channel);

	slotAnimateValueChange(value);

	// Set focus to this slider
	m_valueSlider->setFocus();
}

/*****************************************************************************
 * Menu
 *****************************************************************************/

void ConsoleChannel::initMenu()
{
	Fixture* fixture;
	QLCChannel* ch;
	
	fixture = _app->doc()->fixture(m_fixtureID);
	Q_ASSERT(fixture != NULL);

	ch = fixture->channel(m_channel);
	Q_ASSERT(ch != NULL);
	
	// Get rid of a possible previous menu
	if (m_menu != NULL)
	{
		delete m_menu;
		m_menu = NULL;
	}
	
	// Create a popup menu and set the channel name as its title
	m_menu = new QMenu(this);
	m_presetButton->setMenu(m_menu);
	m_presetButton->setPopupMode(QToolButton::InstantPopup);
	m_menu->setTitle(ch->name());

	if (ch->group() == KQLCChannelGroupPan)
		m_presetButton->setIcon(QIcon(PIXMAPS "/pan.png"));
	else if (ch->group() == KQLCChannelGroupTilt)
		m_presetButton->setIcon(QIcon(PIXMAPS "/tilt.png"));
	else if (ch->group() == KQLCChannelGroupColour)
		m_presetButton->setIcon(QIcon(PIXMAPS "/color.png"));
	else if (ch->group() == KQLCChannelGroupGobo)
		m_presetButton->setIcon(QIcon(PIXMAPS "/gobo.png"));
	else if (ch->group() == KQLCChannelGroupShutter)
		m_presetButton->setIcon(QIcon(PIXMAPS "/shutter.png"));
	else if (ch->group() == KQLCChannelGroupSpeed)
		m_presetButton->setIcon(QIcon(PIXMAPS "/speed.png"));
	else if (ch->group() == KQLCChannelGroupPrism)
		m_presetButton->setIcon(QIcon(PIXMAPS "/prism.png"));
	else if (ch->group() == KQLCChannelGroupMaintenance)
		m_presetButton->setIcon(QIcon(PIXMAPS "/configure.png"));
	else if (ch->group() == KQLCChannelGroupIntensity)
		m_presetButton->setIcon(QIcon(PIXMAPS "/intensity.png"));
	else if (ch->group() == KQLCChannelGroupBeam)
		m_presetButton->setIcon(QIcon(PIXMAPS "/beam.png"));

	// Initialize the preset menu only for normal fixtures,
	// i.e. not for Generic dimmer fixtures
	if (fixture->fixtureDef() != NULL && fixture->fixtureMode() != NULL)
		initCapabilityMenu(ch);
	else
		initPlainMenu();
	
}

void ConsoleChannel::initPlainMenu()
{
	for (t_value i = 0; i < 255; i++)
	{
		QAction* action;
		QString s;

		s.sprintf("%.3d", i);
		action = m_menu->addAction(s);
		action->setData(i);
	}
}

void ConsoleChannel::initCapabilityMenu(QLCChannel* ch)
{
	QLCCapability* cap;
	QMenu* valueMenu;
	QAction* action;
	QString s;
	QString t;

	QListIterator <QLCCapability*> it(*ch->capabilities());
	while (it.hasNext() == true)
	{
		cap = it.next();

		// Set the value range and name as the menu item's name
		s = QString("%1: %2 - %3").arg(cap->name())
			.arg(cap->min()).arg(cap->max());

		if (cap->max() - cap->min() > 0)
		{
			// Create submenu for ranges of more than one value
			valueMenu = new QMenu(m_menu);
			valueMenu->setTitle(s);

			for (int i = cap->min(); i <= cap->max(); i++)
			{
				action = valueMenu->addAction(
					t.sprintf("%.3d", i));
				action->setData(i);
			}
			
			m_menu->addMenu(valueMenu);
		}
		else
		{
			// Just one value in this range, put that into the menu
			action = m_menu->addAction(s);
			action->setData((int) ((cap->min() + cap->max()) / 2));
		}
	}

	// Connect menu item activation signal to this
	connect(m_menu, SIGNAL(triggered(QAction*)),
		this, SLOT(slotContextMenuTriggered(QAction*)));
	
	// Set the menu also as the preset button's popup menu
	m_presetButton->setMenu(m_menu);
}

void ConsoleChannel::contextMenuEvent(QContextMenuEvent* e)
{
	// Show the preset menu only of it has been created.
	// Generic dimmer fixtures don't have capabilities and so
	// they will not have these menus either.
	if (m_menu != NULL)
	{
		m_menu->exec(e->globalPos());
		e->accept();
	}
}

void ConsoleChannel::slotContextMenuTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);

	// The menuitem's data contains a valid DMX value
	slotAnimateValueChange(action->data().toInt());
}

/*****************************************************************************
 * Value
 *****************************************************************************/

void ConsoleChannel::update()
{
	t_value value = 0;
	
	Fixture* fixture = _app->doc()->fixture(m_fixtureID);
	Q_ASSERT(fixture != NULL);
	
	value = _app->dmxMap()->getValue(fixture->universeAddress() +
					 m_channel);
	
	m_valueLabel->setNum(value);
	slotAnimateValueChange(value);
}

void ConsoleChannel::slotValueChange(int value)
{
	Fixture* fixture = _app->doc()->fixture(m_fixtureID);
	Q_ASSERT(fixture != NULL);
	
	value = KChannelValueMax - value;
	
	_app->dmxMap()->setValue(fixture->universeAddress() + m_channel,
				 (t_value) value);
	
	m_valueLabel->setNum(value);
	
	m_value = value;

	emit valueChanged(m_channel, value);
}

int ConsoleChannel::sliderValue() const
{
	return KChannelValueMax - m_valueSlider->value();
}

void ConsoleChannel::slotAnimateValueChange(t_value value)
{
	m_valueSlider->setValue(static_cast<int> (KChannelValueMax - value));
}
