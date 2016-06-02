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

#include "pmlboundary.h"
#include <math.h>
#include <qdebug.h>

#define c           299792458
#define epsilon0    8.8541878176E-12
#define mu0         1.2566370614E-6
#define Z1          mu0/epsilon0        // Free space impedance squared

PMLBoundary::PMLBoundary(Settings settings, QWaitCondition *waitCondition, QMutex *mutex, int *threadCounter)
{
    this->settings = settings;
    this->computation = waitCondition;
    this->mutex = mutex;
    this->threadCounter = threadCounter;
}

PMLBoundary::~PMLBoundary()
{
    for(int k=0; k<8; k++) {
        for(int n=0; n<sizeWorkBuffer; n++) {
            for(int i=0; i<patch[k].iMax - patch[k].iMin; i++) {
                delete[] Hzx[k][n][i];
                delete[] Hzy[k][n][i];
            }
            delete[] Hzx[k][n];
            delete[] Hzy[k][n];
        }
        delete[] Hzx[k];
        delete[] Hzy[k];
    }
    delete[] Hzx;
    delete[] Hzy;
    delete[] sigmaX;
    delete[] sigmaY;
    delete[] sigmaX2;
    delete[] sigmaY2;
}

void PMLBoundary::defineBoundary()
{
    patch.push_back(Area(1, settings.PMLlayers, 1, settings.PMLlayers));                                                        // 0, Left lower corner, keep one row at either side for PEC
    patch.push_back(Area(settings.PMLlayers, settings.PMLlayers+settings.cellsX, 1, settings.PMLlayers));                       // 1, Bottom
    patch.push_back(Area(settings.PMLlayers+settings.cellsX, 2*settings.PMLlayers+settings.cellsX-1, 1, settings.PMLlayers));   // 2, Right lower corner
    patch.push_back(Area(1, settings.PMLlayers, settings.PMLlayers, settings.PMLlayers+settings.cellsY));                       // 3, Left
    patch.push_back(Area(settings.PMLlayers+settings.cellsX, 2*settings.PMLlayers+settings.cellsX-1, settings.PMLlayers, settings.PMLlayers+settings.cellsY));  // 4, Right
    patch.push_back(Area(1, settings.PMLlayers, settings.PMLlayers+settings.cellsY, 2*settings.PMLlayers+settings.cellsY-1));                                   // 5, Upper left
    patch.push_back(Area(settings.PMLlayers, settings.PMLlayers+settings.cellsX, settings.PMLlayers+settings.cellsY, 2*settings.PMLlayers+settings.cellsY-1));  // 6, Top
    patch.push_back(Area(settings.PMLlayers+settings.cellsX, 2*settings.PMLlayers+settings.cellsX-1, settings.PMLlayers+settings.cellsY, 2*settings.PMLlayers+settings.cellsY-1));  // 7, Upper right
}

void PMLBoundary::initBoundary()
{
    defineBoundary();
    Hzx = new double***[8];            // i = x index, j = y index
    Hzy = new double***[8];

    for(int k=0; k<8; k++) {
        Hzx[k] = new double**[sizeWorkBuffer];
        Hzy[k] = new double**[sizeWorkBuffer];
        for(int n=0; n<field->sizeWorkBuffer; n++) {
            Hzx[k][n] = new double*[patch[k].iMax - patch[k].iMin];
            Hzy[k][n] = new double*[patch[k].iMax - patch[k].iMin];
            for(int i=0; i<patch[k].iMax - patch[k].iMin; i++) {
                Hzx[k][n][i] = new double[patch[k].jMax - patch[k].jMin];
                Hzy[k][n][i] = new double[patch[k].jMax - patch[k].jMin];
                for(int j=0; j<patch[k].jMax - patch[k].jMin; j++) {
                    Hzx[k][n][i][j] = 0;
                    Hzy[k][n][i][j] = 0;
                }
            }
        }
    }
    sigmaX = new double[settings.PMLlayers];
    sigmaY = new double[settings.PMLlayers];
    sigmaX2 = new double[settings.PMLlayers];
    sigmaY2 = new double[settings.PMLlayers];

    settings.computeDifferentials();
    dx = settings.dx;
    dy = settings.dy;
    dt = settings.dt;
    for(int k=0; k<settings.PMLlayers; k++) {
        double C = pow(double(k)/(settings.PMLlayers-1), settings.m);
        sigmaX[k] = C*settings.sigmaXMax;
        sigmaY[k] = C*settings.sigmaYMax;

        C = pow((k+0.5)/(settings.PMLlayers-1), settings.m);
        sigmaX2[k] = C*settings.sigmaXMax;     // Shifted over dx/2
        sigmaY2[k] = C*settings.sigmaYMax;

//        qDebug() << sigmaX[k] << sigmaY[k] << sigmamX[k] << sigmamY[k];
    }
}

