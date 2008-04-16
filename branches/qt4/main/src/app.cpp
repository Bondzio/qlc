/*
  Q Light Controller
  app.cpp

  Copyright (c) Heikki Junnila,
                Christopher Staite

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

#include <QMdiSubWindow>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QStatusBar>
#include <QMdiArea>
#include <QMenuBar>
#include <QToolBar>
#include <QToolTip>
#include <iostream>
#include <QAction>
#include <QLabel>
#include <QColor>
#include <QTimer>
#include <QMenu>
#include <QRect>
#include <QFile>
#include <QIcon>

#include "functionconsumer.h"
#include "functionmanager.h"
#include "virtualconsole.h"
#include "fixturemanager.h"
#include "pluginmanager.h"
#include "busproperties.h"
#include "pluginloader.h"
#include "inputmap.h"
#include "aboutbox.h"
#include "monitor.h"
#include "dmxmap.h"
#include "bus.h"
#include "app.h"
#include "doc.h"

#include <X11/Xlib.h>

#include "common/qlcworkspace.h"
#include "common/qlcfile.h"
#include "common/qlcdocbrowser.h"
#include "common/qlcfixturedef.h"

#define KModeTextOperate tr("Operate")
#define KModeTextDesign tr("Design")

extern App* _app;

using namespace std;

/*********************************************************************
 * Initialization
 *********************************************************************/

App::App() : QMainWindow()
{
	m_dmxMap = NULL;
	m_inputMap = NULL;
	m_functionConsumer = NULL;
	m_doc = NULL;

	m_pluginManager = NULL;
	m_functionManager = NULL;
	m_busManager = NULL;
	m_fixtureManager = NULL;
	m_virtualConsole = NULL;
	m_docBrowser = NULL;
	m_monitor = NULL;

	m_mode = Design;
	m_modeIndicator = NULL;

	m_blackoutIndicator = NULL;
	m_blackoutIndicatorTimer = NULL;

	init();
}

App::~App()
{
	// Delete function tree
	if (m_functionManager != NULL)
		delete m_functionManager;
	m_functionManager = NULL;

	// Delete bus properties
	if (m_busManager != NULL)
		delete m_busManager;
	m_busManager = NULL;

	// Delete monitor
	if (m_monitor != NULL)
		delete m_monitor;
	m_monitor = NULL;
	
	// Delete fixture manager view
	if (m_fixtureManager != NULL)
		delete m_fixtureManager;
	m_fixtureManager = NULL;

	// Delete virtual console
	if (m_virtualConsole != NULL)
		delete m_virtualConsole;
	m_virtualConsole = NULL;

	// Delete doc
	if (m_doc != NULL)
		delete m_doc;
	m_doc = NULL;

	// Delete function consumer
	if (m_functionConsumer != NULL)
		delete m_functionConsumer;
	m_functionConsumer = NULL;

	// Delete mode indicator
	if (m_modeIndicator != NULL)
		delete m_modeIndicator;
	m_modeIndicator = NULL;

	// Delete blackout indicator's timer
	if (m_blackoutIndicatorTimer != NULL)
		delete m_blackoutIndicatorTimer;
	m_blackoutIndicatorTimer = NULL;

	// Delete the blackout indicator
	if (m_blackoutIndicator != NULL)
		delete m_blackoutIndicator;
	m_blackoutIndicator = NULL;

	// Delete document browser
	if (m_docBrowser != NULL)
		delete m_docBrowser;
	m_docBrowser = NULL;

	// Delete fixture definitions
	while (m_fixtureDefList.isEmpty() == false)
		delete m_fixtureDefList.takeFirst();

	// Delete output mapper
	if (m_dmxMap != NULL)
		delete m_dmxMap;
	m_dmxMap = NULL;
	
	// Delete input mapper
	if (m_inputMap != NULL)
		delete m_inputMap;
	m_inputMap = NULL;
}


/**
 * Main initialization function
 */
