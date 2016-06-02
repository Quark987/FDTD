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

#ifndef SGInterface_H
#define SGInterface_H

#include <vector>
#include "point.h"
#include "SGField.h"
#include "field.h"
#include "settings.h"

class Field;        // field.h includes SGInterface.h, so forward declaration of field is needed

class SGInterface
{
public:
    SGInterface();
    SGInterface& operator=(const SGInterface& a);
    void advanceH(int n);
    void advanceE(int n);
    void computePosition(Settings settings);

    Field *FA=NULL;
    SGField *FB=NULL;
    std::vector<Point> p;                           // Position
    double index, xRatio=1, yRatio=1, dt, dx, dy;   // x and y refinement ratio
    int iMin, iMax, jMin, jMax;
    int sizeWorkBuffer;
};

#endif // SGInterface_H
