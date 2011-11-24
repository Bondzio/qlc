/*
  Q Light Controller
  cue.h

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

#ifndef CUE_H
#define CUE_H

#include <QString>
#include <QHash>

#include "scenevalue.h"

#define KXMLQLCCue "Cue"
#define KXMLQLCCueName "Name"
#define KXMLQLCCueValue "Value"
#define KXMLQLCCueValueChannel "Channel"

class QDomDocument;
class QDomElement;

class Cue
{
public:
    Cue(const QString& name = QString());
    Cue(const Cue& cue);
    ~Cue();

    void setName(const QString& str);
    QString name() const;

    void setValue(uint channel, uchar value);
    void unsetValue(uint channel);
    uchar value(uint channel) const;

    QHash <uint,uchar> values() const;

    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* stack_root) const;

private:
    QString m_name;
    QHash <uint,uchar> m_values;
};

#endif
