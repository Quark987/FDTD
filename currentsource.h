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

#ifndef CURRENTSOURCE_H
#define CURRENTSOURCE_H


class currentSource
{
public:

    currentSource();
    currentSource& operator=(const currentSource& a);

    double frequency=2000, magnitude=1, xpos=0, ypos=0;
    int i, j;
    char polarization='x', type='s';        // Type 's' = sinusoidal, 'g' = gaussian
    int index=0;


    double timeDelay=0E-9, pulseWidth=0.5E-9;           // Pulse width in ns
    double frequencyG=2000, magnitudeG=1;                // Trailing G denotes gaussian variables
    double xposG=0, yposG=0;
    char polarizationG='x';
    int iG,jG;
};

#endif // CURRENTSOURCE_H
