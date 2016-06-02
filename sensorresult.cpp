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

#include "sensorresult.h"
#include "ui_sensorresult.h"
#include <fftw3.h>
#include <math.h>
#include <vector>
#include <QColorDialog>

#define penWidth   2
#define DELIMITER   ","

SensorResult::SensorResult(std::vector<SensorDefinition> &a, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SensorResult)
{
    ui->setupUi(this);
    this->a = &a;

    //////////////
    // Create the listview on the right hand side
    list = new QStandardItemModel;
//    QStandardItem *rootNode = list->invisibleRootItem();
    list->setHorizontalHeaderLabels(QStringList("Sensors"));        // List name

    fftw_plan my_plan;
    for(int k=0; k<a.size(); k++) {
        if(a[k].size == 0)      // If there were sensors added after the simulation ran, their size will be zero
            continue;

        my_plan = fftw_plan_dft_r2c_1d(a[k].size, a[k].Ex, a[k].fEx, FFTW_ESTIMATE);      // Drop the last element (sometimes strange behaviour for last element due to data transfer in field)
        fftw_execute(my_plan);
        fftw_destroy_plan(my_plan);
        my_plan = fftw_plan_dft_r2c_1d(a[k].size, a[k].Ey, a[k].fEy, FFTW_ESTIMATE);
        fftw_execute(my_plan);
        fftw_destroy_plan(my_plan);
        my_plan = fftw_plan_dft_r2c_1d(a[k].size, a[k].Hz, a[k].fHz, FFTW_ESTIMATE);
        fftw_execute(my_plan);
        fftw_destroy_plan(my_plan);

        QString title = QString::number(k+1)+": ("+QString::number(a[k].xpos)+", "+QString::number(a[k].ypos)+")";
        addSensor(title);
    }

    ui->FrequencyPlot->yAxis->grid()->setPen(QPen(Qt::gray, 1, Qt::DotLine));       // Grid lines thicker, such that they are visible when printing for a report
    ui->FrequencyPlot->xAxis->grid()->setPen(QPen(Qt::gray, 1, Qt::DotLine));
    ui->TimePlot->yAxis->grid()->setPen(QPen(Qt::gray, 1, Qt::DotLine));
    ui->TimePlot->xAxis->grid()->setPen(QPen(Qt::gray, 1, Qt::DotLine));

    connect(list, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(checkboxChanged(QStandardItem*)));
    //register the model
    ui->tree->setModel(list);
    ui->tree->collapseAll();        // Opposite is expandall()

    /////////////

    this->setWindowTitle("Sensor");

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));
    connect(ui->actionExport_time, SIGNAL(triggered(bool)), this, SLOT(exportTime()));
    connect(ui->actionExport_spectrum, SIGNAL(triggered(bool)), this, SLOT(exportSpectrum()));
    connect(ui->actionSave_reference, SIGNAL(triggered(bool)), this, SLOT(saveReference()));
    connect(ui->actionLoad_reference, SIGNAL(triggered(bool)), this, SLOT(loadReference()));

    connect(ui->TimePlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(resize(QMouseEvent*)));
    connect(ui->TimePlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->TimePlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->TimePlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->TimePlot->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->TimePlot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->TimePlot->yAxis, SLOT(setRange(QCPRange)));
    connect(ui->TimePlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->TimePlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    connect(ui->TimePlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(ui->TimePlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->TimePlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));
    connect(ui->TimePlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    connect(ui->FrequencyPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(resize(QMouseEvent*)));
    connect(ui->FrequencyPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->FrequencyPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->FrequencyPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->FrequencyPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    connect(ui->FrequencyPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(ui->FrequencyPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->FrequencyPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));
    connect(ui->FrequencyPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    ui->TimePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);
    ui->TimePlot->xAxis->setLabel("Time [ns]");
    ui->TimePlot->yAxis->setLabel("Field strength");
    ui->TimePlot->axisRect()->setupFullAxesBox();
    ui->TimePlot->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->FrequencyPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);
    ui->FrequencyPlot->xAxis->setLabel("Frequency [GHz]");
    ui->FrequencyPlot->yAxis->setLabel("Field strength");
    ui->FrequencyPlot->yAxis2->setVisible(true);
    ui->FrequencyPlot->xAxis2->setVisible(true);
    ui->FrequencyPlot->xAxis2->setTickLabels(false);
    ui->FrequencyPlot->yAxis2->setTickLabels(false);
    ui->FrequencyPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    QFont legendFont = font();
    legendFont.setPointSize(11);
    ui->FrequencyPlot->legend->setFont(legendFont);
    ui->TimePlot->legend->setFont(legendFont);

    ui->TimePlot->installEventFilter(this);
    ui->FrequencyPlot->installEventFilter(this);

//    updateTimePlot();
//    updateFrequencyPlot();
}

SensorResult::~SensorResult()
{
    delete ui;
}

void SensorResult::addSensor(QString title)
{
    QStandardItem *rootNode = list->invisibleRootItem();
    sensor = new QStandardItem(title);
    sensor->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    rootNode->appendRow(sensor);

    for(int k=0; k<3; k++) {             // Add the plots during construction such that graph indices can be easily mapped to sensorindices
        ui->TimePlot->addGraph();
        ui->TimePlot->graph(ui->TimePlot->graphCount()-1)->setVisible(false);
        ui->TimePlot->graph(ui->TimePlot->graphCount()-1)->removeFromLegend();
    }

    for(int k=0; k<3; k++) {
        ui->FrequencyPlot->addGraph();
        ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-1)->setVisible(false);
        ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-1)->removeFromLegend();
        ui->FrequencyPlot->addGraph(ui->FrequencyPlot->xAxis, ui->FrequencyPlot->yAxis2);
        ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-1)->setVisible(false);
        ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-1)->removeFromLegend();
    }

    addItem("Ex");
    addItem("Ey");
    addItem("Hz");

    QPen pen(Qt::blue, 2, Qt::DashLine);
    for(int k=ui->FrequencyPlot->graphCount()-6; k<ui->FrequencyPlot->graphCount(); k++) {        // Without this, the selected graph will be blue with thickness 2 = the original thickness
        ui->FrequencyPlot->graph(k)->setSelectedPen(pen);
        QPen pen = ui->FrequencyPlot->graph(k)->pen();
        pen.setWidth(penWidth);
        ui->FrequencyPlot->graph(k)->setPen(pen);
    }

    for(int k=ui->TimePlot->graphCount()-3; k<ui->TimePlot->graphCount(); k++) {
        ui->TimePlot->graph(k)->setSelectedPen(pen);
        QPen pen = ui->TimePlot->graph(k)->pen();
        pen.setWidth(penWidth);
        ui->TimePlot->graph(k)->setPen(pen);
    }

}

