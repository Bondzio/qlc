/*
  Q Light Controller
  virtualconsole.cpp

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

#include <QApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMenuBar>
#include <iostream>
#include <QString>
#include <QPoint>
#include <QMenu>
#include <QFile>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "virtualconsoleproperties.h"
#include "virtualconsole.h"
#include "vcdockslider.h"
#include "vcdockarea.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "keybind.h"
#include "app.h"
#include "doc.h"

#include <X11/Xlib.h>

using namespace std;

extern App* _app;
extern QApplication* _qapp;

VirtualConsole::VirtualConsole(QWidget* parent) : QWidget(parent)
{
	m_dockArea = NULL;
	m_drawArea = NULL;

	m_gridEnabled = true;
	m_gridX = 10;
	m_gridY = 10;
	
	m_keyRepeatOff = true;
	m_grabKeyboard = true;
	
	m_selectedWidget = NULL;
	m_clipboardAction = ClipboardNone;

	m_editMenu = NULL;

	new QHBoxLayout(this);
	layout()->setMargin(0);
	layout()->setSpacing(0);

	parentWidget()->setWindowIcon(QIcon(PIXMAPS "/virtualconsole.png"));
	parentWidget()->setWindowTitle(tr("Virtual Console"));
	parentWidget()->resize(300, 400);

	// Init top menu bar
	initMenuBar();

	// Init left dock area
	initDockArea();

	// Init right drawing area
	setDrawArea(new VCFrame(this));
}

VirtualConsole::~VirtualConsole()
{
}

void VirtualConsole::initMenuBar()
{
	QAction* a;

	layout()->setMenuBar(new QMenuBar(this));

	// Add menu
	m_addMenu = new QMenu(layout()->menuBar());
	m_addMenu->setTitle("Add");
	qobject_cast<QMenuBar*> (layout()->menuBar())->addMenu(m_addMenu);
	m_addMenu->addAction(QIcon(PIXMAPS "/button.png"), "Button",
			     this, SLOT(slotAddButton()));
	m_addMenu->addAction(QIcon(PIXMAPS "/slider.png"), "Slider",
			     this, SLOT(slotAddSlider()));
	m_addMenu->addAction(QIcon(PIXMAPS "/frame.png"), "Frame",
			     this, SLOT(slotAddFrame()));
	m_addMenu->addAction(QIcon(PIXMAPS "/xypad.png"), "XY pad",
			     this, SLOT(slotAddXYPad()));
	m_addMenu->addAction(QIcon(PIXMAPS "/label.png"), "Label",
			     this, SLOT(slotAddLabel()));

	// Tools menu
	m_toolsMenu = new QMenu(layout()->menuBar());
	m_toolsMenu->setTitle("Tools");
	qobject_cast<QMenuBar*> (layout()->menuBar())->addMenu(m_toolsMenu);
	m_toolsMenu->addAction(QIcon(PIXMAPS "/configure.png"), "Settings",
			       this, SLOT(slotToolsSettings()));
	m_toolsMenu->addAction(QIcon(PIXMAPS "/slider.png"), "Default sliders",
			       this, SLOT(slotToolsSliders()));
	m_toolsMenu->addAction(QIcon(PIXMAPS "/panic.png"), "Stop ALL functions!",
			       this, SLOT(slotToolsPanic()));

	// Edit menu
	m_editMenu = new QMenu(layout()->menuBar());
	m_editMenu->setTitle("Edit");
	qobject_cast<QMenuBar*> (layout()->menuBar())->addMenu(m_editMenu);
	a = m_editMenu->addAction(QIcon(PIXMAPS "/editcut.png"), "Cut",
				  this, SLOT(slotEditCut()));
	a->setEnabled(false);
	a = m_editMenu->addAction(QIcon(PIXMAPS "/editcopy.png"), "Copy",
				  this, SLOT(slotEditCopy()));
	a->setEnabled(false);
	a = m_editMenu->addAction(QIcon(PIXMAPS "/editpaste.png"), "Paste",
				  this, SLOT(slotEditPaste()));
	a->setEnabled(false);
	m_editMenu->addAction(QIcon(PIXMAPS "/editdelete.png"), "Delete",
			      this, SLOT(slotEditDelete()));
	m_editMenu->addSeparator();
	m_editMenu->addAction(QIcon(PIXMAPS "/configure.png"), "Properties",
			      this, SLOT(slotEditProperties()));
	m_editMenu->addAction(QIcon(PIXMAPS "/editclear.png"), "Rename",
			      this, SLOT(slotEditRename()));
	m_editMenu->addSeparator();

	// Foreground menu
	QMenu* fgMenu = new QMenu(m_editMenu);
	fgMenu->setTitle("Foreground");
	m_editMenu->addMenu(fgMenu);
	fgMenu->addAction(QIcon(PIXMAPS "/color.png"), "Color",
			  this, SLOT(slotForegroundColor()));
	fgMenu->addAction(QIcon(PIXMAPS "/fonts.png"), "Font",
			  this, SLOT(slotForegroundFont()));
	fgMenu->addAction(QIcon(PIXMAPS "/undo.png"), "Default",
			  this, SLOT(slotForegroundNone()));

	// Background Menu
	QMenu* bgMenu = new QMenu(m_editMenu);
	bgMenu->setTitle("Background");
	m_editMenu->addMenu(bgMenu);
	bgMenu->addAction(QIcon(PIXMAPS "/color.png"), "Color",
			  this, SLOT(slotBackgroundColor()));
	bgMenu->addAction(QIcon(PIXMAPS "/undo.png"), "Default",
			  this, SLOT(slotBackgroundNone()));

	// Stacking order menu
	QMenu* stackMenu = new QMenu(m_editMenu);
	stackMenu->setTitle("Stacking");
	m_editMenu->addMenu(stackMenu);
	stackMenu->addAction(QIcon(PIXMAPS "/up.png"), "Raise",
			     this, SLOT(slotStackingRaise()));
	stackMenu->addAction(QIcon(PIXMAPS "/down.png"), "Lower",
			     this, SLOT(slotStackingLower()));
}

void VirtualConsole::initDockArea()
{
	if (m_dockArea != NULL)
		delete m_dockArea;

	m_dockArea = new VCDockArea(this);
	m_dockArea->setSizePolicy(QSizePolicy::Maximum,
				  QSizePolicy::Expanding);
	
	// Add the dock area into the master horizontal layout
	layout()->addWidget(m_dockArea);
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VirtualConsole::loader(QDomDocument* doc, QDomElement* vc_root)
{
	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	return _app->virtualConsole()->loadXML(doc, vc_root);
}

bool VirtualConsole::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	QString str;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVirtualConsole)
	{
		cout << "Virtual Console node not found!" << endl;
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
		else if (tag.tagName() == KXMLQLCVirtualConsoleGrid)
		{
			str = tag.attribute(KXMLQLCVirtualConsoleGridXResolution);
			setGridX(str.toInt());

			str = tag.attribute(KXMLQLCVirtualConsoleGridYResolution);
			setGridY(str.toInt());

			str = tag.attribute(KXMLQLCVirtualConsoleGridEnabled);
			setGridEnabled((bool) str.toInt());
		}
		else if (tag.tagName() == KXMLQLCVCDockArea)
		{
			m_dockArea->loadXML(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVCFrame)
		{
			VCFrame::loader(doc, &tag, this);
		}
		else
		{
			cout << "Unknown virtual console tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}
		
		node = node.nextSibling();
	}

	parentWidget()->setGeometry(x, y, w, h);
	if (visible == true)
		parentWidget()->showNormal();
	else
		parentWidget()->hide();

	return true;
}

bool VirtualConsole::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Virtual Console entry */
	root = doc->createElement(KXMLQLCVirtualConsole);
	wksp_root->appendChild(root);

	/* Save window state */
	QLCFile::saveXMLWindowState(doc, &root, this);

	/* Grid */
	tag = doc->createElement(KXMLQLCVirtualConsoleGrid);
	root.appendChild(tag);
	str.setNum((m_gridEnabled) ? 1 : 0);
	tag.setAttribute(KXMLQLCVirtualConsoleGridEnabled, str);
	str.setNum(m_gridX);
	tag.setAttribute(KXMLQLCVirtualConsoleGridXResolution, str);
	str.setNum(m_gridY);
	tag.setAttribute(KXMLQLCVirtualConsoleGridYResolution, str);

	/* Keyboard settings */
	tag = doc->createElement(KXMLQLCVirtualConsoleKeyboard);
	str.setNum((m_grabKeyboard) ? 1 : 0);
	tag.setAttribute(KXMLQLCVirtualConsoleKeyboardGrab, str);
	str.setNum((m_keyRepeatOff) ? 1 : 0);
	tag.setAttribute(KXMLQLCVirtualConsoleKeyboardRepeat, str);	

	/* Dock Area */
	m_dockArea->saveXML(doc, &root);

	/* Save children */
	if (m_drawArea != NULL)
		m_drawArea->saveXML(doc, &root);

	return true;
}

