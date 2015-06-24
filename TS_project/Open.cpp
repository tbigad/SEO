#include "mainwindow.h"
#include "QFileDialog"
#include "QFile"
#include <xlsxdocument.h>
#include <QMessageBox>
#include <stdio.h>
#include "QtCore/QSettings"
#include "/home/daek/Документы/sandbox9/debian/libopc-dev/usr/include/opc/opc.h"
#include "/home/daek/Документы/sandbox9/debian/libopc-dev/usr/include/opc/container.h"
#include "/home/daek/Документы/sandbox9/debian/libopc-dev/usr/include/opc/inputstream.h"
#include "/home/daek/Документы/sandbox9/debian/libopc-dev/usr/include/opc/part.h"

void MainWindow::LoadSettings()
{
    QList<std::pair<QString,QString> > list;
    QSettings settings("Daek","Seo");
    settings.beginGroup("database");
    list<<std::make_pair("db_name",settings.value("db_name","").toString());
    list<<std::make_pair("db_username",settings.value("db_username","").toString());
    list<<std::make_pair("db_password",settings.value("db_password","").toString());
    list<<std::make_pair("db_hostname",settings.value("db_hostname","").toString());
    list<<std::make_pair("db_port",settings.value("db_port","").toString());
    db_dialog->SetSettings(list);
    list.clear();
    list<<std::make_pair("uniq_shingl",settings.value("uniq_shingl","").toString());
    list<<std::make_pair("uniq_table",settings.value("uniq_table","").toString());
    list<<std::make_pair("uniq_search_type",settings.value("uniq_search_type","0").toString());
    list<<std::make_pair("uniq_column",settings.value("uniq_column","").toString());
    list<<std::make_pair("uniq_field_name",settings.value("uniq_field_name","").toString());
    dbuniq_dialog->SetSettings(list);
    list.clear();
    list<<std::make_pair("add_table_name",settings.value("add_table_name","").toString());
    list<<std::make_pair("add_table_id",settings.value("add_table_id","0").toString());
    db_add_dialog->SetSettings(list);
    settings.endGroup();
}

void MainWindow::on_m_New_pr_triggered()
{
    if (!is_pr_saved)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Текущий проект не сохранён!");
        msgBox.setText("Перед созданием нового, сохранить текущий проект?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Save,"Сохранить");
        msgBox.setButtonText(QMessageBox::Discard,"Создать не сохраняя");
        msgBox.setButtonText(QMessageBox::Cancel,"Отменить создание");
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Save)
            if (!on_m_Save_pr_as_triggered())
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Внимание!");
                msgBox.setText("Текущий проект не сохранён!");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setButtonText(QMessageBox::Ok,"Ок");
                msgBox.exec();
            }
        else
            if (ret == QMessageBox::Cancel)
                return;
    }
    ui->textEdit->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setRowCount(0);

    pr_name = "Новый проект";
    fake_text.clear();
    off_pr(false);
    delete_task_data();
    tpl=0;
    founded_words.clear();
}

