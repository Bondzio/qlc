/*
  Q Light Controller
  doc.cpp

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

#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "common/qlcfixturedef.h"
#include "common/qlcfixturemode.h"
#include "common/qlcfile.h"

#include "virtualconsole.h"
#include "fixturemanager.h"
#include "collection.h"
#include "outputmap.h"
#include "function.h"
#include "inputmap.h"
#include "fixture.h"
#include "monitor.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "app.h"
#include "doc.h"

extern App* _app;

Doc::Doc() : QObject()
{
	m_fileName = QString::null;

	// Allocate fixture array
	m_fixtureArray = (Fixture**) malloc(sizeof(Fixture*) * 
					    KFixtureArraySize);
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
		m_fixtureArray[i] = NULL;
	m_fixtureAllocation = 0;

	// Allocate function array
	m_functionArray = (Function**) malloc(sizeof(Function*) *
					      KFunctionArraySize);
	for (t_function_id i = 0; i < KFunctionArraySize; i++)
		m_functionArray[i] = NULL;
	m_functionAllocation = 0;

	resetModified();
}

Doc::~Doc()
{
	// Delete all functions
	for (t_function_id i = 0; i < KFunctionArraySize; i++)
	{
		if (m_functionArray[i] != NULL)
		{
			delete m_functionArray[i];
			m_functionArray[i] = NULL;

			emit functionRemoved(i);
		}
	}
	delete [] m_functionArray;
	m_functionAllocation = 0;

	// Delete all fixture instances
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		if (m_fixtureArray[i] != NULL)
		{
			delete m_fixtureArray[i];
			m_fixtureArray[i] = NULL;

			emit fixtureRemoved(i);
		}
	}
	delete [] m_fixtureArray;
	m_fixtureAllocation = 0;
}

void Doc::setModified()
{
	m_modified = true;
	emit modified(true);
}

void Doc::resetModified()
{
	m_modified = false;
	emit modified(false);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Doc::loadXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomDocumentType doctype;
	QString errorString;
	bool retval = false;

	Q_ASSERT(fileName != QString::null);

	if (QLCFile::readXML(fileName, &doc) == true)
	{
		if (doc->doctype().name() == KXMLQLCWorkspace)
		{
			if (loadXML(doc) == false)
			{
				QMessageBox::warning(
					_app, 
					"Unable to open file",
					fileName +
					" is not a valid workspace file");
				retval = false;
			}
			else
			{
				m_fileName = fileName;
				resetModified();
				retval = true;
			}
		}
		else
		{
			QMessageBox::warning(_app, "Unable to open file",
					     fileName + 
					     " is not a workspace file");
			retval = false;
		}
	}
	else
	{
		QMessageBox::warning(_app, "Unable to open file",
				     fileName + 
				     " is not a valid workspace file");
		retval = false;
	}

	return retval;
}

bool Doc::loadXML(const QDomDocument* doc)
{
	QDomElement root;
	QDomNode node;
	QDomElement tag;

	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	Q_ASSERT(doc != NULL);

	root = doc->documentElement();

	/* Load workspace background & theme */
	// _app->workspace()->loadXML(doc, &root);

	if (root.tagName() != KXMLQLCWorkspace)
	{
		qWarning("Workspace node not found in file!");
		return false;
	}

	node = root.firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCCreator)
		{
			/* Ignore creator information */
		}
		else if (tag.tagName() == KXMLQLCOutputMap)
		{
			_app->outputMap()->loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCInputMap)
		{
			_app->inputMap()->loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
		}
		else if (tag.tagName() == KXMLFixture)
		{
			Fixture::loader(&tag);
		}
		else if (tag.tagName() == KXMLQLCFunction)
		{
			Function::loader(&tag);
		}
		else if (tag.tagName() == KXMLQLCBus)
		{
			Bus::loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCMonitor)
		{
			Monitor::loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCVirtualConsole)
		{
			VirtualConsole::loadXML(&tag);
		}
		else
		{
			qDebug() << "Unknown Workspace tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	_app->setGeometry(x, y, w, h);

	return true;
}

