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

#include "sensorsettings.h"
#include "ui_sensorsettings.h"
#include <QKeyEvent>
#include <QShortcut>

SensorSettings::SensorSettings(SensorDefinition sensor, std::vector<Point> &points, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorSettings)
{
    ui->setupUi(this);
    this->setWindowTitle("Settings");
    this->sensor = sensor;
    this->points = &points;

    ui->xpos->setValue(sensor.xpos);
    ui->ypos->setValue(sensor.ypos);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));
}

SensorSettings::~SensorSettings()
{
    delete ui;
}

void SensorSettings::on_Ok_clicked()
{
    emit Ok_clicked(sensor);
    this->deleteLater();
}

void SensorSettings::on_Cancel_clicked()
{
    if(plotted)
        emit clearLastDrawnStructure(1);
    this->deleteLater();
}

void SensorSettings::on_grid_clicked()
{
    plotted = true;
    this->setVisible(false);
    emit drawPoint();
}

void SensorSettings::drawingFinished()
{
    this->setVisible(true);
    this->sensor.xpos = (*points)[0].x;
    this->sensor.ypos = (*points)[0].y;
    this->points->clear();

    updateValues();
}

void SensorSettings::updateValues()
{
    ui->xpos->setValue(sensor.xpos);
    ui->ypos->setValue(sensor.ypos);
}

void SensorSettings::on_xpos_valueChanged(double value)
{
    sensor.xpos = value;
}

void SensorSettings::on_ypos_valueChanged(double value)
{
    sensor.ypos = value;
}

void SensorSettings::closeEvent(QCloseEvent *event) {
    on_Cancel_clicked();
}
