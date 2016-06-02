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

#include "fdtd.h"
#include "ui_fdtd.h"
#include <QFileDialog>
#include <QInputDialog>

#define c           299792458
#define epsilon0    8.8541878176E-12
#define mu0         1.2566370614E-6
#define settingsIndex   -1
#define sourceIndex     0
#define TFSFIndex       1
#define materialIndex   2
#define sensorIndex     3
#define hsgIndex        4

FDTD::FDTD(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FDTD)
{
    ui->setupUi(this);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    ui->Fields->setChecked(true);
    ui->grid->setChecked(false);
    ui->PML->setChecked(false);
    ui->structures->setChecked(true);
    ui->sources->setChecked(true);
    ui->sensors->setChecked(true);
    ui->Time->setText(QString("0 ns"));
    ui->Sample->setText(QString::number(1)+QString("/")+QString::number(settings.steps));

    connect(ui->Preferences, SIGNAL(triggered()), this, SLOT(preferencesClicked()));
    connect(ui->Action_Save_As, SIGNAL(triggered()), this, SLOT(saveAsClicked()));
    connect(ui->Action_Open, SIGNAL(triggered(bool)), this, SLOT(openClicked()));
    connect(ui->Action_Export, SIGNAL(triggered(bool)), this, SLOT(exportClicked()));
    connect(ui->action_About, SIGNAL(triggered(bool)), this, SLOT(aboutClicked()));

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this, SLOT(on_start_clicked()));

    QFileDialog f;              // Set default directory on startup
    f.setDirectory(QDir::home());

    ui->frameSlider->setVisible(false);
    ui->frameSlider->setMinimum(0);

    // configure axis rect:
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes); // this will also allow rescaling the color scale by dragging/zooming
//    ui->customPlot->installEventFilter(this);       // Capture resize of customplot
    ui->customPlot->axisRect()->setupFullAxesBox(true);
    ui->customPlot->xAxis->setLabel("x [m]");
    ui->customPlot->yAxis->setLabel("y [m]");
    ui->customPlot->xAxis->grid()->setVisible(false);
    ui->customPlot->yAxis->grid()->setVisible(false);

    // set up the QCPColorMap:
    colorMap = new QCPColorMap(ui->customPlot->xAxis, ui->customPlot->yAxis);
    ui->customPlot->addPlottable(colorMap);

    // add a color scale:
    colorScale = new QCPColorScale(ui->customPlot);
    ui->customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Field Strength");
    colorMap->setInterpolate(false);
    colorMap->data()->setRange(QCPRange(-settings.sizeX/2., settings.sizeX/2.), QCPRange(-settings.sizeY/2., settings.sizeY/2.));
    ui->customPlot->xAxis->setRange(QCPRange(-settings.sizeX/2., settings.sizeX/2.));
    ui->customPlot->yAxis->setRange(QCPRange(-settings.sizeY/2., settings.sizeY/2.));

    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpPolar2);
    colorMap->clearData();
    ui->customPlot->replot();

    connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickedGraph(QMouseEvent*)));        // Double click to auto-resize
    connect(ui->customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->customPlot, SIGNAL(beforeReplot()), this, SLOT(beforeReplot()));
//    connect(ui->customPlot->plotLayout()->elementAt(1), SIGNAL(mouseDoubleClickEvent(QMouseEvent *)), this, SLOT(colorScaleDoubleClick()));

    // Create the listview on the right hand side
    list = new QStandardItemModel;
    QStandardItem *rootNode = list->invisibleRootItem();
    list->setHorizontalHeaderLabels(QStringList("Objects"));

    //building up the hierarchy
    rootNode->appendRow(sourceItem);
    rootNode->appendRow(TFSFItem);
    rootNode->appendRow(materialItem);
    rootNode->appendRow(sensorItem);
    rootNode->appendRow(hsgItem);

    //register the model
    ui->GridObjects->setModel(list);
    ui->GridObjects->expandAll();
    ui->GridObjects->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->GridObjects, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    // Create context menus for sources and materials
    sourcesContextMenu = new QMenu(ui->GridObjects);
    sourcesItemsContextMenu = new QMenu(ui->GridObjects);
    TFSFContextMenu = new QMenu(ui->GridObjects);
    TFSFItemsContexMenu = new QMenu(ui->GridObjects);
    materialContextMenu = new QMenu(ui->GridObjects);
    materialItemsContextMenu = new QMenu(ui->GridObjects);
    SensorContextMenu = new QMenu(ui->GridObjects);
    SensorItemsContextMenu = new QMenu(ui->GridObjects);
    hsgContextMenu = new QMenu(ui->GridObjects);
    hsgItemsContextMenu = new QMenu(ui->GridObjects);

    sourcesContextMenu->addAction("Add new current source", this, SLOT(sourcesAppend()));
    sourcesItemsContextMenu->addAction("Settings", this, SLOT(sourcesItemsSettings()));
    sourcesItemsContextMenu->addAction("Delete", this, SLOT(sourcesItemsDelete()));
    TFSFContextMenu->addAction("Add new TF/SF", this, SLOT(TFSFAppend()));
    TFSFItemsContexMenu->addAction("Settings", this, SLOT(TFSFItemsSettings()));
    TFSFItemsContexMenu->addAction("Delete", this, SLOT(TFSFItemsDelete()));
    materialContextMenu->addAction("Add new material", this, SLOT(materialAppend()));
    materialItemsContextMenu->addAction("Settings", this, SLOT(materialItemsSettings()));
    materialItemsContextMenu->addAction("Delete", this, SLOT(materialItemsDelete()));
    SensorContextMenu->addAction("Add new sensor", this, SLOT(sensorAppend()));
    SensorContextMenu->addAction("View result", this, SLOT(sensorViewResult()));
    SensorItemsContextMenu->addAction("View result", this, SLOT(sensorViewResult()));
    SensorItemsContextMenu->addAction("Settings", this, SLOT(sensorItemsSettings()));
    SensorItemsContextMenu->addAction("Delete", this, SLOT(sensorItemsDelete()));
    hsgContextMenu->addAction("Add new subgridding region", this, SLOT(hsgAppend()));
    hsgItemsContextMenu->addAction("Settings", this, SLOT(hsgItemSettings()));
    hsgItemsContextMenu->addAction("Delete", this, SLOT(hsgItemsDelete()));
}

FDTD::~FDTD()
{
    delete ui;
//    delete colorMap;
//    delete colorScale;
}

void FDTD::beforeReplot()
{
    if(startup == false)
        ui->customPlot->xAxis->setScaleRatio(ui->customPlot->yAxis, 1);     // For some odd reason, this doesn't work, the first time around
    else
        startup = false;
}

void FDTD::axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part)
{
    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "Edit label", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            ui->customPlot->replot();
        }
    }
}