void App::init()
{
	setWindowTitle(KApplicationNameLong);
	setWindowIcon(QIcon(PIXMAPS "/qlc.png"));

	/* MDI Area */
	setCentralWidget(new QMdiArea(this));

	/* Resize the whole application to default size */
	resize(KApplicationDefaultWidth, KApplicationDefaultHeight);

	/* Input & output mappers and their plugins */
	initDMXMap();
	initInputMap();
	PluginLoader::load(QString(PLUGINS), m_dmxMap, m_inputMap);
	m_dmxMap->loadDefaults(KApplicationNameLong);
	m_inputMap->loadDefaults(KApplicationNameLong);

	/* Function running engine */
	initFunctionConsumer();

	/* Buses */
	Bus::init();

	/* Fixture definitions */
	initFixtureDefinitions();

	// The main view
	initStatusBar();
	initMenuBar();
	initToolBar();

	// Document
	initDoc();

	// Virtual Console
	initVirtualConsole();

	// Start up in non-modified state
	m_doc->resetModified();
}

void App::closeEvent(QCloseEvent* e)
{
	int result = 0;

	if (m_mode == Operate)
	{
		QMessageBox::warning(this,
				     "Cannot exit in Operate mode",
				     "You must switch back to Design mode\n" \
				     "to be able to close the application.");
		e->ignore();
		return;
	}

	if (m_doc->isModified())
	{
		result = QMessageBox::information(
			this, 
			"Close Q Light Controller...",
			"Do you wish to save the current workspace \n"	\
			"before closing the application?",
			QMessageBox::Yes,
			QMessageBox::No,
			QMessageBox::Cancel);

		if (result == QMessageBox::Yes)
		{
			slotFileSave();
			e->accept();
		}
		else if (result == QMessageBox::No)
		{
			e->accept();
		}
		else if (result == QMessageBox::Cancel)
		{
			e->ignore();
		}
	}
	else
	{
		e->accept();
	}
}

/*****************************************************************************
 * Output mapping
 *****************************************************************************/

void App::initDMXMap()
{
	m_dmxMap = new DMXMap(KUniverseCount);
	Q_ASSERT(m_dmxMap != NULL);

	connect(m_dmxMap, SIGNAL(blackoutChanged(bool)),
		this, SLOT(slotDMXMapBlackoutChanged(bool)));
}

void App::slotDMXMapBlackoutChanged(bool state)
{
	if (state == true)
	{
		m_blackoutIndicator->setText(tr("Blackout"));

		connect(m_blackoutIndicatorTimer, SIGNAL(timeout()),
			this, SLOT(slotFlashBlackoutIndicator()));
		m_blackoutIndicatorTimer->start(500);
 	}
	else
	{
		m_blackoutIndicator->setText(QString::null);

		m_blackoutIndicatorTimer->stop();
		disconnect(m_blackoutIndicatorTimer, SIGNAL(timeout()),
			   this, SLOT(slotFlashBlackoutIndicator()));

		m_blackoutIndicator->setPalette(QApplication::palette());
	}
}

void App::slotFlashBlackoutIndicator()
{
	QPalette pal;
	QColor bg;
	QColor fg;

	pal = m_blackoutIndicator->palette();
	bg = pal.color(QPalette::Background);
	bg.setRgb(bg.red() ^ 0xff, bg.green() ^ 0xff, bg.blue() ^ 0xff);
	pal.setColor(QPalette::Background, bg);
	
	fg = pal.color(QPalette::Foreground);
	fg.setRgb(fg.red() ^ 0xff, fg.green() ^ 0xff, fg.blue() ^ 0xff);
	pal.setColor(QPalette::Foreground, fg);

	m_blackoutIndicator->setPalette(pal);
}

/*****************************************************************************
 * Input mapping
 *****************************************************************************/

void App::initInputMap()
{
	m_inputMap = new InputMap();
	Q_ASSERT(m_inputMap != NULL);
	m_inputMap->loadDefaults(KApplicationNameLong);
}

/*****************************************************************************
 * Function consumer
 *****************************************************************************/

void App::initFunctionConsumer()
{
	Q_ASSERT(m_dmxMap != NULL);

	m_functionConsumer = new FunctionConsumer(m_dmxMap);
	Q_ASSERT(m_functionConsumer != NULL);

	/* TODO: Put this into some kind of a settings dialog */
	// m_functionConsumer->setTimerType(FunctionConsumer::RTCTimer);
	m_functionConsumer->setTimerType(FunctionConsumer::NanoSleepTimer);
}

