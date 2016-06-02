/*
 * Copyright (c) 2015-2016 Bert De Deckere
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <vector>
#include "coordinatetable.h"
#include "point.h"

//class QCPItemLine;


class MaterialDefinition
{
public:
    std::vector<Point> p;
    double epsr=1, mur=1, sigma=0;
    int index=0;            // This is set to -1 during first creation and remembers that the material can still be deleted if the user presses cancel
    int YuMittra=0;         // Bool would be better, but then there are conflicts when using stream
    bool plotted=false;     // Necessary to know whether or not the square is drawn on the plot, using this it's no longer necessary to recreate the entire plot

    MaterialDefinition();
    MaterialDefinition& operator=(const MaterialDefinition& a);
};

#endif // MATERIALDEFINITION_H