void FDTD::aboutClicked()
{
    QPixmap image = QPixmap(":/images/icon.png");
    image = image.scaledToHeight(70);
    QMessageBox msgBox;
    msgBox.setIconPixmap(image);
    msgBox.setText("Full wave Maxwell field solver (2.5)");
    msgBox.setInformativeText("Based on FDTD with revolutionary subgridding techniques.\nCreated by Bert De Deckere.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void FDTD::openClicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File..."), QString(), tr("BDD-Files (*.bdd);;All Files (*)"));
    QFile f( filename );

    if ( f.open( QIODevice::ReadOnly ) )
    {
        QTextStream stream( &f );

        int reply = QMessageBox::Yes;
        if(materials.size() > 0 || currentSources.size() > 0 || sensors.size() > 0 || TFSF.size() > 0 || hsgSurfaces.size() > 0) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Do you wish to delete all objects?");
            msgBox.setInformativeText("If you continue, all objects and settings will be overwritten.");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            reply = msgBox.exec();
        }

        if (reply == QMessageBox::Yes)
        {
            materials.clear();
            while(materialItem->rowCount() > 0)
                materialItem->removeRow(0);

            currentSources.clear();
            while(sourceItem->rowCount() > 0)
                sourceItem->removeRow(0);

            TFSF.clear();
            while(TFSFItem->rowCount() > 0)
                TFSFItem->removeRow(0);

            sensors.clear();
            while(sensorItem->rowCount() > 0)
                sensorItem->removeRow(0);

            hsgSurfaces.clear();
            while(hsgItem->rowCount() > 0)
                hsgItem->removeRow(0);

            QString title;
            do {
                int header;
                stream >> header;       // Read the header

                switch(header)
                {
                case settingsIndex:
                    stream >> settings.cellsX >> settings.cellsY >> settings.sizeX >> settings.sizeY >> settings.courant;
                    stream >> settings.sigmaXMax >> settings.sigmaYMax;
                    stream >> settings.m >> settings.steps >> settings.numberOfThreads >> settings.PMLlayers;
                    stream >> settings.width >> settings.height >> settings.sampleDistance >> settings.drawNthField;
                    break;

                case materialIndex: {
                    int points;
                    stream >> points;
                    MaterialDefinition m;
                    m.p.clear();

                    for(int n=0; n<points; n++) {
                        double x, y;
                        stream >> x >> y;
                        m.p.push_back(Point(x, y));
                    }

                    stream >> m.epsr >> m.mur >> m.sigma >> m.YuMittra;
                    materials.push_back(m);

                    title = QString::number(m.p.size())+": ("+QString::number(m.p[0].x)+", "+QString::number(m.p[0].y)+")";
                    materialItem->appendRow(new QStandardItem(title));
                    break; }

                case sourceIndex: {
                    currentSource s;
                    stream >> s.frequency >> s.magnitude >> s.xpos >> s.ypos;
                    s.polarization = stream.readLine(2)[1].toLatin1();
                    s.type = stream.readLine(2)[1].toLatin1();
                    stream >> s.timeDelay >> s.pulseWidth >> s.frequencyG >> s.magnitudeG >> s.xposG >> s.yposG;
                    s.polarizationG = stream.readLine(2)[1].toLatin1();

                    currentSources.push_back(s);

                    if(s.type == 's')
                        title = "S ("+QString::number(s.xpos)+", "+QString::number(s.ypos)+")";
                    else
                        title = "G ("+QString::number(s.xposG)+", "+QString::number(s.yposG)+")";
                    sourceItem->appendRow(new QStandardItem(title));
                    break; }

                case TFSFIndex: {
                    PlaneWave p;
                    stream >> p.timeDelay >> p.pulseWidth >> p.centerFrequency >> p.angle >> p.amplitude >> p.p[0].x >> p.p[0].y >> p.p[1].x >> p.p[1].y;
                    TFSF.push_back(p);

                    title = "("+QString::number(p.p[0].x)+", "+QString::number(p.p[0].y)+") - ("+QString::number(p.p[1].x)+", "+QString::number(p.p[1].y)+")";
                    TFSFItem->appendRow(new QStandardItem(title));
                    break; }

                case sensorIndex: {
                    SensorDefinition s;
                    stream >> s.xpos >> s.ypos;
                    sensors.push_back(s);

                    title = "("+QString::number(s.xpos)+", "+QString::number(s.ypos)+")";
                    sensorItem->appendRow(new QStandardItem(title));
                    break; }

                case hsgIndex: {
                    SGInterface h;
                    stream >> h.p[0].x >> h.p[0].y >> h.p[1].x >> h.p[1].y >> h.xRatio >> h.yRatio;
                    hsgSurfaces.push_back(h);

                    title = "("+QString::number(h.p[0].x)+", "+QString::number(h.p[0].y)+") - ("+QString::number(h.p[1].x)+", "+QString::number(h.p[1].y)+")";
                    hsgItem->appendRow(new QStandardItem(title));
                    break; }
                }
            } while(!stream.atEnd());
        }
        f.close();

        if(ui->structures->isChecked() || ui->sources->isChecked())
            this->on_frameSlider_valueChanged(ui->frameSlider->value());
    }
}

void FDTD::saveAsClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("BDD-Files (*.bdd);;All Files (*)"));

    QFile f( filename );
    if ( f.open( QIODevice::WriteOnly ) )
    {
        QTextStream stream( &f );

        stream << settingsIndex << endl;                // Export current settings
        stream << settings.cellsX << " ";
        stream << settings.cellsY << " ";
        stream << settings.sizeX << " ";
        stream << settings.sizeY << " ";
        stream << settings.courant << " ";
        stream << settings.sigmaXMax << " ";
        stream << settings.sigmaYMax << " ";
        stream << settings.m << " ";
        stream << settings.steps << " ";
        stream << settings.numberOfThreads << " ";
        stream << settings.PMLlayers << " ";
        stream << settings.width << " ";
        stream << settings.height << " ";
        stream << settings.sampleDistance << " ";
        stream << settings.drawNthField << " ";

        for(int k=0; k<materials.size(); k++) {
            stream << endl << materialIndex << " " << materials[k].p.size() << endl;

            for(int n=0; n<materials[k].p.size(); n++) {
                stream << materials[k].p[n].x << " ";
                stream << materials[k].p[n].y << " ";
            }
            stream << materials[k].epsr << " ";
            stream << materials[k].mur << " ";
            stream << materials[k].sigma << " ";
            stream << materials[k].YuMittra;
        }

        for(int k=0; k<currentSources.size(); k++) {
            stream << endl << sourceIndex << endl;

            stream << currentSources[k].frequency << " ";
            stream << currentSources[k].magnitude << " ";
            stream << currentSources[k].xpos << " ";
            stream << currentSources[k].ypos << " ";
            stream << currentSources[k].polarization << " ";
            stream << currentSources[k].type << " ";
            stream << currentSources[k].timeDelay << " ";
            stream << currentSources[k].pulseWidth << " ";
            stream << currentSources[k].frequencyG << " ";
            stream << currentSources[k].magnitudeG << " ";
            stream << currentSources[k].xposG << " ";
            stream << currentSources[k].yposG << " ";
            stream << currentSources[k].polarizationG;
        }

        for(int k=0; k<TFSF.size(); k++) {
            stream << endl << TFSFIndex << endl;

            stream << TFSF[k].timeDelay << " ";
            stream << TFSF[k].pulseWidth << " ";
            stream << TFSF[k].centerFrequency << " ";
            stream << TFSF[k].angle << " ";
            stream << TFSF[k].amplitude << " ";
            stream << TFSF[k].p[0].x << " ";
            stream << TFSF[k].p[0].y << " ";
            stream << TFSF[k].p[1].x << " ";
            stream << TFSF[k].p[1].y;
        }

        for(int k=0; k<sensors.size(); k++) {
            stream << endl << sensorIndex << endl;

            stream << sensors[k].xpos << " ";
            stream << sensors[k].ypos;
        }

        for(int k=0; k<hsgSurfaces.size(); k++) {
            stream << endl << hsgIndex << endl;

            stream << hsgSurfaces[k].p[0].x << " ";
            stream << hsgSurfaces[k].p[0].y << " ";
            stream << hsgSurfaces[k].p[1].x << " ";
            stream << hsgSurfaces[k].p[1].y << " ";
            stream << hsgSurfaces[k].xRatio << " ";
            stream << hsgSurfaces[k].yRatio;
        }
    }
    f.close();
}