/*****************************************************************************
 * Doc
 *****************************************************************************/

void App::initDoc()
{
	// Delete existing document object
	if (m_doc != NULL)
		delete m_doc;
	
	// Create a new document object
	m_doc = new Doc();
	connect(m_doc, SIGNAL(modified(bool)),
		this, SLOT(slotDocModified(bool)));
	
	
	/* Connect fixture list change signals from the new document object */
	if (m_fixtureManager != NULL)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureRemoved(t_fixture_id)));
	}
	
	/* Connect fixture list change signals from the new document object */
	if (m_functionManager != NULL)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_functionManager,
			SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_functionManager,
			SLOT(slotFixtureRemoved(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureChanged(t_fixture_id)),
			m_functionManager,
			SLOT(slotFixtureChanged(t_fixture_id)));
	}
}

void App::slotDocModified(bool state)
{
	QString caption(KApplicationNameLong);

	if (m_doc->fileName() != QString::null)
		caption += QString(" - ") + m_doc->fileName();

	if (state == true)
		setWindowTitle(caption + QString(" *"));
	else
		setWindowTitle(caption);
}

/*****************************************************************************
 * Fixture definitions
 *****************************************************************************/

bool App::initFixtureDefinitions()
{
	QLCFixtureDef* fixtureDef = NULL;
	QString path = QString::null;
	
	QDir dir(FIXTURES, "*.qxf", QDir::Name, QDir::Files);
	if (dir.exists() == false)
	{
		QMessageBox::critical(this, 
				      tr("Unable to load fixture definitions"), 
				      FIXTURES " does not exist!");
		return false;
	}
	else if (dir.isReadable() == false)
	{
		QMessageBox::critical(this,
				      tr("Unable to load fixture definitions"), 
				      FIXTURES " cannot be read by user!");
		return false;
	}
	
	/* Attempt to read all specified files from FIXTURES directory */
	QStringList dirlist(dir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		path = QString(FIXTURES) + QDir::separator() + *it;
		fixtureDef = new QLCFixtureDef(path);

		cout << "Loaded fixture definition for "
		     << fixtureDef->manufacturer().toStdString()
		     << fixtureDef->model().toStdString()
		     << endl;

		if (fixtureDef != NULL)
		{
			m_fixtureDefList.append(fixtureDef);
		}
		else
		{
			cout << "Fixture definition loading failed: "
			     << path.toStdString() << endl;
		}
	}

	return true;
}

QLCFixtureDef* App::fixtureDef(const QString& manufacturer,
			       const QString& model)
{
	QListIterator <QLCFixtureDef*> it(m_fixtureDefList);
	while (it.hasNext() == true)
	{	
		QLCFixtureDef* fd = it.next();
		if (fd->manufacturer() == manufacturer && fd->model() == model)
			return fd;
	}
	
	return NULL;
}

/*****************************************************************************
 * Main application Mode
 *****************************************************************************/

void App::slotSetMode(Mode mode)
{
	/* Nothing to do if we're already in the desired mode */
	if (m_mode == mode)
		return;

	if (mode == Design)
	{
		if (m_functionConsumer->runningFunctions())
		{
			int result = QMessageBox::warning(
				this,
				tr("Switch to Design Mode"),
				tr("There are still running functions.\n"
				   "Really stop them and switch back to "
				   "Design mode?"),
				QMessageBox::Yes,
				QMessageBox::No);

			if (result == QMessageBox::No)
				return;
			else
				m_functionConsumer->purge();
		}

		/* Stop function consumer */
		m_functionConsumer->stop();

		m_modeIndicator->setText(KModeTextDesign);
		m_modeDesignAction->setEnabled(true);
		m_modeOperateAction->setEnabled(false);

		m_fileNewAction->setEnabled(true);
		m_fileOpenAction->setEnabled(true);
		m_fileQuitAction->setEnabled(true);
		m_fixtureManagerAction->setEnabled(true);
		m_functionManagerAction->setEnabled(true);
		m_pluginManagerAction->setEnabled(true);
		m_busManagerAction->setEnabled(true);
	}
	else
	{
		m_modeIndicator->setText(KModeTextOperate);
		m_modeDesignAction->setEnabled(false);
		m_modeOperateAction->setEnabled(true);

		m_fileNewAction->setEnabled(false);
		m_fileOpenAction->setEnabled(false);
		m_fileQuitAction->setEnabled(false);
		m_fixtureManagerAction->setEnabled(false);
		m_functionManagerAction->setEnabled(false);
		m_pluginManagerAction->setEnabled(false);
		m_busManagerAction->setEnabled(false);

		/* Close function manager if it's open */
		if (m_functionManager != NULL)
			m_functionManager->close();

		/* Close fixture manager if it's open */
		if (m_fixtureManager != NULL)
			m_fixtureManager->close();

		/* Close bus manager if it's open */
		if (m_busManager != NULL)
			m_busManager->close();

		/* Start function consumer */
		m_functionConsumer->start();
	}

	m_mode = mode;
	emit modeChanged(m_mode);
}

