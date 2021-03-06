/*
  Q Light Controller
  vcframe.cpp
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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

#include "vcframe.h"
#include "vcbutton.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "vcdockslider.h"
#include "floatingedit.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "settings.h"
#include "../../libs/common/minmax.h"
#include "configkeys.h"
#include "vcframeproperties.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <stdio.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qbuttongroup.h>
#include <qobjectlist.h>

#include <assert.h>

extern App* _app;

t_vc_id VCFrame::s_nextVCID = KVCIDMin;

const int KFrameStyle      ( QFrame::StyledPanel | QFrame::Sunken );
const int KColorMask       ( 0xff );

VCFrame::VCFrame(QWidget* parent) 
  : QFrame(parent, "Frame"),
    m_xpos             ( 0 ),
    m_ypos             ( 0 ),
    m_id               ( s_nextVCID++ ),
    m_resizeMode       ( false ),
    m_bottomFrame      ( false ),
    m_buttonBehaviour  ( Normal )
{
}

VCFrame::~VCFrame()
{
}

void VCFrame::init()
{
  setMinimumSize(20, 20);
  
  resize(120, 120);
  setFrameStyle(QFrame::Panel | QFrame::Sunken);

  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

void VCFrame::setBottomFrame(bool set) 
{ 
  m_bottomFrame = set;
  if (set)
    {
      setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    }
  else
    {
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }
}

void VCFrame::setID(t_vc_id newid)
{
  m_id = newid;
  if (newid >= s_nextVCID)
    {
      // Set the biggest id number + 1 to be the next free id
      s_nextVCID = newid + 1;
    }
}

void VCFrame::setButtonBehaviour(ButtonBehaviour b)
{
  m_buttonBehaviour = b;

  if (buttonBehaviour() == VCFrame::Exclusive)
    {
      if (_app->virtualConsole()->selectedWidget())
        {
	  QObjectList* l = _app->virtualConsole()->selectedWidget()->queryList("VCButton");
          QObjectListIt it(*l);
          QObject *obj;
          while ((obj = it.current()) != 0) {
    	    ++it;
	    ((VCButton*)obj)->setExclusive(true);
          }
          delete l;
          setFrameStyle(QFrame::GroupBoxPanel | QFrame::Sunken);
          setLineWidth(2);
	}
      else
        {
	  setFrameStyle(QFrame::GroupBoxPanel | QFrame::Sunken);
          setLineWidth(2);
	}
    }
  else
    {
      if (_app->virtualConsole()->selectedWidget())
        {
	  QObjectList* l = _app->virtualConsole()->selectedWidget()->queryList("VCButton");
          QObjectListIt it(*l);
          QObject *obj;
          while ((obj = it.current()) != 0) {
    	    ++it;
	    ((VCButton*)obj)->setExclusive(false);
          }
          delete l;
      setFrameStyle(KFrameStyle);
      setLineWidth(1);
        }
      else
        {
	  setFrameStyle(KFrameStyle);
	  setLineWidth(1);
	}
    }
}

void VCFrame::saveFramesToFile(QFile& file, t_vc_id parentID)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Frame Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Frame") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + caption() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Parent ID
  if (parentID != 0)
    {
      t.setNum(parentID);
      s = QString("Parent = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Geometry
  if (m_bottomFrame == false)
    {
      // X
      t.setNum(x());
      s = QString("X = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      // Y
      t.setNum(y());
      s = QString("Y = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      // W
      t.setNum(width());
      s = QString("Width = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      // H
      t.setNum(height());
      s = QString("Height = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Palette
  if (ownPalette())
    {
      // Text color
      t.setNum(qRgb(paletteForegroundColor().red(),
		    paletteForegroundColor().green(),
		    paletteForegroundColor().blue()));
      s = QString("Textcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // Background color
      t.setNum(qRgb(paletteBackgroundColor().red(),
		    paletteBackgroundColor().green(),
		    paletteBackgroundColor().blue()));
      s = QString("Backgroundcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Background pixmap
  if (paletteBackgroundPixmap())
    {
      s = QString("Pixmap = " + iconText() + QString("\n"));
      file.writeBlock((const char*) s, s.length());
    }

  // Font
  s = QString("Font = ") + font().toString() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Frame
  if (frameStyle() & KFrameStyle)
    {
      s = QString("Frame = ") + Settings::trueValue() + QString("\n");
    }
  else
    {
      s = QString("Frame = ") + Settings::falseValue() + QString("\n");
    }
  file.writeBlock((const char*) s, s.length());

  // Button Behaviour
  t.setNum(m_buttonBehaviour);
  s = QString("ButtonBehaviour = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  t.setNum(id());
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  if (children() != NULL)
    {
      QObjectList* ol = (QObjectList*) children();
      QObjectListIt it(*ol);
      
      // Child frames
      for (; it.current() != NULL; ++it)
	{
	  if (QString(it.current()->className()) == QString("VCFrame"))
	    {
	      VCFrame* w = (VCFrame*) it.current();
	      w->saveFramesToFile(file, id());
	    }
	}
    }
}

void VCFrame::saveChildrenToFile(QFile& file)
{
  if (children() != NULL)
    {
      QObjectList* ol = (QObjectList*) children();
      QObjectListIt it(*ol);
      
      // Child frames
      while(it.current())
	{
	  if (QString(it.current()->className()) == QString("VCButton"))
	    {
	      ((VCButton*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCLabel"))
	    {
	      ((VCLabel*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className())==QString("VCDockSlider"))
	    {
	      ((VCDockSlider*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCXYPad"))
	    {
	      ((VCXYPad*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCFrame"))
	    {
	      ((VCFrame*) it.current())->saveChildrenToFile(file);
	    }
	  ++it;
	}
    }
}

void VCFrame::createContents(QPtrList <QString> &list)
{
  QRect rect(30, 30, 30, 30);

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("ID"))
	{
	  setID(list.next()->toInt());
	}
      else if (*s == QString("ButtonBehaviour"))
	{
	  setButtonBehaviour(static_cast<ButtonBehaviour>
			     (list.next()->toInt()));
	}
      else if (*s == QString("Parent"))
	{
	  if (m_bottomFrame == false)
	    {
	      VCFrame* parent = 
		_app->virtualConsole()->getFrame(list.next()->toInt());

	      if (parent != NULL)
		{
		  reparent((QWidget*) parent, 0, QPoint(0, 0), true);
		}
	    }
	  else
	    {
	      list.next();
	    }
	}
      else if (*s == QString("Textcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteForegroundColor(qc);
	}
      else if (*s == QString("Backgroundcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteBackgroundColor(qc);
	}
      else if (*s == QString("Color"))
	{
	  // Backwards compatibility for frame background color
	  QString t = *(list.next());
	  int i = t.find(QString(","));
	  int r = t.left(i).toInt();
	  int j = t.find(QString(","), i + 1);
	  int g = t.mid(i+1, j-i-1).toInt();
	  int b = t.mid(j+1).toInt();
	  QColor qc(r, g, b);
	  setPaletteBackgroundColor(qc);
	}
      else if (*s == QString("Pixmap"))
	{
	  QString t;
	  t = *(list.next());
	  
	  QPixmap pm(t);
	  if (pm.isNull() == false)
	    {
	      setIconText(t);
	      setPaletteBackgroundPixmap(pm);
	    }
	}
      else if (*s == QString("Font"))
	{
	  QFont f = font();
	  QString q = *(list.next());
	  f.fromString(q);
	  setFont(f);
	}
      else if (*s == QString("X"))
	{
	  rect.setX(list.next()->toInt());
	}
      else if (*s == QString("Y"))
	{
	  rect.setY(list.next()->toInt());
	}
      else if (*s == QString("Width"))
	{
	  rect.setWidth(list.next()->toInt());
	}
      else if (*s == QString("Height"))
	{
	  rect.setHeight(list.next()->toInt());
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  if (m_bottomFrame == false)
    {
      setGeometry(rect);
    }
}


void VCFrame::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

  QPainter p(this);

  if (_app->mode() == App::Design && 
      _app->virtualConsole()->selectedWidget() == this &&
      m_bottomFrame == false)
    {
      // Draw a dotted line around the widget
      QPen pen(DotLine);
      pen.setWidth(2);
      p.setPen(pen);
      p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

      // Draw a resize handle
      QBrush b(SolidPattern);
      p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
    }
}

void VCFrame::slotModeChanged()
{
  repaint();
}

void VCFrame::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      _app->virtualConsole()->setSelectedWidget(this);

      if (m_resizeMode == true && m_bottomFrame == false)
	{
	  setMouseTracking(false);
	  m_resizeMode = false;
	}
      
      if ((e->button() & LeftButton || e->button() & MidButton)
	  && m_bottomFrame == false)
	{
	  if (e->x() > rect().width() - 10 &&
	      e->y() > rect().height() - 10)
	    {
	      m_resizeMode = true;
	      setMouseTracking(true);
	      setCursor(QCursor(SizeFDiagCursor));
	    }
	  else
	    {
	      m_mousePressPoint = QPoint(e->x(), e->y());
	      setCursor(QCursor(SizeAllCursor));
	    }
	}
      else if (e->button() & RightButton)
	{
	  invokeMenu(mapToGlobal(e->pos()));
	}
    }
  else
    {
      QFrame::mousePressEvent(e);
    }
}

void VCFrame::invokeMenu(QPoint point)
{
	//
	// Add menu
	//
	QPopupMenu* addMenu = new QPopupMenu();
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/button.xpm")), 
		"&Button", KVCMenuAddButton);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/slider.xpm")), 
		"&Slider", KVCMenuAddSlider);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frame.xpm")), 
		"&Frame", KVCMenuAddFrame);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/xypad.xpm")),
		"&XY-Pad", KVCMenuAddXYPad);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/label.xpm")),
		"L&abel", KVCMenuAddLabel);
	
	QPopupMenu* menu = new QPopupMenu();
	// Insert common stuff from virtual console (100% dirty hack)
	menu->insertItem("Edit", _app->virtualConsole()->editMenu());
	
	// Insert the add menu
	menu->insertItem("Add", addMenu);
	
	switch(menu->exec(point))
	{
		case KVCMenuAddButton:
			slotAddButton(mapFromGlobal(point));
		break;
		
		case KVCMenuAddSlider:
			slotAddSlider(mapFromGlobal(point));
		break;
		
		case KVCMenuAddFrame:
			slotAddFrame(mapFromGlobal(point));
		break;
		
		case KVCMenuAddXYPad:
			slotAddXYPad(mapFromGlobal(point));
		break;
		
		case KVCMenuAddLabel:
			slotAddLabel(mapFromGlobal(point));
		break;
		
		default:
			break;
	}
	
	delete addMenu;
	delete menu;
}

void VCFrame::slotAddButton(QPoint p)
{
  VCButton* b = new VCButton(this);
  assert(b);
  b->init();
  b->show();
  
  if (buttonBehaviour() == VCFrame::Exclusive)
    {
      b->setExclusive(true);
    }
  else
    {
      b->setExclusive(false);
    }
  
  b->move(p);
    
  _app->doc()->setModified(true);
}

void VCFrame::slotAddSlider(QPoint p)
{
  VCDockSlider* s = new VCDockSlider(this);
  assert(s);
  s->setBusID(KBusIDDefaultFade);
  s->init();
  s->resize(55, 200);
  s->show();

  s->move(p);
    
  _app->doc()->setModified(true);
}

void VCFrame::slotAddFrame(QPoint p)
{
  VCFrame* f = new VCFrame(this);
  assert(f);
  f->init();
  f->show();

  f->move(p);
    
  _app->doc()->setModified(true);
}

void VCFrame::slotAddXYPad(QPoint p)
{
  VCXYPad* x = new VCXYPad(this);
  assert(x);
  x->init();
  x->show();

  x->move(p);
    
  _app->doc()->setModified(true);
}

void VCFrame::slotAddLabel(QPoint p)
{
  VCLabel* l = new VCLabel(this);
  assert(l);
  l->init();
  l->show();

  l->move(p);
    
  _app->doc()->setModified(true);
}




void VCFrame::parseWidgetMenu(int item)
{
  switch (item)
    {
    case KVCMenuEditProperties:
      {
	VCFrameProperties* vcfp = new VCFrameProperties(this);
	vcfp->init();
	if (vcfp->exec() == QDialog::Accepted)
	  {
	    _app->doc()->setModified(true);
	  }

	delete vcfp;
      }
      break;

    case KVCMenuBackgroundFrame:
      {
	if (frameStyle() & KFrameStyle)
	  {
	    setFrameStyle(NoFrame);
	  }
	else
	  {
	    setFrameStyle(KFrameStyle);
	  }
	_app->doc()->setModified(true);
      }
      break;

    default:
      break;
    }
}

void VCFrame::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      unsetCursor();
      m_resizeMode = false;
      setMouseTracking(false);
    }
  else
    {
      QFrame::mouseReleaseEvent(e);
    }
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      if (m_resizeMode == true)
	{	  
	  QPoint p(QCursor::pos());
	  resizeTo(mapFromGlobal(p));
	  _app->doc()->setModified(true);
	}
      else if (e->state() & LeftButton || e->state() & MidButton)
	{
	  QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
	  p.setX(p.x() - m_mousePressPoint.x());
	  p.setY(p.y() - m_mousePressPoint.y());
		
	  moveTo(p);
	  _app->doc()->setModified(true);
	}
    }
  else
    {
      QFrame::mouseMoveEvent(e);
    }
}

void VCFrame::customEvent(QCustomEvent* e)
{
  if (e->type() == KVCMenuEvent)
    {
      parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
    }
}


void VCFrame::resizeTo(QPoint p)
{
  // Grid settings
  if (_app->virtualConsole()->isGridEnabled())
    {
      p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
      p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
    }

  // Map to parent coordinates so that they can be compared
  p = mapToParent(p);

  // Don't move beyond left or right
  if (p.x() < 0)
    {
      p.setX(0);
    }
  else if (p.x() > parentWidget()->width())
    {
      p.setX(parentWidget()->width());
    }
  
  // Don't move beyond top or bottom
  if (p.y() < 0)
    {
      p.setY(0);
    }
  else if (p.y() > parentWidget()->height())
    {
      p.setY(parentWidget()->height());
    }

  // Map back so that this can be resized
  p = mapFromParent(p);

  // Do the resize
  resize(p.x(), p.y());
}


void VCFrame::moveTo(QPoint p)
{
  // Grid settings
  if (_app->virtualConsole()->isGridEnabled())
    {
      p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
      p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
    }
  
  // Don't move beyond left or right
  if (p.x() < 0)
    {
      p.setX(0);
    }
  else if (p.x() + rect().width() > parentWidget()->width())
    {
      p.setX(parentWidget()->width() - rect().width());
    }
  
  // Don't move beyond top or bottom
  if (p.y() < 0)
    {
      p.setY(0);
    }
  else if (p.y() + rect().height() > parentWidget()->height())
    {
      p.setY(parentWidget()->height() - rect().height());
    }

  // Do the move
  move(p);
}

void VCFrame::mouseDoubleClickEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      invokeMenu(mapToGlobal(e->pos()));
    }
  else
    {
      mousePressEvent(e);
    }
}
