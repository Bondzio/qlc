/*
  Q Light Controller
  fixtureconsole.cpp

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

#include <QHBoxLayout>
#include <iostream>
#include <QIcon>
#include <QtXml>

#include "common/qlcfile.h"

#include "fixtureconsole.h"
#include "consolechannel.h"
#include "sceneeditor.h"
#include "app.h"
#include "doc.h"

extern App* _app;

using namespace std;

FixtureConsole::FixtureConsole(QWidget *parent) : QWidget(parent)
{
	m_sceneEditor = NULL;
}

FixtureConsole::~FixtureConsole()
{
	while (m_unitList.isEmpty() == false)
		delete m_unitList.takeFirst();
	
	if (m_sceneEditor != NULL)
		delete m_sceneEditor;
	m_sceneEditor = NULL;
}

void FixtureConsole::setFixture(t_fixture_id id)
{
	ConsoleChannel* unit = NULL;
	Fixture* fxi = NULL;
	
	m_fixture = id;

	fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	// Set an icon -- TODO: this is done in App now...?
	setWindowIcon(QIcon(PIXMAPS "/console.png"));

	// Set the main horizontal layout
	new QHBoxLayout(this);

	// Create scene editor widget
	if (m_sceneEditor != NULL)
		delete m_sceneEditor;
	m_sceneEditor = new SceneEditor(this);
	layout()->addWidget(m_sceneEditor);
	m_sceneEditor->setFixture(m_fixture);
	m_sceneEditor->show();

	// Catch function add signals
	connect(_app->doc(), SIGNAL(functionAdded(t_function_id)),
		m_sceneEditor, SLOT(slotFunctionAdded(t_function_id)));

	// Catch function remove signals
	connect(_app->doc(), SIGNAL(functionRemoved(t_function_id)),
		m_sceneEditor, SLOT(slotFunctionRemoved(t_function_id)));

	// Catch function change signals
	connect(_app->doc(), SIGNAL(functionChanged(t_function_id)),
		m_sceneEditor, SLOT(slotFunctionChanged(t_function_id)));

	// Create channel units
	for (unsigned int i = 0; i < fxi->channels(); i++)
	{
		unit = new ConsoleChannel(this, m_fixture, i);
		layout()->addWidget(unit);
		unit->update();

		// Channel updates to scene editor
		connect(unit, 
			SIGNAL(changed(t_channel, t_value, Scene::ValueType)),
			m_sceneEditor,
			SLOT(slotChannelChanged(t_channel, t_value, Scene::ValueType)));

		// Scene editor updates to channels
		connect(m_sceneEditor,
			SIGNAL(sceneActivated(SceneValue*, t_channel)),
			unit, 
			SLOT(slotSceneActivated(SceneValue*, t_channel)));

		m_unitList.append(unit);
	}

	/* Resize the console to some sensible proportions if at least
	   one channel unit was inserted */
	if (unit != NULL)
		resize(m_sceneEditor->width() + 
		       (fxi->channels() * unit->width()), 250);

	// Update scene editor (also causes an update to channelunits)
	m_sceneEditor->update();
}

bool FixtureConsole::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	bool visible = false;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFixtureConsole)
	{
		cout << "Fixture console node not found!" << endl;
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
		}
		else
		{
			cout << "Unknown fixture console tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}
		
		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

	return true;
}

bool FixtureConsole::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Console entry */
	root = doc->createElement(KXMLQLCFixtureConsole);
	fxi_root->appendChild(root);

	/* Save window state */
	return QLCFile::saveXMLWindowState(doc, &root, parentWidget());
}
