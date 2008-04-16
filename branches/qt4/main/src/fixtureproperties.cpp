/*
  Q Light Controller
  fixtureproperties.cpp
  
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

#include <QMessageBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "common/qlcfixturedef.h"

#include "fixtureproperties.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

FixtureProperties::FixtureProperties(QWidget* parent, t_fixture_id fixture)
	: QDialog(parent)
{
	setupUi(this);

	m_fixture = fixture;
	Fixture* fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	// Name
	m_nameEdit->setText(fxi->name());

	// Address
	m_addressSpin->setRange(1, 513 - fxi->channels());
	m_addressSpin->setValue(fxi->address() + 1);

	// Universe
	m_universeSpin->setRange(1, KUniverseCount);
	m_universeSpin->setValue(fxi->universe() + 1);
}

FixtureProperties::~FixtureProperties()
{
}

void FixtureProperties::accept()
{
	Fixture* fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	// Name
	fxi->setName(m_nameEdit->text());

	// Address
	fxi->setAddress(m_addressSpin->value() - 1);

	// Universe
	fxi->setUniverse(m_universeSpin->value() - 1);

	QDialog::accept();
}
