/*
  Q Light Controller
  qlcdocbrowser.h

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

#ifndef QLCDOCBROWSER_H
#define QLCDOCBROWSER_H

#include <QWidget>

class QTextBrowser;
class QToolBar;
class QAction;

class QLCDocBrowser : public QWidget
{
	Q_OBJECT

public:
	QLCDocBrowser(QWidget* parent);
	~QLCDocBrowser();
	
	void init();
	void updateButtons();
	
public slots:
	void slotPrevious();
	void slotBackwardAvailable(bool);
	void slotNext();
	void slotForwardAvailable(bool);

protected:
	QToolBar* m_toolbar;
	QAction* m_previousAction;
	QAction* m_nextAction;

	QTextBrowser* m_browser;
};

#endif