void FDTD::exportClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export as..."), QString(), tr("pdf files (*.pdf);;png files (*.png);;jpg files (*.jpg);;bmp files (*.bmp)"));

    QFile f( filename );
    if ( f.open( QIODevice::WriteOnly ) )
    {
        double xAxisSize, yAxisSize, plotWidth, plotHeight;

        xAxisSize = ui->customPlot->yAxis->axisRect()->width();
        yAxisSize = ui->customPlot->xAxis->axisRect()->height();
        plotWidth = ui->customPlot->size().width();
        plotHeight = ui->customPlot->size().height();

        double paddingX = plotWidth - xAxisSize;
        double paddingY = plotHeight - yAxisSize;

        ui->customPlot->resize(plotWidth+paddingX, plotWidth+paddingY);
        ui->customPlot->xAxis->setScaleRatio(ui->customPlot->yAxis, 1);

        ui->customPlot->xAxis->setScaleRatio(ui->customPlot->yAxis, 1);
        if(filename.endsWith(".pdf", Qt::CaseInsensitive))
            ui->customPlot->savePdf(filename, false, settings.width+paddingX, settings.width+paddingY);

        if(filename.endsWith(".png", Qt::CaseInsensitive)) {
            ui->customPlot->savePng(filename, settings.width+paddingX, settings.width+paddingY);
        }

        if(filename.endsWith(".jpg", Qt::CaseInsensitive)) {
            ui->customPlot->saveJpg(filename, settings.width+paddingX, settings.width+paddingY);
        }

        if(filename.endsWith(".bmp", Qt::CaseInsensitive)) {
            ui->customPlot->saveBmp(filename, settings.width+paddingX, settings.width+paddingY);
        }

        ui->customPlot->resize(plotWidth, plotHeight);
        ui->customPlot->xAxis->setScaleRatio(ui->customPlot->yAxis, 1);
        ui->customPlot->replot();
    }
}

void FDTD::onCustomContextMenu(const QPoint &point)
{
    QModelIndex item = ui->GridObjects->currentIndex();
    if(item.parent() == QModelIndex())      // Check the hierarchy level
    {
        switch(item.row())
        {
        case sourceIndex:
            sourcesContextMenu->exec(ui->GridObjects->mapToGlobal(point));  // Determine which context menu to show
            break;
        case TFSFIndex:
            TFSFContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        case materialIndex:
            materialContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        case sensorIndex: {
//            if(SensorContextMenu->actions().size() == 1 && sensors.size() > 0)
//                SensorContextMenu->addAction("View result", this, SLOT(sensorViewResult()));    // Only add the option view result, if there are any
//            else if(SensorContextMenu->actions().size() > 1 && sensors.size() == 0)
//                SensorContextMenu->removeAction(SensorContextMenu->actions()[1]);
            SensorContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        }
        case hsgIndex:
            hsgContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        }
    }
    else
    {
        switch(item.parent().row())
        {
        case sourceIndex:
            sourcesItemsContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        case TFSFIndex:
            TFSFItemsContexMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        case materialIndex:
            materialItemsContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        case sensorIndex:
            SensorItemsContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        case hsgIndex:
            hsgItemsContextMenu->exec(ui->GridObjects->mapToGlobal(point));
            break;
        }
    }
}

//////////////
/// \brief FDTD::sourcesAppend
///

void FDTD::sourcesAppend()
{
//    sources->appendRow(new QStandardItem("Source"));
//    ui->GridObjects->setCurrentIndex(sources->child(sources->rowCount()-1)->index());       // Highlight the last added item (important for index)
//    sourcesItemsSettings();
    currentSource a;
    a.index = -1;
    sourceWindow = new SourceSettings(a, points);
    sourceWindow->show();
    plotted = false;

    connect(sourceWindow, SIGNAL(Ok_clicked(currentSource)), this, SLOT(sourceSettingsOK(currentSource)));
    connect(sourceWindow, SIGNAL(drawPoint()), this, SLOT(drawPoint()));
    connect(sourceWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), sourceWindow, SLOT(drawingFinished()));
}

void FDTD::sourcesItemsSettings()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    currentSource a;
    a.index = -1;
    a = currentSources[selectedIndex];
    a.index = selectedIndex;
    sourceWindow = new SourceSettings(a, points);
    sourceWindow->show();
    plotted = false;

    connect(sourceWindow, SIGNAL(Ok_clicked(currentSource)), this, SLOT(sourceSettingsOK(currentSource)));
    connect(sourceWindow, SIGNAL(drawPoint()), this, SLOT(drawPoint()));
    connect(sourceWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), sourceWindow, SLOT(drawingFinished()));
}

void FDTD::sourcesItemsDelete()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    currentSources.erase(currentSources.begin()+selectedIndex);
    sourceItem->removeRow(selectedIndex);

    if(ui->sources->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::sourceSettingsOK(currentSource a)
{
    QString title;
    if(a.type == 's')
        title = "S ("+QString::number(a.xpos)+", "+QString::number(a.ypos)+")";
    else
        title = "G ("+QString::number(a.xposG)+", "+QString::number(a.yposG)+")";

    if(a.index == -1) {
        sourceItem->appendRow(new QStandardItem(title));
        currentSources.push_back(a);
    }
    else {
        currentSources.erase(currentSources.begin()+a.index);
        sourceItem->child(a.index)->setText(title);
        currentSources.insert(currentSources.begin()+a.index, a);
    }

    if(ui->sources->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

////////////////////
/// \brief FDTD::TFSFItemsSettings
///

void FDTD::TFSFItemsSettings()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    PlaneWave a;
    a = TFSF[selectedIndex];
    a.index = selectedIndex;
    TFSFWindow = new TFSFSettings(a, points, pen);
    TFSFWindow->show();
    plotted = false;

    connect(TFSFWindow, SIGNAL(Ok_clicked(PlaneWave)), this, SLOT(TFSFSettingsOk(PlaneWave)));
    connect(TFSFWindow, SIGNAL(drawSquare()), this, SLOT(drawSquare()));
    connect(TFSFWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), TFSFWindow, SLOT(drawingFinished()));
}

void FDTD::TFSFAppend()
{
    PlaneWave a;
    a.index = -1;       // Use this to remember that the item was newly added (not yet stored)
    TFSFWindow = new TFSFSettings(a, points, pen);
    TFSFWindow->show();
    plotted = false;

    connect(TFSFWindow, SIGNAL(Ok_clicked(PlaneWave)), this, SLOT(TFSFSettingsOk(PlaneWave)));
    connect(TFSFWindow, SIGNAL(drawSquare()), this, SLOT(drawSquare()));
    connect(TFSFWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), TFSFWindow, SLOT(drawingFinished()));
}

void FDTD::TFSFItemsDelete()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    TFSF.erase(TFSF.begin()+selectedIndex);
    TFSFItem->removeRow(selectedIndex);

    if(ui->structures->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::TFSFSettingsOk(PlaneWave a)
{
    QString title = "("+QString::number(a.p[0].x)+", "+QString::number(a.p[0].y)+") - ("+QString::number(a.p[1].x)+", "+QString::number(a.p[1].y)+")";
    if(a.index == -1) {
        TFSFItem->appendRow(new QStandardItem(title));
        TFSF.push_back(a);
    }
    else {
        TFSF.erase(TFSF.begin()+a.index);
        TFSFItem->child(a.index)->setText(title);
        TFSF.insert(TFSF.begin()+a.index, a);
    }

    if(ui->structures->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

/////////////////
/// \brief FDTD::materialAppend
///

void FDTD::materialAppend()
{
    MaterialDefinition a;
    a.index = -1;
    materialWindow = new MaterialSettings(a, points, pen);
    materialWindow->show();
    plotted = false;

    connect(materialWindow, SIGNAL(Ok_clicked(MaterialDefinition)), this, SLOT(materialSettingsOK(MaterialDefinition)));
    connect(materialWindow, SIGNAL(drawSquare()), this, SLOT(drawSquare()));
    connect(materialWindow, SIGNAL(drawPolygon()), this, SLOT(drawPolygon()));
    connect(materialWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), materialWindow, SLOT(drawingFinished()));
}

void FDTD::materialItemsSettings()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    MaterialDefinition a = materials[selectedIndex];
    a.index = selectedIndex;
    materialWindow = new MaterialSettings(a, points, pen);
    materialWindow->show();
    plotted = false;        // Do not set to true goddamnit! This would clear the last drawn structure, which is not necessarily this one

    connect(materialWindow, SIGNAL(Ok_clicked(MaterialDefinition)), this, SLOT(materialSettingsOK(MaterialDefinition)));
    connect(materialWindow, SIGNAL(drawSquare()), this, SLOT(drawSquare()));
    connect(materialWindow, SIGNAL(drawPolygon()), this, SLOT(drawPolygon()));
    connect(materialWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), materialWindow, SLOT(drawingFinished()));
}

