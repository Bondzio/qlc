/*
  Q Light Controller
  main.cpp

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

#include <QtTest>

#include "eplaybackwing_test.h"
#include "eshortcutwing_test.h"
#include "eprogramwing_test.h"
#include "ewing_test.h"

int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);
    int r;

    EWing_Test ewi;
    r = QTest::qExec(&ewi, argc, argv);
    if (r != 0)
        return r;

    EPlaybackWing_Test epl;
    r = QTest::qExec(&epl, argc, argv);
    if (r != 0)
        return r;

    EShortcutWing_Test esh;
    r = QTest::qExec(&esh, argc, argv);
    if (r != 0)
        return r;

    EProgramWing_Test epr;
    r = QTest::qExec(&epr, argc, argv);
    if (r != 0)
        return r;

    return 0;
}
