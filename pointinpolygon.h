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

#ifndef POINTINPOLYGON_H
#define POINTINPOLYGON_H

#include "point.h"
#include <vector>
#include <QDebug>
#include <cmath>

class pointInPolygon
{
public:
    pointInPolygon();
    int inPolygon(double x, double y);
    double distanceX(double x, double y);       // Returns the x distance between the given point and the closest edge
    double distanceY(double x, double y);       // Returns the y distance between the given point and the closest edge

    const std::vector<Point> *vertices;
};

#endif // POINTINPOLYGON_H