/*****************************************************************************
 * Selected widget
 *****************************************************************************/

void VirtualConsole::setSelectedWidget(VCWidget* w)
{
	if (m_selectedWidget != NULL)
	{
		VCWidget* old = m_selectedWidget;
		m_selectedWidget = w;
		old->update();
	}
	else
	{
		m_selectedWidget = w;
	}

	if (m_selectedWidget != NULL)
	{
		m_selectedWidget->update();
	}
	else
	{
		/* Usually the selected widget is NULL only when we have
		   deleted some widget from virtual console. So, clear the
		   clipboard as well so that we don't end up pasting an
		   invalid object -> segfault. */
		clearClipboard();
	}
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

void VirtualConsole::cut(QList <VCWidget*> *widgets)
{
	Q_ASSERT(widgets != NULL);

	/* Clipboard will cut the widgets when paste is invoked */
	m_clipboardAction = ClipboardCut;

	/* Copy the contents of the widget list. Just pointers. */
	m_clipboard = *widgets;
}

void VirtualConsole::copy(QList <VCWidget*> *widgets)
{
	Q_ASSERT(widgets != NULL);

	/* Clipboard will copy the widgets when paste is invoked */
	m_clipboardAction = ClipboardCopy;

	/* Copy the contents of the widget list. Just pointers. */
	m_clipboard = *widgets;
}

void VirtualConsole::paste(VCFrame* parent, QPoint point)
{
	QListIterator <VCWidget*> it(m_clipboard);
	while (it.hasNext() == true)
	{
		VCWidget* widget = it.next();
		if (m_clipboardAction == ClipboardCut)
		{
			widget->setParent(parent);
			widget->move(point);
			widget->show();
		}
		else if (m_clipboardAction == ClipboardCopy)
		{
			copyWidget(widget, parent, point);
		}
		else
		{
			cout << "Cannot paste from an empty clipboard!" << endl;
		}
	}

	/* If the action was about cutting something, the originals are removed
	   by now and moved to another parent. From now on, paste actions just
	   create copies of them, and not move them any further. */
	m_clipboardAction = ClipboardCopy;
}

void VirtualConsole::copyWidget(VCWidget* widget, VCFrame* parent, QPoint point)
{
	/* TODO: 
	   Create a copy of the widget and place it into the parent,
	   at the given point */
}

void VirtualConsole::clearClipboard()
{
	m_clipboard.clear();
	m_clipboardAction = ClipboardNone;
}

/*****************************************************************************
 * Draw area
 *****************************************************************************/

void VirtualConsole::setDrawArea(VCFrame* drawArea)
{
	Q_ASSERT(drawArea != NULL);
	Q_ASSERT(layout() != NULL);

	if (m_drawArea != NULL)
		delete m_drawArea;
	m_drawArea = drawArea;

	/* Add the draw area into the master horizontal layout */
	layout()->addWidget(m_drawArea);
	m_drawArea->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);
}