void SensorResult::addItem(QString title)
{
    QStandardItem *item = new QStandardItem(title);
    sensor->appendRow(item);
    item->appendRow(new QStandardItem("Time"));           // Add the three possible plots
    item->appendRow(new QStandardItem("Magnitude"));
    item->appendRow(new QStandardItem("Phase"));
    item->child(0)->setCheckable(true);                   // And a checkbox next to it
//    item->child(0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);     // Prevent label name adjustments by user
    item->child(1)->setCheckable(true);
//    item->child(1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    item->child(2)->setCheckable(true);
//    item->child(2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
//    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void SensorResult::exportSpectrum()
{
    plot=ui->FrequencyPlot;
    exportClicked();
}

void SensorResult::exportTime()
{
    plot=ui->TimePlot;
    exportClicked();
}

void SensorResult::exportClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export as..."), QString(), tr("pdf files (*.pdf);;png files (*.png);;jpg files (*.jpg);;bmp files (*.bmp);;csv files (*.csv)"));

    QFile f( filename );
    if ( f.open( QIODevice::WriteOnly ) )
    {
        for(int k=0; k<ui->FrequencyPlot->graphCount(); k++)
            ui->FrequencyPlot->graph(k)->setAdaptiveSampling(false);        // Otherwise choppy behaviour when exporting to pdf
        for(int k=0; k<ui->TimePlot->graphCount(); k++)
            ui->TimePlot->graph(k)->setAdaptiveSampling(false);

        double xAxisSize, yAxisSize, plotWidth, plotHeight;

        xAxisSize = plot->yAxis->axisRect()->width();
        yAxisSize = plot->xAxis->axisRect()->height();
        plotWidth = plot->size().width();
        plotHeight = plot->size().height();

        double paddingX = plotWidth - xAxisSize;
        double paddingY = plotHeight - yAxisSize;

        if(filename.endsWith(".pdf", Qt::CaseInsensitive))
            plot->savePdf(filename, false, settings->width+paddingX, settings->height+paddingY);

        if(filename.endsWith(".png", Qt::CaseInsensitive)) {
            plot->savePng(filename, settings->width+paddingX, settings->height+paddingY);
        }

        if(filename.endsWith(".jpg", Qt::CaseInsensitive)) {
            plot->saveJpg(filename, settings->width+paddingX, settings->height+paddingY);
        }

        if(filename.endsWith(".bmp", Qt::CaseInsensitive)) {
            plot->saveBmp(filename, settings->width+paddingX, settings->height+paddingY);
        }

        if(filename.endsWith(".csv", Qt::CaseInsensitive)) {
            QTextStream stream( &f );

            if(plot == ui->FrequencyPlot && a->size() != NULL) {     // When exporting the spectrum

                stream << "Frequency [GHz]" << DELIMITER;
                for(int k=0; k<a->size(); k++) {
                    stream << "Magnitude Ex "+QString::number(k) << DELIMITER << "Phase Ex "+QString::number(k) << DELIMITER;
                    stream << "Magnitude Ey "+QString::number(k) << DELIMITER << "Phase Ey "+QString::number(k) << DELIMITER;
                    stream << "Magnitude Hz "+QString::number(k) << DELIMITER << "Phase Hz "+QString::number(k);

                    if(k < a->size()-1)
                        stream << DELIMITER;
                    else
                        stream << endl;
                }

                for(int n=0; n<(*a)[0].size/2+1; n++)   // Assume they all have the same length
                {                                       // Frequency content is half the size of time domain signal (neg. freqs are dropped)
                    stream << n/(1E9*(*a)[0].size*(*a)[0].dt) << DELIMITER;                                                                      // Frequency
                    for(int k=0; k<a->size(); k++)
                    {
                        stream << sqrt((*a)[k].fEx[n][0]*(*a)[k].fEx[n][0] + (*a)[k].fEx[n][1]*(*a)[k].fEx[n][1])/((*a)[k].size)*2 << DELIMITER;     // Magnitude
                        stream << atan2((*a)[k].fEx[n][1], (*a)[k].fEx[n][0])*180/M_PI << DELIMITER;                                                 // Phase
                        stream << sqrt((*a)[k].fEy[n][0]*(*a)[k].fEy[n][0] + (*a)[k].fEy[n][1]*(*a)[k].fEy[n][1])/((*a)[k].size)*2 << DELIMITER;
                        stream << atan2((*a)[k].fEy[n][1], (*a)[k].fEy[n][0])*180/M_PI << DELIMITER;
                        stream << sqrt((*a)[k].fHz[n][0]*(*a)[k].fHz[n][0] + (*a)[k].fHz[n][1]*(*a)[k].fHz[n][1])/((*a)[k].size)*2 << DELIMITER;
                        stream << atan2((*a)[k].fHz[n][1], (*a)[k].fHz[n][0])*180/M_PI;

                        if(k < a->size()-1)
                            stream << DELIMITER;
                        else
                            stream << endl;
                    }
                }
            }
            if(plot == ui->TimePlot && a->size() != NULL) {
                stream << "Time [s]" << DELIMITER;
                for(int k=0; k<a->size(); k++) {
                    stream << "Ex "+QString::number(k) << DELIMITER;
                    stream << "Ey "+QString::number(k) << DELIMITER;
                    stream << "Hz "+QString::number(k);

                    if(k < a->size()-1)
                        stream << DELIMITER;
                    else
                        stream << endl;
                }

                for(int n=0; n<(*a)[0].size; n++)   // Assume they all have the same length
                {                                       // Frequency content is half the size of time domain signal (neg. freqs are dropped)
                    stream << n*(*a)[0].dt << DELIMITER;                                                                      // Frequency
                    for(int k=0; k<a->size(); k++)
                    {
                        stream << (*a)[k].Ex[n] << DELIMITER;
                        stream << (*a)[k].Ey[n] << DELIMITER;
                        stream << (*a)[k].Hz[n];

                        if(k < a->size()-1)
                            stream << DELIMITER;
                        else
                            stream << endl;
                    }
                }
            }

            f.close();
        }

        for(int k=0; k<ui->FrequencyPlot->graphCount(); k++)
            ui->FrequencyPlot->graph(k)->setAdaptiveSampling(true);        // Restore original settings
        for(int k=0; k<ui->TimePlot->graphCount(); k++)
            ui->TimePlot->graph(k)->setAdaptiveSampling(true);
    }
}


