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

#include "SGSettings.h"
#include "ui_SGSettings.h"

SGSettings::SGSettings(SGInterface hsgSurface, std::vector<Point> &points, QPen &pen, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SGSettings)
{
    ui->setupUi(this);
    this->hsgSurface = hsgSurface;
    this->points = &points;
    pen = QPen(QColor(0,0,128));        // Draw the hsg surfaces in blue
    this->setWindowTitle("Settings");

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));

    model = new CoordinateTable(this->hsgSurface.p, this);
    ui->coordinates->setModel(model);
    updateValues();
}

SGSettings::~SGSettings()
{
    delete ui;
}

void SGSettings::closeEvent(QCloseEvent *)
{
    on_cancel_clicked();
}

void SGSettings::updateValues()
{
    ui->xRatio->setValue(hsgSurface.xRatio);
    ui->yRatio->setValue(hsgSurface.yRatio);
}

void SGSettings::on_ok_clicked()
{
    emit Ok_clicked(hsgSurface);
    this->deleteLater();
}

void SGSettings::on_cancel_clicked()
{
    if(plotted)
        emit clearLastDrawnStructure(4);
    this->deleteLater();
}

void SGSettings::on_xRatio_valueChanged(int value)
{
    hsgSurface.xRatio = value;
}

void SGSettings::on_yRatio_valueChanged(int value)
{
    hsgSurface.yRatio = value;
}

void SGSettings::on_square_clicked()
{
    if(plotted)
        emit clearLastDrawnStructure(4);
    plotted = true;

    this->setVisible(false);
    emit drawSquare();
}

void SGSettings::drawingFinished()
{
    this->setVisible(true);
    hsgSurface.p.clear();
    hsgSurface.p.push_back(Point((*points)[0].x, (*points)[0].y));
    hsgSurface.p.push_back(Point((*points)[1].x, (*points)[1].y));
//    hsgSurface.p.push_back(Point(std::min((*points)[0].x, (*points)[1].x), std::min((*points)[0].y, (*points)[1].y)));
//    hsgSurface.p.push_back(Point(std::max((*points)[0].x, (*points)[1].x), std::max((*points)[0].y, (*points)[1].y)));

    model->updateView();
    points->clear();
}
