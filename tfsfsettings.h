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

#ifndef TFSFSETTINGS_H
#define TFSFSETTINGS_H

#include <QDialog>
#include "planewave.h"
#include "point.h"
#include "coordinatetable.h"

namespace Ui {
class TFSFSettings;
}

class TFSFSettings : public QDialog
{
    Q_OBJECT

public:
    PlaneWave planeWave;
    std::vector<Point> *points;

    explicit TFSFSettings(PlaneWave planeWave, std::vector<Point> &points, QPen &pen, QWidget *parent = 0);
    ~TFSFSettings();
    void updateValues();
    void closeEvent(QCloseEvent *event);
    CoordinateTable *model;

private:
    bool plotted=false;

private slots:
    void on_Ok_clicked();
    void on_Cancel_clicked();
    void on_Angle_valueChanged(int value);
    void on_CenterFrequency_valueChanged(double value);
    void on_Amplitude_valueChanged(double value);
    void on_PulseWidth_valueChanged(double value);
    void on_TimeDelay_valueChanged(double value);
    void on_square_clicked();

public slots:
    void drawingFinished();

signals:
    void clearLastDrawnStructure(int);
    void Ok_clicked(PlaneWave);
    void drawSquare();

private:
    Ui::TFSFSettings *ui;
};

#endif // TFSFSETTINGS_H
