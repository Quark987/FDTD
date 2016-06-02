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

#include "field.h"

#define c           299792458
#define epsilon0    8.8541878176E-12
#define mu0         1.2566370614E-6

Field::Field(Settings settings, QWaitCondition *waitCondition, QMutex* mutex, int* threadCounter)
{
    this->settings = settings;
    this->computation = waitCondition;
    this->mutex = mutex;
    this->threadCounter = threadCounter;
}

void Field::deleteFields()
{
    if(WBEx != NULL) {
        for(int t=0; t<std::ceil((double)settings.steps/settings.sampleDistance); t++) {
            for(int i=0; i<2*settings.PMLlayers+settings.cellsX; i++){
                delete[] OBEx[t][i];
                delete[] OBEy[t][i];
                delete[] OBHz[t][i];
            }
            delete[] OBEx[t];
            delete[] OBEy[t];
            delete[] OBHz[t];
        }

        for(int t=0; t<sizeWorkBuffer; t++) {
            for(int i=0; i<2*settings.PMLlayers+settings.cellsX; i++){
                delete[] WBEx[t][i];
                delete[] WBEy[t][i];
                delete[] WBHz[t][i];
            }
            delete[] WBEx[t];
            delete[] WBEy[t];
            delete[] WBHz[t];
        }

        for(int i=0; i<2*settings.PMLlayers+settings.cellsX; i++) {
            delete[] sigmaR[i];
            delete[] sigmaU[i];
            delete[] epsR[i];
            delete[] epsU[i];
            delete[] muC[i];
        }

        delete[] OBEx;
        delete[] OBEy;
        delete[] OBHz;
        delete[] WBEx;
        delete[] WBEy;
        delete[] WBHz;
        delete[] epsR;
        delete[] epsU;
        delete[] muC;
        delete[] sigmaR;
        delete[] sigmaU;

        OBEx = NULL;
        OBEy = NULL;
        OBHz = NULL;
        WBEx = NULL;
        WBEy = NULL;
        WBHz = NULL;
        epsR = NULL;
        epsU = NULL;
        muC = NULL;
        sigmaR = NULL;
        sigmaU = NULL;
    }
}

Field::~Field()
{
}

