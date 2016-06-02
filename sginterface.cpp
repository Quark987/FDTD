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

#include "sginterface.h"

SGInterface::SGInterface()
{
    p.push_back(Point(-0.02, -0.02));
    p.push_back(Point(0.02, 0.02));
}

SGInterface& SGInterface::operator=(const SGInterface& a)
{
    this->p = a.p;
    this->index = a.index;
    this->xRatio = a.xRatio;
    this->yRatio = a.yRatio;
    this->FA = a.FA;
    this->FB = a.FB;
}

void SGInterface::advanceE(int n)
{
    int Old = (n-1+sizeWorkBuffer)%sizeWorkBuffer;      // Old time
    int New = n%sizeWorkBuffer;                         // New time

    // Correct wrongful E update, first set the wrong terms to 0
    for(int i=iMin; i<iMin+FB->cellsX; i++) {
        int j = jMin-1;
        double C = FA->sigmaU[i][j]*dt/(2*FA->epsU[i][j]);
        FA->WBEx[New][i][j] = (1-C)/(1+C)*FA->WBEx[Old][i][j] + dt/FA->epsU[i][j]/(1+C)*((0 - FA->WBHz[Old][i][j])/dy);     // Bottom

        j = jMax;
        C = FA->sigmaU[i][j]*dt/(2*FA->epsU[i][j]);
        FA->WBEx[New][i][j] = (1-C)/(1+C)*FA->WBEx[Old][i][j] + dt/FA->epsU[i][j]/(1+C)*((FA->WBHz[Old][i][j+1] - 0)/dy);   // Top
    }

    for(int j=jMin; j<jMin+FB->cellsY; j++) {
        int i = iMin-1;
        double C = FA->sigmaR[i][j]*dt/(2*FA->epsR[i][j]);
        FA->WBEy[New][i][j] = (1-C)/(1+C)*FA->WBEy[Old][i][j] - dt/FA->epsR[i][j]/(1+C)*((0 - FA->WBHz[Old][i][j])/dx);     // Left

        i = iMax;
        C = FA->sigmaR[i][j]*dt/(2*FA->epsR[i][j]);
        FA->WBEy[New][i][j] = (1-C)/(1+C)*FA->WBEy[Old][i][j] - dt/FA->epsR[i][j]/(1+C)*((FA->WBHz[Old][i+1][j] - 0)/dx);   // Right
    }

    // And add the correction term from the subgrid
    for(int i=0; i<FB->sizeHzx; i++) {
        int j = jMin-1;
        double factor = (i==0 || i==FB->sizeHzx-1 ? 1 : xRatio);
        double C = FA->sigmaU[iMin+(int)(1+(i-1)/xRatio)][j]*dt/(2*FA->epsU[iMin+(int)(1+(i-1)/xRatio)][j]);
        FA->WBEx[New][iMin+(int)(1+(i-1)/xRatio)][j] += dt/(FA->epsU[iMin+(int)(1+(i-1)/xRatio)][j]*(1+C)*dy)*FB->Hz(Old, i, 0)/factor;                  // Bottom

        j = jMax;
        C = FA->sigmaU[iMin+(int)(1+(i-1)/xRatio)][j]*dt/(2*FA->epsU[iMin+(int)(1+(i-1)/xRatio)][j]);
        FA->WBEx[New][iMin+(int)(1+(i-1)/xRatio)][j] -= dt/(FA->epsU[iMin+(int)(1+(i-1)/xRatio)][j]*(1+C)*dy)*FB->Hz(Old, i, FB->sizeHzy-1)/factor;      // Top
    }

    for(int j=0; j<FB->sizeHzy; j++) {
        int i = iMin-1;
        double factor = (j==0 || j==FB->sizeHzy-1 ? 1 : yRatio);
        double C = FA->sigmaR[i][jMin+(int)(1+(j-1)/yRatio)]*dt/(2*FA->epsR[i][jMin+(int)(1+(j-1)/yRatio)]);
        FA->WBEy[New][i][jMin+(int)(1+(j-1)/yRatio)] -= dt/(FA->epsR[i][jMin+(int)(1+(j-1)/yRatio)]*(1+C)*dx)*FB->Hz(Old, 0, j)/factor;                // Left

        i = iMax;
        C = FA->sigmaR[i][jMin+(int)(1+(j-1)/yRatio)]*dt/(2*FA->epsR[i][jMin+(int)(1+(j-1)/yRatio)]);
        FA->WBEy[New][i][jMin+(int)(1+(j-1)/yRatio)] += dt/(FA->epsR[i][jMin+(int)(1+(j-1)/yRatio)]*(1+C)*dx)*FB->Hz(Old, FB->sizeHzx-1, j)/factor;    // Right
    }

    // Coupling to the subgrid
    FB->s.setZero();
    for(int i=0; i<FB->sizeHzx; i++) {
        FB->s(FB->indexHz(i, 0)) -= dt/(FB->muC(i, 0)*dy)*FA->WBEx[New][iMin+(int)(1+(i-1)/xRatio)][jMin-1];     // Bottom
        FB->s(FB->indexHz(i, FB->sizeHzy-1)) += dt/(FB->muC(i, FB->sizeHzy-1)*dy)*FA->WBEx[New][iMin+(int)(1+(i-1)/xRatio)][jMax];     // Top
    }

    for(int j=0; j<FB->sizeHzy; j++) {
        FB->s(FB->indexHz(0, j)) += dt/(FB->muC(0, j)*dx)*FA->WBEy[New][iMin-1][jMin+(int)(1+(j-1)/yRatio)];  // Left
        FB->s(FB->indexHz(FB->sizeHzx-1, j)) -= dt/(FB->muC(FB->sizeHzx-1, j)*dx)*FA->WBEy[New][iMax][jMin+(int)(1+(j-1)/yRatio)];  // Right
    }

    FB->updateFields(n);
}

void SGInterface::advanceH(int n) {

}

void SGInterface::computePosition(Settings settings)
{
    Point topRight = Point(std::max(p[0].x, p[1].x), std::max(p[0].y, p[1].y));
    Point bottomLeft = Point(std::min(p[0].x, p[1].x), std::min(p[0].y, p[1].y));

    topRight = Point(ceil(topRight.x/settings.dx)*settings.dx, ceil(topRight.y/settings.dy)*settings.dy);   // Align the surface to the grid
    bottomLeft = Point(ceil(bottomLeft.x/settings.dx)*settings.dx, ceil(bottomLeft.y/settings.dy)*settings.dy);
//    p.clear();
//    p.push_back(bottomLeft);
//    p.push_back(topRight);

    settings.computeDifferentials();
    this->dt = settings.dt;
    this->dx = settings.dx;
    this->dy = settings.dy;
    this->sizeWorkBuffer = FA->sizeWorkBuffer;
    iMin = floor(bottomLeft.x/settings.dx+settings.cellsX/2.0+settings.PMLlayers);
    jMin = floor(bottomLeft.y/settings.dy+settings.cellsY/2.0+settings.PMLlayers);

    iMax = iMin + ceil((topRight.x - bottomLeft.x)/settings.dx) - 1;        // This is the same definition as used in SGField
    jMax = jMin + ceil((topRight.y - bottomLeft.y)/settings.dy) - 1;

//    qDebug() << iMin << iMax << jMin << jMax << p[0].x << p[1].x;
}
