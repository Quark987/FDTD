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

#include "planewave.h"
#include "math.h"
#include <qdebug.h>
#include <algorithm>

#define eta         377                 // Free space impedance
#define c           299792458
#define epsilon0    8.8541878176E-12
#define mu0         1.2566370614E-6

PlaneWave::PlaneWave()
{
    p.push_back(Point(-0.4, -0.4));
    p.push_back(Point(0.4, 0.4));
}

void PlaneWave::computePosition()
{
    settings.computeDifferentials();
    Point p1(std::min(p[0].x, p[1].x), std::min(p[0].y, p[1].y));
    Point p2(std::max(p[0].x, p[1].x), std::max(p[0].y, p[1].y));

    p.clear();
    p.push_back(p1);
    p.push_back(p2);

    i0 = p[0].x/settings.dx+settings.cellsX/2.0+settings.PMLlayers;
    i1 = p[1].x/settings.dx+settings.cellsX/2.0+settings.PMLlayers;
    j0 = p[0].y/settings.dy+settings.cellsY/2.0+settings.PMLlayers;
    j1 = p[1].y/settings.dy+settings.cellsY/2.0+settings.PMLlayers;

    setOrigin();
}

void PlaneWave::setOrigin()
{
    double centerX = (p[0].x+p[1].x)/2.0;
    double centerY = (p[0].y+p[1].y)/2.0;
    double diagonal = sqrt((p[0].x-p[1].x)*(p[0].x-p[1].x)+(p[0].y-p[1].y)*(p[0].y-p[1].y));

    originX = centerX+0.8*diagonal*cos(angle/180.0*M_PI);
    originY = centerY+0.8*diagonal*sin(angle/180.0*M_PI);
}

double PlaneWave::computeFields(double i, double j, double n)       // Double because it can be half an integer
{
    double x = settings.dx*(i-settings.cellsX/2.0-settings.PMLlayers);
    double y = settings.dy*(j-settings.cellsY/2.0-settings.PMLlayers);
    double t = settings.dt*n;

    double angleRad = angle/180.0*M_PI;     // Change this to relative angle if TFSF is not at center
    double argument = t - timeDelay - pulseWidth*1E-9 + (cos(angleRad)*(x-originX) + sin(angleRad)*(y-originY))/c;
    double F = exp(-argument*argument/(pulseWidth*pulseWidth))*sin(2*M_PI*centerFrequency*argument)*amplitude;

    Ex = -F*sin(angleRad);
    Ey = F*cos(angleRad);
    Hz = -F/eta;
}

PlaneWave& PlaneWave::operator=(const PlaneWave& a)
{
    this->index = a.index;
    this->p = a.p;
    this->angle = a.angle;
    this->timeDelay = a.timeDelay;
    this->pulseWidth = a.pulseWidth;
    this->centerFrequency = a.centerFrequency;
    this->originX = a.originX;
    this->originY = a.originY;
    this->Ex = a.Ex;
    this->Ey = a.Ey;
    this->Hz = a.Hz;
    this->settings = a.settings;
    this->amplitude = a.amplitude;
    this->timeDelay = a.timeDelay;
    return *this;
}
