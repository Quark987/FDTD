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

#include "inputrange.h"
#include "ui_inputrange.h"
#include "qcustomplot.h"
#include <QShortcut>

inputRange::inputRange(double min, double max, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inputRange)
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(close()));
    ui->setupUi(this);

    ui->maximum->setText(QString::number(max));
    ui->minimum->setText(QString::number(min));

    this->setWindowTitle("Set Range");
}

inputRange::~inputRange()
{
    delete ui;
}

void inputRange::closeEvent(QCloseEvent *) {
    on_Cancel_clicked();
}

void inputRange::on_Ok_clicked()
{
    emit range(QCPRange(min, max));
    this->deleteLater();
}

void inputRange::on_Cancel_clicked()
{
    emit range(QCPRange(min, max));
    this->deleteLater();
}

void inputRange::on_minimum_editingFinished()
{
    min = ui->minimum->text().toDouble();
}

void inputRange::on_maximum_editingFinished()
{
    max = ui->maximum->text().toDouble();
}
