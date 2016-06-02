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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include "settings.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT

public:
    Settings *settings;
    bool processing = false;

    explicit Preferences(QWidget *parent = 0, Settings *settings = NULL);
    ~Preferences();
    void closeEvent(QCloseEvent *event);

signals:
    void transferData();
    void preferencesClosed();

private slots:
    // Size
    void on_Panel_currentRowChanged(int currentRow);
    void on_cellsX_valueChanged(int value);
    void on_cellsY_valueChanged(int value);
    void on_sizeX_valueChanged(double value);
    void on_sizeY_valueChanged(double value);
    void on_courant_valueChanged(double value);
    void on_time_valueChanged(double value);
    void on_timesteps_valueChanged(int value);
    void on_PMLlayers_valueChanged(int value);

    // PML
    void on_sigmaxMax_valueChanged(double value);
    void on_sigmayMax_valueChanged(double value);
    void on_Auto_clicked();
    void on_NoOfThreads_valueChanged(int value);
    void on_m_valueChanged(double value);

    // Save
    void on_sampleDistance_valueChanged(int value);
    void on_width_valueChanged(int value);
    void on_height_valueChanged(int value);

    // Computation
    void on_DrawNthField_valueChanged(int value);

private:
    Ui::Preferences *ui;
};

#endif // PREFERENCES_H
