#include "mainwindow.h"
#include "QFileDialog"
#include "QFile"
#include <xlsxdocument.h>
#include <QMessageBox>
#include "QtCore/QSettings"

void MainWindow::SaveSettings()
{
    QSettings settings("Daek","Seo");
    settings.beginGroup("database");
    foreach (SS_pair data, db_dialog->GetSettings())
        settings.setValue(data.first,data.second);
    foreach (SS_pair data, dbuniq_dialog->GetSettings())
        settings.setValue(data.first,data.second);
    foreach (SS_pair data, db_add_dialog->GetSettings())
        settings.setValue(data.first,data.second);
    settings.endGroup();
}

bool MainWindow::on_m_Save_pr_as_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getSaveFileName(this,tr("Сохранить проект"),"/home","*");
    if (fileName.isEmpty())
        return false;
    if (!fileName.contains(".sce"))
        fileName+=".sce";
    QFile file (fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    QString text_saver = ui->textEdit->toHtml().toStdString().c_str();
    out << text_saver;
    out.flush();
    file.write("\a");
    if (is_job_done)
    {
        file.write("y");
        char digit_buf[20];
        for (unsigned int i=0; i<words_count; i++)
        {
            out << (m_k[i])->main_word;
            out.flush();
            file.write("\n");
            sprintf(digit_buf, "%d", (m_k[i])->met);
            file.write(digit_buf);
            file.write("\n");
            sprintf(digit_buf, "%d", (m_k[i])->low_met);
            file.write(digit_buf);
            file.write("\n");
            low_task* fiter = (*m_k[i]).head;
            if (fiter!=NULL)
            if ((*m_k[i]).head!=NULL)
                while (fiter!=NULL)
                {
                    out << fiter->word;
                    out.flush();
                    file.write("\n");
                    sprintf(digit_buf, "%d", fiter->met);
                    file.write(digit_buf);
                    file.write("\n");
                    fiter=fiter->next;
                }
            file.write("\r");
        }
        out.flush();
        file.write("\a");
        table_show_task();
    }
    else
    {
        file.write("n");
    }
    for (unsigned int cur_row =0; ui->tableWidget->item(cur_row,0); cur_row++)
    {
        out << ui->tableWidget->item(cur_row,0)->text();
        out.flush();
        file.write("\n");
        for (int cur_col=1; ui->tableWidget->item(cur_row,cur_col); cur_col ++)
        {
            if (cur_col+1>ui->tableWidget->columnCount())
            {
                ui->tableWidget->setColumnCount(cur_col+1);
                QTableWidgetItem *header_item = new QTableWidgetItem(tr("Словоформа"));
                ui->tableWidget->setHorizontalHeaderItem(cur_col,header_item);
            }
            out << ui ->tableWidget->item(cur_row,cur_col)->text();
            out.flush();
            file.write("\n");
        }
        file.write("\r");
    }
    out.flush();
    pr_name = fileName;
    setWindowTitle(pr_name);
    ui->m_Save_pr_as->setEnabled(false);
    is_pr_saved = true;
    return true;
}

void MainWindow::on_m_Save_r_as_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    fileName.append(".xlsx");
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) && is_job_done)
        file.close();
    else
        return;

    QXlsx::Document xlsx;
    QString index = "A1";
    off_cch = true;
    table_show_results();
    off_cch = false;
    for (unsigned int cur_row =0; ui->tableWidget->item(cur_row,0); cur_row++)
    {
        index = "A";
        char cur_index[15];
        sprintf(cur_index, "%d", cur_row+2);
        index += cur_index;
        xlsx.write(index,ui->tableWidget->item(cur_row,0)->text());
        char literal = 'A';
        for (unsigned int cur_col=1; ui->tableWidget->item(cur_row,cur_col); cur_col ++)
        {
            literal+=1;
            index[0] = literal;
            xlsx.write(index,ui->tableWidget->item(cur_row,cur_col)->text());
        }
        index[0] = 'A';
    }
    xlsx.saveAs(fileName);
}


void MainWindow::on_m_Save_file_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    fileName+=".txt";
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
    out.flush();
    file.close();
}