void Field::initFields()
{
    OBEx = new double**[(int)std::ceil((double)settings.steps/settings.sampleDistance)];
    OBEy = new double**[(int)std::ceil((double)settings.steps/settings.sampleDistance)];
    OBHz = new double**[(int)std::ceil((double)settings.steps/settings.sampleDistance)];

    WBEx = new double**[sizeWorkBuffer];
    WBEy = new double**[sizeWorkBuffer];
    WBHz = new double**[sizeWorkBuffer];

    epsR = new double*[2*settings.PMLlayers+settings.cellsX];
    epsU = new double*[2*settings.PMLlayers+settings.cellsX];
    muC = new double*[2*settings.PMLlayers+settings.cellsX];
    sigmaR = new double*[2*settings.PMLlayers+settings.cellsX];
    sigmaU = new double*[2*settings.PMLlayers+settings.cellsX];

    for(int k=0; k<std::ceil((double)settings.steps/settings.sampleDistance); k++) {
        OBEx[k] = new double*[2*settings.PMLlayers+settings.cellsX];
        OBEy[k] = new double*[2*settings.PMLlayers+settings.cellsX];
        OBHz[k] = new double*[2*settings.PMLlayers+settings.cellsX];

        for(int m=0; m<2*settings.PMLlayers+settings.cellsX; m++) {
            OBEx[k][m] = new double[2*settings.PMLlayers+settings.cellsY];
            OBEy[k][m] = new double[2*settings.PMLlayers+settings.cellsY];
            OBHz[k][m] = new double[2*settings.PMLlayers+settings.cellsY];

            for(int j=0; j<2*settings.PMLlayers+settings.cellsY; j++) {
                OBEx[k][m][j] = 0;            // Initialize everything to 0, because WB swaps with OB using these values
                OBEy[k][m][j] = 0;            // It'd suffice to initialize the boundary to zero, but this is easier coding :-)
                OBHz[k][m][j] = 0;
            }
        }
    }

    for(int k=0; k<sizeWorkBuffer; k++) {
        WBEx[k] = new double*[2*settings.PMLlayers+settings.cellsX];
        WBEy[k] = new double*[2*settings.PMLlayers+settings.cellsX];
        WBHz[k] = new double*[2*settings.PMLlayers+settings.cellsX];

        for(int m=0; m<2*settings.PMLlayers+settings.cellsX; m++) {
            WBEx[k][m] = new double[2*settings.PMLlayers+settings.cellsY];
            WBEy[k][m] = new double[2*settings.PMLlayers+settings.cellsY];
            WBHz[k][m] = new double[2*settings.PMLlayers+settings.cellsY];

            for(int j=0; j<2*settings.PMLlayers+settings.cellsY; j++) {
                WBEx[k][m][j] = 0;            // Initialize everything to 0, because the boundary won't be updated in the FDTD routines
                WBEy[k][m][j] = 0;            // It'd suffice to initialize the boundary to zero, but this is easier coding :-)
                WBHz[k][m][j] = 0;
            }
        }
    }

    for(int k=0; k<2*settings.PMLlayers+settings.cellsX; k++) {           // Epsilon and mu are not time dependent
        epsR[k] = new double[2*settings.PMLlayers+settings.cellsY];
        epsU[k] = new double[2*settings.PMLlayers+settings.cellsY];
        muC[k] = new double[2*settings.PMLlayers+settings.cellsY];
        sigmaR[k] = new double[2*settings.PMLlayers+settings.cellsY];
        sigmaU[k] = new double[2*settings.PMLlayers+settings.cellsY];

        for(int m=0; m<2*settings.PMLlayers+settings.cellsY; m++) {
            epsR[k][m] = epsilon0;
            epsU[k][m] = epsilon0;
            muC[k][m] = mu0;
            sigmaR[k][m] = 0;
            sigmaU[k][m] = 0;
        }
    }
}

void Field::computeDifferentials() {
    dx = (double)(settings.sizeX)/settings.cellsX;
    dy = (double)(settings.sizeY)/settings.cellsY;
    dt = settings.courant/sqrt(1/(dx*dx)+1/(dy*dy))/c;
}

void Field::transferSample(int n) {
    int WBpos = n%sizeWorkBuffer;
    int OBpos = n/settings.sampleDistance;

    std::swap(OBEx[OBpos], WBEx[WBpos]);    // Give work buffer a new empty array
    std::swap(OBEy[OBpos], WBEy[WBpos]);    // And give output buffer the computed data
    std::swap(OBHz[OBpos], WBHz[WBpos]);
}

