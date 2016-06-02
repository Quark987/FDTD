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

#ifndef SENSOR_H
#define SENSOR_H

#include <QMainWindow>
#include "sensordefinition.h"
#include <fftw3.h>
#include <qcustomplot.h>
#include "settings.h"
#include "inputrange.h"
#include <QStandardItem>
#include <QStandardItemModel>

namespace Ui {
class SensorResult;
}

class SensorResult : public QMainWindow
{
    Q_OBJECT

public:
    std::vector<SensorDefinition> *a = NULL;
    QCustomPlot *plot;      // Pointer to the active plot
    Settings *settings;
    QStandardItem *sensor;      // Pointer to the last added sensor, the other pointers are stored in the treeview
    enum PlotType {Time=0, Magnitude, Phase};
    enum FieldIndex {Ex=0, Ey, Hz};

    explicit SensorResult(std::vector<SensorDefinition> &a, QWidget *parent = 0);
    ~SensorResult();
    void closeEvent(QCloseEvent *event);
    void plotMagnitude(int sensorIndex, FieldIndex field);
    void plotPhase(int sensorIndex, FieldIndex field);
    void plotTime(int sensorIndex, FieldIndex field);
    bool eventFilter(QObject *object, QEvent *);

private slots:
    void resize(QMouseEvent*);
    void mouseWheel();
    void mousePress(QMouseEvent*);
    void selectionChanged();
    void exportClicked();
    void exportSpectrum();
    void exportTime();
    void axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart);
    void legendDoubleClick(QCPLegend *, QCPAbstractLegendItem *);
    void contextMenuRequest(QPoint);
    void removeSelectedGraph();
    void moveLegend();
    void removeAllGraphs();
    void changeColor();
    void saveReference();
    void loadReference();
    void addSensor(QString title);      // Add a new sensor to the treeview, with accompanying graphs
    void addItem(QString title);        // Used in addSensor to populate the tree node with 3 fields
    void checkboxChanged(QStandardItem*);

private:
    QStandardItemModel *list;
    Ui::SensorResult *ui;
};

#endif // SENSOR_H