bool Doc::saveXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	bool retval = false;

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
		return false;

	if (QLCFile::getXMLHeader(KXMLQLCWorkspace, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* THE MASTER XML ROOT NODE */
		root = doc->documentElement();

		/* Write output mapping */
		_app->outputMap()->saveXML(doc, &root);

		/* Write input mapping */
		_app->inputMap()->saveXML(doc, &root);

		/* Write background image and theme */
		// _app->workspace()->saveXML(doc, &root);

		/* Write window state & size */
		QLCFile::saveXMLWindowState(doc, &root, _app);

		/* Write fixtures into an XML document */
		for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
			if (m_fixtureArray[i] != NULL)
				m_fixtureArray[i]->saveXML(doc, &root);

		/* Write functions into an XML document */
		for (t_function_id i = 0; i < KFunctionArraySize; i++)
			if (m_functionArray[i] != NULL)
				m_functionArray[i]->saveXML(doc, &root);

		/* Write Monitor state */
		Monitor::saveXML(doc, &root);

		/* Write virtual console */
		VirtualConsole::saveXML(doc, &root);

		/* Write buses */
		Bus::saveXML(doc, &root);

		/* Set file name and write the document to the stream */
		m_fileName = fileName;
		stream << doc->toString() << "\n";

		/* Mark this Doc object as unmodified */
		resetModified();

		/* Delete the XML document */
		delete doc;

		retval = true;
	}
	else
	{
		retval = false;
	}

	file.close();

	return retval;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

Fixture* Doc::newFixture(QLCFixtureDef* fixtureDef,
			 QLCFixtureMode* mode,
			 t_channel address,
			 t_channel universe,
			 const QString& name)
{
	Fixture* fxi = NULL;

	if (m_fixtureAllocation == KFixtureArraySize)
		return NULL;

	/* Find the next free slot for a new fixture */
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		if (m_fixtureArray[i] == NULL)
		{
			fxi = new Fixture(fixtureDef, mode, address, universe,
					  name, i);
			Q_ASSERT(fxi != NULL);

			m_fixtureArray[i] = fxi;
			m_fixtureAllocation++;

			setModified();

			// Patch fixture change events thru Doc
			connect(fxi, SIGNAL(changed(t_fixture_id)),
				this, SLOT(slotFixtureChanged(t_fixture_id)));

			emit fixtureAdded(i);
			break;
		}
	}

	return fxi;
}

Fixture* Doc::newGenericFixture(t_channel address,
				t_channel universe,
				t_channel channels,
				const QString& name)
{
	Fixture* fxi = NULL;

	if (m_fixtureAllocation == KFixtureArraySize)
		return NULL;

	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		if (m_fixtureArray[i] == NULL)
		{
			fxi = new Fixture(address, universe, channels, name, i);
			Q_ASSERT(fxi != NULL);

			m_fixtureArray[i] = fxi;
			m_fixtureAllocation++;

			setModified();

			// Patch fixture change events thru Doc
			connect(fxi, SIGNAL(changed(t_fixture_id)),
				this, SLOT(slotFixtureChanged(t_fixture_id)));

			emit fixtureAdded(i);
			break;
		}
	}

	return fxi;
}

bool Doc::newFixture(Fixture* fxi)
{
	t_fixture_id id = 0;

	if (m_fixtureAllocation == KFixtureArraySize)
		return false;

	if (fxi == NULL)
		return false;

	id = fxi->id();

	if (id < 0 || id > KFixtureArraySize)
	{
		qDebug() << QString("Fixture ID %1 out of bounds (%2 - %3)!")
				    .arg(id).arg(0).arg(KFixtureArraySize);
		return false;
	}
	else if (m_fixtureArray[id] != NULL)
	{
		qDebug() << QString("Fixture ID %1 already taken!").arg(id);
		return false;
	}
	else
	{
		m_fixtureArray[id] = fxi;
		m_fixtureAllocation++;

		// Patch fixture change events thru Doc
		connect(fxi, SIGNAL(changed(t_fixture_id)),
			this, SLOT(slotFixtureChanged(t_fixture_id)));

		emit fixtureAdded(id);

		return true;
	}
}

bool Doc::deleteFixture(t_fixture_id id)
{
	if (id < 0 || id > KFixtureArraySize)
		return false;

	if (m_fixtureArray[id] != NULL)
	{
		delete m_fixtureArray[id];
		m_fixtureArray[id] = NULL;
		m_fixtureAllocation--;

		emit fixtureRemoved(id);
		setModified();
		return true;
	}
	else
	{
		qDebug() << QString("No such fixture ID: %1").arg(id);
		return false;
	}
}

Fixture* Doc::fixture(t_fixture_id id)
{
	if (id >= 0 && id < KFixtureArraySize)
		return m_fixtureArray[id];
	else
		return NULL;
}

t_channel Doc::findAddress(t_channel numChannels)
{
	/* Try to find contiguous space from one universe at a time */
	for (int universe = 0; universe < KUniverseCount; universe++)
	{
		t_channel ch = findAddress(universe, numChannels);
		if (ch != KChannelInvalid)
			return ch;
	}

	return KChannelInvalid;
}