bool SensorResult::eventFilter(QObject *object, QEvent *event)
{
//    qDebug() << object << " " << event->type() << " " << QEvent::MouseButtonPress;

    if (object == ui->TimePlot && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Wheel)) {
        if(plot == ui->FrequencyPlot) {     // Only allow selections within one plot
            ui->FrequencyPlot->xAxis->setSelectedParts(QCPAxis::spNone);
            ui->FrequencyPlot->xAxis2->setSelectedParts(QCPAxis::spNone);
            ui->FrequencyPlot->yAxis->setSelectedParts(QCPAxis::spNone);
            ui->FrequencyPlot->yAxis2->setSelectedParts(QCPAxis::spNone);
            ui->FrequencyPlot->replot();
        }

        plot = ui->TimePlot;
    }
    if(object == ui->FrequencyPlot && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Wheel)) {
        if(plot == ui->TimePlot) {
            ui->TimePlot->xAxis->setSelectedParts(QCPAxis::spNone);
            ui->TimePlot->xAxis2->setSelectedParts(QCPAxis::spNone);
            ui->TimePlot->yAxis->setSelectedParts(QCPAxis::spNone);
            ui->TimePlot->yAxis2->setSelectedParts(QCPAxis::spNone);
            ui->TimePlot->replot();
        }

        plot = ui->FrequencyPlot;
    }

    return false;       // False if further handling of the event is required
}

