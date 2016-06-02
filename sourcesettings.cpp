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

#include "sourcesettings.h"
#include "ui_sourcesettings.h"
#include <qdebug.h>
#include <QShortcut>

SourceSettings::SourceSettings(currentSource source, std::vector<Point> &points, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SourceSettings)
{
    this->source = source;
    this->points = &points;
    ui->setupUi(this);
    this->setWindowTitle("Settings");

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));

    ui->xpos->setValue(source.xpos);
    ui->ypos->setValue(source.ypos);
    ui->Magnitude->setValue(source.magnitude);
    ui->Frequency->setValue(source.frequency);

    ui->xposGaussian->setValue(source.xposG);
    ui->yposGaussian->setValue(source.yposG);
    ui->FrequencyGaussian->setValue(source.frequencyG);
    ui->PulseWidthGaussian->setValue(source.pulseWidth*1E9);
    ui->TimeDelayGaussian->setValue(source.timeDelay*1E9);
    ui->MagnitudeGaussian->setValue(source.magnitudeG);

    if(source.polarization == 'x')
        ui->Jx->setChecked(true);
    else
        ui->Jy->setChecked(true);

    if(source.polarizationG == 'x')
        ui->JxGaussian->setChecked(true);
    else
        ui->JyGaussian->setChecked(true);

    if(source.type == 'g')
        ui->tabWidget->setCurrentIndex(1);
}

void SourceSettings::closeEvent(QCloseEvent *event) {
    on_Cancel_clicked();
}

SourceSettings::~SourceSettings()
{
    delete ui;
}

void SourceSettings::drawingFinished()
{
    this->setVisible(true);
    if(ui->tabWidget->currentIndex() == 0) {
        this->source.xpos = (*points)[0].x;
        this->source.ypos = (*points)[0].y;
    }
    else {
        this->source.xposG = (*points)[0].x;
        this->source.yposG = (*points)[0].y;
    }
    this->points->clear();

    updateValues();
}

void SourceSettings::updateValues()
{
    ui->xpos->setValue(source.xpos);
    ui->ypos->setValue(source.ypos);

    ui->xposGaussian->setValue(source.xposG);
    ui->yposGaussian->setValue(source.yposG);

}

void SourceSettings::on_Frequency_valueChanged(double value)
{
    source.frequency = value;
}

void SourceSettings::on_Magnitude_valueChanged(double value)
{
    source.magnitude = value;
}

void SourceSettings::on_xpos_valueChanged(double value)
{
    source.xpos = value;
}

void SourceSettings::on_ypos_valueChanged(double value)
{
    source.ypos = value;
}

void SourceSettings::on_Ok_clicked()
{
    if(ui->tabWidget->currentIndex() == 0)
        source.type = 's';
    else
        source.type = 'g';

    emit Ok_clicked(source);
    this->deleteLater();
}

void SourceSettings::on_Cancel_clicked()
{
    if(plotted)
        emit clearLastDrawnStructure(1);
    this->deleteLater();
}

void SourceSettings::on_Grid_clicked()
{
    plotted=true;
    this->setVisible(false);
    emit drawPoint();
}

void SourceSettings::on_Jx_clicked()
{
    source.polarization = 'x';
    ui->Jy->setChecked(false);
    ui->Jx->setChecked(true);
}

void SourceSettings::on_Jy_clicked()
{
    source.polarization = 'y';
    ui->Jy->setChecked(true);
    ui->Jx->setChecked(false);
}

void SourceSettings::on_GridGaussian_clicked()
{
    on_Grid_clicked();
}

void SourceSettings::on_OkGaussian_clicked()
{
    on_Ok_clicked();
}

void SourceSettings::on_CancelGaussian_clicked()
{
    on_Cancel_clicked();
}

void SourceSettings::on_FrequencyGaussian_valueChanged(double value)
{
    source.frequencyG = value;
}

void SourceSettings::on_MagnitudeGaussian_valueChanged(double value)
{
    source.magnitudeG = value;
}

void SourceSettings::on_PulseWidthGaussian_valueChanged(double value)
{
    source.pulseWidth = value*1E-9;
}

void SourceSettings::on_TimeDelayGaussian_valueChanged(double value)
{
    source.timeDelay = value*1E-9;
}

void SourceSettings::on_xposGaussian_valueChanged(double value)
{
    source.xposG = value;
}

void SourceSettings::on_yposGaussian_valueChanged(double value)
{
    source.yposG = value;
}

void SourceSettings::on_JxGaussian_clicked()
{
    source.polarizationG = 'x';
    ui->JyGaussian->setChecked(false);
}

void SourceSettings::on_JyGaussian_clicked()
{
    source.polarizationG = 'y';
    ui->JxGaussian->setChecked(false);
}