/*****************************************************************************
 * Virtual Console
 *****************************************************************************/

void App::initVirtualConsole(void)
{
	if (m_virtualConsole != NULL)
		delete m_virtualConsole;

	QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
	m_virtualConsole = new VirtualConsole(sub);
	m_virtualConsole->show();

	sub->setWidget(m_virtualConsole);
	sub->setWindowIcon(QIcon(PIXMAPS "/virtualconsole.png"));

	connect(m_virtualConsole, SIGNAL(closed()),
		this, SLOT(slotVirtualConsoleClosed()));

	sub->resize(300, 300);
	sub->hide();
}

/*****************************************************************************
 * Actions, menubar, toolbar, statusbar
 *****************************************************************************/

void App::initActions()
{
	/* File actions */
	m_fileNewAction = new QAction(QIcon(PIXMAPS "/filenew.png"),
				      tr("New"), this);
	connect(m_fileNewAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileNew()));

	m_fileOpenAction = new QAction(QIcon(PIXMAPS "/fileopen.png"),
				       tr("Open"), this);
	connect(m_fileOpenAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileOpen()));

	m_fileSaveAction = new QAction(QIcon(PIXMAPS "/filesave.png"),
				       tr("Save"), this);
	connect(m_fileSaveAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSave()));

	m_fileSaveAsAction = new QAction(QIcon(PIXMAPS "/filesave.png"),
					 tr("Save As..."), this);
	connect(m_fileSaveAsAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSaveAs()));

	m_fileSaveDefaultsAction = new QAction(QIcon(PIXMAPS "/configure.png"),
					       tr("Save defaults"), this);
	connect(m_fileSaveDefaultsAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSaveDefaults()));

	m_fileQuitAction = new QAction(QIcon(PIXMAPS "exit.png"),
				       tr("Quit"), this);
	connect(m_fileQuitAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileQuit()));

	/* Manager actions */
	m_fixtureManagerAction = new QAction(QIcon(PIXMAPS "/fixture.png"),
					     tr("Fixtures"), this);
	connect(m_fixtureManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFixtureManager()));

	m_functionManagerAction = new QAction(QIcon(PIXMAPS "/function.png"),
					      tr("Functions"), this);
	connect(m_functionManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFunctionManager()));

	m_busManagerAction = new QAction(QIcon(PIXMAPS "/bus.png"),
					 tr("Buses"), this);
	connect(m_busManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBusManager()));

	m_pluginManagerAction = new QAction(QIcon(PIXMAPS "/plugin.png"),
					    tr("Plugins"), this);
	connect(m_pluginManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotPluginManager()));

	/* Mode actions */
	m_modeOperateAction = new QAction(QIcon(PIXMAPS "/operate.png"),
					  tr("Operate"), this);
	connect(m_modeOperateAction, SIGNAL(triggered(bool)),
		this, SLOT(slotModeOperate()));

	m_modeDesignAction = new QAction(QIcon(PIXMAPS "/design.png"),
					 tr("Design"), this);
	connect(m_modeDesignAction, SIGNAL(triggered(bool)),
		this, SLOT(slotModeDesign()));

	/* Control actions */
	m_controlVCAction = new QAction(QIcon(PIXMAPS "/virtualconsole.png"),
					tr("Virtual Console"), this);
	connect(m_controlVCAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlVirtualConsole()));

	m_controlMonitorAction = new QAction(QIcon(PIXMAPS "/monitor.png"),
					     tr("DMX Monitor"), this);
	connect(m_controlMonitorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlMonitor()));

	m_controlBlackoutAction = new QAction(QIcon(PIXMAPS "/blackout.png"),
					      tr("Toggle Blackout"), this);
	connect(m_controlBlackoutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlBlackout()));
	
	m_controlPanicAction = new QAction(QIcon(PIXMAPS "/panic.png"),
					   tr("Panic!"), this);
	connect(m_controlPanicAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlPanic()));

	/* Help actions */
	m_helpIndexAction = new QAction(QIcon(PIXMAPS "/help.png"),
					tr("Index"), this);
	connect(m_helpIndexAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpIndex()));

	m_helpAboutAction = new QAction(QIcon(PIXMAPS "/Q.png"),
					tr("About QLC"), this);
	connect(m_helpAboutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAbout()));

	m_helpAboutQtAction = new QAction(QIcon(PIXMAPS "/qt.png"),
					  tr("About Qt"), this);
	connect(m_helpAboutQtAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAboutQt()));
}

