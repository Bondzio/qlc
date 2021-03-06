/*
  Q Light Controller
  outputpatch.cpp

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

#include <algorithm>
#include <QObject>
#include <QtXml>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"
#include "outputpatch.h"
#include "outputmap.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputPatch::OutputPatch(QObject* parent) : QObject(parent)
{
	m_plugin = NULL;
	m_output = -1;
}

OutputPatch::~OutputPatch()
{
	if (m_plugin != NULL)
		m_plugin->close(m_output);
}

/****************************************************************************
 * Plugin & Output
 ****************************************************************************/

void OutputPatch::set(QLCOutPlugin* plugin, int output)
{
	/* TODO: This closes the plugin line always, regardless of whether
	   the line has been assigned to more than one output universe */
	if (m_plugin != NULL)
		m_plugin->close(m_output);

	m_plugin = plugin;
	m_output = output;

	if (m_plugin != NULL)
		m_plugin->open(m_output);
}

QString OutputPatch::pluginName() const
{
	if (m_plugin != NULL)
		return m_plugin->name();
	else
		return KOutputNone;
}

QString OutputPatch::outputName() const
{
	if (m_plugin != NULL && m_output != KOutputInvalid &&
	    m_output < m_plugin->outputs().count())
		return m_plugin->outputs()[m_output];
	else
		return KOutputNone;
}

/*****************************************************************************
 * Value dump
 *****************************************************************************/

void OutputPatch::dump(const char* universe)
{
	/* Don't do anything if there is no plugin and/or output line. */
	if (m_plugin != NULL && m_output != KOutputInvalid)
		m_plugin->writeRange(m_output, 0, (t_value*) universe, 512);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool OutputPatch::saveXML(QDomDocument* doc, QDomElement* map_root,
			  int universe)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(m_plugin != NULL);

	/* Patch entry */
	root = doc->createElement(KXMLQLCOutputPatch);
	map_root->appendChild(root);

	/* Universe */
	str.setNum(universe);
	root.setAttribute(KXMLQLCOutputPatchUniverse, str);

	/* Plugin */
	tag = doc->createElement(KXMLQLCOutputPatchPlugin);
	root.appendChild(tag);
	text = doc->createTextNode(m_plugin->name());
	tag.appendChild(text);

	/* Output */
	tag = doc->createElement(KXMLQLCOutputPatchOutput);
	root.appendChild(tag);
	str.setNum(m_output);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	return true;
}

bool OutputPatch::loader(const QDomElement* root, OutputMap* outputMap)
{
	QDomNode node;
	QDomElement tag;
	QString str;
	QString pluginName;
	int output = 0;
	int universe = 0;

	Q_ASSERT(root != NULL);
	Q_ASSERT(outputMap != NULL);

	if (root->tagName() != KXMLQLCOutputPatch)
	{
		qWarning() << "Patch node not found!";
		return false;
	}

	/* QLC universe that this patch has been made for */
	universe = root->attribute(KXMLQLCOutputPatchUniverse).toInt();

	/* Load patch contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCOutputPatchPlugin)
		{
			/* Plugin name */
			pluginName = tag.text();
		}
		else if (tag.tagName() == KXMLQLCOutputPatchOutput)
		{
			/* Plugin output */
			output = tag.text().toInt();
		}
		else
		{
			qWarning() << "Unknown Patch tag: " << tag.tagName();
		}

		node = node.nextSibling();
	}

	return outputMap->setPatch(universe, pluginName, output);
}
