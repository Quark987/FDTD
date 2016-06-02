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

#ifndef PMLBOUNDARY_H
#define PMLBOUNDARY_H

#include <vector>
#include <QObject>
#include "area.h"
#include "settings.h"
#include "field.h"


class PMLBoundary : public QObject
{
    Q_OBJECT
public:
    double ***WBEx=NULL, ***WBEy=NULL, ***WBHz=NULL, **epsR=NULL, **epsU=NULL;      // WB = workbuffer
    double **muC=NULL, *sigmaX=NULL, *sigmaY=NULL, *sigmaX2=NULL, *sigmaY2=NULL;
    double ****Hzx=NULL, ****Hzy=NULL;
    int* threadCounter;
    int sizeWorkBuffer;
    double dx, dy, dt;

    std::vector<Area> patch;        // Order: LU, T, RU, L, R, LB, B, RB (LU = left upper, T = top, ...)
    Settings settings;
    PMLBoundary(Settings settings, QWaitCondition *waitCondition, QMutex* mutex, int* threadCounter);
    ~PMLBoundary();

    void defineBoundary();
    void mapFields(Field *a);
    void initBoundary();

public slots:
    void updateFields();

private:
    QMutex *mutex;
    QWaitCondition* computation;
    Field *field;

signals:
    void fieldUpdateFinished(int n);
    void updateGUI(double, double, double, double, double, double);
    void finished();
};

#endif // PMLBOUNDARY_H
