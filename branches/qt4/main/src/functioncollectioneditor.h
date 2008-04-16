/*
  Q Light Controller
  functioncollectioneditor.h

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

#ifndef FUNCTIONCOLLECTIONEDITOR_H
#define FUNCTIONCOLLECTIONEDITOR_H

#include <QDialog>
#include "ui_functioncollectioneditor.cpp"
#include "common/qlctypes.h"

class FunctionSelection;
class FunctionCollection;

class FunctionCollectionEditor : public QDialog,
				 public Ui_FunctionCollectionEditor
{
	Q_OBJECT

public:
	FunctionCollectionEditor(QWidget* parent, FunctionCollection* fc);
	~FunctionCollectionEditor();

protected slots:
	void slotAdd();
	void slotRemove();
	void accept();

protected:
	bool isAlreadyMember(t_function_id fid);
	void updateFunctionList();

protected:
	FunctionCollection* m_fc;
	FunctionCollection* m_original;
};

#endif

