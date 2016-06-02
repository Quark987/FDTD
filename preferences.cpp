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

#include "preferences.h"
#include "ui_preferences.h"
#include <math.h>
#include <qdebug.h>
#include <QShortcut>

#define c           299792458       // Speed of light
#define eta         377             // Free space impedance

Preferences::Preferences(QWidget *parent, Settings* settings) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));

    this->setWindowTitle("Preferences");
    this->settings = settings;
    ui->cellsX->setValue(settings->cellsX);
    ui->cellsY->setValue(settings->cellsY);
    ui->sizeX->setValue(settings->sizeX);
    ui->sizeY->setValue(settings->sizeY);
    ui->courant->setValue(settings->courant);
    ui->timesteps->setValue(settings->steps);

    ui->PMLlayers->setValue(settings->PMLlayers);
    ui->sigmaxMax->setValue(settings->sigmaXMax);
    ui->sigmayMax->setValue(settings->sigmaYMax);
    ui->NoOfThreads->setValue(settings->numberOfThreads);
    ui->m->setValue(settings->m);

    ui->sampleDistance->setValue(settings->sampleDistance);
    ui->height->setValue(settings->height);
    ui->width->setValue(settings->width);
    ui->DrawNthField->setValue(settings->drawNthField);
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::closeEvent(QCloseEvent *event) {
    emit preferencesClosed();
//    this->deleteLater();      // Do not delete, this keeps the selected item selected when reopening
}

void Preferences::on_Panel_currentRowChanged(int currentRow)
{
    ui->Settings->setCurrentIndex(currentRow);
}

void Preferences::on_cellsX_valueChanged(int value)
{
    if(!processing) {
        processing = true;
        settings->cellsX = value;
        settings->computeDifferentials();
        ui->time->setValue(1E9*settings->dt*(settings->steps-1));
        settings->maxTime = (int)(settings->dt*settings->steps);
        processing = false;
    }
}

void Preferences::on_cellsY_valueChanged(int value)
{
    if(!processing) {
        processing = true;
        settings->cellsY = value;
        settings->computeDifferentials();
        ui->time->setValue(1E9*settings->dt*(settings->steps-1));
        settings->maxTime = (int)(settings->dt*settings->steps);
        processing = false;
    }
}

void Preferences::on_sizeX_valueChanged(double value)
{
    if(!processing) {
        processing = true;
        settings->sizeX = value;
        settings->computeDifferentials();
        ui->time->setValue(1E9*settings->dt*(settings->steps-1));
        settings->maxTime = (int)(settings->dt*settings->steps);
        processing = false;
    }
}

void Preferences::on_sizeY_valueChanged(double value)
{
    if(!processing) {
        processing = true;
        settings->sizeY = value;
        settings->computeDifferentials();
        ui->time->setValue(1E9*settings->dt*(settings->steps-1));
        settings->maxTime = (int)(settings->dt*settings->steps);
        processing = false;
    }
}

void Preferences::on_courant_valueChanged(double value)
{
    if(!processing) {
        processing = true;
        settings->courant = value;
        settings->computeDifferentials();
        ui->time->setValue(1E9*settings->dt*(settings->steps-1));
        processing = false;
    }
}

void Preferences::on_time_valueChanged(double value)
{
    if(!processing) {
        processing = true;
        settings->maxTime = 1E-9*value;
        settings->computeDifferentials();
        ui->timesteps->setValue((int)(settings->maxTime/settings->dt));
        settings->steps = (int)(settings->maxTime/settings->dt);
        processing = false;
    }
}

void Preferences::on_timesteps_valueChanged(int value)
{
    if(!processing) {
        processing = true;
        settings->steps = value;
        settings->computeDifferentials();
        ui->time->setValue(1E9*settings->dt*(settings->steps-1));
        settings->steps = value;
        processing = false;
    }
}

void Preferences::on_PMLlayers_valueChanged(int value)
{
    settings->PMLlayers = value;
}

void Preferences::on_sigmaxMax_valueChanged(double value)
{
    settings->sigmaXMax = value;
}

void Preferences::on_sigmayMax_valueChanged(double value)
{
    settings->sigmaYMax = value;
}

void Preferences::on_Auto_clicked()
{
    settings->computeDifferentials();
    settings->sigmaXMax = 0.8*(settings->m+1)/(eta*settings->dx);
    settings->sigmaYMax = 0.8*(settings->m+1)/(eta*settings->dy);

    ui->sigmaxMax->setValue(settings->sigmaXMax);
    ui->sigmayMax->setValue(settings->sigmaYMax);
}

void Preferences::on_NoOfThreads_valueChanged(int value)
{
    settings->numberOfThreads = value;
}

void Preferences::on_m_valueChanged(double value)
{
    settings->m = value;
}

void Preferences::on_sampleDistance_valueChanged(int value)
{
    settings->sampleDistance = value;
}

void Preferences::on_width_valueChanged(int value)
{
    settings->width = value;
}

void Preferences::on_height_valueChanged(int value)
{
    settings->height = value;
}

void Preferences::on_DrawNthField_valueChanged(int value)
{
    settings->drawNthField = value;
}