void FDTD::materialItemsDelete()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    materials.erase(materials.begin()+selectedIndex);
    materialItem->removeRow(selectedIndex);

    if(ui->structures->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::materialSettingsOK(MaterialDefinition a)
{
    QString title = QString::number(a.p.size())+": ("+QString::number(a.p[0].x)+", "+QString::number(a.p[0].y)+")";
    if(a.index == -1) {
        materialItem->appendRow(new QStandardItem(title));
        materials.push_back(a);
    }
    else {
        materials.erase(materials.begin()+a.index);
        materialItem->child(a.index)->setText(title);
        materials.insert(materials.begin()+a.index, a);
    }

    if(ui->structures->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

///////////////////////////
/// \brief FDTD::sensorAppend
///

void FDTD::sensorAppend()
{
//    sources->appendRow(new QStandardItem("Source"));
//    ui->GridObjects->setCurrentIndex(sources->child(sources->rowCount()-1)->index());       // Highlight the last added item (important for index)
//    sourcesItemsSettings();
    SensorDefinition a;
    a.index = -1;
    sensorWindow = new SensorSettings(a, points);
    sensorWindow->show();
    plotted = false;

    connect(sensorWindow, SIGNAL(Ok_clicked(SensorDefinition)), this, SLOT(SensorSettingsOk(SensorDefinition)));
    connect(sensorWindow, SIGNAL(drawPoint()), this, SLOT(drawPoint()));
    connect(sensorWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), sensorWindow, SLOT(drawingFinished()));
}

void FDTD::sensorItemsSettings()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    SensorDefinition a = sensors[selectedIndex];
    a.index = selectedIndex;
    sensorWindow = new SensorSettings(a, points);
    sensorWindow->show();
    plotted = false;

    connect(sensorWindow, SIGNAL(Ok_clicked(SensorDefinition)), this, SLOT(SensorSettingsOk(SensorDefinition)));
    connect(sensorWindow, SIGNAL(drawPoint()), this, SLOT(drawPoint()));
    connect(sensorWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), sensorWindow, SLOT(drawingFinished()));
}

void FDTD::sensorItemsDelete()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    sensors.erase(sensors.begin()+selectedIndex);
    sensorItem->removeRow(selectedIndex);

    if(ui->sensors->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::sensorViewResult()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    sensorResult = new SensorResult(sensors);
    sensorResult->settings = &settings;
    sensorResult->show();
}

void FDTD::SensorSettingsOk(SensorDefinition a)
{
    QString title = "("+QString::number(a.xpos)+", "+QString::number(a.ypos)+")";
    if(a.index == -1) {
        sensorItem->appendRow(new QStandardItem(title));
        sensors.push_back(a);
    }
    else {
        sensors.erase(sensors.begin()+a.index);
        sensorItem->child(a.index)->setText(title);
        sensors.insert(sensors.begin()+a.index, a);
    }

    if(ui->sensors->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

//////////////////////
/// \brief FDTD::hsgAppend
///

void FDTD::hsgAppend()
{
    SGInterface a;
    a.index = -1;

    hsgWindow = new SGSettings(a, points, pen);     // points holds the points drawn on the grid
    hsgWindow->show();
    plotted = false;

    connect(hsgWindow, SIGNAL(Ok_clicked(SGInterface)), this, SLOT(hsgSettingsOk(SGInterface)));
    connect(hsgWindow, SIGNAL(drawSquare()), this, SLOT(drawSquare()));
    connect(hsgWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), hsgWindow, SLOT(drawingFinished()));
}

void FDTD::hsgItemSettings()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    SGInterface a = hsgSurfaces[selectedIndex];
    a.index = selectedIndex;
    hsgWindow = new SGSettings(a, points, pen);
    hsgWindow->show();
    plotted = false;        // Do not set to true goddamnit! This would clear the last drawn structure, which is not necessarily this one

    connect(hsgWindow, SIGNAL(Ok_clicked(SGInterface)), this, SLOT(hsgSettingsOk(SGInterface)));
    connect(hsgWindow, SIGNAL(drawSquare()), this, SLOT(drawSquare()));
    connect(hsgWindow, SIGNAL(clearLastDrawnStructure(int)), this, SLOT(clearLastDrawnStructure(int)));
    connect(this, SIGNAL(drawingFinished()), hsgWindow, SLOT(drawingFinished()));
}

void FDTD::hsgItemsDelete()
{
    int selectedIndex = ui->GridObjects->currentIndex().row();
    hsgSurfaces.erase(hsgSurfaces.begin()+selectedIndex);
    hsgItem->removeRow(selectedIndex);

    if(ui->structures->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::hsgSettingsOk(SGInterface a)
{
    QString title = "("+QString::number(a.p[0].x)+", "+QString::number(a.p[0].y)+") - ("+QString::number(a.p[1].x)+", "+QString::number(a.p[1].y)+")";
    if(a.index == -1) {
        hsgItem->appendRow(new QStandardItem(title));
        hsgSurfaces.push_back(a);
    }
    else {
        hsgSurfaces.erase(hsgSurfaces.begin()+a.index);
        hsgItem->child(a.index)->setText(title);
        hsgSurfaces.insert(hsgSurfaces.begin()+a.index, a);
    }

    if(ui->structures->isChecked())
        this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

///////////////
///
/// Everything below corresponds to drawing squares, points, ...
///

void FDTD::drawPoint()
{
    numberOfPoints = 1;
    type = 'p';
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(graphClicked(QMouseEvent*)));
}

void FDTD::drawSquare()
{
    numberOfPoints = 2;
    type = 's';
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(graphClicked(QMouseEvent*)));
}

void FDTD::drawPolygon()
{
    numberOfPoints = 2;     // any number > 1 will do
    type = 'n';
//    this->on_frameSlider_valueChanged(ui->frameSlider->value());
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(graphClicked(QMouseEvent*)));
}

void FDTD::clearLastDrawnStructure(int size)
{
    if(plotted) {
        for(int k=0; k<size; k++)          // Remove last drawn square
            ui->customPlot->removeItem(ui->customPlot->itemCount()-1);
    }

    ui->customPlot->replot();
    plotted = true;
}

void FDTD::plotPoint(QMouseEvent *event)
{
    double x0 = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y0 = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    clearLastDrawnStructure(1);

    settings.computeDifferentials();
    double radius = std::min(settings.dx, settings.dy);

    QCPItemEllipse *ellipse = new QCPItemEllipse(ui->customPlot);
    ui->customPlot->addItem(ellipse);
    ellipse->setAntialiased(true);
    ellipse->setBrush(QBrush(QColor(0, 0, 0)));
    ellipse->topLeft->setCoords(x0-radius/2, y0+radius/2);
    ellipse->bottomRight->setCoords(x0+radius/2, y0-radius/2);

    ui->customPlot->replot();
}

void FDTD::plotSquare(QMouseEvent* event)
{
    double x1 = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y1 = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    clearLastDrawnStructure(4);

    QCPItemLine *item1 = new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(item1);
    item1->setPen(pen);
    item1->start->setCoords(points[0].x, points[0].y);
    item1->end->setCoords(points[0].x, y1);

    QCPItemLine *item2 = new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(item2);
    item2->setPen(pen);
    item2->start->setCoords(points[0].x, y1);
    item2->end->setCoords(x1, y1);

    QCPItemLine *item3 = new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(item3);
    item3->setPen(pen);
    item3->start->setCoords(x1, y1);
    item3->end->setCoords(x1, points[0].y);

    QCPItemLine *item4 = new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(item4);
    item4->setPen(pen);
    item4->start->setCoords(x1, points[0].y);
    item4->end->setCoords(points[0].x, points[0].y);

    ui->customPlot->replot();
}