t_channel Doc::findAddress(int universe, t_channel numChannels)
{
	t_channel freeSpace = 0;
	t_channel maxChannels = 512;

	/* Construct a map of unallocated channels */
	int map[maxChannels];
	std::fill(map, map + maxChannels, 0);

	/* Go thru all fixtures and mark their address spaces to the map */
	for (t_fixture_id fxi_id = 0; fxi_id < KFixtureArraySize; fxi_id++)
	{
		Fixture* fxi = m_fixtureArray[fxi_id];
		if (fxi == NULL)
			continue;

		if (fxi->universe() != universe)
			continue;

		for (t_channel ch = 0; ch < fxi->channels(); ch++)
			map[fxi->universeAddress() + ch] = 1;
	}

	/* Try to find the next contiguous free address space */
	for (t_channel ch = 0; ch < maxChannels; ch++)
	{
		if (map[ch] == 0)
			freeSpace++;
		else
			freeSpace = 0;

		if (freeSpace == numChannels)
			return (ch - freeSpace + 1) | (universe << 9);
	}

	return KChannelInvalid;
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

Function* Doc::newFunction(Function::Type type)
{
	if (functions() >= KFunctionArraySize)
	{
		qDebug() << "Cannot add more than" << KFunctionArraySize
			 << "functions";
		return NULL;
	}

	Function* function = NULL;

	// Find the next free space from function array
	for (t_function_id id = 0; id < KFunctionArraySize; id++)
	{
		if (m_functionArray[id] == NULL)
		{
			function = createFunction(type);
			Q_ASSERT(function != NULL);
			m_functionArray[id] = function;
			m_functionAllocation++;

			function->setID(id);

			emit functionAdded(id);

			break;
		}
	}

	return function;
}

Function* Doc::newFunction(Function::Type type, t_function_id fid,
			   const QString& name, const QDomElement* root)
{
	Function* function = NULL;

	Q_ASSERT(fid >= 0 && fid < KFunctionArraySize);
	Q_ASSERT(root != NULL);

	/* Put the function to its place (==ID) in the function array */
	if (m_functionArray[fid] == NULL)
	{
		function = createFunction(type);
		Q_ASSERT(function != NULL);
		m_functionArray[fid] = function;
		m_functionAllocation++;

		function->setID(fid);
		function->setName(name);

		/* Continue loading the function contents */
		if (function->loadXML(root) == false)
		{
			delete function;
			function = NULL;
			m_functionArray[fid] = NULL;
		}

		emit functionAdded(fid);
	}
	else
	{
		qDebug() << QString("Function ID %1 already taken.").arg(fid);
	}

	return function;
}

Function* Doc::createFunction(Function::Type type)
{
	Function* function;

	switch (type)
	{
	case Function::Scene:
	        function = new Scene(this);
		break;

	case Function::Chaser:
	        function = new Chaser(this);
		break;

	case Function::Collection:
		function = new Collection(this);
		break;

	case Function::EFX:
		function = new EFX(this);
		break;

	default:
		function = NULL;
		break;
	}

	if (function != NULL)
	{
		/* Listen to fixture removals so that functions can
		   get rid of nonexisting members. */
		connect(this, SIGNAL(fixtureRemoved(t_fixture_id)),
			function, SLOT(slotFixtureRemoved(t_fixture_id)));

		/* Pass function change signals thru */
		connect(function, SIGNAL(changed(t_function_id)),
			this, SLOT(slotFunctionChanged(t_function_id)));
	}

	return function;
}

void Doc::deleteFunction(t_function_id id)
{
	if (m_functionArray[id] != NULL)
	{
		delete m_functionArray[id];
		m_functionArray[id] = NULL;
		m_functionAllocation--;

		emit functionRemoved(id);
	}
}

Function* Doc::function(t_function_id id)
{
	if (id >= 0 && id < KFunctionArraySize)
		return m_functionArray[id];
	else
		return NULL;
}

void Doc::slotFunctionChanged(t_function_id fid)
{
	setModified();
	emit functionChanged(fid);
}

/*****************************************************************************
 * Miscellaneous
 *****************************************************************************/

void Doc::slotModeChanged(App::Mode mode)
{
	Function* function;
	int i;

	if (mode == App::Operate)
	{
		/* Arm all functions, i.e. allocate everything that is
		   needed if the function is run. */
		for (i = 0; i < KFunctionArraySize; i++)
		{
			function = m_functionArray[i];
			if (function != NULL)
				function->arm();
		}
	}
	else
	{
		/* Disarm all functions, i.e. delete everything that was
		   allocated when the functions were armed. */
		for (i = 0; i < KFunctionArraySize; i++)
		{
			function = m_functionArray[i];
			if (function != NULL)
				function->disarm();
		}
	}
}

void Doc::slotFixtureChanged(t_fixture_id id)
{
	setModified();
	emit fixtureChanged(id);
}
