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
#include "fixture.h"

#define KXMLQLCFixtureGroup "FixtureGroup"

class QDomDocument;
class QDomElement;
class Doc;

/************************************************************************
 * GroupHead
 ************************************************************************/

class GroupHead
{
public:
    GroupHead(quint32 aFxi = Fixture::invalidId(), int aHead = -1);
    GroupHead(const GroupHead& another);
    ~GroupHead();

    bool isValid() const;
    bool operator==(const GroupHead& another) const;

public:
    quint32 fxi;
    int head;
};

/************************************************************************
 * FixtureGroup
 ************************************************************************/

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
    enum DisplayStyle
    {
        DisplayIcon     = 1 << 0, //! Show an icon for each head
        DisplayName     = 1 << 1, //! Show each fixture's name
        DisplayAddress  = 1 << 2, //! Show each fixture's DMX address
        DisplayUniverse = 1 << 3, //! Show each fixture's DMX universe
        DisplayHead     = 1 << 4  //! Show each head number
    };

    /** Set the name of a fixture group */
    void setName(const QString& name);

    /** Get the name of a fixture group */
    QString name() const;

    /** Set fixture display style */
    void setDisplayStyle(int s);

    /** Get fixture display style */
    int displayStyle() const;

    /** Get an info text for FixtureManager */
    QString infoText() const;

private:
    QString m_name;
    int m_displayStyle;

    /************************************************************************
     * Fixtures
     ************************************************************************/
public:
    /**
     * Assign the given fixture $id to a FixtureGroup by placing all of its
     * heads in consecutive order, starting from point $pt.
     *
     * @param id The ID of the fixture to add
     * @param point The point to start from
     */
    void assignFixture(quint32 id, const QLCPoint& pt = QLCPoint());

    /**
     * Resign a fixture, along with all of its heads from a group.
     *
     * @param id Fixture ID to remove
     */
    void resignFixture(quint32 id);

    /**
     * Switch places with fixture heads at two points a and b.
     *
     * @param a First point
     * @param b Second point
     */
    void swap(const QLCPoint& a, const QLCPoint& b);

    /**
     * Get a fixture head by its position in the group. If nothing has been assigned
     * at the given point, returns an invalid GroupHead.
     *
     * @param pt Get the fixture head at this point
     * @return The fixture head at the given point or an invalid head
     */
    GroupHead head(const QLCPoint& pt) const;

    /** Get a list of fixtures assigned to a group */
    QList <GroupHead> headList() const;

    /** Get the fixture head hash */
    QHash <QLCPoint,GroupHead> headHash() const;

    /** Get a list of fixtures assigned to the group */
    QList <quint32> fixtureList() const;

private:
    /**
     * Assign a fixture head to a group at the given point. If point is null,
     * then the fixture will be automatically placed to the next free slot.
     * If the fixture head is already present in the group, it is moved from its
     * current position to the new position. If another fixture head occupies the
     * new point, the two fixture heads will simply switch places.
     *
     * @param pt The point to assign to
     * @param head The fixture head to assign
     */
    void assignHead(const QLCPoint& pt, const GroupHead& head);

private slots:
    /** Listens to Doc fixture removals */
    void slotFixtureRemoved(quint32 id);

private:
    QHash <QLCPoint,GroupHead> m_heads;

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
    static bool loader(const QDomElement& root, Doc* doc);
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
};

#endif