void MainWindow::on_m_Open_pr_triggered()
{
    if (!is_pr_saved)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Текущий проект не сохранён!");
        msgBox.setText("Перед открытием, сохранить текущий проект?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Save,"Сохранить");
        msgBox.setButtonText(QMessageBox::Discard,"Открыть не сохраняя");
        msgBox.setButtonText(QMessageBox::Cancel,"Отменить открытие");
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Save)
            if (!on_m_Save_pr_as_triggered())
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Внимание!");
                msgBox.setText("Текущий проект не сохранён!");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setButtonText(QMessageBox::Ok,"Ок");
                msgBox.exec();
            }
        if (ret == QMessageBox::Cancel)
            return;
    }

    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getOpenFileName(this,tr("Открыть проект"),"/home", "*.sce");
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
        return;

    off_cch = true;
    off_ct = true;
    delete_task_data();
    ui->textEdit->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(4);

    std::string text_of_file = QString(file.readAll()).toStdString();
    std::string::iterator iter = text_of_file.begin();
    std::string buf_cell="";
    std::string text="";
    bool job_doned = false;
    while (iter!=text_of_file.end())
    {
        if ((*(iter))=='\a')
            break;
        text+=(*(iter));
        iter++;
    }
    ui->textEdit->insertHtml(text.c_str());

    off_cch = true;
    iter++;
    buf_cell+=(*(iter));
    iter++;
    if (buf_cell[0]=='y')
    {
        job_doned = true;
        buf_cell ="";
        std::string::iterator help_iter = iter;
        while (help_iter!=text_of_file.end())
        {
            if ((*(help_iter))=='\r')
                words_count++;
            else
                if ((*(help_iter))=='\a')
                    break;
            help_iter++;
        }

        m_k = new mini_task*[words_count];
        int row = 0;
        int col = 0;
        bool its_str = true;
        while (iter!=text_of_file.end())
        {
            if ((*(iter))=='\n')
            {
                switch (col)
                {
                case 0:
                {
                    mini_task* new_task = new mini_task;
                    m_k[row] = new_task;
                    (m_k[row])->main_word = buf_cell.c_str();
                    (m_k[row])->head=NULL;
                    buf_cell="";
                    col++;
                    break;
                }
                case 1:
                {
                    (m_k[row])->met=atoi(buf_cell.c_str());
                    buf_cell="";
                    col++;
                    break;
                }
                case 2:
                {
                    (m_k[row])->low_met=atoi(buf_cell.c_str());
                    buf_cell="";
                    col++;
                    break;
                }
                case 3:
                {
                    low_task* new_low_task = new low_task;
                    new_low_task->word = buf_cell.c_str();
                    new_low_task->next = NULL;
                    (m_k[row])->head = new_low_task;
                    buf_cell="";
                    col++;
                    break;
                }
                case 4:
                {
                    (m_k[row])->head->met = atoi(buf_cell.c_str());
                    buf_cell = "";
                    col++;
                    break;
                }
                case 5:
                {
                    low_task* iter = (m_k[row])->head;
                    if (its_str)
                    {
                        low_task* new_low_task = new low_task;
                        new_low_task->next=NULL;
                        while (iter->next!=NULL)
                            iter = iter->next;
                        iter->next = new_low_task;
                        new_low_task->word = buf_cell.c_str();
                        its_str = false;
                    }
                    else
                    {
                        while (iter->next!=NULL)
                            iter = iter->next;
                        iter->met = atoi(buf_cell.c_str());
                        its_str = true;
                    }
                    buf_cell="";
                    break;
                }
                }
            }
            else
                if ((*(iter))=='\r')
                {
                    col=0;
                    row++;
                    buf_cell="";
                }
                else
                    if ((*(iter))=='\a')
                        break;
                    else
                        buf_cell+=(*(iter));
            iter++;
        }
    }
    buf_cell="";
    int i=0;
    int j=0;
    while (iter!=text_of_file.end())
    {
        if ((*(iter))=='\n')
        {
            ui->tableWidget->setRowCount(i+1);
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(tr(buf_cell.c_str())));
            if (j+2 > ui->tableWidget->columnCount())
            {
                ui->tableWidget->setColumnCount(j+2);
                char buf_int[20];
                sprintf(buf_int, "%d", j-1);
                ui->tableWidget->setHorizontalHeaderItem(j+1,new QTableWidgetItem(tr("Словоформа ") + buf_int));
            }
            j++;
            buf_cell="";
        }
        else
            if ((*(iter))=='\r')
            {
                buf_cell="";
                j=0;
                i++;
                if (i+1 > ui->tableWidget->rowCount())
                    ui->tableWidget->setRowCount(i+1);
            }
            else
                buf_cell+=(*(iter));
        iter++;
    }
    if (job_doned)
    {
        count_mas = new int[ui->tableWidget->rowCount()];
        count_mas2 = new int[ui->tableWidget->rowCount()];
        for (int i=0; i<ui->tableWidget->rowCount(); i++)
        {
            if (ui->tableWidget->item(i,1))
                count_mas[i]=ui->tableWidget->item(i,1)->text().toInt();
            else
                count_mas[i]=0;
            if (ui->tableWidget->item(i,1))
                count_mas2[i]=ui->tableWidget->item(i,2)->text().toInt();
            else
                count_mas2[i]=0;
        }
    }
    pr_name = fileName;
    off_pr(false);
    if (!job_doned)
    {
        ui->m_show_results->setEnabled(false);
        ui->m_show_task->setEnabled(false);
        ui->m_show_results->setChecked(false);
        ui->m_show_task->setChecked(false);
        show_results = false;
        is_job_done = false;
    }
    else
    {
        ui->m_show_results->setEnabled(true);
        ui->m_show_task->setEnabled(true);
        table_show_results();
        show_results = true;
        is_job_done = true;
    }
    ui->m_proccessing->setChecked(is_job_done);
    if (ui->tableWidget->colorCount()>4)
        ui->tableWidget->setColumnCount(ui->tableWidget->columnCount()-1);
    off_cch = false;
    off_ct = false;
    founded_words.clear();
    setWindowTitle(fileName);
    file.close();
    for (int col =0; col<ui->tableWidget->columnCount(); col++)
        ui->tableWidget->resizeColumnToContents(col);
    tpl=ui->textEdit->toPlainText().length();
    fake_text = ui->textEdit->toHtml();
    clear_staks();
    barl->setText(QString("%1 символов, %2 без пробелов").arg(tpl).arg(tpl-ui->textEdit->toPlainText().count(' ')));
}