void FDTD::plotPolygon(QMouseEvent *event)
{
    double x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    if(!plotted) {
        plotted = true;
        clearLastDrawnStructure(points.size()-1);
    }
    else
        clearLastDrawnStructure(points.size());

    int n;
    for(n=0; n<points.size()-1; n++) {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(points[n].x, points[n].y);
        item->end->setCoords(points[n+1].x, points[n+1].y);
    }

    QCPItemLine *item = new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(item);
    item->setPen(pen);
    item->start->setCoords(points[n].x, points[n].y);
    item->end->setCoords(x, y);

    ui->customPlot->replot();
}

void FDTD::polygonFinished(QMouseEvent *event)
{
    QCPItemLine *item = new QCPItemLine(ui->customPlot);
    ui->customPlot->addItem(item);
    item->setPen(pen);
    item->start->setCoords(points[points.size()-1].x, points[points.size()-1].y);
    item->end->setCoords(points[0].x, points[0].y);

    ui->customPlot->replot();
    emit drawingFinished();

    disconnect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(polygonFinished(QMouseEvent*)));
    disconnect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(plotPolygon(QMouseEvent*)));
    disconnect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(graphClicked(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickedGraph(QMouseEvent*)));
    plotted = true;
}

void FDTD::graphClicked(QMouseEvent* event)
{
    numberOfPoints--;
    double x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());
    points.push_back(Point(x, y));

    if(numberOfPoints == 0) {       // If this was the last point, ...
        switch(type)
        {
        case 's':
            disconnect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(plotSquare(QMouseEvent*)));
            disconnect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(graphClicked(QMouseEvent*)));
            break;
        case 'p':
            plotPoint(event);
            disconnect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(graphClicked(QMouseEvent*)));
            break;
        }
        emit drawingFinished();
    }
    else {
        switch(type)
        {
        case 's':
            plotted = false;
            connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(plotSquare(QMouseEvent*)));
            break;
        case 'n':
            plotted = false;
            numberOfPoints++;
            disconnect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickedGraph(QMouseEvent*)));
            connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(polygonFinished(QMouseEvent*)));
            connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(plotPolygon(QMouseEvent*)));
            break;
        }
    }
}

/////////////
///
///

void FDTD::preferencesClicked() {
    if (preferencesWindow != NULL) {
        preferencesWindow->show();
    }
    else {
        preferencesWindow = new Preferences(0, &settings);
        connect(preferencesWindow, SIGNAL(preferencesClosed()), this, SLOT(preferencesClosed()));
        preferencesWindow->show();
    }
}

void FDTD::preferencesClosed() {
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
    delete preferencesWindow;
    preferencesWindow = NULL;
}

//////////////////////
/// \brief FDTD::on_start_clicked
/// This is where everything happens
///

void FDTD::on_start_clicked()
{
    if(ui->start->isEnabled())
    {
        ui->start->setEnabled(false);

        //
        // Start with the definition of the main grid
        //
        if(field != NULL) {
            field->deleteFields();      // seperate function, because the field is broken into pieces and given to several threads
            delete field;               // after computation, the destructor of every thread is called, but you dont want the fields to be deleted
        }

        field = new Field(settings, &computation, &mutex, &threadCounter);
        field->initFields();
        field->defineSources(currentSources);
        field->defineMaterial(materials);

        for(int k=0; k<sensors.size(); k++)
            sensors[k].initVariables(settings);
        field->sensors = sensors;

        if(TFSF.size() != 0) {
            for(int k=0; k<TFSF.size(); k++) {
                TFSF[k].settings = settings;
                TFSF[k].computePosition();
            }

            field->TFSF = TFSF;
        }

        settings.computeDifferentials();
        for(int k=0; k<hsgSurfaces.size(); k++) {
            hsgSurfaces[k].FA = field;          // This can only be done at runtime, because only now the settings are fixed
            hsgSurfaces[k].computePosition(settings);   // Order is important, first assign field, then compute position
            if(hsgSurfaces[k].FB != NULL)
                delete hsgSurfaces[k].FB;

            hsgSurfaces[k].FB = new SGField(hsgSurfaces[k].xRatio, hsgSurfaces[k].yRatio, settings, hsgSurfaces[k].p[0], hsgSurfaces[k].p[1], field->sizeWorkBuffer);
            hsgSurfaces[k].FB->initUpdateMatrices(materials);
        }
        field->hsgSurfaces = hsgSurfaces;

        ui->progressBar->setVisible(true);
        ui->frameSlider->setVisible(false);
        ui->progressBar->setMaximum(settings.steps>0? settings.steps-1 : 0);
        ui->frameSlider->setMaximum(settings.steps>0? std::ceil((double)settings.steps/settings.sampleDistance-1) : 0);

        threadCounter = 0;
        maxEx = 0; minEx = 0; maxEy = 0; minEy = 0; maxHz = 0; minHz = 0;

        //
        // Map the main grid onto different areas, and assign each area to a different thread
        //
        interior.clear();
        threadPool = new QThread*[settings.numberOfThreads];
        for(int k=0; k<settings.numberOfThreads-1; k++) {               // Keep one thread for the boundary
            interior.push_back(new Field(settings, &computation, &mutex, &threadCounter));
            interior[k]->shallowCopyFields(field);
        }

        for(int i=settings.PMLlayers; i<settings.cellsX+settings.PMLlayers; i++) {
            for(int j=settings.PMLlayers; j<settings.cellsY+settings.PMLlayers; j++) {
                bool assign = true;
                for(int k=0; k<hsgSurfaces.size(); k++) {
                    if(i >= hsgSurfaces[k].iMin - 1 &&
                       i <= hsgSurfaces[k].iMax + 1 &&
                       j >= hsgSurfaces[k].jMin - 1 &&
                       j <= hsgSurfaces[k].jMax + 1)
                        assign = false;
                }
                if(assign)
                    interior[i%(settings.numberOfThreads-1)]->patch.push_back(Area(i, i+1, j, j+1));
            }
        }

        for(int k=0; k<hsgSurfaces.size(); k++) {
            for(int i=hsgSurfaces[k].iMin - 1; i <= hsgSurfaces[k].iMax + 1; i++) {
                for(int j=hsgSurfaces[k].jMin - 1; j <=hsgSurfaces[k].jMax + 1; j++)
//                    if(i <= hsgSurfaces[k].iMin || i >= hsgSurfaces[k].iMax || j <= hsgSurfaces[k].jMin || j >= hsgSurfaces[k].jMax)
                        interior[k%(settings.numberOfThreads-1)]->patch.push_back(Area(i, i+1, j, j+1));
            }
        }

        for(int k=0; k<settings.numberOfThreads-1; k++) {
            threadPool[k] = new QThread;
            interior[k]->computeDifferentials();            // Only on startup, compute dx and such
            interior[k]->moveToThread(threadPool[k]);
            connect(threadPool[k], SIGNAL(started()), interior[k], SLOT(updateFields()));
            connect(interior[k], SIGNAL(fieldUpdateFinished(int)), this, SLOT(fieldUpdateFinished(int)));
            connect(interior[k], SIGNAL(updateGUI(double, double, double, double, double, double)), this, SLOT(computationsAreDone(double, double, double, double, double, double)));
            connect(interior[k], SIGNAL(finished()), threadPool[k], SLOT(quit()));
            connect(interior[k], SIGNAL(finished()), interior[k], SLOT(deleteLater()));
            connect(threadPool[k], SIGNAL(finished()), threadPool[k], SLOT(deleteLater()));
            threadPool[k]->start();
        }

        //
        // Finally, use the outer layer of the main grid for the boundary
        //
        boundary = new PMLBoundary(settings, &computation, &mutex, &threadCounter);
        boundary->mapFields(field);     // Order is important here, because sizeWorkBuffer gets transferred here, which is needed hereafter
        boundary->initBoundary();

        int index = settings.numberOfThreads-1;
        threadPool[index] = new QThread;
        boundary->moveToThread(threadPool[index]);
        connect(threadPool[index], SIGNAL(started()), boundary, SLOT(updateFields()));
        connect(boundary, SIGNAL(fieldUpdateFinished(int)), this, SLOT(fieldUpdateFinished(int)));
        connect(boundary, SIGNAL(updateGUI(double, double, double, double, double, double)), this, SLOT(computationsAreDone(double, double, double, double, double, double)));
        connect(boundary, SIGNAL(finished()), threadPool[index], SLOT(quit()));
        connect(boundary, SIGNAL(finished()), boundary, SLOT(deleteLater()));
        connect(threadPool[index], SIGNAL(finished()), threadPool[index], SLOT(deleteLater()));
        threadPool[index]->start();
    }
}

