/*
  Q Light Controller
  addfixture.h

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

#ifndef ADDFIXTURE_H
#define ADDFIXTURE_H

#include <QWidget>
#include <QList>

#include "ui_addfixture.h"
#include "fixture.h"

class QLCFixtureDefCache;
class QTreeWidgetItem;
class QLCFixtureMode;
class QLCFixtureDef;
class OutputMap;
class QString;
class Doc;

class AddFixture : public QDialog, public Ui_AddFixture
{
    Q_OBJECT
    Q_DISABLE_COPY(AddFixture)

public:
    /**
     * Create a new dialog for fixture definition selection.
     * If selectManufacturer and selectModel parameters are omitted, the
     * dialog selects "Generic Dimmer".
     *
     * @param parent The parent object that owns the dialog
     * @param fixtureDefCache A cache that holds available fixture defs
     * @param doc QLC Doc used to resolve a free address automatically
     * @param fxi Fixture to edit (optional)
     */
    AddFixture(QWidget* parent, const QLCFixtureDefCache& fixtureDefCache,
               const Doc* doc, const OutputMap* outputMap, const Fixture* fxi = NULL);

    /** Destructor */
    ~AddFixture();

    /*********************************************************************
     * Value getters
     *********************************************************************/
public:
    /** Get the selected QLCFixtureDef */
    const QLCFixtureDef* fixtureDef() const;

    /** Get the selected QLCFixtureMode */
    const QLCFixtureMode* mode() const;

    /** Get the assigned friendly name */
    QString name() const;

    /** Get the assigned DMX address */
    quint32 address() const;

    /** Get the assigned DMX universe */
    quint32 universe() const;

    /** Get the number of similar fixtures to add */
    int amount() const;

    /** Get the number of channels to leave between two fixtures */
    quint32 gap() const;

    /** Get the number of channels to use (ONLY for generic dimmers) */
    quint32 channels() const;

protected:
    const QLCFixtureDefCache& m_fixtureDefCache;
    const Doc* m_doc;
    const OutputMap* m_outputMap;

    const QLCFixtureDef* m_fixtureDef;
    const QLCFixtureMode* m_mode;

    QString m_nameValue;

    quint32 m_addressValue;
    quint32 m_universeValue;
    int m_amountValue;
    quint32 m_gapValue;
    quint32 m_channelsValue;

    /*********************************************************************
     * Fillers
     *********************************************************************/
protected:
    /** Fill all known fixture definitions to the tree view. Select the
        given model from the given manufacturer. */
    void fillTree(const QString& selectManufacturer,
                  const QString& selectModel);

    /** Fill all modes of the current fixture to the mode combo */
    void fillModeCombo(const QString& text = QString());

    /** Find the next free address space for current fixture selection,
        amount and address gap. Sets the address to address spin. */
    void findAddress();

    /**
     * Attempt to find the next contiguous free address space for the given
     * number of channels. The address will not span multiple universes.
     * If a suitable address space cannot be found, QLCChannel::invalid() is
     * returned
     *
     * @param numChannels Number of channels in the address space
     * @return The address or QLCChannel::invalid() if not found
     */
    static quint32 findAddress(quint32 numChannels, QList <Fixture*> fixtures,
                               quint32 maxUniverses);

    /**
     * Try to find the next free address from the given universe for
     * the given number of channels. QLCChannel::invalid() is returned if
     * an adequate address range cannot be found.
     *
     * @param universe The universe to search from
     * @param numChannels Number of free channels required
     * @return An address or QLCChannel::invalid() if address space not available
     */
    static quint32 findAddress(quint32 universe, quint32 numChannels,
                               QList <Fixture*> fixtures);

    /** Update the maximum amount of fixtures for the universe */
    void updateMaximumAmount();

    /*********************************************************************
     * Slots
     *********************************************************************/
protected slots:
    /** Callback for mode selection changes */
    void slotModeActivated(const QString& modeName);

    /** Callback for universe combo activations */
    void slotUniverseActivated(int universe);

    /** Callback for address spin changes */
    void slotAddressChanged(int value);

    /** Callback for channels spin value changes */
    void slotChannelsChanged(int value);

    /** Callback for tree view selection changes */
    void slotSelectionChanged();

    /** Callback for tree double clicks (same as select + OK) */
    void slotTreeDoubleClicked(QTreeWidgetItem* item);

    /** Callback for friendly name editing */
    void slotNameEdited(const QString &text);

    /** Callback for fixture amount value changes */
    void slotAmountSpinChanged(int value);

    /** Callback for address gap value changes */
    void slotGapSpinChanged(int value);
};

#endif