void App::initMenuBar()
{
	/* File Menu */
	m_fileMenu = new QMenu(menuBar());
	m_fileMenu->setTitle(tr("File"));
	menuBar()->addMenu(m_fileMenu);
	m_fileMenu->addAction(m_fileNewAction);
	m_fileMenu->addAction(m_fileOpenAction);
	m_fileMenu->addAction(m_fileSaveAction);
	m_fileMenu->addAction(m_fileSaveAsAction);
	m_fileMenu->addAction(m_fileSaveAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_fileSaveDefaultsAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_fileQuitAction);

	/* Manager Menu */
	m_managerMenu = new QMenu(menuBar());
	m_managerMenu->setTitle(tr("Manager"));
	menuBar()->addMenu(m_managerMenu);
	m_managerMenu->addAction(m_fixtureManagerAction);
	m_managerMenu->addAction(m_functionManagerAction);
	m_managerMenu->addAction(m_busManagerAction);
	m_managerMenu->addSeparator();
	m_managerMenu->addAction(m_pluginManagerAction);

	/* Mode menu */
	m_modeMenu = new QMenu(menuBar());
	m_modeMenu->setTitle(tr("Mode"));
	m_modeMenu->addAction(m_modeDesignAction);
	m_modeMenu->addAction(m_modeOperateAction);

	/* Control Menu */
	m_controlMenu = new QMenu(menuBar());
	m_modeMenu->setTitle(tr("Control"));
	menuBar()->addMenu(m_controlMenu);
	m_controlMenu->addMenu(m_modeMenu);
	m_controlMenu->addSeparator();
	m_controlMenu->addAction(m_controlVCAction);
	m_controlMenu->addAction(m_controlMonitorAction);
	m_controlMenu->addSeparator();
	m_controlMenu->addAction(m_controlPanicAction);
	m_controlMenu->addAction(m_controlBlackoutAction);

	menuBar()->addSeparator();

	/* Help menu */
	m_helpMenu = new QMenu(menuBar());
	m_helpMenu->setTitle(tr("Help"));
	menuBar()->addMenu(m_helpMenu);
	m_helpMenu->addAction(m_helpIndexAction);
	m_helpMenu->addAction(m_helpAboutAction);
	m_helpMenu->addAction(m_helpAboutQtAction);
}

void App::initStatusBar()
{
	/* Mode Indicator */
	m_modeIndicator = new QLabel(statusBar());
	m_modeIndicator->setText(KModeTextDesign);
	statusBar()->addWidget(m_modeIndicator, 0);

	/* Blackout Indicator */
	m_blackoutIndicatorTimer = new QTimer(this);
	m_blackoutIndicator = new QLabel(statusBar());
	statusBar()->addWidget(m_blackoutIndicator, 0);
}

