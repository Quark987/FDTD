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

#include "materialsettings.h"
#include "ui_materialsettings.h"
#include <QPen>
#include <QShortcut>
#include <QStandardItemModel>
#include <QMessageBox>
#include <qdebug.h>

MaterialSettings::MaterialSettings(MaterialDefinition material, std::vector<Point> &points, QPen &pen, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MaterialSettings)
{
    ui->setupUi(this);
    this->material = material;
    this->points = &points;
    pen = QPen(QColor(128,0,128));
    this->setWindowTitle("Settings");

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));

    model = new CoordinateTable(this->material.p, this);
    ui->coordinates->setModel(model);
    updateValues();
}

MaterialSettings::~MaterialSettings()
{
    delete ui;
}

void MaterialSettings::updateValues()
{
    ui->epsr->setValue(material.epsr);
    ui->mur->setValue(material.mur);
    ui->sigma->setValue(material.sigma);
    ui->YuMittra->setChecked(material.YuMittra);
}

void MaterialSettings::drawingFinished()
{
    this->setVisible(true);
    material.p.clear();
    for(int k=0; k<(*points).size(); k++)
        material.p.push_back((*points)[k]);

    model->updateView();
    points->clear();
}

void MaterialSettings::on_epsr_valueChanged(double value)
{
    material.epsr = value;
}

void MaterialSettings::on_mur_valueChanged(double value)
{
    material.mur = value;
}

void MaterialSettings::on_sigma_valueChanged(double value)
{
    material.sigma = value;
}

void MaterialSettings::on_cancel_clicked()
{
    if(type == 's')
        emit clearLastDrawnStructure(4);
    if(type == 'p')
        emit clearLastDrawnStructure(material.p.size());
    this->deleteLater();
}

void MaterialSettings::on_ok_clicked()
{
//    if (material.p.size() == 2) {     // If a rectangle was drawn
//        material.p.push_back(Point(material.p[1].x, material.p[0].y));
//        material.p.insert(material.p.begin()+1, Point(material.p[0].x, material.p[1].y));      // Add the two remaining coordinates
//    }

    emit Ok_clicked(material);
    this->deleteLater();
}

void MaterialSettings::closeEvent(QCloseEvent *event)
{
    on_cancel_clicked();
//    on_ok_clicked();        // Pressing command+w will act as if ok was clicked -> not good, closing window also equals ok
}

void MaterialSettings::on_square_clicked()
{
    if(type == 'p' && material.p.size() > 0)
        emit clearLastDrawnStructure(material.p.size());
    if(type == 's' && material.p.size() > 0)
        emit clearLastDrawnStructure(4);
    points->clear();

    type = 's';
    this->setVisible(false);
    emit drawSquare();
}

void MaterialSettings::on_addRow_clicked()
{
    QItemSelectionModel *select = ui->coordinates->selectionModel();
    if(select->hasSelection() && (!select->selectedRows().isEmpty())) {
        int start = select->selectedRows().first().row();
        material.p.insert(material.p.begin()+start, Point(0, 0));
    }
    else
        material.p.push_back(Point(0, 0));
    model->updateView();
}

void MaterialSettings::on_removeRow_clicked()
{
    QItemSelectionModel *select = ui->coordinates->selectionModel();
    if(select->hasSelection() && (!select->selectedRows().isEmpty())) {
        int start = select->selectedRows().first().row();
        int stop =  select->selectedRows().last().row();

        for(int k=0; k < stop-start+1; k++)
            material.p.erase(material.p.begin()+start);
    }
    else if (material.p.size() > 0)
        material.p.erase(material.p.end()-1);

    model->updateView();
}

void MaterialSettings::on_polygon_clicked()
{
    if(type == 's' && material.p.size() > 0)
        emit clearLastDrawnStructure(4);
    if(type == 'p' && material.p.size() > 0)
        emit clearLastDrawnStructure(material.p.size());
    points->clear();

    type = 'p';
    this->setVisible(false);
    emit drawPolygon();
}

void MaterialSettings::on_YuMittra_clicked()
{
    if((material.sigma != 0 || material.mur != 1) && this->ui->YuMittra->isChecked() == true)
    {
        int reply = QMessageBox::Yes;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Are you sure you want to proceed?");
        msgBox.setInformativeText("Yu-Mittra is only inteded for materials where σ = 0 and μr = 1");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        reply = msgBox.exec();

        if (reply == QMessageBox::Yes)
            material.YuMittra = ui->YuMittra->isChecked();
    }
    else
        material.YuMittra = ui->YuMittra->isChecked();
}
