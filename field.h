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

#ifndef FIELD_H
#define FIELD_H

#include "settings.h"
#include "area.h"
#include "math.h"
#include "currentsource.h"
#include "materialdefinition.h"
#include "planewave.h"
#include "sensordefinition.h"
#include "SGInterface.h"
#include "pointinpolygon.h"
#include <vector>
#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <cmath>
#include <qdebug.h>
#include <vector>
#include <algorithm>

class SGInterface;

class Field : public QObject
{
    Q_OBJECT
public:
    double ***WBEx=NULL, ***WBEy=NULL, ***WBHz=NULL, **epsR=NULL, **epsU=NULL;      // WB = workbuffer
    double ***OBEx=NULL, ***OBEy=NULL, ***OBHz=NULL;                                // OB = output buffer
    double **muC=NULL, **sigmaR=NULL, **sigmaU=NULL;
    int *threadCounter;
    double minEx=0, maxEx=0, minEy=0, maxEy=0, minHz=0, maxHz=0;
    int sizeWorkBuffer=4;
    double dx, dy, dt;

    std::vector<Area> patch;
    std::vector<PlaneWave> TFSF;        // Total field/scattered field
    std::vector<SensorDefinition> sensors;
    std::vector<currentSource> current;
    std::vector<SGInterface> hsgSurfaces;
    Settings settings;

    Field(Settings settings, QWaitCondition *waitCondition, QMutex* mutex, int* threadCounter);
    ~Field();
    void initFields();
    void deleteFields();

    void transferSample(int n);
    void computeDifferentials();
    void shallowCopyFields(Field *a);
    void defineSources(const std::vector<currentSource> current);
    void defineMaterial(const std::vector<MaterialDefinition> &material);

private:
    QMutex *mutex;
    QWaitCondition* computation;

public slots:
    void updateFields();

signals:
    void finished();
    void updateGUI(double, double, double, double, double, double);
    void fieldUpdateFinished(int n);
};

#endif // FIELD_H
