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

#ifndef INPUTRANGE_H
#define INPUTRANGE_H

#include <QDialog>
#include "qcustomplot.h"

namespace Ui {
class inputRange;
}

class inputRange : public QDialog
{
    Q_OBJECT

public:
    explicit inputRange(double min, double max, QWidget *parent = 0);
    ~inputRange();

signals:
    void range(QCPRange);

private slots:
    void on_maximum_editingFinished();
    void on_minimum_editingFinished();
    void on_Ok_clicked();
    void on_Cancel_clicked();
    void closeEvent(QCloseEvent *);

private:
    double min=0, max=0;
    Ui::inputRange *ui;
};

#endif // INPUTRANGE_H
