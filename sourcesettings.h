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

#ifndef SOURCESETTINGS_H
#define SOURCESETTINGS_H

#include <QMainWindow>
#include "currentsource.h"
#include "point.h"

namespace Ui {
class SourceSettings;
}

class SourceSettings : public QMainWindow
{
    Q_OBJECT

public:
    currentSource source;
    std::vector<Point> *points;

    explicit SourceSettings(currentSource source, std::vector<Point> &points, QWidget *parent = 0);
    ~SourceSettings();

    void updateValues();

private slots:
    void on_Frequency_valueChanged(double value);
    void on_Magnitude_valueChanged(double value);
    void on_xpos_valueChanged(double value);
    void on_ypos_valueChanged(double value);
    void on_Ok_clicked();
    void on_Cancel_clicked();
    void on_Grid_clicked();
    void on_Jx_clicked();
    void on_Jy_clicked();

    void on_GridGaussian_clicked();
    void on_OkGaussian_clicked();
    void on_CancelGaussian_clicked();
    void on_FrequencyGaussian_valueChanged(double value);
    void on_MagnitudeGaussian_valueChanged(double value);
    void on_PulseWidthGaussian_valueChanged(double value);
    void on_TimeDelayGaussian_valueChanged(double value);
    void on_xposGaussian_valueChanged(double value);
    void on_yposGaussian_valueChanged(double value);
    void on_JxGaussian_clicked();
    void on_JyGaussian_clicked();

public slots:
    void drawingFinished();

signals:
    void Ok_clicked(currentSource);
    void drawPoint();
    void clearLastDrawnStructure(int);

private:
    bool plotted=false;
    void closeEvent(QCloseEvent *event);
    Ui::SourceSettings *ui;
};

#endif // SOURCESETTINGS_H
