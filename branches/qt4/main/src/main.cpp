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

#include <QApplication>
#include <iostream>
#include <QString>

#include <sys/types.h>
#include <unistd.h>

#include "app.h"
#include "doc.h"

#include <X11/Xlib.h>

using namespace std;

App* _app;
QApplication* _qapp;

void print_version()
{
	cout << endl;
	cout << KApplicationNameLong.toStdString();
	cout << " " << KApplicationVersion.toStdString() << endl;
	cout << "This program is licensed under the terms of GNU GPL." << endl;
	cout << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)" << endl;
	cout << endl;
}

void print_usage()
{
	print_version();

	cout << "Usage:" << endl;
	cout << "  qlc [options]" << endl;
	cout << "\nOptions:" << endl;
	cout << "  -o <file> or --open <file>    Open the specified workspace file" << endl;
	cout << "  -p or --operate               Start in operate mode" << endl;
	cout << "  -h or --help                  Print this help" << endl;
	cout << "  -v or --version               Print version information" << endl;
	cout << endl;
}

/**
 * Parse command line arguments
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 *
 * @return true to continue application init; otherwise false
 */
bool parseArgs(int argc, char **argv)
{
	bool result = true;
	int i = 0;
	QString s;

	for (i = 1; i < argc; i++)
	{
		if (::strcmp(argv[i], "-v") == 0 ||
		    ::strcmp(argv[i], "--version") == 0)
		{
			print_version();
			result = false;
		}
		else if (::strcmp(argv[i], "-h") == 0 ||
			 ::strcmp(argv[i], "--help") == 0)
		{
			print_usage();
			result = false;
		}
		else if (::strcmp(argv[i], "-p") == 0 ||
			 ::strcmp(argv[i], "--operate") == 0)
		{
			_app->slotSetMode(App::Operate);
			result = true;
		}
		else if (::strcmp(argv[i], "-o") == 0 ||
			 ::strcmp(argv[i], "--open") == 0)
		{
			s = QString((const char*) argv[++i]);
			_app->newDocument();
			//_app->doc()->loadWorkspaceAs(s);
			result = true;
		}
		else
		{
			result = true;
		}
	}

	return result;
}


/**
 * THE entry point for the application
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 */
int main(int argc, char **argv)
{
	Display* display;
	int result = 0;

	/* Initialize QApplication object */
	_qapp = new QApplication(argc, argv);

	/* Construct the main application class */
	_app = new App();

	/* Parse command line arguments */
	if (parseArgs(argc, argv) == false)
		return 0;

	_app->show();

	// Main application loop
	result = _qapp->exec();

	// Delete main application
	delete _app;
	delete _qapp;

	// Set key repeat on in case QLC is set to turn it off in operate mode.
	// It's pretty safe to assume that most users have it always turned on.
	display = XOpenDisplay(NULL);
	Q_ASSERT(display != NULL);
	XAutoRepeatOn(display);
	XCloseDisplay(display);

	return result;
}
