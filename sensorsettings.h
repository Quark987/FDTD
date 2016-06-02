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

#ifndef SENSORSETTINGS_H
#define SENSORSETTINGS_H

#include <QDialog>
#include "sensordefinition.h"
#include "point.h"

namespace Ui {
class SensorSettings;
}

class SensorSettings : public QDialog
{
    Q_OBJECT

public:
    SensorDefinition sensor;
    std::vector<Point> *points;

    explicit SensorSettings(SensorDefinition sensor, std::vector<Point> &points, QWidget *parent = 0);
    ~SensorSettings();
    void updateValues();
    void closeEvent(QCloseEvent *event);


private slots:
    void on_Ok_clicked();
    void on_Cancel_clicked();
    void on_grid_clicked();
    void on_xpos_valueChanged(double value);
    void on_ypos_valueChanged(double value);

public slots:
    void drawingFinished();

signals:
    void Ok_clicked(SensorDefinition);
    void drawPoint();
    void clearLastDrawnStructure(int);

private:
    bool plotted = false;
    Ui::SensorSettings *ui;
};

#endif // SENSORSETTINGS_H
