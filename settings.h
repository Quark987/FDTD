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

#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{
public:
    int cellsX=200, cellsY=200;
    double sizeX=1, sizeY=1, courant=0.9999, maxTime;
    double sigmaXMax=1.91, sigmaYMax=1.91, m=3.5;
    int steps=500, numberOfThreads=8;
    int PMLlayers = 10;
    double dx, dy, dt;
    int sampleDistance=1;
    double width=240, height=180;
    int drawNthField=0;

    Settings();
    Settings& operator=(const Settings& a);
    void computeDifferentials();
};

#endif // SETTINGS_H