void FDTD::fieldUpdateFinished(int n)
{
    if(field->settings.drawNthField != 0) {
        if((n+1)%field->settings.drawNthField == 0) {
            drawField(n/field->settings.sampleDistance);
            ui->customPlot->replot();
        }
    }
    ui->progressBar->setValue(n);
}

void FDTD::computationsAreDone(double minEx, double maxEx, double minEy, double maxEy, double minHz, double maxHz) {
    mutex.lock();
    this->minEx = std::min(this->minEx, minEx);
    this->maxEx = std::max(this->maxEx, maxEx);
    this->minEy = std::min(this->minEy, minEy);
    this->maxEy = std::max(this->maxEy, maxEy);
    this->minHz = std::min(this->minHz, minHz);
    this->maxHz = std::max(this->maxHz, maxHz);

    threadCounter++;
    if(threadCounter == field->settings.numberOfThreads)
        showResults();

    mutex.unlock();
}

void FDTD::showResults() {
    ui->start->setEnabled(true);
    ui->progressBar->setVisible(false);
    ui->frameSlider->setVisible(true);
    setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());        // Update the graph
}

void FDTD::on_frameSlider_valueChanged(int timeIndex)
{
    clearPlot();

    if(field != NULL) {
        field->settings.computeDifferentials();
        ui->Time->setText(QString::number(field->settings.dt*1E9*ui->frameSlider->value()*field->settings.sampleDistance)+QString(" ns"));
        ui->Sample->setText(QString::number(ui->frameSlider->value()+1)+QString("/")+QString::number(std::ceil((double)field->settings.steps/field->settings.sampleDistance)));
    }
    else {
        settings.computeDifferentials();
        ui->Time->setText(QString::number(settings.dt*1E9*ui->frameSlider->value()*settings.sampleDistance)+QString(" ns"));
        ui->Sample->setText(QString::number(ui->frameSlider->value()+1)+QString("/")+QString::number(std::ceil((double)settings.steps/settings.sampleDistance)));
    }

    if((field != NULL) && ui->Fields->isChecked())
        drawField(timeIndex);
    if(ui->grid->isChecked()) {
        drawGrid();
        if(ui->PML->isChecked())
            drawPMLGrid();
    }
    if(ui->structures->isChecked())
        drawStructures();
    if(ui->sources->isChecked())
        drawSources();
    if(ui->sensors->isChecked())
        drawSensors();

    ui->customPlot->replot();
    // rescale the key (x) and value (y) axes so the whole color map is visible:
//    setAxisRange();
}

void FDTD::doubleClickedGraph(QMouseEvent *)
{
    if(colorScale->axis()->selectedParts().testFlag(QCPAxis::spTickLabels)) {       // Colorscale double clicked
        inputRange *temp = new inputRange(colorScale->dataRange().lower, colorScale->dataRange().upper);
        temp->show();
        connect(temp, SIGNAL(range(QCPRange)), colorMap, SLOT(setDataRange(QCPRange)));
        connect(temp, SIGNAL(destroyed(QObject*)), ui->customPlot, SLOT(replot()));
    }
    else if(ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        inputRange *temp = new inputRange(ui->customPlot->xAxis->range().lower, ui->customPlot->xAxis->range().upper);
        temp->show();
        connect(temp, SIGNAL(range(QCPRange)), ui->customPlot->xAxis, SLOT(setRange(QCPRange)));
        connect(temp, SIGNAL(destroyed(QObject*)), ui->customPlot, SLOT(replot()));
    }
    else if(ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        inputRange *temp = new inputRange(ui->customPlot->yAxis->range().lower, ui->customPlot->yAxis->range().upper);
        temp->show();
        connect(temp, SIGNAL(range(QCPRange)), ui->customPlot->yAxis, SLOT(setRange(QCPRange)));
        connect(temp, SIGNAL(destroyed(QObject*)), ui->customPlot, SLOT(replot()));
    }
    else
        setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::setAxisRange()
{
    settings.computeDifferentials();

    double factor = 1;
    switch(ui->field->currentIndex()) {
        case 0:
            colorMap->setDataRange(QCPRange(minEx/factor, maxEx/factor));
            break;
        case 1:
            colorMap->setDataRange(QCPRange(minEy/factor, maxEy/factor));
            break;
        case 2:
            colorMap->setDataRange(QCPRange(minHz/factor, maxHz/factor));
            break;
    }

    if(ui->PML->isChecked()) {
        ui->customPlot->xAxis->setRange(QCPRange(-dx*(settings.cellsX/2.0+settings.PMLlayers), dx*(settings.cellsX/2.0+settings.PMLlayers)));
        ui->customPlot->yAxis->setRange(QCPRange(-dy*(settings.cellsY/2.0+settings.PMLlayers), dy*(settings.cellsY/2.0+settings.PMLlayers)));
    }
    else {
        ui->customPlot->xAxis->setRange(QCPRange(-dx*settings.cellsX/2.0, dx*settings.cellsX/2.0));
        ui->customPlot->yAxis->setRange(QCPRange(-dy*settings.cellsY/2.0, dy*settings.cellsY/2.0));
    }

    ui->customPlot->xAxis->setScaleRatio(ui->customPlot->yAxis, 1);
    ui->customPlot->replot();
}

void FDTD::on_field_currentIndexChanged(int index)                      // Different field selected?
{
    setAxisRange();
    if(field != NULL)
        this->on_frameSlider_valueChanged(ui->frameSlider->value());    // Replot
}

void FDTD::on_grid_clicked()
{
//    setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());        // Replot
}

void FDTD::on_PML_clicked()
{
//    setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::on_Fields_clicked()
{
//    setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::on_structures_clicked()
{
//    setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::on_sources_clicked()
{
//    setAxisRange();
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::on_sensors_clicked()
{
    this->on_frameSlider_valueChanged(ui->frameSlider->value());
}

void FDTD::drawGrid()       // Possibly erroneous behaviour when changing grid after updating the fields, the grid is updated, while the fields aren't
{
    settings.computeDifferentials();
    dx = settings.dx;
    dy = settings.dy;

    QPen pen(Qt::gray);
    for (int i=0; i<settings.cellsX+1; i++)                     // Draw inner region
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(dx*(settings.cellsX/2.0-i), -dy*settings.cellsY/2.0);
        item->end->setCoords(dx*(settings.cellsX/2.0-i), dy*settings.cellsY/2.0);
    }

    for (int j=0; j<settings.cellsY+1; j++)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(dx*settings.cellsX/2.0, dy*(settings.cellsY/2.0-j));
        item->end->setCoords(-dx*settings.cellsX/2.0, dy*(settings.cellsY/2.0-j));
    }
}

void FDTD::drawPMLGrid()
{
    settings.computeDifferentials();
    dx = settings.dx;
    dy = settings.dy;

    QPen pen(QColor(255,170,100));
    for (int i=0; i<settings.PMLlayers+1; i++)                     // Draw PML (right)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(dx*(settings.cellsX/2.0+settings.PMLlayers-i), -dy*(settings.cellsY/2.0+settings.PMLlayers));
        item->end->setCoords(dx*(settings.cellsX/2.0+settings.PMLlayers-i), dy*(settings.cellsY/2.0+settings.PMLlayers));
    }

    for (int j=settings.PMLlayers; j<settings.cellsY+settings.PMLlayers+1; j++)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(dx*settings.cellsX/2.0, dy*(settings.cellsY/2.0+settings.PMLlayers-j));
        item->end->setCoords(dx*(settings.cellsX/2.0+settings.PMLlayers), dy*(settings.cellsY/2.0+settings.PMLlayers-j));
    }

    for (int i=0; i<settings.PMLlayers+1; i++)                     // Draw PML (left)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(-dx*(settings.cellsX/2.0+settings.PMLlayers-i), -dy*(settings.cellsY/2.0+settings.PMLlayers));
        item->end->setCoords(-dx*(settings.cellsX/2.0+settings.PMLlayers-i), dy*(settings.cellsY/2.0+settings.PMLlayers));
    }

    for (int j=settings.PMLlayers; j<settings.cellsY+settings.PMLlayers+1; j++)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(-dx*settings.cellsX/2.0, dy*(settings.cellsY/2.0+settings.PMLlayers-j));
        item->end->setCoords(-dx*(settings.cellsX/2.0+settings.PMLlayers), dy*(settings.cellsY/2.0+settings.PMLlayers-j));
    }

    for (int i=0; i<settings.PMLlayers+1; i++)                     // Draw PML (bottom)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(-dx*(settings.cellsX/2.0+settings.PMLlayers), -dy*(settings.cellsY/2.0+settings.PMLlayers-i));
        item->end->setCoords(dx*(settings.cellsX/2.0+settings.PMLlayers), -dy*(settings.cellsY/2.0+settings.PMLlayers-i));
    }

    for (int j=0; j<settings.cellsX; j++)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(-dx*(settings.cellsX/2.0-j), -dy*(settings.cellsY/2.0+settings.PMLlayers));
        item->end->setCoords(-dx*(settings.cellsX/2.0-j), -dy*settings.cellsY/2.0);
    }

    for (int i=0; i<settings.PMLlayers+1; i++)                     // Draw PML (top)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(-dx*(settings.cellsX/2.0+settings.PMLlayers), dy*(settings.cellsY/2.0+settings.PMLlayers-i));
        item->end->setCoords(dx*(settings.cellsX/2.0+settings.PMLlayers), dy*(settings.cellsY/2.0+settings.PMLlayers-i));
    }

    for (int j=0; j<settings.cellsX; j++)
    {
        QCPItemLine *item = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item);
        item->setPen(pen);
        item->start->setCoords(dx*(settings.cellsX/2.0-j), dy*(settings.cellsY/2.0+settings.PMLlayers));
        item->end->setCoords(dx*(settings.cellsX/2.0-j), dy*settings.cellsY/2.0);
    }
}