void App::initToolBar()
{
	m_toolbar = new QToolBar(tr("Workspace"), this);
	m_toolbar->addAction(m_fileNewAction);
	m_toolbar->addAction(m_fileOpenAction);
	m_toolbar->addAction(m_fileSaveAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_fixtureManagerAction);
	m_toolbar->addAction(m_functionManagerAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_controlVCAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_controlMonitorAction);
	m_toolbar->addAction(m_controlBlackoutAction);
	m_toolbar->addAction(m_modeDesignAction);
	m_toolbar->addAction(m_modeOperateAction);
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

bool App::slotFileNew()
{
	bool result = false;

	if (doc()->isModified())
	{
		QString msg;
		msg = "Do you wish to save the current workspace?\n";
		msg += "Changes will be lost if you don't save them.";
		int result = QMessageBox::warning(this, "New Workspace", msg,
						  QMessageBox::Yes,
						  QMessageBox::No,
						  QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			slotFileSave();
			newDocument();
			result = true;
		}
		else if (result == QMessageBox::No)
		{
			newDocument();
			result = true;
		}
		else
		{
			result = false;
		}
	}
	else
	{
		newDocument();
		result = true;
	}

	return result;
}

void App::newDocument()
{
	setWindowTitle(KApplicationNameLong);

	initDoc();
	initVirtualConsole();
	doc()->resetModified();
}

void App::slotFileOpen()
{
	QString fileName;

	/* Check that the user is aware of losing previous changes */
	if (doc()->isModified())
	{
		QString msg;
		msg = "Do you wish to save the current workspace?\n";
		msg += "Changes will be lost if you don't save them.";
		int result = QMessageBox::warning(this, "Open Workspace", msg,
						  QMessageBox::Yes,
						  QMessageBox::No,
						  QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			/* Save first */
			slotFileSave();
		}
		else if (result == QMessageBox::No)
		{
			/* Nah, who cares? */
		}
		else
		{
			/* Whoops, go back! */
			return;
		}
	}

	fileName = QFileDialog::getOpenFileName(this, m_doc->fileName(), 
						QString("*") + KExtWorkspace);
	if (fileName.isEmpty() == false)
	{
		newDocument();
		
		if (doc()->loadXML(fileName) == false)
		{
			QMessageBox::critical(this,
					      "Unable to open file!",
					      "The file seems to be corrupt.");
		}
	}

	doc()->resetModified();
}

void App::slotFileSave()
{
	if (m_doc->fileName().isEmpty())
	{
		slotFileSaveAs();
	}
	else
	{
		if (m_doc->saveXML(m_doc->fileName()) == true)
		{
			setWindowTitle(KApplicationNameLong + " - " +
				       doc()->fileName());
		}
	}
}

void App::slotFileSaveAs()
{
	QString fileName;

	fileName = QFileDialog::getSaveFileName(this, m_doc->fileName(),
						QString("*") + KExtWorkspace);
	if (fileName.isEmpty() == false)
	{
		// Use the suffix ".qxw" always
		if (fileName.right(4) != KExtWorkspace)
		{
			fileName += KExtWorkspace;
		}

		if (m_doc->saveXML(fileName) == true)
		{
			setWindowTitle(KApplicationNameLong + QString(" - ") +
				       doc()->fileName());
		}
	}
}

void App::slotFileSaveDefaults()
{
	int r = 0;

	r = QMessageBox::question(this, "Save Defaults?",
				  QString("Do you wish to save defaults for:\n")
				  + QString("* Workspace background & theme\n")
				  + QString("* Universe mapping"),
				  QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		m_dmxMap->saveDefaults(KApplicationNameLong);
	}
}

void App::slotFileQuit()
{
	close();
}

/*****************************************************************************
 * Manager action slots
 *****************************************************************************/

void App::slotPluginManager()
{
	if (m_pluginManager == NULL)
	{
		QMdiSubWindow* sub;

		sub = new QMdiSubWindow(centralWidget());
		m_pluginManager = new PluginManager(sub);

		sub->setWidget(m_pluginManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(PIXMAPS "/plugin.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		m_pluginManager->show();
		sub->resize(700, 400);
		sub->show();

		connect(m_pluginManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotPluginManagerDestroyed(QObject*)));
	}
}

void App::slotPluginManagerDestroyed(QObject* object)
{
	Q_ASSERT(object == m_pluginManager);
	m_pluginManager = NULL;
}

void App::slotBusManager()
{
	if (m_busManager == NULL)
	{
		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_busManager = new BusProperties(sub);

		sub->setWidget(m_busManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(PIXMAPS "/bus.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_busManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotBusManagerDestroyed(QObject*)));

		m_busManager->show();
		sub->show();
	}
}

void App::slotBusManagerDestroyed(QObject* object)
{
	Q_ASSERT(object == m_busManager);
	m_busManager = NULL;
}

void App::slotFixtureManager()
{
	if (m_fixtureManager == NULL)
	{
		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_fixtureManager = new FixtureManager(sub);

		sub->setWidget(m_fixtureManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(PIXMAPS "/fixture.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_fixtureManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotFixtureManagerDestroyed(QObject*)));
		
		connect(m_doc,
			SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc,
			SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureRemoved(t_fixture_id)));

		m_fixtureManager->show();
		sub->show();
	}
}

void App::slotFixtureManagerDestroyed(QObject* object)
{
	Q_ASSERT(m_fixtureManager == object);
	m_fixtureManager = NULL;
}

void App::slotFunctionManager()
{
	if (m_functionManager == NULL)
	{
		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_functionManager = new FunctionManager(sub);

		sub->setWidget(m_functionManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(PIXMAPS "/function.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_functionManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotFunctionManagerDestroyed(QObject*)));

		connect(m_functionManager, SIGNAL(closed()),
			this, SLOT(slotFunctionManagerClosed()));

		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_functionManager,
			SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_functionManager,
			SLOT(slotFixtureRemoved(t_fixture_id)));

		connect(m_doc, SIGNAL(fixtureChanged(t_fixture_id)),
			m_functionManager,
			SLOT(slotFixtureChanged(t_fixture_id)));

		connect(m_doc, SIGNAL(functionAdded(t_function_id)),
			m_functionManager,
			SLOT(slotFunctionAdded(t_function_id)));

		connect(m_doc, SIGNAL(functionRemoved(t_function_id)),
			m_functionManager,
			SLOT(slotFunctionRemoved(t_function_id)));

		connect(m_doc, SIGNAL(functionChanged(t_function_id)),
			m_functionManager,
			SLOT(slotFunctionChanged(t_function_id)));
	}
}

void App::slotFunctionManagerDestroyed(QObject* object)
{
	Q_ASSERT(m_functionManager == object);
	m_functionManager = NULL;
}

/*****************************************************************************
 * Control action slots
 *****************************************************************************/

void App::slotControlBlackout()
{
	Q_ASSERT(m_dmxMap != NULL);

	if (m_dmxMap->blackout() == true)
		m_dmxMap->setBlackout(false);
	else
		m_dmxMap->setBlackout(true);
}

void App::slotControlVirtualConsole()
{
	Q_ASSERT(m_virtualConsole != NULL);

	m_virtualConsole->parentWidget()->show();
	m_virtualConsole->setFocus();
}

void App::slotVirtualConsoleClosed()
{
}

void App::slotControlMonitor()
{
	if (m_monitor == NULL)
	{
		Q_ASSERT(m_dmxMap != NULL);

		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_monitor = new Monitor(sub, m_dmxMap);

		sub->setWidget(m_monitor);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(PIXMAPS "/monitor.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_monitor, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotMonitorDestroyed(QObject*)));
		
		m_monitor->show();
		m_monitor->setFocus();
		sub->show();
	}
}

void App::slotMonitorDestroyed(QObject* object)
{
	Q_ASSERT(m_monitor == object);
	m_monitor = NULL;
}

void App::slotControlPanic()
{
	/* Shut down all running functions */
	m_functionConsumer->purge();
}

/*****************************************************************************
 * Help action slots
 *****************************************************************************/

void App::slotHelpIndex()
{
	if (m_docBrowser == NULL)
	{
		m_docBrowser = new QLCDocBrowser(this);
		m_docBrowser->setAttribute(Qt::WA_DeleteOnClose);

		connect(m_docBrowser, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotDocBrowserDestroyed(QObject*)));

		m_docBrowser->show();
	}
}

void App::slotDocBrowserDestroyed(QObject* object)
{
	Q_ASSERT(m_docBrowser == object);
	m_docBrowser = NULL;
}

void App::slotHelpAbout()
{
	AboutBox ab(this);
	ab.exec();
}

void App::slotHelpAboutQt()
{
	QMessageBox::aboutQt(this, QString("Q Light Controller"));
}