/*****************************************************************************
 * Add menu callbacks
 *****************************************************************************/

void VirtualConsole::slotAddButton()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddButton();
}

void VirtualConsole::slotAddSlider()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddSlider();
}

void VirtualConsole::slotAddFrame()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddFrame();
}

void VirtualConsole::slotAddXYPad()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddXYPad();
}

void VirtualConsole::slotAddLabel()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddLabel();
}

/*********************************************************************
 * Tools menu callbacks
 *********************************************************************/

void VirtualConsole::slotToolsSettings()
{
	VirtualConsoleProperties prop(this);
	t_bus_value lo = 0;
	t_bus_value hi = 0;

	Q_ASSERT(m_dockArea != NULL);

	prop.setKeyRepeatOff(m_keyRepeatOff);
	prop.setGrabKeyboard(m_grabKeyboard);

	prop.setGrid(m_gridEnabled, m_gridX, m_gridY);

	m_dockArea->defaultFadeSlider()->busRange(lo, hi);
	prop.setFadeLimits(lo, hi);
	m_dockArea->defaultHoldSlider()->busRange(lo, hi);
	prop.setHoldLimits(lo, hi);

	if (prop.exec() == QDialog::Accepted)
	{
		setGridEnabled(prop.isGridEnabled());
		setGridX(prop.gridX());
		setGridY(prop.gridY());
		
		setKeyRepeatOff(prop.isKeyRepeatOff());
		setGrabKeyboard(prop.isGrabKeyboard());

		lo = prop.fadeLowLimit();
		hi = prop.fadeHighLimit();
		m_dockArea->defaultFadeSlider()->setBusRange(lo, hi);

		lo = prop.holdLowLimit();
		hi = prop.holdHighLimit();
		m_dockArea->defaultHoldSlider()->setBusRange(lo, hi);
	}
}

