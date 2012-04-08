/*
  Q Light Controller
  midiout.h

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

#ifndef MIDIOUT_H
#define MIDIOUT_H

#include <QStringList>
#include <QtPlugin>
#include <QList>

#include <alsa/asoundlib.h>

#include "qlcoutplugin.h"

class ConfigureMIDIOut;
class MIDIDevice;
class QString;

/*****************************************************************************
 * MIDIOut
 *****************************************************************************/

class MIDIOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    friend class ConfigureMIDIOut;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~MIDIOut();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

protected slots:
    /** Listen to HAL device additions/removals */
    void slotDeviceAddedRemoved(const QString& name);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** Open the given output */
    void openOutput(quint32 output);

    /** Close the given output */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe);

protected:
    void subscribeDevice(MIDIDevice* device);
    void unsubscribeDevice(MIDIDevice* device);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    void openInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    void closeInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    QStringList inputs() { return QStringList(); }

    /** @reimp */
    QString inputInfo(quint32 input) { Q_UNUSED(input); return QString(); }

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); }

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /*********************************************************************
     * ALSA
     *********************************************************************/
protected:
    /** Initialize ALSA for proper operation */
    void initALSA();

public:
    /** Get the ALSA sequencer handle */
    snd_seq_t* alsa() {
        return m_alsa;
    }

    /** Get the plugin's own ALSA port that collates all events */
    const snd_seq_addr_t* address() {
        return m_address;
    }

protected:
    /** The plugin's ALSA sequencer interface handle */
    snd_seq_t* m_alsa;

    /** This sequencer client's port address */
    snd_seq_addr_t* m_address;

    /*********************************************************************
     * Devices
     *********************************************************************/
public:
    /** Find out what kinds of MIDI devices there are available */
    void rescanDevices();

protected:
    /** Get a MIDIDevice by its ALSA address */
    MIDIDevice* device(const snd_seq_addr_t* address);

    /** Get a MIDIDevice by its index in the QList */
    MIDIDevice* device(unsigned int index);

    /** Add a new MIDIDevice and associate it with the given output */
    void addDevice(MIDIDevice* device);

    /** Remove the MIDIDevice associated with the output from the QMap */
    void removeDevice(MIDIDevice* device);

signals:
    /** Configuration dialog listens to this signal to refill its list */
    void deviceAdded(MIDIDevice* device);

    /** Configuration dialog listens to this signal to refill its list */
    void deviceRemoved(MIDIDevice* device);

protected:
    /** A map of available MIDI devices */
    QList <MIDIDevice*> m_devices;
};

#endif
