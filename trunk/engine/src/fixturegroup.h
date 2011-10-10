/*
  Q Light Controller
  fixturegroup.h

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

#ifndef FIXTUREGROUP_H
#define FIXTUREGROUP_H

#include <QObject>
#include <QList>
#include <QSize>
#include <QHash>

#include "qlcpoint.h"

#define KXMLQLCFixtureGroup "FixtureGroup"

class QDomDocument;
class QDomElement;
class Doc;

class FixtureGroup : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    FixtureGroup(Doc* parent);
    ~FixtureGroup();

    /** Copy properties from $grp to this */
    void copyFrom(const FixtureGroup* grp);

private:
    Doc* doc() const;

    /************************************************************************
     * ID
     ************************************************************************/
public:
    /** Set a fixture group's id (only Doc is allowed to do this!) */
    void setId(quint32 id);

    /** Get a fixture group's unique id */
    quint32 id() const;

    /** Get an invalid fixture group id */
    static quint32 invalidId();

private:
    quint32 m_id;

    /************************************************************************
     * Information
     ************************************************************************/
public:
    /** Set the name of a fixture group */
    void setName(const QString& name);

    /** Get the name of a fixture group */
    QString name() const;

    /** Get an info text for FixtureManager */
    QString infoText() const;

private:
    QString m_name;

    /************************************************************************
     * Fixtures
     ************************************************************************/
public:
    /**
     * Assign a fixture to a group at the given point. If point is null,
     * then the fixture will be automatically placed to the next free slot.
     * If the fixture is already present in the group, it is moved from its
     * current position to the new position. If another fixture occupies the new
     * point, the two fixtures will simply switch places.
     */
    void assignFixture(quint32 id, const QLCPoint& point = QLCPoint());

    /** Resign a fixture from a group */
    void resignFixture(quint32 id);

    /** Switch places with fixtures at two points a and b. */
    void swap(const QLCPoint& a, const QLCPoint& b);

    /** Get a fixture by its position in the group */
    quint32 fixture(const QLCPoint& pt) const;

    /** Get a list of fixtures assigned to a group */
    QList <quint32> fixtureList() const;

    /** Get the fixture map */
    QHash <QLCPoint,quint32> fixtureHash() const;

private slots:
    /** Listens to Doc fixture removals */
    void slotFixtureRemoved(quint32 id);

private:
    QHash <QLCPoint,quint32> m_fixtures;

    /************************************************************************
     * Size
     ************************************************************************/
public:
    /** Set the group's matrix size */
    void setSize(const QSize& sz);

    /** Get the group's matrix size */
    QSize size() const;

private:
    QSize m_size;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    static bool loader(const QDomElement* root, Doc* doc);
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
};

#endif
