/*
  Q Light Controller
  main.cpp
  
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

#include <qapplication.h>
#include <qstring.h>
#include <unistd.h>
#include <sys/types.h>
#include <qstylefactory.h>

#include "settings.h"
#include "configkeys.h"
#include "sequencetimer.h"
#include "app.h"

#include <X11/Xlib.h>

App* _app;
QApplication* _qapp;

/*
 * Parse command line arguments
 */
int parseArgs(int argc, char **argv)
{
  int ret = 0;

  if (argc < 2)
    {
      // No args
      ret = 0;
    }
  else
    {
      for (int i = 1; i < argc; i++)
	{
	  if (::strcmp(argv[i], "-h") == 0 || 
	      ::strcmp(argv[i], "--help") == 0)
	    {
	      qDebug("Usage:");
	      qDebug("%s [options]", argv[0]);
	      qDebug("\nOptions:");
	      qDebug("-h or --help      Print this help");
	      qDebug("-v or --version   Print version information");
	      
	      ret = 1;
	    }
	  else if (::strcmp(argv[i], "-v") == 0 ||
		   ::strcmp(argv[i], "--version") == 0)
	    {
	      qDebug(KApplicationVersion);
	      ret = 1;
	    }
	  else
	    {
	      ret = 0;
	    }
	}
    }

  return ret;
}

/*
 * main; entry point for program
 */
int main(int argc, char **argv)
{
  int result;
  uid_t uid;

  //
  // Parse any command line arguments
  if (parseArgs(argc, argv) == 1)
    {
      return 0;
    }

  qDebug("* " + KApplicationNameLong + " " + KApplicationVersion);
  qDebug("* This program is licensed under the terms of the GNU General Public License.");
  qDebug("* Copyright (c) Heikki Junnila (hjunnila@iki.fi)");

  //
  // Warn the user if qlc is being run (effectively) as root
  uid = ::getuid();
  if (uid == 0)
    {
      qDebug("\nDo not run QLC as root. Instead, make the executable suid root:");
      qDebug("    chown root.root qlc ; chmod +s qlc\n");
    }

  //
  // Construct and init sequence timer object
  SequenceTimer* timer = new SequenceTimer();
  timer->init();

  //
  // Initialize QApplication object
  QApplication a(argc, argv);
  _qapp = &a;

  //
  // Construct settings class and load its contents
  Settings* settings;
  settings = new Settings();
  settings->load();

  //
  // Get the widget style from settings
  QString widgetStyle;
  settings->get(KEY_WIDGET_STYLE, widgetStyle);

  //
  // Construct the style thru stylefactory and set it if it's valid
  QStyleFactory f;
  QStyle* style = f.create(widgetStyle);
  if (style != NULL)
    {
      a.setStyle(style);
    }

  //
  // Construct the main application class
  _app = new App(settings);
  _app->setCaption(KApplicationNameLong);
  _app->setSequenceTimer(timer);
  _app->initView();
  a.setMainWidget(_app);
  _app->show();
  
  //
  // Main application loop
  //
  result = a.exec();

  //
  // Delete main app class
  delete _app;

  //
  // Delete settings
  delete settings;

  // Delete sequence timer
  delete timer;

  //
  // Set key repeat on
  Display* display;
  display = XOpenDisplay(NULL);
  ASSERT(display != NULL);
  XAutoRepeatOn(display);
  XCloseDisplay(display);

  return result;
}
