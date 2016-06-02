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

#include "area.h"

Area::Area(int iMin, int iMax, int jMin, int jMax)
{
//    qDebug() << iMin << "\t" << iMax << "\t" << jMin << "\t" << jMax;
    this->iMax=iMax;
    this->iMin=iMin;
    this->jMax=jMax;
    this->jMin=jMin;
}

Area& Area::operator=(const Area& a)
{
    this->iMax = a.iMax;
    this->iMin = a.iMin;
    this->jMax = a.jMax;
    this->jMin = a.jMin;
}
