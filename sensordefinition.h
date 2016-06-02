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

#ifndef SENSORDEFINITION_H
#define SENSORDEFINITION_H

#include "settings.h"
#include <stddef.h>     // NULL declaration
#include "fftw3.h"

class SensorDefinition
{
public:
    SensorDefinition();
    SensorDefinition& operator=(const SensorDefinition& a);
    void initVariables(Settings settings);
    void deleteVariables();

    bool phasePlotted=false, magnitudePlotted=false, timePlotted=false;
    double xpos=0, ypos=0, dt=0;
    double *Ex=NULL, *Ey=NULL, *Hz=NULL;
    fftw_complex *fEx=NULL, *fEy=NULL, *fHz=NULL;         // Container for the spectra
    int index=0, i=0, j=0, size=0;                  // Size of the pointer arrays
};

#endif // SENSORDEFINITION_H
