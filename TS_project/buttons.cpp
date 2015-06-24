#include "mainwindow.h"

void MainWindow::on_toolButton_clicked()
{
    QList<int> sizes;
    if (ui->toolButton->arrowType()==Qt::RightArrow)
    {
        ui->toolButton->setArrowType(Qt::LeftArrow);
        sizes.append(ui->textEdit->width());
        sizes.append(0);
    }
    else
    if (ui->toolButton->arrowType()==Qt::LeftArrow)
    {
        ui->toolButton->setArrowType(Qt::RightArrow);
        sizes.append(ui->textEdit->width()*3/4);
        sizes.append(ui->textEdit->width()/4);
    }
    else
    if (ui->toolButton->arrowType()==Qt::DownArrow)
    {
        ui->toolButton->setArrowType(Qt::UpArrow);
        sizes.append(ui->textEdit->width()/2);
        sizes.append(0);
    }
    else
    if (ui->toolButton->arrowType()==Qt::UpArrow)
    {
        ui->toolButton->setArrowType(Qt::DownArrow);
        sizes.append(ui->textEdit->width()/2);
        sizes.append(ui->textEdit->width()/2);
    }
    ui->splitter->setSizes(sizes);
}

void MainWindow::on_display_horiz_triggered()
{
    if (ui->splitter->orientation()==Qt::Horizontal)
    {
        ui->splitter->setOrientation(Qt::Vertical);
        ui->gridLayout->addWidget(ui->toolButton,2,0);
        ui->toolButton->setMaximumWidth(1600000);
        ui->toolButton->setMaximumHeight(15);
        if (ui->tableWidget->isHidden() || ui->tableWidget->height()==0)
            ui->toolButton->setArrowType(Qt::UpArrow);
        else
            ui->toolButton->setArrowType(Qt::DownArrow);
        ui->toolButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
        ui->display_horiz->setText("Отобразить вертикально");
    }
    else
    {
        ui->splitter->setOrientation(Qt::Horizontal);
        ui->gridLayout->addWidget(ui->toolButton,0,1);
        ui->gridLayout->addWidget(ui->textEdit,0,0);
        ui->toolButton->setMaximumHeight(1600000);
        ui->toolButton->setMaximumWidth(15);
        if (ui->tableWidget->isHidden() || ui->tableWidget->width()==0)
            ui->toolButton->setArrowType(Qt::LeftArrow);
        else
            ui->toolButton->setArrowType(Qt::RightArrow);
        ui->toolButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
        ui->display_horiz->setText("Отобразить горизонтально");
    }
}

void MainWindow::on_m_show_results_triggered()
{
    off_cch = true;
    table_show_results();
    off_cch = false;
}

void MainWindow::on_m_show_task_triggered()
{
    off_cch = true;
    table_show_task();
    off_cch = false;
}

void MainWindow::on_m_hide_table_triggered()
{
    if (ui->m_hide_table->isChecked())
    {
        ui->tableWidget->hide();
        if (ui->splitter->orientation()==Qt::Horizontal)
            ui->toolButton->setArrowType(Qt::LeftArrow);
        else
            ui->toolButton->setArrowType(Qt::UpArrow);
    }
    else
    {
        ui->tableWidget->show();
        if (ui->splitter->orientation()==Qt::Horizontal)
            if (ui->tableWidget->width()>0)
                ui->toolButton->setArrowType(Qt::RightArrow);
            else
                ui->toolButton->setArrowType(Qt::LeftArrow);
        else
            if (ui->tableWidget->height()>0)
                ui->toolButton->setArrowType(Qt::DownArrow);
            else
                ui->toolButton->setArrowType(Qt::UpArrow);
    }
}

void MainWindow::on_m_Quit_triggered()
{
    this->close();
}
