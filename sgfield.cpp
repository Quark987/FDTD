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

#include "SGField.h"
#include <ctime>
#include <iostream>

#define c           299792458
#define epsilon0    8.8541878176E-12     // 1.0519 op 10 samples/lambda
#define mu0         1.2566370614E-6

using namespace Eigen;

SGField::SGField(int xRatio, int yRatio, Settings settings, Point pA, Point pB, int sizeWorkBuffer)
{
    this->xRatio = xRatio;
    this->yRatio = yRatio;
    this->sizeWorkBuffer = sizeWorkBuffer;

    this->settings = settings;
    Point topRight = Point(std::max(pA.x, pB.x), std::max(pA.y, pB.y));
    Point bottomLeft = Point(std::min(pA.x, pB.x), std::min(pA.y, pB.y));

    this->topRight = Point(ceil(topRight.x/settings.dx)*settings.dx, ceil(topRight.y/settings.dy)*settings.dy);   // Align the surface to the grid
    this->bottomLeft = Point(ceil(bottomLeft.x/settings.dx)*settings.dx, ceil(bottomLeft.y/settings.dy)*settings.dy);

    cellsX = ceil((this->topRight.x - this->bottomLeft.x)/settings.dx);
    cellsY = ceil((this->topRight.y - this->bottomLeft.y)/settings.dy);

    sizeExx = xRatio*(cellsX-2)+2;
    sizeExy = yRatio*(cellsY-2)+1;
    sizeEyx = xRatio*(cellsX-2)+1;
    sizeEyy = yRatio*(cellsY-2)+2;
    sizeHzx = xRatio*(cellsX-2)+2;
    sizeHzy = yRatio*(cellsY-2)+2;

    sizeHz = sizeHzx*sizeHzy;
    sizeEx = sizeExx*sizeExy;
    sizeEy = sizeEyx*sizeEyy;

    settings.computeDifferentials();
    dx = settings.dx/xRatio;
    dy = settings.dy/yRatio;

    A = SparseMatrix<double>(sizeEx+sizeEy+sizeHz, sizeEx+sizeEy+sizeHz);
    B = SparseMatrix<double>(sizeEx+sizeEy+sizeHz, sizeEx+sizeEy+sizeHz);
    C = SparseMatrix<double>(sizeEx+sizeEy+sizeHz, sizeEx+sizeEy+sizeHz);
    A.reserve(VectorXi::Constant(sizeEx+sizeEy+sizeHz,5));
    B.reserve(VectorXi::Constant(sizeEx+sizeEy+sizeHz,5));
    C.reserve(VectorXi::Constant(sizeEx+sizeEy+sizeHz,5));
//    A.reserve(3*sizeEx+3*sizeEy+5*sizeHz);
//    B.reserve(3*sizeEx+3*sizeEy+sizeHz);
//    C.reserve(2*sizeEx+2*sizeEy);
    materialParameter = VectorXd(sizeEx+sizeEy+sizeHz);
    conductivity = VectorXd(sizeEx+sizeEy);

    for(int i=0; i<sizeHz; i++)
        materialParameter(i) = mu0;

    for(int i=sizeHz; i<sizeHz+sizeEx+sizeEy; i++)
        materialParameter(i) = epsilon0;

    for(int i=0; i<sizeEx+sizeEy; i++)
        conductivity(i) = 0;

    qDebug() << cellsX << cellsY << sizeExx << sizeExy << sizeEyx << sizeEyy << sizeHzx << sizeHzy << sizeHz+sizeEx+sizeEy;
//    qDebug() << indexHz(0,0) << indexHz(sizeHzx-1, sizeHzy-1)
//             << indexEx(0,0) << indexEx(sizeExx-1, sizeExy-1)
//             << indexEy(0,0) << indexEy(sizeEyx-1, sizeEyy-1);
}

double SGField::sigmaR(int i, int j)
{
    return conductivity(indexEy(i, j) - sizeHz);
}

double SGField::sigmaU(int i, int j)
{
    return conductivity(indexEx(i, j) - sizeHz);
}

double SGField::epsR(int i, int j)
{
    return materialParameter(indexEy(i, j));
}

double SGField::epsU(int i, int j)
{
    return materialParameter(indexEx(i, j));
}

double SGField::muC(int i, int j)
{
    return materialParameter(indexHz(i, j));
}

double SGField::Ex(int n, int i, int j)
{
    return (*WBf[n])(indexEx(i, j));
}

double SGField::Ey(int n, int i, int j)
{
    return (*WBf[n])(indexEy(i, j));
}

