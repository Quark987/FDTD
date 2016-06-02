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

#ifndef SGField_H
#define SGField_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/OrderingMethods>
#include <Eigen/IterativeLinearSolvers>
#include <vector>
#include <QWaitCondition>
#include <QMutex>
#include <QDebug>
#include <iostream>
#include "pointinpolygon.h"
#include "materialdefinition.h"
#include "settings.h"

using namespace Eigen;

class SGField : public QObject
{
    Q_OBJECT
public:
    SGField(int xRatio, int yRatio, Settings settings, Point pA, Point pB, int sizeWorkBuffer);
    void initMaterial();
    void initUpdateMatrices(const std::vector<MaterialDefinition> &material);
    void updateFields(int n);
    void transferSample(int n);
    double Ex(int n, int i, int j);     // With correction for separation and padding distance (for plot purposes)
    double Ey(int n, int i, int j);
    double Hz(int n, int i, int j);
    int indexEx(int i, int j);
    int indexEy(int i, int j);
    int indexHz(int i, int j);
    double muC(int i, int j);
    double sigmaR(int i, int j);
    double sigmaU(int i, int j);
    double epsR(int i, int j);
    double epsU(int i, int j);

    Point bottomLeft, topRight;
    int sizeWorkBuffer;
    int cellsX, cellsY;
    int xRatio, yRatio;
    int sizeEy, sizeEx, sizeHz;
    int sizeEyx, sizeEyy, sizeExx, sizeExy, sizeHzx, sizeHzy;       // Size Ey along x = Eyx
    double dx, dy;
    double minEx=0, maxEx=0, minEy=0, maxEy=0, minHz=0, maxHz=0;
    Settings settings;
//    std::vector<VectorXd> WBf, OBf, s;            // Work buffer fields and output buffer fields
    VectorXd **WBf=NULL, **OBf=NULL;            // Work buffer fields and output buffer fields
    VectorXd s;
    SparseMatrix<double, ColMajor> A, B, C;
    MatrixXd Ainv, U1, U2;                     // Af(n+1) = Bf(n) + s, s=sources, U = update matrix (=Ainv*B)
    VectorXd materialParameter, conductivity;
    Eigen::BiCGSTAB<SparseMatrix<double>, Eigen::IncompleteLUT<double> > solver;
    double dt;

signals:
    void finished();
    void updateGUI(double, double, double, double, double, double);
    void fieldUpdateFinished(int n);
};

#endif // SGField_H