void SensorResult::checkboxChanged(QStandardItem* item)
{
    int sensorIndex = item->parent()->parent()->row();              // which sensor is this
    FieldIndex fieldIndex = (FieldIndex)item->parent()->row();      // what field is it
    PlotType plotType = (PlotType)item->row();                      // what plottype is it

    Qt::CheckState checkState = item->checkState();

    if(checkState == Qt::Checked)
    {
        switch(plotType)
        {
        case Time:
            plotTime(sensorIndex, fieldIndex);
            break;
        case Magnitude:
            plotMagnitude(sensorIndex, fieldIndex);
            break;
        case Phase:
            plotPhase(sensorIndex, fieldIndex);
            break;
        }
    }
    else if(checkState == Qt::Unchecked)
    {
        switch(plotType)
        {
        case Time:
            ui->TimePlot->graph(3*sensorIndex + fieldIndex)->setVisible(false);            // Do not delete the plots, or the indices get mixed up
            break;
        case Magnitude:
            ui->FrequencyPlot->graph(6*sensorIndex + 2*fieldIndex)->setVisible(false);
            break;
        case Phase:
            ui->FrequencyPlot->graph(6*sensorIndex + 2*fieldIndex + 1)->setVisible(false);        // *2 because there are two frequency plots for each sensor (phase and magnitude)
            break;
        }
    }

    if(plotType == Time) {
        int numberOfVisiblePlots = 0;
        for(int k=0; k<ui->TimePlot->graphCount(); k++) {
            if(ui->TimePlot->graph(k)->visible() == true)
                numberOfVisiblePlots++;
        }

        if(numberOfVisiblePlots > 1) {          // If more than one plot, add the legend of all visible items to the legend
            for(int k=0; k<ui->TimePlot->graphCount(); k++) {
                if(ui->TimePlot->graph(k)->visible() == true)
                    ui->TimePlot->graph(k)->addToLegend();
                else
                    ui->TimePlot->graph(k)->removeFromLegend();
            }
            ui->TimePlot->legend->setVisible(true);
        }
        else {
            for(int k=0; k<ui->TimePlot->graphCount(); k++)
                ui->TimePlot->graph(k)->removeFromLegend();
            ui->TimePlot->legend->setVisible(false);
        }

//        ui->TimePlot->rescaleAxes(true);
        ui->TimePlot->replot();
    }
    else {
        int numberOfVisibleMagnitudes = 0;
        int numberOfVisiblePhases = 0;
        for(int k=0; k<ui->FrequencyPlot->graphCount(); k++) {
            if(ui->FrequencyPlot->graph(k)->visible() == true)
                numberOfVisibleMagnitudes++;
            k++;
            if(ui->FrequencyPlot->graph(k)->visible() == true)
                numberOfVisiblePhases++;
        }

        if(numberOfVisiblePhases == 0 || numberOfVisibleMagnitudes == 0) {
            ui->FrequencyPlot->yAxis2->setTickLabels(false);
            connect(ui->FrequencyPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->FrequencyPlot->yAxis2, SLOT(setRange(QCPRange)));
            connect(ui->FrequencyPlot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->FrequencyPlot->yAxis, SLOT(setRange(QCPRange)));

            if(numberOfVisiblePhases == 0) {
                if(ui->FrequencyPlot->yAxis2->label() != "")        // This is needed to sync the tick marks
                    ui->FrequencyPlot->yAxis2->setRange(ui->FrequencyPlot->yAxis->range());

                ui->FrequencyPlot->yAxis->setLabel("Field strength");
                ui->FrequencyPlot->yAxis2->setLabel("");
            }
            else {
                if(ui->FrequencyPlot->yAxis2->label() != "")        // If yaxis2 is swapped to yaxis, also take the scale with it
                    ui->FrequencyPlot->yAxis->setRange(ui->FrequencyPlot->yAxis2->range());

                ui->FrequencyPlot->yAxis->setLabel("Phase [°]");
                ui->FrequencyPlot->yAxis2->setLabel("");
            }
        }
        else {
            ui->FrequencyPlot->yAxis->setLabel("Field strength");
            ui->FrequencyPlot->yAxis2->setLabel("Phase [°]");
            ui->FrequencyPlot->yAxis2->setTickLabels(true);
            disconnect(ui->FrequencyPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->FrequencyPlot->yAxis2, SLOT(setRange(QCPRange)));
            disconnect(ui->FrequencyPlot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->FrequencyPlot->yAxis, SLOT(setRange(QCPRange)));
        }

        if(numberOfVisibleMagnitudes + numberOfVisiblePhases > 1) {      // If more than one plot
            for(int k=0; k<ui->FrequencyPlot->graphCount(); k++) {
                if(ui->FrequencyPlot->graph(k)->visible() == true)
                    ui->FrequencyPlot->graph(k)->addToLegend();
                else
                    ui->FrequencyPlot->graph(k)->removeFromLegend();
            }
            ui->FrequencyPlot->legend->setVisible(true);
        }
        else {
            for(int k=0; k<ui->FrequencyPlot->graphCount(); k++)
                ui->FrequencyPlot->graph(k)->removeFromLegend();
            ui->FrequencyPlot->legend->setVisible(false);
        }

//        ui->FrequencyPlot->rescaleAxes(true);       // Only plotted items
        ui->FrequencyPlot->replot();
    }
}

