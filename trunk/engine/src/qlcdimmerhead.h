/*
  Q Light Controller
  qlcdimmerhead.h

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

#ifndef QLCDIMMERHEAD_H
#define QLCFIMMERHEAD_H

#include "qlcfixturehead.h"

/**
 * A small specialization class from QLCFixtureHead to be used for each
 * Generic Dimmer channel to make them work as separate heads.
 */
class QLCDimmerHead : public QLCFixtureHead
{
public:
    /**
     * Construct a new QLCDimmerHead with the given $head number representing
     * the relative channel number within the dimmer. The $head number is set
     * as the head's masterIntensityChannel(), with all other cached channels
     * left invalid.
     *
     * @param head The channel/head number to represent
     */
    QLCDimmerHead(int head);
    ~QLCDimmerHead();
};

#endif
