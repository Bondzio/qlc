/*
  Q Light Controller
  vcwidget.h

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

#ifndef VCWIDGET_H
#define VCWIDGET_H

#include <QKeySequence>
#include <QWidget>

#include "common/qlctypes.h"
#include "app.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class QMouseEvent;
class QString;
class QMenu;
class QFile;

#define KXMLQLCVCCaption "Caption"
#define KXMLQLCVCFrameStyle "FrameStyle"

#define KXMLQLCVCWidgetAppearance "Appearance"

#define KXMLQLCVCWidgetForegroundColor "ForegroundColor"
#define KXMLQLCVCWidgetBackgroundColor "BackgroundColor"
#define KXMLQLCVCWidgetColorDefault "Default"

#define KXMLQLCVCWidgetFont "Font"
#define KXMLQLCVCWidgetFontDefault "Default"

#define KXMLQLCVCWidgetBackgroundImage "BackgroundImage"
#define KXMLQLCVCWidgetBackgroundImageNone "None"

#define KVCFrameStyleSunken (QFrame::StyledPanel | QFrame::Sunken)
#define KVCFrameStyleRaised (QFrame::StyledPanel | QFrame::Raised)
#define KVCFrameStyleNone   (QFrame::NoFrame)

#define KXMLQLCVCWidgetInput "Input"
#define KXMLQLCVCWidgetInputUniverse "Universe"
#define KXMLQLCVCWidgetInputChannel "Channel"

class VCWidget : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
protected:
	/** Protected constructor so nobody makes an instance of this class
	    unless inherited */
	VCWidget(QWidget* parent);

public:
	virtual ~VCWidget();

private:
	Q_DISABLE_COPY(VCWidget)

	/*********************************************************************
	 * Clipboard
	 *********************************************************************/
public:
	/** Create a copy of this widget into the given parent and return it */
	virtual VCWidget* createCopy(VCWidget* parent) = 0;

protected:
	/** Copy the contents for this widget from the given widget */
	virtual bool copyFrom(const VCWidget* widget);

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Set the widget's background image */
	virtual void setBackgroundImage(const QString& path);

	/** Get the widget's background image */
	virtual QString backgroundImage() { return m_backgroundImage; }

protected:
	QString m_backgroundImage;

	/*********************************************************************
	 * Background color
	 *********************************************************************/
public:
	/** Set the widget's background color and invalidate background image */
	virtual void setBackgroundColor(const QColor& color);

	/** Get the widget's background color. The color is invalid if the
	    widget has a background image. */
	virtual QColor backgroundColor() const {
		return palette().color(QPalette::Window); }

	/** Check, whether the widget has a custom background color */
	virtual bool hasCustomBackgroundColor() const {
		return m_hasCustomBackgroundColor; }

	/** Reset the widget's background color to whatever the platform uses */
	virtual void resetBackgroundColor();

protected:
	bool m_hasCustomBackgroundColor;

	/*********************************************************************
	 * Foreground color
	 *********************************************************************/
public:
	/** Set the widget's foreground color */
	virtual void setForegroundColor(const QColor& color);

	/** Get the widget's foreground color */
	virtual QColor foregroundColor() const {
		return palette().color(QPalette::WindowText); }

	/** Check, whether the widget has a custom foreground color */
	virtual bool hasCustomForegroundColor() const {
		return m_hasCustomForegroundColor; }

	/** Reset the widget's foreground color to whatever the platform uses */
	virtual void resetForegroundColor();

protected:
	bool m_hasCustomForegroundColor;

	/*********************************************************************
	 * Font
	 *********************************************************************/
public:
	/** Set the font used for the widget's caption */
	virtual void setFont(const QFont& font);

	/** Get the font used for the widget's caption */
	virtual QFont font() const { return QWidget::font(); }

	/** Check, whether the widget has a custom font */
	virtual bool hasCustomFont() const { return m_hasCustomFont; }

	/** Reset the font used for the widget's caption to whatever the
	    platform uses */
	virtual void resetFont();

protected:
	bool m_hasCustomFont;

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	/** Set this widget's caption text */
	virtual void setCaption(const QString& text);

	/** Get this widget's caption text */
	virtual QString caption() const { return windowTitle(); }

	/*********************************************************************
	 * Frame style
	 *********************************************************************/
public:
	void setFrameStyle(int style);
	int frameStyle() const { return m_frameStyle; }
	void resetFrameStyle();

public:
	static QString frameStyleToString(int style);
	static int stringToFrameStyle(const QString& style);

protected:
	int m_frameStyle;

	/*********************************************************************
	 * Capability to have children
	 *********************************************************************/
public:
	/** Return true if this widget can hold children. */
	virtual bool canHaveChildren() const { return false; }

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	virtual void editProperties();

	/*********************************************************************
	 * External input
	 *********************************************************************/
public:
	/** Set external input universe & channel number to listen to */
	void setInputSource(t_input_universe uni, t_input_channel ch);

	/** Get the assigned external input universe */
	t_input_universe inputUniverse() const { return m_inputUniverse; }

	/** Get the assigned external input channel within inputUniverse() */
	t_input_channel inputChannel() const { return m_inputChannel; }

protected slots:
	/** Slot that receives external input data */
	virtual void slotInputValueChanged(t_input_universe universe,
					   t_input_channel channel,
					   t_input_value value);

protected:
	t_input_universe m_inputUniverse;
	t_input_channel m_inputChannel;

	/*********************************************************************
	 * Key sequence handler
	 *********************************************************************/
protected slots:
	/** Handle key presses. Default implementation passes to children. */
	virtual void slotKeyPressed(const QKeySequence& keySequence);

	/** Handle key releases. Default implementation passes to children. */
	virtual void slotKeyReleased(const QKeySequence& keySequence);

signals:
	/** Tell listeners that a key was pressed */
	void keyPressed(const QKeySequence& keySequence);

	/** Tell listeners that a key was released */
	void keyReleased(const QKeySequence& keySequence);

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	virtual bool loadXML(const QDomElement* vc_root) = 0;
	virtual bool saveXML(QDomDocument* doc, QDomElement* vc_root) = 0;

protected:
	bool loadXMLAppearance(const QDomElement* appearance_root);
	bool loadXMLInput(const QDomElement* root);

	bool saveXMLAppearance(QDomDocument* doc, QDomElement* widget_root);
	bool saveXMLInput(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * QLC Mode change
	 *********************************************************************/
protected slots:
	virtual void slotModeChanged(App::Mode mode);

signals:
	void modeChanged(App::Mode);

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	/** Invoke a context menu */
	virtual void invokeMenu(const QPoint& point);

	/*********************************************************************
	 * Custom menu
	 *********************************************************************/
public:
	/** Get a custom menu specific to this widget. Ownership is transferred
	    to the caller, which must delete the returned menu pointer. */
	virtual QMenu* customMenu(QMenu* parentMenu);

	/*********************************************************************
	 * Widget move & resize
	 *********************************************************************/
public:
	/** Resize this widget to given size. TODO: Change to QSize. */
	virtual void resize(QPoint p);

	/** Move this widget to the given point */
	virtual void move(QPoint p);

	/** Get the point where the mouse was clicked last in this widget */
	QPoint lastClickPoint() const { return m_mousePressPoint; }

protected:
	QPoint m_mousePressPoint;
	bool m_resizeMode;

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	virtual void paintEvent(QPaintEvent* e);

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void handleWidgetSelection(QMouseEvent* e);

	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);

};

#endif