void SensorResult::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (plot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
      int visibleGraphs = 0;
      for(int k=0; k<plot->graphCount(); k++) {
          if(plot->graph(k)->visible() == true)
              visibleGraphs++;
      }
    if (visibleGraphs > 0)
        menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    if (plot->selectedGraphs().size() > 0) {
        menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        menu->addAction("Change color", this, SLOT(changeColor()));
    }
  }

  menu->popup(plot->mapToGlobal(pos));
}

void SensorResult::removeAllGraphs()
{
    for(int k=0; k<plot->graphCount(); k++)
        plot->graph(k)->setVisible(false);

    QStandardItem *rootNode = list->invisibleRootItem();
    if(plot == ui->FrequencyPlot) {
        for(int k=0; k<list->rowCount(); k++) {
            rootNode->child(k)->child(0)->child(1)->setCheckState(Qt::Unchecked);       // Sensor -> Ex -> Magnitude
            rootNode->child(k)->child(0)->child(2)->setCheckState(Qt::Unchecked);       // Sensor -> Ex -> Phase
            rootNode->child(k)->child(1)->child(1)->setCheckState(Qt::Unchecked);       // Sensor -> Ey -> Magnitude
            rootNode->child(k)->child(1)->child(2)->setCheckState(Qt::Unchecked);       // ...
            rootNode->child(k)->child(2)->child(1)->setCheckState(Qt::Unchecked);
            rootNode->child(k)->child(2)->child(2)->setCheckState(Qt::Unchecked);
        }
        ui->FrequencyPlot->replot();
    }
    else {
        for(int k=0; k<list->rowCount(); k++) {
            rootNode->child(k)->child(0)->child(0)->setCheckState(Qt::Unchecked);       // Sensor -> Ex -> Time
            rootNode->child(k)->child(1)->child(0)->setCheckState(Qt::Unchecked);       // Sensor -> Ey -> Time
            rootNode->child(k)->child(2)->child(0)->setCheckState(Qt::Unchecked);       // Sensor -> Hz -> Time
        }

        ui->TimePlot->replot();
    }
}

void SensorResult::removeSelectedGraph()
{
    QStandardItem *rootNode = list->invisibleRootItem();
    for(int k=0; k<plot->selectedGraphs().size(); k++)
        plot->selectedGraphs()[k]->setVisible(false);

    for(int k=0; k<a->size(); k++) {        // Loop over all sensors and sync the checkboxes in the treeview
        if(plot == ui->FrequencyPlot)
        {
            if(plot->graph(6*k)->visible() == false)
                rootNode->child(k)->child(0)->child(1)->setCheckState(Qt::Unchecked);       // Sensor -> Ex -> Magnitude
            if(plot->graph(6*k+1)->visible() == false)
                rootNode->child(k)->child(0)->child(2)->setCheckState(Qt::Unchecked);       // Sensor -> Ex -> Phase
            if(plot->graph(6*k+2)->visible() == false)
                rootNode->child(k)->child(1)->child(1)->setCheckState(Qt::Unchecked);       // Sensor -> Ey -> Magnitude
            if(plot->graph(6*k+3)->visible() == false)
                rootNode->child(k)->child(1)->child(2)->setCheckState(Qt::Unchecked);
            if(plot->graph(6*k+4)->visible() == false)
                rootNode->child(k)->child(2)->child(1)->setCheckState(Qt::Unchecked);
            if(plot->graph(6*k+5)->visible() == false)
                rootNode->child(k)->child(2)->child(2)->setCheckState(Qt::Unchecked);
        }
        else
        {
            if(plot->graph(3*k)->visible() == false)
                rootNode->child(k)->child(0)->child(0)->setCheckState(Qt::Unchecked);       // Sensor -> Ex -> Time
            if(plot->graph(3*k+1)->visible() == false)
                rootNode->child(k)->child(1)->child(0)->setCheckState(Qt::Unchecked);       // Sensor -> Ey -> Time
            if(plot->graph(3*k+2)->visible() == false)
                rootNode->child(k)->child(1)->child(0)->setCheckState(Qt::Unchecked);       // Sensor -> Hz -> Time
        }
    }

    plot->replot();
}

void SensorResult::changeColor()
{
    QColor initialColor = plot->selectedGraphs()[0]->pen().color();
    QColor newColor = QColorDialog::getColor(initialColor);

    for(int k=0; k<plot->selectedGraphs().size(); k++)
        plot->selectedGraphs()[k]->setPen(QPen(newColor, penWidth));
}

