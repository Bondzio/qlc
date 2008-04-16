/*
  Q Light Controller
  dummyinplugin.h
  
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

#ifndef DUMMYINPLUGIN_H
#define DUMMYINPLUGIN_H

#include <QThread>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

class ConfigureDummyInPlugin;
class QWidget;
class QString;

class DummyInPlugin : public QLCInPlugin
{
	Q_OBJECT

	friend class ConfigureDummyInPlugin;

public:
	DummyInPlugin();
	virtual ~DummyInPlugin();
	
	int open();
	int close();

	t_input inputs();
	t_input_channel channels(t_input input);

	int configure(QWidget* parentWidget);

	QString infoText();
};

#endif