double SGField::Hz(int n, int i, int j)
{
    return (*WBf[n])(indexHz(i, j));
}

int SGField::indexHz(int i, int j)
{
    return i + j*sizeHzx;
}

int SGField::indexEx(int i, int j)
{
    return sizeHz + i + j*sizeExx;
}

int SGField::indexEy(int i, int j)
{
    return sizeHz + sizeEx + i + j*sizeEyx;
}

void SGField::initUpdateMatrices(const std::vector<MaterialDefinition> &material)
{
    if(WBf != NULL) {
        for(int i=0; i<sizeWorkBuffer; i++)
            delete[] WBf[i];
    }
    delete[] WBf;

    if(OBf != NULL) {
        for(int i=0; i<settings.steps/settings.sampleDistance; i++)
            delete[] OBf[i];
    }
    delete[] OBf;

    settings.computeDifferentials();
    this->dt = settings.dt;

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
            p.vertices = &(material[k].p);

        for(int i=0; i<sizeHzx; i++) {
            for(int j=0; j<sizeHzy; j++) {
                double testX = bottomLeft.x + settings.dx/2 + (i+(xRatio-1)/2.0)*dx;
                double testY = bottomLeft.y + settings.dy/2 + (j+(yRatio-1)/2.0)*dy;

                if(p.inPolygon(testX, testY))
                    materialParameter(indexHz(i, j)) = material[k].mur*mu0;

                if(i < sizeHzx-1 && p.inPolygon(testX+dx/2.0, testY)) {
                    materialParameter(indexEy(i, j)) = material[k].epsr*epsilon0;
                    conductivity(indexEy(i, j) - sizeHz) = material[k].sigma;
                }

                if(j < sizeHzy-1 && p.inPolygon(testX, testY+dy/2.0)) {
                    materialParameter(indexEx(i, j)) = material[k].epsr*epsilon0;
                    conductivity(indexEx(i, j) - sizeHz) = material[k].sigma;
                }
            }
        }
    }

    // Initialize update matrices
    for(int i=0; i<sizeExx; i++) {
        for(int j=0; j<sizeExy; j++) {
            double factor = 1;
            if(j==0 || j==sizeExy-1)
                factor = 0.5*(1+yRatio);

            A.insert(indexEx(i, j), indexEx(i, j)) = 1;
            B.insert(indexEx(i, j), indexEx(i, j)) = (2*epsU(i, j) - sigmaU(i, j)*dt)/(2*epsU(i, j) + sigmaU(i, j)*dt);

//            B.insert(indexEx(i, j), indexHz(i, j)) = -2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(dy*factor);        // unconditional stability
//            B.insert(indexEx(i, j), indexHz(i, j+1)) = 2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(dy*factor);       // along x
            A.insert(indexEx(i, j), indexHz(i, j+1)) = -2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(4*dy*factor);
            B.insert(indexEx(i, j), indexHz(i, j+1)) = 2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(2*dy*factor);
            C.insert(indexEx(i, j), indexHz(i, j+1)) = 2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(4*dy*factor);

            A.insert(indexEx(i, j), indexHz(i, j)) = 2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(4*dy*factor);
            B.insert(indexEx(i, j), indexHz(i, j)) = -2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(2*dy*factor);
            C.insert(indexEx(i, j), indexHz(i, j)) = -2*dt/(2*epsU(i, j) + sigmaU(i, j)*dt)/(4*dy*factor);
        }
    }

    for(int i=0; i<sizeEyx; i++) {
        for(int j=0; j<sizeEyy; j++) {
            double factor = 1;
            if(i==0 || i==sizeEyx-1)
                factor = 0.5*(1+xRatio);

            A.insert(indexEy(i, j), indexEy(i, j)) = 1;
            B.insert(indexEy(i, j), indexEy(i, j)) = (2*epsR(i, j) - sigmaR(i, j)*dt)/(2*epsR(i, j) + sigmaR(i, j)*dt);

//            B.insert(indexEy(i, j), indexHz(i+1, j)) = -2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(dx*factor);
//            B.insert(indexEy(i, j), indexHz(i, j)) = 2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(dx*factor);
            A.insert(indexEy(i, j), indexHz(i+1, j)) = 2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(4*dx*factor);
            B.insert(indexEy(i, j), indexHz(i+1, j)) = -2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(2*dx*factor);
            C.insert(indexEy(i, j), indexHz(i+1, j)) = -2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(4*dx*factor);

            A.insert(indexEy(i, j), indexHz(i, j)) = -2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(4*dx*factor);
            B.insert(indexEy(i, j), indexHz(i, j)) = 2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(2*dx*factor);
            C.insert(indexEy(i, j), indexHz(i, j)) = 2*dt/(2*epsR(i, j) + sigmaR(i, j)*dt)/(4*dx*factor);
        }
    }

    for(int i=0; i<sizeHzx; i++) {
        for(int j=0; j<sizeHzy; j++) {
            double factor;

            A.insert(indexHz(i, j), indexHz(i, j)) = 1;
            B.insert(indexHz(i, j), indexHz(i, j)) = 1;

            if(j==0 || j==sizeHzy-1)
                factor = yRatio;
            else
                factor = 1;
            if(j < sizeExy)
                A.insert(indexHz(i, j), indexEx(i, j)) = -dt/(muC(i, j)*dy*factor);
            if(j > 0)
                A.insert(indexHz(i, j), indexEx(i, j-1)) = dt/(muC(i, j)*dy*factor);

            if(i==0 || i==sizeHzx-1)
                factor = xRatio;
            else
                factor = 1;
            if(i < sizeEyx)
                A.insert(indexHz(i, j), indexEy(i, j)) = dt/(muC(i, j)*dx*factor);
            if(i > 0)
                A.insert(indexHz(i, j), indexEy(i-1, j)) = -dt/(muC(i, j)*dx*factor);
        }
    }

    qDebug() << "Matrix rank: " << A.rows();
    A.makeCompressed();

    s = VectorXd::Zero(sizeEx+sizeEy+sizeHz);
    WBf = new VectorXd*[sizeWorkBuffer];
    for(int n=0; n<sizeWorkBuffer; n++) {
        WBf[n] = new VectorXd(sizeEx+sizeEy+sizeHz);
        WBf[n]->setZero(sizeEx+sizeEy+sizeHz);
    }

    OBf = new VectorXd*[(int)std::ceil((double)settings.steps/settings.sampleDistance)];
    for(int n=0; n<std::ceil((double)settings.steps/settings.sampleDistance); n++)
        OBf[n] = new VectorXd(sizeEx+sizeEy+sizeHz);

    solver.preconditioner().setDroptol(1E-50);
    solver.preconditioner().setFillfactor(sqrt(xRatio*yRatio)+50);
    solver.compute(A);
    solver.setMaxIterations(static_cast<int>(1000));
    solver.setTolerance(1E-100);
}

