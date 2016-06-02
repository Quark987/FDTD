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

#include "sensordefinition.h"
#include <stddef.h>
#include "fftw3.h"

SensorDefinition::SensorDefinition()
{
}

SensorDefinition& SensorDefinition::operator=(const SensorDefinition& a)
{
    this->xpos = a.xpos;
    this->ypos = a.ypos;
    this->Ex = a.Ex;
    this->Ey = a.Ey;
    this->Hz = a.Hz;
    this->index = a.index;
    this->size = a.size;
    this->dt = a.dt;
    this->i = a.i;
    this->j = a.j;
    return *this;
}

void SensorDefinition::initVariables(Settings settings)
{
    settings.computeDifferentials();
    i = xpos/settings.dx+settings.cellsX/2.0+settings.PMLlayers;
    j = ypos/settings.dy+settings.cellsY/2.0+settings.PMLlayers;
    dt = settings.dt;
    size = settings.steps;

    deleteVariables();
    Ex = new double[settings.steps];//    Ex[0] = 0; Ex[1] = 0;          // First two values are always assumed 0
    Ey = new double[settings.steps];//    Ey[0] = 0; Ey[1] = 0;
    Hz = new double[settings.steps];//    Hz[0] = 0; Hz[1] = 0;

    fEx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * settings.steps/2+1);
    fEy = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * settings.steps/2+1);
    fHz = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * settings.steps/2+1);
}

void SensorDefinition::deleteVariables()
{
    if(Ex != NULL) {
        delete[] Ex;
        delete[] Ey;
        delete[] Hz;
    }

    fftw_free(fEx);
    fftw_free(fEy);
    fftw_free(fHz);
}
