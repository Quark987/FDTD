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

#ifndef PLANEWAVE_H
#define PLANEWAVE_H

#include <vector>
#include "settings.h"
#include "area.h"
#include "point.h"

class PlaneWave
{
public:
    double timeDelay=0E-9, pulseWidth=0.5E-9;            // Pulse width in ns
    double centerFrequency=1E9, angle=45, amplitude=1;
    double originX=0, originY=0, Ex, Ey, Hz;
    int index, i0, j0, i1, j1;

    std::vector<Point> p;      // This defines the area occupied by the total/scattered field region
    Settings settings;

    PlaneWave();
    PlaneWave& operator=(const PlaneWave& a);
    void computePosition();
    double computeFields(double i, double j, double n);
    void setOrigin();
    void defineEdge();

};

#endif // PLANEWAVE_H