void MainWindow::on_m_Open_task_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.xlsx");
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
        file.close();
    else
        return;

    QXlsx::Document xlsx(fileName);
    QString cur_text = "1";
    QString index = "A";
    delete_task_data();
    ui->tableWidget->clearContents();
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setRowCount(0);
    for (unsigned int cur_row =2; cur_text != NULL; cur_row++)
    {
        index = "A";
        char cur_index[15];
        sprintf(cur_index, "%d", cur_row);
        index += cur_index;
        cur_text = xlsx.read(index).toString();
        ui->tableWidget->setRowCount(cur_row-1);
        ui->tableWidget->setItem(cur_row-2,0,new QTableWidgetItem(cur_text));
        char literal = 'A';
        for (int cur_col=1; cur_text != NULL; cur_col ++)
        {
            literal++;
            index[0] = literal;
            cur_text = xlsx.read(index).toString();
            if (cur_col+1>ui->tableWidget->columnCount())
            {
                ui->tableWidget->setColumnCount(cur_col+1);
                char buf_int[20];
                sprintf(buf_int, "%d", cur_col-2);
                ui->tableWidget->setHorizontalHeaderItem(cur_col,new QTableWidgetItem(tr("Словоформа ")+buf_int));
            }
            ui->tableWidget->setItem(cur_row-2,cur_col,new QTableWidgetItem(cur_text));
        }
        index[0] = 'A';
        cur_text = xlsx.read(index).toString();
    }
    off_pr();
    clear_staks();
    founded_words.clear();
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()-1);
    ui->tableWidget->setColumnCount(ui->tableWidget->columnCount()-2);
    for (int col =0; col<ui->tableWidget->columnCount(); col++)
        ui->tableWidget->resizeColumnToContents(col);
}

QString fileName;
void MainWindow::on_m_Open_file_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home");

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
        return;

    setWindowTitle(pr_name+'*');
    ui->textEdit->clear();
    clear_staks();
    off_pr();
    off_ct = true;
    if (fileName.contains(".doc") && !fileName.contains(".docx"))
    {
        console = new QProcess;
        ::fileName = fileName + 'x';
        QStringList args;
        args<<"--headless"<<"--convert-to"<<"docx"<<fileName;
        //args<<"--accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager";
        console->setReadChannel (QProcess::StandardOutput);
        console->start("soffice",args);
        console->waitForFinished();
        for (int i=fileName.length()-1; i>-1; i--)
        {
            if (fileName[i]=='/')
            {
                ::fileName = fileName.right(fileName.length()-i-1) + 'x';
                break;
            }
        }
        console_end();
        file.close();
        return;
    }
    if (fileName.contains(".docx"))
    {
        ::fileName = fileName;
        console_end();
        file.close();
        return;
    }

    QString buf_read = file.readAll();
    fake_text = ui->textEdit->toPlainText();
    ui->textEdit->insertPlainText(buf_read);
    off_ct = true;
    file.close();
}

void MainWindow::console_end()
{
    opcContainer *c=opcContainerOpen(_X(::fileName.toStdString().c_str()),
                                     OPC_OPEN_READ_ONLY,
                                     NULL, NULL);
    opcPart part=opcPartFind(c, _X("word/document.xml"), NULL, 0);
    opcContainerInputStream *stream=opcContainerOpenInputStream(c, part);
    opc_uint8_t buf[1000000];
    opc_uint32_t len=0;
    QString *pof = new QString;
    QString poza;
    while((len=opcContainerReadInputStream(stream, buf, sizeof(buf)))>0)
    {
        printf("%.*s", len, buf);
        poza += reinterpret_cast<char*>(buf);
    }
    (*pof) = poza;
    (*pof).replace(QRegularExpression("<w\:pStyle"),QString("<Style"));
    (*pof).replace(QRegularExpression("<w\:hAnsi"),QString("<Ansi"));
    (*pof).replace(QRegularExpression("<w\:"),QString("<"));
    (*pof).replace(QRegularExpression("</w\:"),QString("</"));
    (*pof).replace(QRegularExpression(" w\:"),QString(" "));
    (*pof).replace(QRegularExpression("/>"),QString(">"));

    (*pof).replace(QRegularExpression("<i val=\"false\">"),QString(""));
    (*pof).replace(QRegularExpression("<iCs val=\"false\">"),QString(""));
    (*pof).replace(QRegularExpression("<rPr>"),QString(""));
    (*pof).replace(QRegularExpression("</rPr>"),QString(""));
    (*pof).replace(QRegularExpression("<jc val=\"center\""), QString ("<div align='center'"));
    (*pof).replace(QRegularExpression("<jc val=\"right\""), QString ("<div align='right'"));
    (*pof).replace(QRegularExpression("<tab val=\"left\""), QString ("<tab"));
    (*pof).replace(QRegularExpression("<tab val=\"right\""), QString ("<tab"));
    (*pof).replace(QRegularExpression("<tab val=\"center\""), QString ("<tab"));
    (*pof).replace(QRegularExpression("<pPr>"), QString (""));
    (*pof).replace(QRegularExpression("</pPr>"), QString (""));
    (*pof).replace(QRegularExpression("<Style val=\"([^<]*)\">"), QString (""));
    (*pof).replace(QRegularExpression("</Style>"), QString (""));
    (*pof).replace(QRegularExpression("<sz val=\"([^<]*)>"),QString("<font size=\"\\1>"));


    (*pof) = (*pof).left((*pof).indexOf("<docGrid type"));
    ui->textEdit->insertHtml(*pof);
    //ui->textEdit->insertPlainText(*pof);
    fake_text = ui->textEdit->toHtml();
    opcContainerCloseInputStream(stream);
    opcContainerClose(c, OPC_CLOSE_NOW);
    off_ct = false;
}