void SGField::transferSample(int n) {
    int WBpos = n%sizeWorkBuffer;
    int OBpos = n/settings.sampleDistance;

    std::swap(OBf[OBpos], WBf[WBpos]);
}

void SGField::updateFields(int n)
{
    int Old2 = (n-2+sizeWorkBuffer)%sizeWorkBuffer;
    int Old1 = (n-1+sizeWorkBuffer)%sizeWorkBuffer;      // Old time
    int New = n%sizeWorkBuffer;                         // New time

    if(n%settings.sampleDistance == 0)
        transferSample(n);                  // Transfer sample from work buffer to output buffer

//    SparseLU<SparseMatrix<double, ColMajor>, COLAMDOrdering<int> > solver;
//    solver.compute(A);

//    *WBf[New] = solver.solve(B*(*WBf[Old1]) + C*(*WBf[Old2]) + s);
    *WBf[New] = solver.solveWithGuess(B*(*WBf[Old1]) + C*(*WBf[Old2]) + s, *WBf[Old1]);

//    *WBf[New] = U1*(*WBf[Old1]) + U2*(*WBf[Old2]) + Ainv*s;

//    qDebug() << n << solver.iterations() << solver.error();

    for(int k=sizeHz; k<sizeHz+sizeEx; k++)
    {
        if(maxEx < (*WBf[New])(k))
            maxEx = (*WBf[New])(k);
        if(minEx > (*WBf[New])(k))
            minEx = (*WBf[New])(k);
    }

    for(int k=sizeHz+sizeEx; k<sizeHz+sizeEx+sizeEy; k++)
    {
        if(maxEy < (*WBf[New])(k))
            maxEy = (*WBf[New])(k);
        if(minEy > (*WBf[New])(k))
            minEy = (*WBf[New])(k);
    }

    for(int k=0; k<sizeHz; k++)
    {
        if(maxHz < (*WBf[New])(k))
            maxHz = (*WBf[New])(k);
        if(minHz > (*WBf[New])(k))
            minHz = (*WBf[New])(k);
    }
    n++;
}
