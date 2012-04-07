/*
  Q Light Controller
  qlcioplugin.h

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

#ifndef QLCIOPLUGIN_H
#define QLCIOPLUGIN_H

#include <QtPlugin>
#include <QObject>
#include <climits>
#include <QSet>

#include "qlcoutputline.h"
#include "qlcinputline.h"

/**
 * QLCIOPlugin is an interface for all input & output plugins. Plugins provide
 * the means for QLC to actually send DMX data to fixtures as well as receive
 * input data from various input devices. Each plugin can be understood as an
 * adaptation layer betwen QLC and an interconnecting protocol used by a certain
 * family of I/O hardware.
 *
 * When QLC has successfully loaded an I/O plugin, it will call init()
 * exactly once for the plugin. Hardware device additions/removals must not
 * be monitored by plugins but instead they must rely on QLC's common hot plug
 * monitor notifications thru usbDeviceAdded() and usbDeviceRemoved() methods.
 *
 * Plugins and their I/O lines must not use any resources unless open()
 * has been called for a particular I/O line. And even then, the plugin should
 * open only such resources that are needed for that specific I/O line given.
 * Respectively, when close() is called, the plugin should relinquish all resources
 * associated to the closed I/O line (unless shared with other I/O lines).
 */
class QLCIOPlugin : public QObject
{
    Q_OBJECT

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /**
     * De-initialize the plugin. This is the last thing that is called
     * for the plugin so make sure nothing is lingering in the twilight
     * after this call. The default implementation does nothing but needs
     * to be in-place for C++ sake.
     *
     * All plugins must implement their own destructors.
     */
    virtual ~QLCIOPlugin() { /* NOP */ }

    /**
     * Initialize the plugin. Since Qt plugins cannot have a user-defined
     * constructor, any initialization prior to opening any HW must be
     * done thru this second-stage initialization method. QLC calls
     * this method for all plugins exactly once after loading, before
     * calling any other method provided by the plugin.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual void init() = 0;

    /**
     * Get the plugin's unique identifier that is used internally by QLC to find
     * a particular plugin. The UID must not change over time.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual qulonglong uid() = 0;

    /**
     * Get the plugin's name. The name is shown to the user so make it friendly.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual QString name() = 0;

    /**
     * Get the plugin's description (informational text shown to the user).
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual QString description() = 0;

    /*************************************************************************
     * Hot Plugging
     *************************************************************************/
public slots:
    /**
     * Notify the plugin that a new device has been plugged in.
     *
     * @param vid The Vendor ID of the USB device
     * @param pid The Product ID of the USB device
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual void slotDeviceAdded(uint vid, uint pid) = 0;

    /**
     * Notify the plugin that an existing device has been removed.
     *
     * @param vid The Vendor ID of the USB device
     * @param pid The Product ID of the USB device
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual void slotDeviceRemoved(uint vid, uint pid) = 0;

    /*************************************************************************
     * Outputs
     *************************************************************************/
public:
    /**
     * Get a list of output lines provided by the plugin. Ownership of the
     * returned pointer is not transferred to the caller.
     *
     * This is an optional method for input-only plugins, but mandatory for
     * output plugins.
     *
     * @return A list of QLCOutputLine objects
     */
    virtual QSet <QLCOutputLine*> outputLines() { return QSet <QLCOutputLine*>(); }

signals:
    /** Notifies upper layer that a new output line has been added. */
    void outputLineAdded(QLCOutputLine* outputLine);

    /** Notifies listeners that an existing output line has been removed. */
    void outputLineRemoved(QLCOutputLine* outputLine);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /**
     * Get a list of input lines provided by the plugin. Ownership of the
     * returned pointer is not transferred to the caller.
     *
     * This is an optional method for output-only plugins. Mandatory for input plugins.
     *
     * @return A list of QLCInputLine objects
     */
    virtual QSet <QLCInputLine*> inputLines() { return QSet <QLCInputLine*>(); }

signals:
    /** Notifies upper layer that a new output line has been added. */
    void inputLineAdded(QLCInputLine* inputLine);

    /** Notifies listeners that an existing output line has been removed. */
    void inputLineRemoved(QLCInputLine* inputLine);

    /** Notifies upper layer about a changed value in an input line */
    void valueChanged(QLCInputLine* inputLine, quint32 channel, uchar value);

    /*************************************************************************
     * Configure
     *************************************************************************/
public:
    /**
     * Invoke a configuration dialog for the plugin
     *
     * This is an optional method if the plugin has no configurable options.
     * However, if canConfigure() has been overridden to return true, this method
     * must display a configuration dialog.
     */
    virtual void configure() { /* NOP */ }

    /**
     * Check, whether calling configure() on the plugin has any effect.
     *
     * This is an optional method. See configure() for more details.
     *
     * @return true if the plugin can be configured, otherwise false.
     */
    virtual bool canConfigure() { return false; }

signals:
    /**
     * Tells that the plugin's configuration has changed. Usually this means
     * that an output line has dis/appeared.
     */
    void configurationChanged();
};

Q_DECLARE_INTERFACE(QLCIOPlugin, "QLCIOPlugin")

#endif
