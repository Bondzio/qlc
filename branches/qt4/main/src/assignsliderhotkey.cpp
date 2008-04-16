/*
  Q Light Controller
  assignhotkey.cpp
  
  Copyright (C) Heikki Junnila
  
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

#include <QTextBrowser>
#include <QKeyEvent>
#include <QLineEdit>

#include "assignsliderhotkey.h"
#include "sliderkeybind.h"

AssignSliderHotKey::AssignSliderHotKey(QWidget* parent) : QDialog(parent)
{
	setupUi(this);
	m_sliderKeyBind = new SliderKeyBind();

	QString str = QString::null;
	str += QString("<HTML><HEAD><TITLE>Assign Slider Keys</TITLE></HEAD><BODY>");
	str += QString("<CENTER><H1>Assign Slider Keys</H1>");
	str += QString("Move the Mouse Cursor over the field, for the");
	str += QString("orientation you want to assign. ");
	str += QString("Hit the key-combination that you want to assign. ");
	str += QString("You may hit either a single key or a key-combination");
	str += QString("with a CTRL, ALT, and/or SHIFT.</CENTER>");
	str += QString("</BODY></HTML>");
	m_infoText->setText(str);
	m_infoText->setFocusPolicy(Qt::NoFocus);
	
	m_previewUpEdit->setReadOnly(true);
	m_previewUpEdit->setAlignment(Qt::AlignCenter);
	
	m_previewDownEdit->setReadOnly(true);
	m_previewDownEdit->setAlignment(Qt::AlignCenter);
}

AssignSliderHotKey::~AssignSliderHotKey()
{
}

void AssignSliderHotKey::keyPressEvent(QKeyEvent* e)
{
	QString str;
		
	if (m_previewUpEdit->hasFocus())
	{
		Q_ASSERT (m_sliderKeyBind != NULL);
		
		m_sliderKeyBind->setKeyUp(e->key());
		m_sliderKeyBind->setModUp(e->modifiers());
		
		str = m_sliderKeyBind->keyStringUp();
		m_previewUpEdit->setText(str);
	}
	else if (m_previewDownEdit->hasFocus())
	{
		Q_ASSERT (m_sliderKeyBind != NULL);
		
		m_sliderKeyBind->setKeyDown(e->key());
		m_sliderKeyBind->setModDown(e->modifiers());
		
		str = m_sliderKeyBind->keyStringDown();
		m_previewDownEdit->setText(str);
	}
}