void SensorResult::plotMagnitude(int sensorIndex, FieldIndex field)
{
    int position = 6*sensorIndex + 2*field;
    if(ui->FrequencyPlot->graph(position)->visible() == false)
    {
        ui->FrequencyPlot->graph(position)->setVisible(true);            // If it was already plotted, just set it back to visible
        if(ui->FrequencyPlot->graph(position)->data()->size() == 0)      // If the data was not yet plotted
        {
            int size = (*a)[sensorIndex].size;
            QVector<double> x(size), y1(size);
            switch(field)
            {
            case Ex:
                ui->FrequencyPlot->graph(position)->setName("Magnitude Ex "+QString::number(sensorIndex+1));
                for (int i=0; i<size/2+1; ++i)
                {
                    x[i] = i/(1E9*(*a)[sensorIndex].size*(*a)[sensorIndex].dt);
                    y1[i] = sqrt((*a)[sensorIndex].fEx[i][0]*(*a)[sensorIndex].fEx[i][0] + (*a)[sensorIndex].fEx[i][1]*(*a)[sensorIndex].fEx[i][1])/((*a)[sensorIndex].size)*2;
                }
                break;
            case Ey:
                ui->FrequencyPlot->graph(position)->setName("Magnitude Ey "+QString::number(sensorIndex+1));
                for (int i=0; i<size/2+1; ++i)
                {
                    x[i] = i/(1E9*(*a)[sensorIndex].size*(*a)[sensorIndex].dt);
                    y1[i] = sqrt((*a)[sensorIndex].fEy[i][0]*(*a)[sensorIndex].fEy[i][0] + (*a)[sensorIndex].fEy[i][1]*(*a)[sensorIndex].fEy[i][1])/((*a)[sensorIndex].size)*2;
                }
              break;
            case Hz:
                ui->FrequencyPlot->graph(position)->setName("Magnitude Hz "+QString::number(sensorIndex+1));
                for (int i=0; i<size/2+1; ++i)
                {
                    x[i] = i/(1E9*(*a)[sensorIndex].size*(*a)[sensorIndex].dt);
                    y1[i] = sqrt((*a)[sensorIndex].fHz[i][0]*(*a)[sensorIndex].fHz[i][0] + (*a)[sensorIndex].fHz[i][1]*(*a)[sensorIndex].fHz[i][1])/((*a)[sensorIndex].size)*2;
                }
              break;
            }

            ui->FrequencyPlot->graph(position)->setData(x, y1);
        }
    }

    int visiblePlots = 0;
    for(int k=0; k<ui->FrequencyPlot->graphCount(); k++) {
        if(ui->FrequencyPlot->graph(k)->visible())
            visiblePlots++;
    }

    if(visiblePlots == 1)
        ui->FrequencyPlot->rescaleAxes(true);
}

void SensorResult::plotPhase(int sensorIndex, FieldIndex field)
{
    int position = 6*sensorIndex + 2*field + 1;                          // +1, because this is the phase
    if(ui->FrequencyPlot->graph(position)->visible() == false)
    {
        ui->FrequencyPlot->graph(position)->setVisible(true);            // If it was already plotted, just set it back to visible
        if(ui->FrequencyPlot->graph(position)->data()->size() == 0)      // If the data was not yet plotted
        {
            ui->FrequencyPlot->graph(position)->setPen(QPen(QColor(0, 150, 0), penWidth));

            int size = (*a)[sensorIndex].size;
            QVector<double> x(size), y2(size);
            switch(field)
            {
            case Ex:
                ui->FrequencyPlot->graph(position)->setName("Phase Ex "+QString::number(sensorIndex+1));
                for (int i=0; i<size/2+1; ++i)
                {
                    x[i] = i/(1E9*(*a)[sensorIndex].size*(*a)[sensorIndex].dt);
                    y2[i] = atan2((*a)[sensorIndex].fEx[i][1], (*a)[sensorIndex].fEx[i][0])*180/M_PI;
                }
                break;
            case Ey:
                ui->FrequencyPlot->graph(position)->setName("Phase Ey "+QString::number(sensorIndex+1));
                for (int i=0; i<size/2+1; ++i)
                {
                    x[i] = i/(1E9*(*a)[sensorIndex].size*(*a)[sensorIndex].dt);
                    y2[i] = atan2((*a)[sensorIndex].fEy[i][1], (*a)[sensorIndex].fEy[i][0])*180/M_PI;
                }
                break;
            case Hz:
                ui->FrequencyPlot->graph(position)->setName("Phase Hz "+QString::number(sensorIndex+1));
                for (int i=0; i<size/2+1; ++i)
                {
                    x[i] = i/(1E9*(*a)[sensorIndex].size*(*a)[sensorIndex].dt);
                    y2[i] = atan2((*a)[sensorIndex].fHz[i][1], (*a)[sensorIndex].fHz[i][0])*180/M_PI;
                }
                break;
            }

            ui->FrequencyPlot->graph(position)->setData(x, y2);
        }
    }

    int visiblePlots = 0;
    for(int k=0; k<ui->FrequencyPlot->graphCount(); k++) {
        if(ui->FrequencyPlot->graph(k)->visible())
            visiblePlots++;
    }

    if(visiblePlots == 1)
        ui->FrequencyPlot->rescaleAxes(true);
}