void Field::updateFields()
{
    for(int n=0; n<settings.steps; n++) {
        int Old = (n-1+sizeWorkBuffer)%sizeWorkBuffer;      // Old time
        int New = n%sizeWorkBuffer;                         // New time

        mutex->lock();
        (*threadCounter)++;
        if(*threadCounter < settings.numberOfThreads) {
            computation->wait(mutex);              // Wait for the other threads to synchronize
        }
        else {
            *threadCounter = 0;
            if(n%settings.sampleDistance == 0)
                transferSample(n);        // Transfer sample from work buffer to output buffer
            computation->wakeAll();
        }
        mutex->unlock();

        for(int m=0; m<patch.size(); m++) {
           for(int i=patch[m].iMin; i<patch[m].iMax; i++) {
                for(int j=patch[m].jMin; j<patch[m].jMax; j++) {
                    double C = sigmaU[i][j]*dt/(2*epsU[i][j]);      // Add 1 to the time index of Hz and 0.5 to that of Ex and Ey
                    WBEx[New][i][j] = (1-C)/(1+C)*WBEx[Old][i][j] + dt/epsU[i][j]/(1+C)*((WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy);      // Add 0.5 to the second index (j)

                    C = sigmaR[i][j]*dt/(2*epsR[i][j]);
                    WBEy[New][i][j] = (1-C)/(1+C)*WBEy[Old][i][j] - dt/epsR[i][j]/(1+C)*((WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx);      // Add 0.5 to the first index (i)

                    for(int k=0; k<TFSF.size(); k++) {
                        if(i == TFSF[k].i0-1 && j >= TFSF[k].j0 && j <= TFSF[k].j1) {                   // Left boundary
                            TFSF[k].computeFields(i+1, j, n);                                           // +1 to i, because i = i0-1 and has to be == i0
                            WBEy[New][i][j] += dt/epsR[i][j]*TFSF[k].Hz/dx;
                        }

                        if(i == TFSF[k].i1 && j >= TFSF[k].j0 && j <= TFSF[k].j1) {                     // Right boundary
                            TFSF[k].computeFields(i, j, n);
                            WBEy[New][i][j] -= dt/epsR[i][j]*TFSF[k].Hz/dx;
                        }

                        if(j == TFSF[k].j0-1 && i >= TFSF[k].i0 && i <= TFSF[k].i1) {                   // Bottom boundary
                            TFSF[k].computeFields(i, j+1, n);
                            WBEx[New][i][j] -= dt/epsU[i][j]*TFSF[k].Hz/dy;
                        }

                        if(j == TFSF[k].j1 && i >= TFSF[k].i0 && i <= TFSF[k].i1) {                     // Top boundary
                            TFSF[k].computeFields(i, j, n);
                            WBEx[New][i][j] += dt/epsU[i][j]*TFSF[k].Hz/dy;
                        }
                    }
                }
            }

           for(int k=0; k<current.size(); k++)
           {
               if(current[k].type == 's') {
                   if(current[k].i >= patch[m].iMin && current[k].i < patch[m].iMax && current[k].j >= patch[m].jMin && current[k].j < patch[m].jMax) {
                       double C = sigmaU[current[k].i][current[k].j]*dt/(2*epsU[current[k].i][current[k].j]);
                       if(current[k].polarization == 'x')
                           WBEx[New][current[k].i][current[k].j] -= dt/epsU[current[k].i][current[k].j]/(1+C)*current[k].magnitude*sin(2*M_PI*current[k].frequency*1E6*n*dt);
                       else
                           WBEy[New][current[k].i][current[k].j] -= dt/epsR[current[k].i][current[k].j]/(1+C)*current[k].magnitude*sin(2*M_PI*current[k].frequency*1E6*n*dt);
                   }
               }
               else {
                   if(current[k].iG >= patch[m].iMin && current[k].iG < patch[m].iMax && current[k].jG >= patch[m].jMin && current[k].jG < patch[m].jMax) {
                       double C = sigmaU[current[k].iG][current[k].jG]*dt/(2*epsU[current[k].iG][current[k].jG]);
                       double argument = n*dt - current[k].timeDelay - 3*current[k].pulseWidth;
                       double value = exp(-argument*argument/(current[k].pulseWidth*current[k].pulseWidth))*sin(2*M_PI*current[k].frequencyG*1E6*argument)*current[k].magnitudeG;
                       if(current[k].polarizationG == 'x')
                           WBEx[New][current[k].iG][current[k].jG] -= dt/epsU[current[k].iG][current[k].jG]/(1+C)*value;
                       else
                           WBEy[New][current[k].iG][current[k].jG] -= dt/epsR[current[k].iG][current[k].jG]/(1+C)*value;
                   }
               }
           }

           for(int k=0; k<sensors.size(); k++)
           {
               if(sensors[k].i >= patch[m].iMin && sensors[k].i < patch[m].iMax && sensors[k].j >= patch[m].jMin && sensors[k].j < patch[m].jMax)
               {
                   sensors[k].Ex[n] = WBEx[New][sensors[k].i][sensors[k].j];
                   sensors[k].Ey[n] = WBEy[New][sensors[k].i][sensors[k].j];
               }
           }
        }

        for(int m=0; m<patch.size(); m++) {         // First evaluate the main grid
            for(int k=0; k<hsgSurfaces.size(); k++) {
                if(hsgSurfaces[k].iMin >= patch[m].iMin && hsgSurfaces[k].iMin < patch[m].iMax &&
                        hsgSurfaces[k].jMin >= patch[m].jMin && hsgSurfaces[k].jMin < patch[m].jMax)
                    hsgSurfaces[k].advanceE(n);    // Only update E if this is the responsible thread
            }
        }

        mutex->lock();
        (*threadCounter)++;
        if(*threadCounter < settings.numberOfThreads) {
            computation->wait(mutex);              // Wait for the other threads to synchronize
        }
        else {
            *threadCounter = 0;
            computation->wakeAll();
        }
        mutex->unlock();

        for(int m=0; m<patch.size(); m++) {
           for(int i=patch[m].iMin; i<patch[m].iMax; i++) {
                for(int j=patch[m].jMin; j<patch[m].jMax; j++) {
                    WBHz[New][i][j] = WBHz[Old][i][j] + dt/muC[i][j]*((WBEx[New][i][j]-WBEx[New][i][j-1])/dy - (WBEy[New][i][j]-WBEy[New][i-1][j])/dx);  // Add 1 to the time index of Hz, O.5 to Ex and Ey

                    for(int k=0; k<TFSF.size(); k++) {
                        if(i == TFSF[k].i0 && j >= TFSF[k].j0 && j <= TFSF[k].j1) {                    // Left boundary
                            TFSF[k].computeFields(i-0.5, j, n+0.5);
                            WBHz[New][i][j] += dt/muC[i][j]*TFSF[k].Ey/dx;
                        }

                        if(i == TFSF[k].i1 && j >= TFSF[k].j0 && j <= TFSF[k].j1) {                    // Right boundary
                            TFSF[k].computeFields(i+0.5, j, n+0.5);
                            WBHz[New][i][j] -= dt/muC[i][j]*TFSF[k].Ey/dx;
                        }

                        if(j == TFSF[k].j0 && i >= TFSF[k].i0 && i <= TFSF[k].i1) {                    // Bottom boundary
                            TFSF[k].computeFields(i, j-0.5, n+0.5);
                            WBHz[New][i][j] -= dt/muC[i][j]*TFSF[k].Ex/dy;
                        }

                        if(j == TFSF[k].j1 && i >= TFSF[k].i0 && i <= TFSF[k].i1) {                    // Top boundary
                            TFSF[k].computeFields(i, j+0.5, n+0.5);
                            WBHz[New][i][j] += dt/muC[i][j]*TFSF[k].Ex/dy;
                        }
                    }

//                    for(int k=0; k<current.size(); k++)
//                    {
//                        if(current[k].iG >= patch[m].iMin && current[k].iG < patch[m].iMax && current[k].jG >= patch[m].jMin && current[k].jG < patch[m].jMax) {
//                            double C = sigmaU[current[k].iG][current[k].jG]*dt/(2*epsU[current[k].iG][current[k].jG]);
//                            double argument = n*dt - current[k].timeDelay - 3*current[k].pulseWidth;
//                            double value = exp(-argument*argument/(current[k].pulseWidth*current[k].pulseWidth))*sin(2*M_PI*current[k].frequencyG*1E6*argument)*current[k].magnitudeG;

//                            WBHz[New][current[k].iG][current[k].jG] += dt/muC[i][j]*value/dy;
//                        }
//                    }

                    if(WBEx[New][i][j] > maxEx)
                        maxEx = WBEx[New][i][j];
                    if(WBEy[New][i][j] > maxEy)
                        maxEy = WBEy[New][i][j];

                    if(WBEx[New][i][j] < minEx)
                        minEx = WBEx[New][i][j];
                    if(WBEy[New][i][j] < minEy)
                        minEy = WBEy[New][i][j];

                    if(WBHz[New][i][j] > maxHz)
                        maxHz = WBHz[New][i][j];
                    if(WBHz[New][i][j] < minHz)
                        minHz = WBHz[New][i][j];
                }
           }

           for(int k=0; k<sensors.size(); k++)
           {
               if(sensors[k].i >= patch[m].iMin && sensors[k].i < patch[m].iMax && sensors[k].j >= patch[m].jMin && sensors[k].j < patch[m].jMax) {
                   sensors[k].Hz[n] = WBHz[New][sensors[k].i][sensors[k].j];
               }
           }
        }

        for(int m=0; m<patch.size(); m++) { // First evaluate the main grid
            for(int k=0; k<hsgSurfaces.size(); k++) {
                if(hsgSurfaces[k].iMin >= patch[m].iMin && hsgSurfaces[k].iMin < patch[m].iMax &&
                        hsgSurfaces[k].jMin >= patch[m].jMin && hsgSurfaces[k].jMin < patch[m].jMax) {

                    hsgSurfaces[k].advanceH(n);    // Only update H if this is the responsible thread
                    for(int p=0; p<sensors.size(); p++) {
                        if(sensors[p].i >= hsgSurfaces[k].iMin && sensors[p].i < hsgSurfaces[k].iMax && sensors[p].j >= hsgSurfaces[k].jMin && sensors[p].j < hsgSurfaces[k].jMax) {
                            int i = fmax((sensors[p].xpos - hsgSurfaces[k].FB->bottomLeft.x)/(dx/hsgSurfaces[k].FB->xRatio) - hsgSurfaces[k].FB->xRatio + 1, 0);
                            int j = fmax((sensors[p].ypos - hsgSurfaces[k].FB->bottomLeft.y)/(dy/hsgSurfaces[k].FB->yRatio) - hsgSurfaces[k].FB->yRatio + 1, 0);
                            sensors[p].Hz[n] = hsgSurfaces[k].FB->Hz(New, i, j);
//                            if(n == 10)
//                                qDebug() << i << j;
                        }
                    }
                }
            }
        }

        mutex->lock();
        (*threadCounter)++;
        if(*threadCounter < settings.numberOfThreads) {
            computation->wait(mutex);              // Wait for the other threads to synchronize
        }
        else {
            *threadCounter = 0;
            emit fieldUpdateFinished(n);
            computation->wakeAll();
        }
        mutex->unlock();

        if(n > settings.steps-2) {
            emit updateGUI(minEx, maxEx, minEy, maxEy, minHz, maxHz);
            emit finished();
        }
    }
}

void Field::shallowCopyFields(Field* a)
{
    this->WBEx = a->WBEx;
    this->WBEy = a->WBEy;
    this->WBHz = a->WBHz;
    this->OBEx = a->OBEx;
    this->OBEy = a->OBEy;
    this->OBHz = a->OBHz;
    this->epsR = a->epsR;
    this->epsU = a->epsU;
    this->muC = a->muC;
    this->sigmaR = a->sigmaR;
    this->sigmaU = a->sigmaU;
    this->TFSF = a->TFSF;
    this->sensors = a->sensors;
    this->current = a->current;
    this->hsgSurfaces = a->hsgSurfaces;
}

void Field::defineSources(const std::vector<currentSource> current)
{
    this->current = current;
    settings.computeDifferentials();
    for (int k=0; k<current.size(); k++) {
        this->current[k].i = round(current[k].xpos/settings.dx+settings.cellsX/2.0+settings.PMLlayers);      // Determine the final position
        this->current[k].j = round(current[k].ypos/settings.dy+settings.cellsY/2.0+settings.PMLlayers);

        this->current[k].iG = round(current[k].xposG/settings.dx+settings.cellsX/2.0+settings.PMLlayers);      // Determine the final position
        this->current[k].jG = round(current[k].yposG/settings.dy+settings.cellsY/2.0+settings.PMLlayers);
    }
}

void Field::defineMaterial(const std::vector<MaterialDefinition> &material)
{
    settings.computeDifferentials();
    dx = settings.dx;
    dy = settings.dy;
    for(int k=0; k<material.size(); k++) {
        pointInPolygon p;
        std::vector<Point> points;      // To prevent out of scope
        if(material[k].p.size() == 2)
        {
            points.push_back(Point(material[k].p[0].x, material[k].p[0].y));
            points.push_back(Point(material[k].p[0].x, material[k].p[1].y));
            points.push_back(Point(material[k].p[1].x, material[k].p[1].y));
            points.push_back(Point(material[k].p[1].x, material[k].p[0].y));
            p.vertices = &points;
        }
        else
            p.vertices = &material[k].p;

        for(int i=settings.PMLlayers; i<settings.PMLlayers+settings.cellsX; i++) {
            for(int j=settings.PMLlayers; j<settings.PMLlayers+settings.cellsY; j++) {
                if(p.inPolygon((i-settings.PMLlayers-settings.cellsX/2.0+0.5)*dx, (j-settings.PMLlayers-settings.cellsY/2.0+0.5)*dy))
                {
                    muC[i][j] = material[k].mur*mu0;
                }

                if(p.inPolygon((i+0.5-settings.PMLlayers-settings.cellsX/2.0+0.5)*dx, (j-settings.PMLlayers-settings.cellsY/2.0+0.5)*dy))
                {
                    epsR[i][j] = material[k].epsr*epsilon0;
                    sigmaR[i][j] = material[k].sigma;
                }

                if(p.inPolygon((i-settings.PMLlayers-settings.cellsX/2.0+0.5)*dx, (j+0.5-settings.PMLlayers-settings.cellsY/2.0+0.5)*dy))
                {
                    epsU[i][j] = material[k].epsr*epsilon0;
                    sigmaU[i][j] = material[k].sigma;
                }
            }
        }

        if(material[k].YuMittra == true)        // Do this after the initialization of epsilon, mu and sigma
        {
            for(int i=settings.PMLlayers; i<settings.PMLlayers+settings.cellsX; i++) {
                for(int j=settings.PMLlayers; j<settings.PMLlayers+settings.cellsY; j++) {
                    double distanceX = p.distanceX((i+0.5-settings.PMLlayers)*dx - settings.sizeX/2.0, (j+0.5-settings.PMLlayers-settings.cellsY/2.0)*dy);     // Compute the x distance between the given point and the closest edge
                    double distanceY = p.distanceY((i+0.5-settings.PMLlayers-settings.cellsX/2.0)*dx, (j+0.5-settings.PMLlayers)*dy - settings.sizeY/2.0);     // Compute the x distance between the given point and the closest edge

                    if(std::abs(distanceX) < dx) {   // We're on the boundary
                        double distance1 = std::abs(distanceX);
                        double distance2 = dx - distance1;

                        if(distanceX > 0) {     // Left side
                            if(distance1 <= dx/2)
                                epsU[i+1][j] = (distance1*epsU[i][j] + distance2*epsU[i+1][j])/dx;
                            else
                                epsU[i+1][j] = (distance1*epsU[i+1][j] + distance2*epsU[i+2][j])/dx;
                        }
                        else {                  // Right side
                            if(distance1 < dx/2)
                                epsU[i][j] = (distance2*epsU[i][j] + distance1*epsU[i+1][j])/dx;
                            else
                                epsU[i][j] = (distance2*epsU[i-1][j] + distance1*epsU[i][j])/dx;
                        }
                    }

                    if(std::abs(distanceY) < dy) {
                        double distance1 = std::abs(distanceY);
                        double distance2 = dy - distance1;

                        if(distanceY < 0) {     // Top
                            if(distance1 < dy/2)
                                epsR[i][j] = (distance2*epsR[i][j] + distance1*epsR[i][j+1])/dy;
                            else
                                epsR[i][j] = (distance2*epsR[i][j-1] + distance1*epsR[i][j])/dy;
                        }
                        else {                  // Bottom
                            if(distance1 <= dy/2)
                                epsR[i][j+1] = (distance1*epsR[i][j] + distance2*epsR[i][j+1])/dy;
                            else
                                epsR[i][j+1] = (distance1*epsR[i][j+1] + distance2*epsR[i][j+2])/dy;
                        }
                    }
                }
            }
        }
    }
}
