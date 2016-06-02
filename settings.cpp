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

#include "settings.h"
#include <math.h>

#define c           299792458
#define epsilon0    8.8541878176E-12
#define mu0         1.2566370614E-6

Settings::Settings()
{

}


Settings& Settings::operator=(const Settings& a){
    this->cellsX = a.cellsX;
    this->cellsY = a.cellsY;
    this->courant = a.courant;
    this->maxTime = a.maxTime;
    this->PMLlayers = a.PMLlayers;
    this->sizeX = a.sizeX;
    this->sizeY = a.sizeY;
    this->steps = a.steps;
    this->sigmaXMax = a.sigmaXMax;
    this->sigmaYMax = a.sigmaYMax;
    this->m = a.m;
    this->numberOfThreads = a.numberOfThreads;
    this->sampleDistance = a.sampleDistance;
    this->height = a.height;
    this->width = a.width;
    this->dx = a.dx;
    this->dy = a.dy;
    this->dt = a.dt;
    this->drawNthField = a.drawNthField;
    return *this;
}

void Settings::computeDifferentials()
{
    dx = (double)(sizeX)/cellsX;
    dy = (double)(sizeY)/cellsY;
    dt = courant/sqrt(1/(dx*dx)+1/(dy*dy))/c;
}