void FDTD::drawField(int timeIndex)
{
    field->settings.computeDifferentials();
    dx = field->settings.dx;
    dy = field->settings.dy;
    int nx, ny, start;
    double offsetX, offsetY;
    switch(ui->field->currentIndex()) {
    case 0:
        offsetX = 0;
        offsetY = 0.5;
        break;
    case 1:
        offsetX = 0.5;
        offsetY = 0;
        break;
    case 2:
        offsetX = 0;
        offsetY = 0;
        break;
    }

    if(ui->PML->isChecked()) {
        start=0;
        nx = field->settings.cellsX+2*field->settings.PMLlayers;
        ny = field->settings.cellsY+2*field->settings.PMLlayers;
        colorMap->data()->setRange(QCPRange(-dx*(field->settings.cellsX/2.0+field->settings.PMLlayers-0.5-offsetX), dx*(field->settings.cellsX/2.0+field->settings.PMLlayers-0.5+offsetX)),
                                   QCPRange(-dy*(field->settings.cellsY/2.0+field->settings.PMLlayers-0.5-offsetY), dy*(field->settings.cellsY/2.0+field->settings.PMLlayers-0.5+offsetY)));
    }
    else {
        start=field->settings.PMLlayers;
        nx = field->settings.cellsX+settings.PMLlayers;
        ny = field->settings.cellsY+settings.PMLlayers;
        colorMap->data()->setRange(QCPRange(-dx*(field->settings.cellsX/2.0-0.5-offsetX), dx*(field->settings.cellsX/2.0-0.5+offsetX)),
                                   QCPRange(-dy*(field->settings.cellsY/2.0-0.5-offsetY), dy*(field->settings.cellsY/2.0-0.5+offsetY)));
    }

    colorMap->data()->setSize(nx-start, ny-start); // we want the color map to have nx * ny data points
    for (int xIndex=start; xIndex<nx; xIndex++)
    {
      for (int yIndex=start; yIndex<ny; yIndex++)
      {
//        colorMap->data()->coordToCell(dx*(xIndex-settings.PMLlayers-settings.cellsX/2.0), dy*(yIndex-settings.PMLlayers-settings.cellsY/2.0), &x, &y);
        switch(ui->field->currentIndex()) {
            case 0:
                colorMap->data()->setCell(xIndex-start, yIndex-start, field->OBEx[timeIndex][xIndex][yIndex]);
                break;
            case 1:
                colorMap->data()->setCell(xIndex-start, yIndex-start, field->OBEy[timeIndex][xIndex][yIndex]);
                break;
            case 2:
                colorMap->data()->setCell(xIndex-start, yIndex-start, field->OBHz[timeIndex][xIndex][yIndex]);
                break;
        }
      }
    }

//    colorMap->rescaleDataRange();     // Not good, because time dependent
//    colorMap->setDataRange(QCPRange(colorMap->dataRange().upper/2, colorMap->dataRange().lower/2));
    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup marginGroup(ui->customPlot);
    ui->customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, &marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, &marginGroup);


    while(ui->customPlot->plottableCount() > 1)
        ui->customPlot->removePlottable(ui->customPlot->plottable(1));

    for(int k=0; k<hsgSurfaces.size(); k++)
    {
        if(hsgSurfaces[k].FB != NULL)
        {
            QCPColorMap *colorMapk = new QCPColorMap(ui->customPlot->xAxis, ui->customPlot->yAxis);
            ui->customPlot->addPlottable(colorMapk);

//            qDebug() << hsgSurfaces[k].iMin << hsgSurfaces[k].iMax << hsgSurfaces[k].FB->bottomLeft.x << hsgSurfaces[k].FB->topRight.x;

            colorMapk->setColorScale(colorScale);
            colorMapk->setInterpolate(false);
            colorMapk->setGradient(QCPColorGradient::gpPolar2);

            switch(ui->field->currentIndex()) {
            case 0:
                colorMapk->data()->setRange(QCPRange(hsgSurfaces[k].FB->bottomLeft.x+hsgSurfaces[k].FB->dx/2, hsgSurfaces[k].FB->topRight.x-hsgSurfaces[k].FB->dx/2),
                                            QCPRange(hsgSurfaces[k].FB->bottomLeft.y+hsgSurfaces[k].FB->dy, hsgSurfaces[k].FB->topRight.y-hsgSurfaces[k].FB->dy));
                colorMapk->data()->setSize(hsgSurfaces[k].FB->sizeExx, hsgSurfaces[k].FB->sizeExy);
                for(int i=0; i<hsgSurfaces[k].FB->sizeExx; i++) {
                    for(int j=0; j<hsgSurfaces[k].FB->sizeExy; j++) {
                        colorMapk->data()->setCell(i, j, (*hsgSurfaces[k].FB->OBf[timeIndex])(hsgSurfaces[k].FB->indexEx(i, j)));
                    }
                }
                break;
            case 1:
                colorMapk->data()->setRange(QCPRange(hsgSurfaces[k].FB->bottomLeft.x+hsgSurfaces[k].FB->dx, hsgSurfaces[k].FB->topRight.x-hsgSurfaces[k].FB->dx),
                                            QCPRange(hsgSurfaces[k].FB->bottomLeft.y+hsgSurfaces[k].FB->dy/2, hsgSurfaces[k].FB->topRight.y-hsgSurfaces[k].FB->dy/2));
                colorMapk->data()->setSize(hsgSurfaces[k].FB->sizeEyx, hsgSurfaces[k].FB->sizeEyy);
                for(int i=0; i<hsgSurfaces[k].FB->sizeEyx; i++) {
                    for(int j=0; j<hsgSurfaces[k].FB->sizeEyy; j++) {
                        colorMapk->data()->setCell(i, j, (*hsgSurfaces[k].FB->OBf[timeIndex])(hsgSurfaces[k].FB->indexEy(i, j)));
                    }
                }
                break;
            case 2:
                colorMapk->data()->setRange(QCPRange(hsgSurfaces[k].FB->bottomLeft.x+hsgSurfaces[k].FB->dx/2, hsgSurfaces[k].FB->topRight.x-hsgSurfaces[k].FB->dx/2),
                                            QCPRange(hsgSurfaces[k].FB->bottomLeft.y+hsgSurfaces[k].FB->dy/2, hsgSurfaces[k].FB->topRight.y-hsgSurfaces[k].FB->dy/2));
                colorMapk->data()->setSize(hsgSurfaces[k].FB->sizeHzx, hsgSurfaces[k].FB->sizeHzy);
                for(int i=0; i<hsgSurfaces[k].FB->sizeHzx; i++) {
                    for(int j=0; j<hsgSurfaces[k].FB->sizeHzy; j++) {
                        colorMapk->data()->setCell(i, j, (*hsgSurfaces[k].FB->OBf[timeIndex])(hsgSurfaces[k].FB->indexHz(i, j)));
                    }
                }
                break;
            }
        }
    }
}

