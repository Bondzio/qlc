/*
  Q Light Controller
  functionselection.h

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

#ifndef FUNCTIONSELECTION_H
#define FUNCTIONSELECTION_H

#include <QDialog>
#include <QList>

#include "common/qlctypes.h"
#include "ui_functionselection.h"
#include "function.h"

class QTreeWidgetItem;
class QWidget;

class Fixture;
class Doc;

class FunctionSelection : public QDialog, public Ui_FunctionSelection
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Constructor
	 *
	 * @param parent The parent widget
	 * @param doc The Doc* object holding all fixtures and functions
	 * @param multiple Set true to enable multiple selection
	 * @param disableID A function ID to disable (when adding steps to
	 *                  a chaser, disable the chaser itself)
	 * @param filter Show only functions of the given type (use
	 *               Function::Undefined to show all)
	 */
	FunctionSelection(QWidget* parent, Doc* doc, bool multiple,
			  t_function_id disableFunction = KNoID,
			  Function::Type filter = Function::Undefined);

	/**
	 * Destructor
	 */
	~FunctionSelection();

	/**
	 * List of selected function IDs
	 */
	QList <t_function_id> selection;

	/*********************************************************************
	 * Internal
	 *********************************************************************/
protected:
	/**
	 * Find a top-level item that matches the given fixture instance or
	 * create one if it doesn't exist.
	 *
	 * @param fxi_id The fixture ID to search for
	 * @param doc A QLC Doc* pointer that contains all fixture instances
	 */
	QTreeWidgetItem* fixtureItem(t_fixture_id fxi_id, Doc* doc);

protected slots:
	/**
	 * Item double clicks
	 */
	void slotItemDoubleClicked(QTreeWidgetItem* item);

	/**
	 * OK button click
	 */
	void accept();
};

#endif
