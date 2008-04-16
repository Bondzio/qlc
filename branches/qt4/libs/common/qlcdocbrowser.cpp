/*
  Q Light Controller
  qlcdocbrowser.cpp

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
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QUrl>

#include "qlcdocbrowser.h"

QLCDocBrowser::QLCDocBrowser(QWidget* parent) : QWidget(parent)
{
	init();
}

QLCDocBrowser::~QLCDocBrowser()
{
}

void QLCDocBrowser::init()
{
	setWindowTitle("Q Light Controller - Document Browser");
	resize(600, 600);
	
	new QVBoxLayout(this);

	/* Actions */
	m_previousAction = new QAction(QIcon(PIXMAPS "/back.png"),
				       tr("Previous"), this);
	m_previousAction->setEnabled(false);
	m_nextAction = new QAction(QIcon(PIXMAPS "/forward.png"),
				   tr("Next"), this);
	m_nextAction->setEnabled(false);

	/* Toolbar */
	m_toolbar = new QToolBar("Document Browser", this);
	m_toolbar->addAction(m_previousAction);
	m_toolbar->addAction(m_nextAction);
	
	/* Browser */
	m_browser = new QTextBrowser(this);
	layout()->addWidget(m_browser);

	connect(m_browser, SIGNAL(backwardAvailable(bool)),
		this, SLOT(slotBackwardAvailable(bool)));
	connect(m_browser, SIGNAL(forwardAvailable(bool)),
		this, SLOT(slotForwardAvailable(bool)));

	m_browser->setSource(QUrl(DOCUMENTS "/index.html"));
}

void QLCDocBrowser::slotPrevious()
{
	m_browser->backward();
}

void QLCDocBrowser::slotBackwardAvailable(bool available)
{
	m_previousAction->setEnabled(available);
}

void QLCDocBrowser::slotNext()
{
	m_browser->forward();
}

void QLCDocBrowser::slotForwardAvailable(bool available)
{
	m_nextAction->setEnabled(available);
}