void VirtualConsole::slotToolsSliders()
{
	if (m_dockArea->isHidden())
		m_dockArea->show();
	else
		m_dockArea->hide();

	_app->doc()->setModified();
}

void VirtualConsole::slotToolsPanic()
{
	// Panic button pressed: stop all running functions
	_app->slotControlPanic();
}

/*********************************************************************
 * Edit menu callbacks
 *********************************************************************/

void VirtualConsole::slotEditCut()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VirtualConsole::slotEditCopy()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VirtualConsole::slotEditPaste()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VirtualConsole::slotEditDelete()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotDelete();
}

void VirtualConsole::slotEditProperties()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotProperties();
}

void VirtualConsole::slotEditRename()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotRename();
}

/*********************************************************************
 * Foreground menu callbacks
 *********************************************************************/

void VirtualConsole::slotForegroundFont()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseFont();
}

void VirtualConsole::slotForegroundColor()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseForegroundColor();
}

void VirtualConsole::slotForegroundNone()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotResetForegroundColor();
}

/*********************************************************************
 * Background menu callbacks
 *********************************************************************/

void VirtualConsole::slotBackgroundColor()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseBackgroundColor();
}

void VirtualConsole::slotBackgroundImage()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseBackgroundImage();
}

void VirtualConsole::slotBackgroundNone()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotResetBackgroundColor();
}

/*********************************************************************
 * Stacking menu callbacks
 *********************************************************************/

void VirtualConsole::slotStackingRaise()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->raise();
}

void VirtualConsole::slotStackingLower()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->lower();
}

/*********************************************************************
 * Misc callbacks
 *********************************************************************/

void VirtualConsole::slotModeChanged(App::Mode mode)
{
	QString config;

	/* Key repeat */
	if (isKeyRepeatOff() == true)
	{
		Display* display;
		display = XOpenDisplay(NULL);
		Q_ASSERT(display != NULL);

		if (mode == App::Design)
			XAutoRepeatOn(display);
		else
			XAutoRepeatOff(display);
		
		XCloseDisplay(display);
	}
	
	/* Grab keyboard */
	if (isGrabKeyboard() == true)
	{
		if (mode == App::Design)
			releaseKeyboard();
		else
			grabKeyboard();
	}

	if (mode == App::Operate)
	{
		// Don't allow editing in operate mode
		m_editMenu->setEnabled(false);
		m_addMenu->setEnabled(false);
	}
	else
	{
		// Allow editing in design mode
		m_editMenu->setEnabled(true);
		m_addMenu->setEnabled(true);
	}

	/* Patch the event thru to all children */
	emit modeChanged(mode);
}

/*********************************************************************
 * Event handlers
 *********************************************************************/

void VirtualConsole::closeEvent(QCloseEvent* e)
{
	e->accept();
	emit closed();
}

void VirtualConsole::keyPressEvent(QKeyEvent* e)
{
	if (_app->mode() == App::Operate)
	{
		emit keyPressed(e);
		e->accept();
	}
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* e)
{
	if (_app->mode() == App::Operate)
	{
		emit keyReleased(e);
		e->accept();
	}
}