void SensorResult::plotTime(int sensorIndex, FieldIndex field)
{
    int position = 3*sensorIndex + field;
    if(ui->TimePlot->graph(position)->visible() == false)
    {
        ui->TimePlot->graph(position)->setVisible(true);            // If it was already plotted, just set it back to visible
        if(ui->TimePlot->graph(position)->data()->size() == 0)      // If the data was not yet plotted
        {
            int size = (*a)[sensorIndex].size;
            QVector<double> x(size), y(size);
            switch(field)
            {
            case Ex:
                ui->TimePlot->graph(position)->setName("Ex "+QString::number(sensorIndex+1));
                for (int i=0; i<size; ++i)
                {
                    x[i] = i*(*a)[sensorIndex].dt*1E9;
                    y[i] = (*a)[sensorIndex].Ex[i];
                }
                break;
            case Ey:
                ui->TimePlot->graph(position)->setName("Ey "+QString::number(sensorIndex+1));
                for (int i=0; i<size; ++i)
                {
                    x[i] = i*(*a)[sensorIndex].dt*1E9;
                    y[i] = (*a)[sensorIndex].Ey[i];
                }
                break;
            case Hz:
                ui->TimePlot->graph(position)->setName("Hz "+QString::number(sensorIndex+1));
                for (int i=0; i<size; ++i)
                {
                    x[i] = i*(*a)[sensorIndex].dt*1E9;
                    y[i] = (*a)[sensorIndex].Hz[i];
                }
                break;
            }

            ui->TimePlot->graph(position)->setData(x, y);
        }
    }

    int visiblePlots = 0;
    for(int k=0; k<ui->TimePlot->graphCount(); k++) {
        if(ui->TimePlot->graph(k)->visible())
            visiblePlots++;
    }

    if(visiblePlots == 1)
        ui->TimePlot->rescaleAxes(true);
}

void SensorResult::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      plot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      plot->replot();
    }
  }
}

void SensorResult::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    if(part == QCPAxis::spTickLabels)
    {
        inputRange *temp = new inputRange(axis->range().lower, axis->range().upper);
        temp->show();
        connect(temp, SIGNAL(range(QCPRange)), axis, SLOT(setRange(QCPRange)));
        connect(temp, SIGNAL(destroyed(QObject*)), plot, SLOT(replot()));
    }

    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "Edit label", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            plot->replot();
        }
    }
}

void SensorResult::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Rename a graph by double clicking on its legend item
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
      plItem->plottable()->setName(newName);
      plot->replot();
    }
  }
}

void SensorResult::selectionChanged()
{
  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || plot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      plot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || plot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    plot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    plot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || plot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
    plot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);

  if (plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || plot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    plot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);

  // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<plot->graphCount(); ++i)
    {
      QCPGraph *graph = plot->graph(i);
      QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);
      if (plot->legend->visible() && graph->visible() && (item->selected() || graph->selected()))
      {
        item->setSelected(true);
        graph->setSelected(true);
      }
    }
}

void SensorResult::mousePress(QMouseEvent *event)
{
//    event->source();

    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        plot->axisRect()->setRangeDragAxes(plot->xAxis, plot->yAxis);
        plot->axisRect()->setRangeDrag(plot->xAxis->orientation());
    }
    else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        plot->axisRect()->setRangeDragAxes(plot->xAxis, plot->yAxis);
        plot->axisRect()->setRangeDrag(plot->yAxis->orientation());
    }
    else if (plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        plot->axisRect()->setRangeDragAxes(plot->xAxis, plot->yAxis2);
        plot->axisRect()->setRangeDrag(plot->yAxis2->orientation());
    }
    else {
        plot->axisRect()->setRangeDragAxes(plot->xAxis, plot->yAxis);
        plot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    }
}

void SensorResult::mouseWheel()
{
    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        plot->axisRect()->setRangeZoomAxes(plot->xAxis, plot->yAxis);
        plot->axisRect()->setRangeZoom(plot->xAxis->orientation());
    }
    else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        plot->axisRect()->setRangeZoomAxes(plot->xAxis, plot->yAxis);
        plot->axisRect()->setRangeZoom(plot->yAxis->orientation());
    }
    else if (plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        plot->axisRect()->setRangeZoomAxes(plot->xAxis, plot->yAxis2);
        plot->axisRect()->setRangeZoom(plot->yAxis2->orientation());
    }
    else {
        plot->axisRect()->setRangeZoomAxes(plot->xAxis, plot->yAxis);
        plot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    }
}

void SensorResult::closeEvent(QCloseEvent *event)
{
    this->deleteLater();
}

void SensorResult::resize(QMouseEvent *)
{
    if(plot->selectedAxes().size() == 0) {
        plot->rescaleAxes(true);
        plot->replot();
    }
}

