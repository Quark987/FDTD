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

#ifndef MATERIALSETTINGS_H
#define MATERIALSETTINGS_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "materialdefinition.h"
#include "point.h"

namespace Ui {
class MaterialSettings;
}

class MaterialSettings : public QMainWindow
{
    Q_OBJECT

public:
    MaterialDefinition material;
    std::vector<Point> *points;
    char type='x';
    void updateValues();

    explicit MaterialSettings(MaterialDefinition material, std::vector<Point> &points, QPen &pen, QWidget *parent = 0);
    ~MaterialSettings();

private:
    CoordinateTable *model;
    Ui::MaterialSettings *ui;
    void closeEvent(QCloseEvent *event);

signals:
    void Ok_clicked(MaterialDefinition);
    void drawSquare();
    void drawPolygon();
    void clearLastDrawnStructure(int);

private slots:
    void on_epsr_valueChanged(double value);
    void on_mur_valueChanged(double value);
    void on_sigma_valueChanged(double value);
    void on_cancel_clicked();
    void on_ok_clicked();
    void drawingFinished();
    void on_square_clicked();
    void on_addRow_clicked();
    void on_removeRow_clicked();
    void on_polygon_clicked();
    void on_YuMittra_clicked();
};

#endif // MATERIALSETTINGS_H
