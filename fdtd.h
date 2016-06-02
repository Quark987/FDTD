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

#ifndef FDTD_H
#define FDTD_H

#include <QMainWindow>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>                   // qDebug() << ...
#include <QPen>
#include <math.h>
#include <algorithm>
#include <vector>
#include "preferences.h"
#include "settings.h"
#include "field.h"
#include "pmlboundary.h"
#include "materialdefinition.h"
#include "materialsettings.h"
#include "currentsource.h"
#include "sourcesettings.h"
#include "planewave.h"
#include "tfsfsettings.h"
#include "sensordefinition.h"
#include "sensorsettings.h"
#include "sensorresult.h"
#include "SGSettings.h"
#include "SGInterface.h"
#include "inputrange.h"

class QCPColorMap;
class QCPColorScale;

namespace Ui {
class FDTD;
}

class FDTD : public QMainWindow
{
    Q_OBJECT
    bool startup = true;
    QCPColorMap *colorMap;                      // This is what is plotted
    QCPColorScale *colorScale;                  // The scale next to it
    Preferences *preferencesWindow = NULL;      // Pointer to the preferences window
    SourceSettings *sourceWindow = NULL;        // Pointer to a source window
    MaterialSettings *materialWindow = NULL;    // Pointer to a material window
    TFSFSettings *TFSFWindow = NULL;            // Pointer to a total field/scattered field window
    SensorSettings *sensorWindow = NULL;        // Pointer to a sensor window
    SensorResult *sensorResult = NULL;          // Pointer to the result window of a sensor
    SGSettings *hsgWindow = NULL;               // Pointer to define the settings of the subgridded region
    Settings settings;                          // This stores the settings set in the preferences
    Field *field=NULL;                          // The goal is to cut this into pieces and give every piece to another thread
    std::vector<Field*> interior;               // This stores the cut up pieces of "field"
    PMLBoundary *boundary=NULL;                 // Pointer to the boundary (currently PML is always single threaded)
    std::vector<currentSource> currentSources;  // This stores the sources defined in the source window
    std::vector<MaterialDefinition> materials;  // This stores the materials defined in the material window
    std::vector<PlaneWave> TFSF;                // This stores the plane wave used in total field/scattered field
    std::vector<SensorDefinition> sensors;      // This stores the settings of the sensors
    std::vector<SGInterface> hsgSurfaces;

    double minEx=0, maxEx=0, minEy=0, maxEy=0, minHz=0, maxHz=0;
    double dx, dy, dt;              // Necessary to compute distances in the grid
    int threadCounter;              // Used to synchronize threads

    std::vector<Point> points;      // Variable which holds the points during drawing
    int numberOfPoints=0;
    bool plotted = false;
    char type;                      // p: point, s: square, n: n-gon

    QThread** threadPool;

public:
    void showResults();             // After computation, run this function to visualize the result
    void drawGrid();
    void drawPMLGrid();
    void drawField(int timeIndex);
    void drawStructures();
    void drawSources();
    void drawSensors();
    void clearPlot();
    void setAxisRange();

    explicit FDTD(QWidget *parent = 0);
    ~FDTD();

public slots:
    void computationsAreDone(double minEx, double maxEx, double minEy, double maxEy, double minHz, double maxHz);
    void fieldUpdateFinished(int n);
    void preferencesClosed();
    void doubleClickedGraph(QMouseEvent *event);
    void axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart);
    void onCustomContextMenu(const QPoint &point);
    void beforeReplot();

    void sourceSettingsOK(currentSource a);
    void materialSettingsOK(MaterialDefinition a);
    void TFSFSettingsOk(PlaneWave a);           // Total field scattered field = TFSF
    void SensorSettingsOk(SensorDefinition a);
    void hsgSettingsOk(SGInterface a);

    void graphClicked(QMouseEvent *event);
    void plotSquare(QMouseEvent *event);        // Plot the drawn square on customPlot
    void plotPoint(QMouseEvent *event);
    void plotPolygon(QMouseEvent *event);
    void polygonFinished(QMouseEvent* event);
    void drawSquare();                          // Capture user input (still drawing)
    void drawPolygon();
    void drawPoint();
    void clearLastDrawnStructure(int size);

signals:
    void drawingFinished();

private slots:
    void preferencesClicked();
    void aboutClicked();
    void saveAsClicked();
    void openClicked();
    void exportClicked();
    void on_start_clicked();                                // When clicking start
    void on_frameSlider_valueChanged(int timeIndex);        // When changing the frame
    void on_field_currentIndexChanged(int index);       // When viewing a different field

    void sourcesAppend();                               // When adding a new source
    void sourcesItemsSettings();                        // When going to the settings of that source
    void sourcesItemsDelete();                          // When deleting that source
    void materialAppend();
    void materialItemsSettings();
    void materialItemsDelete();
    void TFSFAppend();
    void TFSFItemsSettings();
    void TFSFItemsDelete();
    void sensorAppend();
    void sensorItemsSettings();
    void sensorItemsDelete();
    void sensorViewResult();
    void hsgAppend();
    void hsgItemSettings();
    void hsgItemsDelete();

    void on_grid_clicked();
    void on_PML_clicked();
    void on_Fields_clicked();
    void on_structures_clicked();
    void on_sources_clicked();
    void on_sensors_clicked();

private:
    QWaitCondition computation;
    QStandardItemModel *list;
    QStandardItem *sourceItem = new QStandardItem("Sources");
    QStandardItem *materialItem =  new QStandardItem("Material");
    QStandardItem *TFSFItem = new QStandardItem("Total field/scattered field");
    QStandardItem *sensorItem = new QStandardItem("Sensor");
    QStandardItem *hsgItem = new QStandardItem("Dispersive FDTD");
    QMenu* sourcesContextMenu;
    QMenu* sourcesItemsContextMenu;
    QMenu* materialContextMenu;
    QMenu* materialItemsContextMenu;
    QMenu* TFSFContextMenu;
    QMenu* TFSFItemsContexMenu;
    QMenu* SensorContextMenu;
    QMenu* SensorItemsContextMenu;
    QMenu* hsgContextMenu;
    QMenu* hsgItemsContextMenu;
    QMutex mutex;
    QPen pen = QPen(QColor(128,0,128));
    Ui::FDTD *ui;


};

#endif // FDTD_H
