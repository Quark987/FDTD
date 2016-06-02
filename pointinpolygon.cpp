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

#include "pointinpolygon.h"

#define INF 1E10

pointInPolygon::pointInPolygon()
{
}

int pointInPolygon::inPolygon(double x, double y)
{
    int i, j, c = 0, nvert=vertices->size();
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
      if ( (((*vertices)[i].y>y) != ((*vertices)[j].y>y)) &&
            (x < ((*vertices)[j].x-(*vertices)[i].x) * (y-(*vertices)[i].y) / ((*vertices)[j].y-(*vertices)[i].y) + (*vertices)[i].x) )
         c = !c;
    }
    return c;
}

double pointInPolygon::distanceX(double x, double y)
{
    std::vector<double> distance;

    int i, j, nvert=vertices->size();
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if (((*vertices)[i].y>y) != ((*vertices)[j].y>y))       // The edge is crossed
            distance.push_back(((*vertices)[j].x-(*vertices)[i].x) * (y-(*vertices)[i].y) / ((*vertices)[j].y-(*vertices)[i].y) + (*vertices)[i].x - x);
//            qDebug() << x - ((*vertices)[j].x-(*vertices)[i].x) * (y-(*vertices)[i].y) / ((*vertices)[j].y-(*vertices)[i].y) + (*vertices)[i].x;
    }

    if(distance.size() > 0)
    {
        double minDistance = INF, signedDistance = INF, sign=-1;        // Find the distance with the lowest absolute value, and return the signed type, the sign determines if the exterior of the edge is to the right or left
        for(int k=0; k<distance.size(); k++)
        {
            if(distance[k] < 0)
                sign = sign > 0 ? -1 : 1;                               // Only work at the exterior of the polygon

            if(sign < 0 && minDistance > std::abs(distance[k])) {
                minDistance = std::abs(distance[k]);
                signedDistance = distance[k];
            }
        }
        return signedDistance;
    }

    return INF;
}

double pointInPolygon::distanceY(double x, double y)
{
    std::vector<double> distance;

    int i, j, nvert=vertices->size();
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if (((*vertices)[i].x>x) != ((*vertices)[j].x>x))       // The edge is crossed
            distance.push_back(((*vertices)[j].y-(*vertices)[i].y) * (x-(*vertices)[i].x) / ((*vertices)[j].x-(*vertices)[i].x) + (*vertices)[i].y - y);
//            qDebug() << x - ((*vertices)[j].x-(*vertices)[i].x) * (y-(*vertices)[i].y) / ((*vertices)[j].y-(*vertices)[i].y) + (*vertices)[i].x;
    }

    if(distance.size() > 0)
    {
        double minDistance = INF, signedDistance = INF, sign=-1;        // Find the distance with the lowest absolute value, and return the signed type, the sign determines if the exterior of the edge is to the top or bottom
        for(int k=0; k<distance.size(); k++)
        {
            if(distance[k] < 0)
                sign = sign > 0 ? -1 : 1;                               // Only work at the exterior of the polygon

            if(sign < 0 && minDistance > std::abs(distance[k])) {
                minDistance = std::abs(distance[k]);
                signedDistance = distance[k];
            }

        }
        return signedDistance;
    }

    return INF;
}
