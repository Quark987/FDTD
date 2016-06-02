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

#include "tfsfsettings.h"
#include "ui_tfsfsettings.h"
#include <qdebug.h>
#include <QPen>
#include <QShortcut>

TFSFSettings::TFSFSettings(PlaneWave planeWave, std::vector<Point> &points, QPen &pen, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TFSFSettings)
{
    ui->setupUi(this);
    this->planeWave = planeWave;
    this->points = &points;
    pen = QPen(QColor(0,128,128));        // Draw the hsg surfaces in blue
    this->setWindowTitle("Settings");

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));

    model = new CoordinateTable(this->planeWave.p, this);
    ui->coordinates->setModel(model);
    updateValues();
}

TFSFSettings::~TFSFSettings()
{
    delete ui;
}

void TFSFSettings::closeEvent(QCloseEvent *event)
{
    on_Cancel_clicked();
}

void TFSFSettings::updateValues()
{
    ui->Angle->setValue(planeWave.angle);
    ui->PulseWidth->setValue(planeWave.pulseWidth*1E9);
    ui->CenterFrequency->setValue(planeWave.centerFrequency/1E6);
    ui->Amplitude->setValue(planeWave.amplitude);
    ui->TimeDelay->setValue(planeWave.timeDelay*1E9);
}

void TFSFSettings::on_Ok_clicked()
{
    emit Ok_clicked(planeWave);
    this->deleteLater();
}

void TFSFSettings::on_Cancel_clicked()
{
    if(plotted)
        emit clearLastDrawnStructure(4);
    this->deleteLater();
}

void TFSFSettings::on_Angle_valueChanged(int value)
{
    planeWave.angle = value;
}

void TFSFSettings::on_CenterFrequency_valueChanged(double value)
{
    planeWave.centerFrequency = value*1E6;
}

void TFSFSettings::drawingFinished()
{
    this->setVisible(true);
    planeWave.p.clear();
    for(int k=0; k<(*points).size(); k++)
        planeWave.p.push_back((*points)[k]);

    model->updateView();
    points->clear();
}

void TFSFSettings::on_Amplitude_valueChanged(double value)
{
    planeWave.amplitude = value;
}

void TFSFSettings::on_PulseWidth_valueChanged(double value)
{
    planeWave.pulseWidth = value * 1E-9;
}

void TFSFSettings::on_TimeDelay_valueChanged(double value)
{
    planeWave.timeDelay = value * 1E-9;
}

void TFSFSettings::on_square_clicked()
{
    if(plotted)
        emit clearLastDrawnStructure(4);

    plotted = true;
    this->setVisible(false);
    emit drawSquare();
}
