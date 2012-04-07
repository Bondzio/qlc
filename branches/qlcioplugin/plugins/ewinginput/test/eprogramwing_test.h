/*
  Q Light Controller
  testeprogramwing.

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

#ifndef TESTEPROGRAMWING_H
#define TESTEPROGRAMWING_H

#include <QByteArray>
#include <QObject>

class EProgramWing;
class EProgramWing_Test : public QObject
{
    Q_OBJECT

protected:
    QByteArray data();

private slots:
    void initTestCase();

    void firmware();
    void address();
    void isOutputData();
    void name();
    void infoText();
    void tooShortData();

    void buttons_data();
    void buttons();

    void encoders_data();
    void encoders();

    void cleanupTestCase();

private:
    EProgramWing* m_ewing;
};

#endif
