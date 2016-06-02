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

#include "currentsource.h"
#include <math.h>

currentSource::currentSource()
{
}

currentSource& currentSource::operator=(const currentSource& a)
{
    this->frequency = a.frequency;
    this->magnitude = a.magnitude;
    this->xpos = a.xpos;
    this->ypos = a.ypos;
    this->polarization = a.polarization;
    this->index = a.index;
    this->i = a.i;
    this->j = a.j;

    this->type = a.type;
    this->timeDelay = a.timeDelay;
    this->pulseWidth = a.pulseWidth;
    this->frequencyG = a.frequencyG;
    this->magnitudeG = a.magnitudeG;
    this->xposG = a.xposG;
    this->yposG = a.yposG;
    this->iG = a.iG;
    this->jG = a.jG;
    this->polarizationG = a.polarizationG;
}