void SensorResult::saveReference()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("REF-Files (*.ref);;All Files (*)"));

    QFile f( filename );
    if ( f.open( QIODevice::WriteOnly ) )
    {
        QDataStream stream( &f );

        stream << qint32(a->size());     // Number of sensors
        for(int k=0; k<a->size(); k++)
        {
            stream << (*a)[k].size;     // Amount of data for each sensor

            for(int n=0; n<(*a)[k].size; n++)
            {
                stream << n*(*a)[k].dt*1E9;      // Time index
                stream << (*a)[k].Ex[n];
                stream << (*a)[k].Ey[n];
                stream << (*a)[k].Hz[n];
            }

            for(int n=0; n<(*a)[k].size/2+1; n++) {
                stream << n/(1E9*(*a)[k].size*(*a)[k].dt);                                                                      // Frequency
                stream << sqrt((*a)[k].fEx[n][0]*(*a)[k].fEx[n][0] + (*a)[k].fEx[n][1]*(*a)[k].fEx[n][1])/((*a)[k].size)*2;     // Magnitude
                stream << atan2((*a)[k].fEx[n][1], (*a)[k].fEx[n][0])*180/M_PI;                                                 // Phase
                stream << sqrt((*a)[k].fEy[n][0]*(*a)[k].fEy[n][0] + (*a)[k].fEy[n][1]*(*a)[k].fEy[n][1])/((*a)[k].size)*2;
                stream << atan2((*a)[k].fEy[n][1], (*a)[k].fEy[n][0])*180/M_PI;
                stream << sqrt((*a)[k].fHz[n][0]*(*a)[k].fHz[n][0] + (*a)[k].fHz[n][1]*(*a)[k].fHz[n][1])/((*a)[k].size)*2;
                stream << atan2((*a)[k].fHz[n][1], (*a)[k].fHz[n][0])*180/M_PI;
            }
        }
    }
    f.close();
}

void SensorResult::loadReference()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File..."), QString(), tr("REF-Files (*.ref);;All Files (*)"));
    QFile f( filename );
    QFileInfo fileInfo(f.fileName());
    QString referenceName(fileInfo.baseName());

    if ( f.open( QIODevice::ReadOnly ) )
    {
        QDataStream stream( &f );
        int sensors;
        stream >> sensors;

        for(int k=0; k<sensors; k++)
            addSensor(referenceName+" "+QString::number(k+1));

        for(int k=0; k<sensors; k++)
        {
            int data, fdata;    // # values in time and frequency domain
            stream >> data;
            fdata = data/2+1;
            QVector<double> t(data), Ex(data), Ey(data), Hz(data), f(fdata), fExM(fdata), fExP(fdata), fEyM(fdata), fEyP(fdata), fHzM(fdata), fHzP(fdata);     // M=magnitude, P=phase

            for(int i=0; i<data; i++)
                stream >> t[i] >> Ex[i] >> Ey[i] >> Hz[i];

            for(int i=0; i<fdata; i++)
                stream >> f[i] >> fExM[i] >> fExP[i] >> fEyM[i] >> fEyP[i]>> fHzM[i] >> fHzP[i];

            ui->TimePlot->graph(ui->TimePlot->graphCount()-3*sensors+3*k)->setData(t, Ex);
            ui->TimePlot->graph(ui->TimePlot->graphCount()-3*sensors+3*k)->setName("Ex "+referenceName+" "+QString::number(k+1));
            ui->TimePlot->graph(ui->TimePlot->graphCount()-3*sensors+3*k+1)->setData(t, Ey);
            ui->TimePlot->graph(ui->TimePlot->graphCount()-3*sensors+3*k+1)->setName("Ey "+referenceName+" "+QString::number(k+1));
            ui->TimePlot->graph(ui->TimePlot->graphCount()-3*sensors+3*k+2)->setData(t, Hz);
            ui->TimePlot->graph(ui->TimePlot->graphCount()-3*sensors+3*k+2)->setName("Hz "+referenceName+" "+QString::number(k+1));

            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k)->setData(f, fExM);
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k)->setName("Magnitude Ex "+referenceName+" "+QString::number(k+1));

            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+1)->setData(f, fExP);
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+1)->setPen(QPen(QColor(0, 150, 0), penWidth));
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+1)->setName("Phase Ex "+referenceName+" "+QString::number(k+1));

            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+2)->setData(f, fEyM);
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+2)->setName("Magnitude Ey "+referenceName+" "+QString::number(k+1));

            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+3)->setData(f, fEyP);
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+3)->setPen(QPen(QColor(0, 150, 0), penWidth));
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+3)->setName("Phase Ey "+referenceName+" "+QString::number(k+1));

            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+4)->setData(f, fHzM);
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+4)->setName("Magnitude Hz "+referenceName+" "+QString::number(k+1));

            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+5)->setData(f, fHzP);
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+5)->setName("Phase Hz "+referenceName+" "+QString::number(k+1));
            ui->FrequencyPlot->graph(ui->FrequencyPlot->graphCount()-6*sensors+6*k+5)->setPen(QPen(QColor(0, 150, 0), penWidth));
        }
    }
    f.close();
}
