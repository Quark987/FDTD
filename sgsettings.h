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

#ifndef SGSettings_H
#define SGSettings_H

#include <QDialog>
#include <QPen>
#include <QShortcut>
#include "SGInterface.h"
#include "coordinatetable.h"

namespace Ui {
class SGSettings;
}

class SGSettings : public QDialog
{
    Q_OBJECT

public:
    explicit SGSettings(SGInterface hsgSurface, std::vector<Point> &points, QPen &pen, QWidget *parent = 0);
    ~SGSettings();
    void updateValues();
    void closeEvent(QCloseEvent *event);

private:
    bool plotted = false;
    Ui::SGSettings *ui;
    SGInterface hsgSurface;
    std::vector<Point> *points;
    CoordinateTable *model;

signals:
    void Ok_clicked(SGInterface);
    void drawSquare();
    void clearLastDrawnStructure(int);

private slots:
    void on_ok_clicked();
    void on_cancel_clicked();
    void on_xRatio_valueChanged(int value);
    void on_yRatio_valueChanged(int value);
    void on_square_clicked();
    void drawingFinished();
};

#endif // SGSettings_H