void PMLBoundary::mapFields(Field *a)
{
    this->field = a;
    this->sizeWorkBuffer = a->sizeWorkBuffer;
    this->WBEx = a->WBEx;
    this->WBEy = a->WBEy;
    this->WBHz = a->WBHz;
    this->epsR = a->epsR;
    this->epsU = a->epsU;
    this->muC = a->muC;
}

void PMLBoundary::updateFields()
{
    // Hz in the regular domain is mapped to Hzy
    for(int n=0; n<settings.steps; n++) {       // Loop over time
        int Old = (n-1+sizeWorkBuffer)%sizeWorkBuffer;      // Old time
        int New = n%sizeWorkBuffer;                         // New time

        mutex->lock();
        (*threadCounter)++;
        if(*threadCounter < settings.numberOfThreads) {
            computation->wait(mutex);              // Wait for the other threads to synchronize
        }
        else {
            *threadCounter = 0;
            if(n%field->settings.sampleDistance == 0)
                field->transferSample(n);        // Transfer sample from work buffer to output buffer
            computation->wakeAll();
        }
        mutex->unlock();

        for(int m=0; m<8; m++) {                // Loop over boundary
            for(int i=patch[m].iMin; i<patch[m].iMax; i++) {
                for(int j=patch[m].jMin; j<patch[m].jMax; j++) {
                    int i0 = i-patch[m].iMin, i1 = patch[m].iMax-i-1;
                    int j0 = j-patch[m].jMin, j1 = patch[m].jMax-j-1;   // j0 is increasing with j, j1 decreases with j
                    double C1, C2;

                    switch(m) {
                    case 0:
                        C1 = (2*epsilon0-dt*sigmaY[j1])/(2*epsilon0+dt*sigmaY[j1]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaY[j1]);
                        WBEx[New][i][j] = C1*WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C1 = (2*epsilon0-dt*sigmaX[i1])/(2*epsilon0+dt*sigmaX[i1]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaX[i1]);
                        WBEy[New][i][j] = C1*WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 1:
                        C1 = (2*epsilon0-dt*sigmaY[j1])/(2*epsilon0+dt*sigmaY[j1]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaY[j1]);
                        WBEx[New][i][j] = C1*WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C2 = dt/epsilon0;
                        WBEy[New][i][j] = WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 2:
                        C1 = (2*epsilon0-dt*sigmaY[j1])/(2*epsilon0+dt*sigmaY[j1]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaY[j1]);
                        WBEx[New][i][j] = C1*WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C1 = (2*epsilon0-dt*sigmaX2[i0])/(2*epsilon0+dt*sigmaX2[i0]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaX2[i0]);
                        WBEy[New][i][j] = C1*WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 3:
                        C2 = dt/epsilon0;
                        WBEx[New][i][j] = WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C1 = (2*epsilon0-dt*sigmaX[i1])/(2*epsilon0+dt*sigmaX[i1]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaX[i1]);
                        WBEy[New][i][j] = C1*WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 4:
                        C2 = dt/epsilon0;
                        WBEx[New][i][j] = WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C1 = (2*epsilon0-dt*sigmaX2[i0])/(2*epsilon0+dt*sigmaX2[i0]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaX2[i0]);
                        WBEy[New][i][j] = C1*WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 5:
                        C1 = (2*epsilon0-dt*sigmaY2[j0])/(2*epsilon0+dt*sigmaY2[j0]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaY2[j0]);
                        WBEx[New][i][j] = C1*WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C1 = (2*epsilon0-dt*sigmaX[i1])/(2*epsilon0+dt*sigmaX[i1]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaX[i1]);
                        WBEy[New][i][j] = C1*WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 6:
                        C1 = (2*epsilon0-dt*sigmaY2[j0])/(2*epsilon0+dt*sigmaY2[j0]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaY2[j0]);
                        WBEx[New][i][j] = C1*WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C2 = dt/epsilon0;
                        WBEy[New][i][j] = WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    case 7:
                        C1 = (2*epsilon0-dt*sigmaY2[j0])/(2*epsilon0+dt*sigmaY2[j0]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaY2[j0]);
                        WBEx[New][i][j] = C1*WBEx[Old][i][j] + C2*(WBHz[Old][i][j+1]-WBHz[Old][i][j])/dy;

                        C1 = (2*epsilon0-dt*sigmaX2[i0])/(2*epsilon0+dt*sigmaX2[i0]);
                        C2 = 2*dt/(2*epsilon0+dt*sigmaX2[i0]);
                        WBEy[New][i][j] = C1*WBEy[Old][i][j] - C2*(WBHz[Old][i+1][j]-WBHz[Old][i][j])/dx;
                        break;
                    }
                }
            }
        }

        mutex->lock();
        (*threadCounter)++;
        if(*threadCounter < settings.numberOfThreads)
            computation->wait(mutex);              // Wait for the other threads to synchronize
        else {
            *threadCounter = 0;
            computation->wakeAll();
        }
        mutex->unlock();

        for(int m=0; m<8; m++) {                // Loop over boundary
            for(int i=patch[m].iMin; i<patch[m].iMax; i++) {
                for(int j=patch[m].jMin; j<patch[m].jMax; j++) {
                    int i0 = i-patch[m].iMin, i1 = patch[m].iMax-i-1;
                    int j0 = j-patch[m].jMin, j1 = patch[m].jMax-j-1;
                    double C1, C2;

                    switch(m) {
                    case 0:
                        C1 = (2*mu0-dt*sigmaY2[j1]*Z1)/(2*mu0+dt*sigmaY2[j1]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaY2[j1]*Z1);
                        Hzy[m][New][i0][j0] = C1*Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C1 = (2*mu0-dt*sigmaX2[i1]*Z1)/(2*mu0+dt*sigmaX2[i1]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaX2[i1]*Z1);
                        Hzx[m][New][i0][j0] = C1*Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 1:
                        C1 = (2*mu0-dt*sigmaY2[j1]*Z1)/(2*mu0+dt*sigmaY2[j1]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaY2[j1]*Z1);
                        Hzy[m][New][i0][j0] = C1*Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C2 = dt/mu0;
                        Hzx[m][New][i0][j0] = Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 2:
                        C1 = (2*mu0-dt*sigmaY2[j1]*Z1)/(2*mu0+dt*sigmaY2[j1]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaY2[j1]*Z1);
                        Hzy[m][New][i0][j0] = C1*Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C1 = (2*mu0-dt*sigmaX[i0]*Z1)/(2*mu0+dt*sigmaX[i0]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaX[i0]*Z1);
                        Hzx[m][New][i0][j0] = C1*Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 3:
                        C2 = dt/mu0;
                        Hzy[m][New][i0][j0] = Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C1 = (2*mu0-dt*sigmaX2[i1]*Z1)/(2*mu0+dt*sigmaX2[i1]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaX2[i1]*Z1);
                        Hzx[m][New][i0][j0] = C1*Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 4:
                        C2 = dt/mu0;
                        Hzy[m][New][i0][j0] = Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C1 = (2*mu0-dt*sigmaX[i0]*Z1)/(2*mu0+dt*sigmaX[i0]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaX[i0]*Z1);
                        Hzx[m][New][i0][j0] = C1*Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 5:
                        C1 = (2*mu0-dt*sigmaY[j0]*Z1)/(2*mu0+dt*sigmaY[j0]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaY[j0]*Z1);
                        Hzy[m][New][i0][j0] = C1*Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C1 = (2*mu0-dt*sigmaX2[i1]*Z1)/(2*mu0+dt*sigmaX2[i1]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaX2[i1]*Z1);
                        Hzx[m][New][i0][j0] = C1*Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 6:
                        C1 = (2*mu0-dt*sigmaY[j0]*Z1)/(2*mu0+dt*sigmaY[j0]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaY[j0]*Z1);
                        Hzy[m][New][i0][j0] = C1*Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C2 = dt/mu0;
                        Hzx[m][New][i0][j0] = Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    case 7:
                        C1 = (2*mu0-dt*sigmaY[j0]*Z1)/(2*mu0+dt*sigmaY[j0]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaY[j0]*Z1);
                        Hzy[m][New][i0][j0] = C1*Hzy[m][Old][i0][j0] + C2*(WBEx[New][i][j] - WBEx[New][i][j-1])/dy;

                        C1 = (2*mu0-dt*sigmaX[i0]*Z1)/(2*mu0+dt*sigmaX[i0]*Z1);
                        C2 = 2*dt/(2*mu0+dt*sigmaX[i0]*Z1);
                        Hzx[m][New][i0][j0] = C1*Hzx[m][Old][i0][j0] - C2*(WBEy[New][i][j] - WBEy[New][i-1][j])/dx;
                        WBHz[New][i][j] = Hzx[m][New][i0][j0] + Hzy[m][New][i0][j0];
                        break;
                    }
                }
            }
        }

        mutex->lock();
        (*threadCounter)++;
        if(*threadCounter < settings.numberOfThreads)
            computation->wait(mutex);              // Wait for the other threads to synchronize
        else {
            *threadCounter = 0;
            emit fieldUpdateFinished(n);
            computation->wakeAll();
        }
        mutex->unlock();

        if(n > settings.steps-2) {
            emit updateGUI(0, 0, 0, 0, 0, 0);
            emit finished();
        }
    }
}