void FDTD::drawStructures()
{
    QPen pen(QColor(128,0,128));        // Purple

    for(int k=0; k<materials.size(); k++) {
        if(materials[k].p.size() != 2)
        {
            int n;
            for(n=0; n<materials[k].p.size()-1; n++)
            {
                QCPItemLine *item = new QCPItemLine(ui->customPlot);
                ui->customPlot->addItem(item);
                item->setPen(pen);
                item->start->setCoords(materials[k].p[n].x, materials[k].p[n].y);
                item->end->setCoords(materials[k].p[n+1].x, materials[k].p[n+1].y);
            }

            QCPItemLine *item = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(item);
            item->setPen(pen);
            item->start->setCoords(materials[k].p[n].x, materials[k].p[n].y);
            item->end->setCoords(materials[k].p[0].x, materials[k].p[0].y);
        }
        else
        {
            QCPItemLine *item1 = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(item1);
            item1->setPen(pen);
            item1->start->setCoords(materials[k].p[0].x, materials[k].p[0].y);
            item1->end->setCoords(materials[k].p[1].x, materials[k].p[0].y);

            QCPItemLine *item2 = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(item2);
            item2->setPen(pen);
            item2->start->setCoords(materials[k].p[1].x, materials[k].p[0].y);
            item2->end->setCoords(materials[k].p[1].x, materials[k].p[1].y);

            QCPItemLine *item3 = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(item3);
            item3->setPen(pen);
            item3->start->setCoords(materials[k].p[1].x, materials[k].p[1].y);
            item3->end->setCoords(materials[k].p[0].x, materials[k].p[1].y);

            QCPItemLine *item4 = new QCPItemLine(ui->customPlot);
            ui->customPlot->addItem(item4);
            item4->setPen(pen);
            item4->start->setCoords(materials[k].p[0].x, materials[k].p[1].y);
            item4->end->setCoords(materials[k].p[0].x, materials[k].p[0].y);
        }
    }

    pen = QPen(QColor(0,128,128));        // Ugly green

    for(int k=0; k<TFSF.size(); k++) {             // Draw four sides of the rectangle
        QCPItemLine *item1 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item1);
        item1->setPen(pen);
        item1->start->setCoords(TFSF[k].p[0].x, TFSF[k].p[0].y);
        item1->end->setCoords(TFSF[k].p[1].x, TFSF[k].p[0].y);

        QCPItemLine *item2 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item2);
        item2->setPen(pen);
        item2->start->setCoords(TFSF[k].p[1].x, TFSF[k].p[0].y);
        item2->end->setCoords(TFSF[k].p[1].x, TFSF[k].p[1].y);

        QCPItemLine *item3 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item3);
        item3->setPen(pen);
        item3->start->setCoords(TFSF[k].p[1].x, TFSF[k].p[1].y);
        item3->end->setCoords(TFSF[k].p[0].x, TFSF[k].p[1].y);

        QCPItemLine *item4 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item4);
        item4->setPen(pen);
        item4->start->setCoords(TFSF[k].p[0].x, TFSF[k].p[1].y);
        item4->end->setCoords(TFSF[k].p[0].x, TFSF[k].p[0].y);
    }

    pen = QPen(QColor(0,0,128));        // Ugly green

    for(int k=0; k<hsgSurfaces.size(); k++) {             // Draw four sides of the rectangle
        QCPItemLine *item1 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item1);
        item1->setPen(pen);
        item1->start->setCoords(hsgSurfaces[k].p[0].x, hsgSurfaces[k].p[0].y);
        item1->end->setCoords(hsgSurfaces[k].p[1].x, hsgSurfaces[k].p[0].y);

        QCPItemLine *item2 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item2);
        item2->setPen(pen);
        item2->start->setCoords(hsgSurfaces[k].p[1].x, hsgSurfaces[k].p[0].y);
        item2->end->setCoords(hsgSurfaces[k].p[1].x, hsgSurfaces[k].p[1].y);

        QCPItemLine *item3 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item3);
        item3->setPen(pen);
        item3->start->setCoords(hsgSurfaces[k].p[1].x, hsgSurfaces[k].p[1].y);
        item3->end->setCoords(hsgSurfaces[k].p[0].x, hsgSurfaces[k].p[1].y);

        QCPItemLine *item4 = new QCPItemLine(ui->customPlot);
        ui->customPlot->addItem(item4);
        item4->setPen(pen);
        item4->start->setCoords(hsgSurfaces[k].p[0].x, hsgSurfaces[k].p[1].y);
        item4->end->setCoords(hsgSurfaces[k].p[0].x, hsgSurfaces[k].p[0].y);
    }
}

void FDTD::drawSources()
{
    settings.computeDifferentials();
    dx = settings.dx;
    dy = settings.dy;

    for(int k=0; k<currentSources.size(); k++) {
        QCPItemEllipse *ellipse = new QCPItemEllipse(ui->customPlot);
        ui->customPlot->addItem(ellipse);
        ellipse->setAntialiased(true);
        ellipse->setBrush(QBrush(QColor(0, 0, 0)));     // Color the interior black
        double radius = std::min(settings.dx, settings.dy);
        if(currentSources[k].type == 's') {
            ellipse->topLeft->setCoords(currentSources[k].xpos-radius/2, currentSources[k].ypos+radius/2);
            ellipse->bottomRight->setCoords(currentSources[k].xpos+radius/2, currentSources[k].ypos-radius/2);
        }
        else {
            ellipse->topLeft->setCoords(currentSources[k].xposG-radius/2, currentSources[k].yposG+radius/2);
            ellipse->bottomRight->setCoords(currentSources[k].xposG+radius/2, currentSources[k].yposG-radius/2);
        }
    }
}

void FDTD::drawSensors()
{
    settings.computeDifferentials();
    dx = settings.dx;
    dy = settings.dy;

    for(int k=0; k<sensors.size(); k++) {
        double radius = std::min(settings.dx, settings.dy);

        QCPItemEllipse *ellipse = new QCPItemEllipse(ui->customPlot);
        ui->customPlot->addItem(ellipse);
        ellipse->setAntialiased(true);
        ellipse->setBrush(QBrush(QColor(0, 255, 0)));
        ellipse->topLeft->setCoords(sensors[k].xpos-radius/2, sensors[k].ypos+radius/2);
        ellipse->bottomRight->setCoords(sensors[k].xpos+radius/2, sensors[k].ypos-radius/2);
    }
}

void FDTD::clearPlot()
{
    ui->customPlot->clearItems();
    colorMap->clearData();
}
