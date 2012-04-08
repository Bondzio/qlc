/*
  Q Light Controller
  qlcoutplugin.h

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

#ifndef QLCOUTPLUGIN_H
#define QLCOUTPLUGIN_H

#include <QStringList>
#include <QtPlugin>
#include <QObject>
#include <climits>

/**
 * QLCOutPlugin is an interface for all output plugins. Output plugins provide
 * the means for QLC to actually output DMX data to fixtures, using DMX hard-
 * ware supported by various output plugins.
 *
 * Each plugin can be understood as an adaptation layer betwen QLC and an
 * interconnecting protocol used by a certain family of DMX (well, any
 * lighting protocol) dongles. For example, an FTDI output plugin provides
 * the means for QLC to control fixtures using, for example, an ENTTEC DMX
 * dongles. Each plugin must provide at least one output line for QLC in order
 * to work properly. Then again, if there are no such devices currently
 * connected to the computer that would be supported by the plugin, the plugin
 * can choose to provide no lines at all (until the user plugs in a supported
 * device).
 *
 * When QLC has successfully loaded an output plugin, it will call init()
 * exactly once for each plugin. After that, it is assumed that either the
 * plugin auto-senses the devices it supports or the user must manually try
 * to search for new devices thru a custom configuration dialog that can be
 * opened with configure().
 *
 * Plugins should not leave any resources open unless open() is called. And
 * even then, the plugin should open only such resources that are needed for
 * the specific output line given in the call to open(). Respectively, when
 * close() is called, the plugin should relinquish all resources associated to
 * the closed output line (unless shared with other lines).
 *
 * Plugins have a name that is shown to users as a list item. Each output
 * line name, preceded by its index, is shown under the plugin name as a
 * selectable list entry. Therefore, these names should be descriptive, but
 * relatively short. Output line indices are shown on the UI as 1-based, but
 * they are still handled internally as 0-based.
 *
 * An info text can be fetched for each plugin with infoText(). If the output
 * parameter == QLCOutPlugin::invalidOutput(), the plugin should provide a brief status snippet
 * on its overall state. If the output line parameter is given, the plugin
 * should provide information concerning ONLY that particular output line.
 * This info is displayed to the user as-is.
 *
 * DMX data is written by QLC to plugins with writeData(). Complete 512-channel
 * universes are written at a time. Traffic from output plugins towards QLC is
 * not possible.
 */
class QLCOutPlugin : public QObject
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
    virtual ~QLCOutPlugin() { /* NOP */ }

    /**
     * Initialize the plugin. Since plugins cannot have a user-defined
     * constructor, any initialization prior to opening any HW must be
     * done thru this second-stage initialization method. OutputMap calls
     * this method for all plugins exactly once after loading, before
     * calling any other method from the plugin.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual void init() = 0;

    /**
     * Get the plugin's name. Plugin's name must not change over time.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual QString name() = 0;

    /** Invalid input/output number */
    static quint32 invalidLine() { return UINT_MAX; }

    /*************************************************************************
     * Outputs
     *************************************************************************/
public:
    /**
     * Open the specified output line so that the plugin can start sending
     * DMX data thru that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param output The output line to open
     */
    virtual void openOutput(quint32 output) = 0;

    /**
     * Close the specified output line so that the plugin can stop
     * sending output data thru that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param output The output line to close
     */
    virtual void closeOutput(quint32 output) = 0;

    /**
     * Get a list of output lines' names. The names must be always in the
     * same order i.e. the first name is the name of output line number 0,
     * the next one is output line number 1, etc..
     *
     * @return Number of outputs provided by the plugin
     */
    virtual QStringList outputs() = 0;

    /**
     * Write a complete 512-channel DMX universe to the plugin.
     *
     * @param output The output universe to write to
     * @param universe The universe data to write
     */
    virtual void writeUniverse(quint32 output, const QByteArray& universe) = 0;

    /**
     * Provide an information text to be displayed in the output manager.
     * If @output is QLCOutPlugin::invalidOutput(), the info text contains info regarding
     * the whole plugin. Otherwise it contains info on the specific output.
     * This information is meant to help users during output mapping.
     *
     * This is a pure virtual method that must be implemented
     * in all plugins.
     *
     * @param output The output to get info from
     */
    virtual QString outputInfo(quint32 output) = 0;

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /**
     * Open the specified input line so that the plugin can start sending input
     * data from that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input The input line to open
     */
    virtual void openInput(quint32 input) = 0;

    /**
     * Close the specified input line so that the plugin can stop sending input
     * data from that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input The input line to close
     */
    virtual void closeInput(quint32 input) = 0;

    /**
     * Get a list of input lines' names. The names must be always in the
     * same order i.e. the first name is the name of input line number 0,
     * the next one is input line number 1, etc.. These indices are used
     * with open() and close().
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @return A list of available input names
     */
    virtual QStringList inputs() = 0;

    /**
     * Send a value back to an input line's channel. This method can be
     * used for example to move motorized sliders with QLC sliders. If the
     * hardware /that the plugin provides access to) doesn't support this,
     * the implementation can be left empty.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input The input line to send feedback to
     * @param channel A channel in the input line to send feedback to
     * @param value An input value to send back to the input channel
     */
    virtual void sendFeedBack(quint32 input, quint32 channel, uchar value) = 0;

    /**
     * Provide an information text to be displayed in the plugin manager
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input If specified, information for the given input line is
     *              expected. Otherwise provides information for the plugin
     */
    virtual QString inputInfo(quint32 input) = 0;

signals:
    /**
     * Tells that the value of a channel in an input line has changed and needs
     * to be reacted to (if applicable). This is practically THE WAY for
     * input plugins to provide input data to QLC.
     *
     * @param input The input line whose channel has changed value
     * @param channel The channel that has changed its value
     * @param value The newly-changed channel value
     */
    void valueChanged(quint32 input, quint32 channel, uchar value);

    /*************************************************************************
     * Configure
     *************************************************************************/
public:
    /**
     * Invoke a configuration dialog for the plugin
     *
     * This is a pure virtual method that must be implemented by all plugins.
     * However, if there's nothing to configure (canConfigure() returns false),
     * the implementation can be left completely empty.
     */
    virtual void configure() = 0;

    /**
     * Check, whether calling configure() on a plugin has any effect. If this
     * method returns false, the configuration button in OutputManager will be
     * disabled.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @return true if the plugin can be configured, otherwise false.
     */
    virtual bool canConfigure() = 0;

signals:
    /**
     * Tells that the plugin's configuration has changed. Usually this means
     * that an output line has dis/appeared. Used by the OutputManager to
     * re-read the plugin's current configuration.
     */
    void configurationChanged();
};

Q_DECLARE_INTERFACE(QLCOutPlugin, "QLCOutPlugin")

#endif
