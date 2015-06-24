#include "wf_mainwindow.h"
#include "ui_wf_mainwindow.h"
#include "titem.h"
#include "QFileDialog"
#include "QFile"
#include <QBoxLayout>
#include <xlsxdocument.h>
#include <QTextCodec>
#include <QXmlStreamReader>

void wf_mainwindow::ImportFromExcel()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.xlsx");
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
        file.close();
    else
        return;

    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    QXlsx::Document xlsx(fileName);
    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());
    tModel* model = (tModel*)tv->model();
    QAbstractItemModel* model2 = GetTV2Editor(NEGATIVE_TABLE)->model();

    int cur_col = 0;
    int irow;
    QVariant xdata;

    //const unsigned int total_rows = xlsx.dimension().rowCount();
    int total_rows =0 ;
    for (int i=1; !xlsx.read(i,1).isNull(); i++)
        total_rows++;

    if (!total_rows)
    {
        file.close();
        return;
    }

    if (ui->tabWidget->currentIndex()==1)
    {
        DeleteWords(AB_NEGATIVE_TABLE);
        DeleteWords(NEGATIVE_TABLE);
    }

    bool need_decode = true;
    QTextDecoder* decoder = QTextCodec::codecForName("Windows-1251")->makeDecoder();
    QString test_array = xlsx.read(1,1).toString();
    if (decoder->toUnicode(test_array.toLatin1()).contains("??"))
        need_decode = false;
    else
    {
        QTextDecoder* decoder1 = QTextCodec::codecForName("UTF-8")->makeDecoder();
        if (!decoder1->toUnicode(test_array.toLatin1()).contains("��"))
            need_decode = false;
    }

    progress->setMaximum(total_rows);
    const int from_row = tv->model()->rowCount();
    tv->model()->insertRows(from_row,total_rows);
    QTextDocument document;
    QSet<QString> words;
    GetTVLabel(ui->tabWidget->currentIndex())->hide();
    QString labelText = GetTVLabel(ui->tabWidget->currentIndex())->text();
    if (ui->tabWidget->currentIndex()==1)
    {
        GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
        GetTVLabel(ui->tabWidget->currentIndex())->setMaximumWidth(275);
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Удаление отмеченных запросов/слов");
        GetTVLabel(ui->tabWidget->currentIndex())->show();
        DeleteWords(AB_NEGATIVE_TABLE);
        DeleteWords(NEGATIVE_TABLE);
    }
    if (ui->tabWidget->currentIndex()==1)
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Загрузка файла");
    for (int cur_row=0; cur_row<total_rows; cur_row++)
    {
        if (need_decode)
            document.setHtml(decoder->toUnicode(xlsx.read(cur_row+1,1).toString().toLatin1()));
        else
            document.setHtml(xlsx.read(cur_row+1,1).toString());
        model->setData(model->index(cur_row+from_row,1),document.toPlainText(),Qt::EditRole);
        for (cur_col=1; cur_col!=4;cur_col++)
        {
            xdata = xlsx.read(cur_row+1,cur_col+1);
            if (!xdata.isNull())
                model->setData(model->index(cur_row+from_row,cur_col+2),xdata,Qt::EditRole);
        }
        model->Init(cur_row+from_row);

        if (ui->tabWidget->currentIndex()==1)
            foreach(QString word, *model->Words(cur_row+from_row))
                words<<word;

        progress->setValue(cur_row);
    }
    if (ui->tabWidget->currentIndex()==1)
    {
        irow = model2->rowCount();
        model2->insertRows(model2->rowCount(),words.count());
        foreach (const QString word, words)
        {
            model2->setData(model2->index(irow,2),word,Qt::EditRole);
            irow++;
        }
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Обновление вкладок");
        DeleteClones(2,false,NEGATIVE_TABLE);
        GetTV2Editor(NEGATIVE_TABLE)->ResizeContext();
        for (int i=0; i<GetTV2Editor(NEGATIVE_TABLE)->model()->rowCount(); i++)
            if (GetTV2Editor(NEGATIVE_TABLE)->model()->data(GetTV2Editor(NEGATIVE_TABLE)->model()->index(i,1)).toBool())
                for (int j=0; j<tree_list.count(); j++)
                        ((tModel*)tree_list[j].view->model())->PickWords(model2->data(model2->index(i,2)).toString());
        for (int j=0; j<tree_list.count(); j++)
        {
            UpdateWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteClones(1,false,1,tree_list[j].view);
            UpdateWords(AB_NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(AB_NEGATIVE_TABLE,tree_list[j].view);
        }
    }
    if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
        DeleteClones(1,true);
    int append_value = model->rowCount()-from_row;
    if (ui->tabWidget->currentIndex()==1)
    {
        if (current_tab->text(0)!="Все запросы")
        {
            append_value = tree_list.first().view->model()->rowCount();
            tree_list.first().view->model()->insertRows(append_value-1,model->rowCount()-from_row);
            for (int i=append_value, j=from_row; j<model->rowCount(); i++,j++)
            {
                tData tmp = *model->data(j);
                tmp.num = i+1;
                ((tModel*)tree_list.first().view->model())->setData(i,&tmp);
            }
            DeleteClones(1,true,1,tree_list.first().view);
            append_value = tree_list.first().view->model()->rowCount() - append_value;
        }
    }
    GetTVLabel(ui->tabWidget->currentIndex())->setText(labelText);
    UpdateStats(append_value,1);
    progress->setValue(progress->maximum());
    GetTVLabel(ui->tabWidget->currentIndex())->show();
    GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    tv->ResizeContext();
    if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
    for (int i=from_row, j=1; i<model->rowCount(); i++, j++)
        model->setData(model->index(i,0),from_row+j,Qt::EditRole);
    if (ui->tabWidget->currentIndex()==0)
    for (int cur_row=from_row; cur_row<model->rowCount(); cur_row++)
        cache.append(((tModel*)tv->model())->data(cur_row));
    if (ui->tabWidget->currentIndex()==1)
        current_tab->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>1)
        current_tabs[ui->tabWidget->currentIndex()-2]->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
}

void wf_mainwindow::ExportToExcel()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".xlsx"))
        fileName.append(".xlsx");
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
        file.close();
    else
        return;

    QXlsx::Document xlsx;//(fileName);
    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());

    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(tv->model()->rowCount());
    for (int cur_row=0; cur_row<tv->model()->rowCount(); cur_row++)
    {
        xlsx.write(cur_row+1,1,tv->model()->data(tv->model()->index(cur_row,1)));
        for (int cur_col=2; cur_col<5;cur_col++)
            xlsx.write(cur_row+1,cur_col,tv->model()->data(tv->model()->index(cur_row,cur_col+1)));
        progress->setValue(cur_row);
    }
    progress->setValue(progress->maximum());
    xlsx.saveAs(fileName);
}

typedef QPair<QString,QString> spair;
void wf_mainwindow::ImportFromCSV()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.csv");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());
    tModel* model = (tModel*)tv->model();
    QAbstractItemModel* model2 = GetTV2Editor(NEGATIVE_TABLE)->model();

    int cur_col = 0;
    int irow;
    QString row;
    QStringList list;
    int total_rows = 0;
    for (;!file.readLine().isNull();)
        total_rows++;
    if (!total_rows)
    {
        file.close();
        return;
    }
    file.seek(0);
    bool need_decode = true;
    int ktotal =-1;
    int old_rows=-1;
    QLabel* label = GetTVLabel(ui->tabWidget->currentIndex());
    //if (ui->tabWidget->currentIndex()>0)
        ktotal = label->text().mid(label->text().indexOf('/')+2,label->text().indexOf(' ',label->text().indexOf('/')+2)-label->text().indexOf('/')-1).toInt();
    QTextDecoder* decoder = QTextCodec::codecForName("Windows-1251")->makeDecoder();
    QByteArray test_array = file.readLine();
    if (decoder->toUnicode(test_array).contains("??"))
        need_decode = false;
    else
    {
        QTextDecoder* decoder1 = QTextCodec::codecForName("UTF-8")->makeDecoder();
        if (!decoder1->toUnicode(test_array).contains("��"))
            need_decode = false;
    }
    file.seek(0);
    GetTVLabel(ui->tabWidget->currentIndex())->hide();
    QString labelText = GetTVLabel(ui->tabWidget->currentIndex())->text();
    if (ui->tabWidget->currentIndex()==1)
    {
        GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
        GetTVLabel(ui->tabWidget->currentIndex())->setMaximumWidth(275);
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Предвартельная загрузка");
        GetTVLabel(ui->tabWidget->currentIndex())->show();
        DeleteWords(AB_NEGATIVE_TABLE);
        DeleteWords(NEGATIVE_TABLE);
    }
    QSet<QString> string_set;
    {
        if (ui->tabWidget->currentIndex()==1)
        foreach (tree_item item, tree_list)
        {
            tModel* model = (tModel*)item.view->model();
            for (int i=0; i<model->rowCount(); i++)
                string_set<<model->data(i)->free_html;
        }
        else if (ui->tabWidget->currentIndex()>0)
            foreach (tree_item item, tree_lists[ui->tabWidget->currentIndex()-2])
            {
                tModel* model = (tModel*)item.view->model();
                for (int i=0; i<model->rowCount(); i++)
                    string_set<<model->data(i)->free_html;
            }
        else
            for (int i=0; i<model->rowCount(); i++)
                string_set<<model->data(i)->free_html;
    }
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(total_rows);
    const int from_row = model->rowCount();
    QSet<QString> words;
    QTextDocument document;
    QByteArray brow;
    QList<QPair<QString,QString> > file_data;
    for (int cur_row=0; !(brow = file.readLine()).isEmpty();cur_row++)
    {
        if (need_decode)
            row = decoder->toUnicode(brow);
        else
            row = brow;
        document.setHtml(row);
        row = document.toPlainText().toLower();

        list = row.left(row.length()-1).split(",");
        QStringList w;
        w = list[0].split(' ');
        w.removeDuplicates();
        list[0] = w.join(' ');
        file_data<<QPair<QString,QString>(row,list[0]);
        progress->setValue(cur_row);
    }
    QSet<QString> end_set;
    int count = 0;
    foreach (spair item, file_data)
    {
        end_set<<item.second;
        if (count==end_set.count())
            file_data.removeOne(item);
        count = end_set.count();
    }

    end_set.clear();
    count = string_set.count();
    for (int i=0; i<file_data.count(); i++)
    {
        string_set<<file_data[i].second;
        if (count!=string_set.count())
            end_set<<file_data[i].first;
        count = string_set.count();
    }
    QStringList end_list = end_set.toList();
    end_list.removeDuplicates();
    if (ui->tabWidget->currentIndex()!=1)
    {
        int old_rows = model->rowCount();
        model->insertRows(model->rowCount(),end_set.count());
        for (int i=old_rows, j=0; i<model->rowCount(); i++, j++)
        {
            list = end_list[j].left(end_list[j].length()-1).split(",");
            QStringList w;
            w = list[0].split(' ');
            w.removeDuplicates();
            list[0] = w.join(' ');
            model->setData(model->index(i,1),list[0],Qt::EditRole);
            for (cur_col=1; cur_col!=list.count(); cur_col++)
            {
                if (!list[cur_col].isEmpty() && list[cur_col]!="\n")
                    model->setData(model->index(i,cur_col+2),list[cur_col],Qt::EditRole);
            }
            model->Init(i);
        }
        ktotal+=model->rowCount()-old_rows;
        UpdateStats(end_list.count(),1,ui->tabWidget->currentIndex(),ktotal);
        for (int i=old_rows, j=1; i<model->rowCount(); i++, j++)
            model->setData(model->index(i,0),old_rows+j,Qt::EditRole);
        if (ui->tabWidget->currentIndex()==0)
            for (int cur_row=from_row; cur_row<model->rowCount(); cur_row++)
                cache.append(((tModel*)tv->model())->data(cur_row));
    }
    else
    {
        QSet<QString> words_set;
        for (int i=0; i<model2->rowCount(); i++)
            words_set<<model2->data(model2->index(i,2)).toString();
        int old_rows = model->rowCount();
        int old_rows2 = model->rowCount();
        model->insertRows(model->rowCount(),end_set.count());
        for (int i=old_rows, j=0; i<model->rowCount(); i++, j++)
        {
            list = end_list[j].left(end_list[j].length()-1).split(",");
            QStringList w;
            w = list[0].split(' ');
            w.removeDuplicates();
            list[0] = w.join(' ');

            model->setData(model->index(i,1),list[0],Qt::EditRole);
            for (cur_col=1; cur_col!=list.count(); cur_col++)
            {
                if (!list[cur_col].isEmpty() && list[cur_col]!="\n")
                    model->setData(model->index(i,cur_col+2),list[cur_col],Qt::EditRole);
            }
            model->Init(i);
            foreach(QString word, *model->Words(i))
                words<<word;
        }
        QStringList end_words = words.toList();
        QSet<QString> end_end_words;
        int count = words_set.count();
        for (int i=0; i<end_words.count(); i++)
        {
            words_set<<end_words[i];
            if (count!=words_set.count())
                end_end_words<<end_words[i];
            count = words_set.count();
        }
        end_words = end_end_words.toList();
        end_words.removeDuplicates();
        old_rows = model2->rowCount();
        model2->insertRows(model2->rowCount(),end_words.count());
        for (int i=old_rows, j=0; i<model2->rowCount(); i++, j++)
            model2->setData(model2->index(i,2),end_words[j]);

        GetTVLabel(ui->tabWidget->currentIndex())->setText("Обновление вкладок");
        DeleteClones(2,true,NEGATIVE_TABLE);

        ktotal+=model->rowCount()-old_rows2;
        for (int i=0; i<GetTV2Editor(NEGATIVE_TABLE)->model()->rowCount(); i++)
            if (GetTV2Editor(NEGATIVE_TABLE)->model()->data(GetTV2Editor(NEGATIVE_TABLE)->model()->index(i,1)).toBool())
                for (int j=0; j<tree_list.count(); j++)
                        ((tModel*)tree_list[j].view->model())->PickWords(model2->data(model2->index(i,2)).toString());
        for (int j=0; j<tree_list.count(); j++)
        {
            UpdateWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteClones(1,false,1,tree_list[j].view);
            UpdateWords(AB_NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(AB_NEGATIVE_TABLE,tree_list[j].view);
        }
        GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        GetTVLabel(ui->tabWidget->currentIndex())->setText(labelText);
        UpdateStats(end_list.count(),1,ui->tabWidget->currentIndex(),ktotal);
        UpdateStats(end_words.count(),2,NEGATIVE_TABLE);
        UpdateWords(NEGATIVE_TABLE);
        for (int i=old_rows, j=1; i<model2->rowCount(); i++, j++)
            model2->setData(model2->index(i,0),old_rows+j);
    }

    if (ui->tabWidget->currentIndex()==1)
    foreach (tree_item item, tree_list)
        item.node->setText(1,QString::number(item.view->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>0)
        foreach (tree_item item, tree_lists[ui->tabWidget->currentIndex()-2])
            item.node->setText(1,QString::number(item.view->model()->rowCount()));

    /*GetTVLabel(ui->tabWidget->currentIndex())->setText(labelText);
    UpdateStats(end_list.count(),1);
    progress->setValue(progress->maximum());
    GetTVLabel(ui->tabWidget->currentIndex())->show();
    GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);*/
    progress->setValue(progress->maximum());
    GetTVLabel(ui->tabWidget->currentIndex())->show();
    file.close();
    tv->ResizeContext();
    /*if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
    for (int i=from_row, j=1; i<model->rowCount(); i++, j++)
        model->setData(model->index(i,0),from_row+j,Qt::EditRole);*/

    /*if (ui->tabWidget->currentIndex()==1)
        current_tab->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>1)
        current_tabs[ui->tabWidget->currentIndex()-2]->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));*/
    return;
    if (ui->tabWidget->currentIndex()==1)
    {
        irow = model2->rowCount();
        model2->insertRows(model2->rowCount(),words.count());
        foreach (const QString word, words)
        {
            model2->setData(model2->index(irow,2),word,Qt::EditRole);
            irow++;
        }
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Обновление вкладок");
        DeleteClones(2,false,NEGATIVE_TABLE);
        DeleteClones(1,false,1,GetTVEditor(1));

        int count = string_set.count();
        QList<int> to_delete;
        for (int i=from_row; i<model->rowCount(); i++)
        {
            string_set<<model->data(i)->free_html;
            if (count==string_set.count())
                to_delete<<i;
            count = string_set.count();
        }
        model->SpecialRemoveRows(to_delete,NULL);
        if (current_tab->text(0)=="Все запросы")
            ktotal += GetTVEditor(1)->model()->rowCount() - from_row;
        else
            old_rows = GetTVEditor(1)->model()->rowCount();
        GetTV2Editor(NEGATIVE_TABLE)->ResizeContext();
        for (int i=0; i<GetTV2Editor(NEGATIVE_TABLE)->model()->rowCount(); i++)
            if (GetTV2Editor(NEGATIVE_TABLE)->model()->data(GetTV2Editor(NEGATIVE_TABLE)->model()->index(i,1)).toBool())
                for (int j=0; j<tree_list.count(); j++)
                        ((tModel*)tree_list[j].view->model())->PickWords(model2->data(model2->index(i,2)).toString());
        for (int j=0; j<tree_list.count(); j++)
        {
            UpdateWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteClones(1,false,1,tree_list[j].view);
            UpdateWords(AB_NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(AB_NEGATIVE_TABLE,tree_list[j].view);
        }
    }
    else if (ui->tabWidget->currentIndex()>1)
    {
        DeleteClones(1,false);

        int count = string_set.count();
        QList<int> to_delete;
        for (int i=from_row; i<model->rowCount(); i++)
        {
            string_set<<model->data(i)->free_html;
            if (count==string_set.count())
                to_delete<<i;
            count = string_set.count();
        }
        model->SpecialRemoveRows(to_delete,NULL);
        if (current_tabs[ui->tabWidget->currentIndex()-2]->text(0)=="Все запросы")
            ktotal += GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount() - from_row;
        else
            old_rows = GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount();
    }
    if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
        DeleteClones(1,true);
    int append_value = model->rowCount()-from_row;
    if (ui->tabWidget->currentIndex()==1)
    {
        if (current_tab->text(0)!="Все запросы")
        {
            /*old_rows -= GetTVEditor(1)->model()->rowCount();
            append_value = tree_list.first().view->model()->rowCount();
            tree_list.first().view->model()->insertRows(append_value-1,model->rowCount()-from_row);
            for (int i=append_value, j=from_row; j<model->rowCount(); i++,j++)
            {
                tData tmp = *model->data(j);
                tmp.num = i+1;
                ((tModel*)tree_list.first().view->model())->setData(i,&tmp);
            }
            DeleteClones(1,true,1,tree_list.first().view);
            append_value = tree_list.first().view->model()->rowCount() - append_value;
            ktotal += append_value+old_rows;*/
        }
    }
    else if (ui->tabWidget->currentIndex()>1)
    {
        if (current_tabs[ui->tabWidget->currentIndex()-2]->text(0)!="Все запросы")
        {
            /*old_rows -= GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount();
            append_value = tree_lists[ui->tabWidget->currentIndex()-2].first().view->model()->rowCount();
            tree_lists[ui->tabWidget->currentIndex()-2].first().view->model()->insertRows(append_value-1,model->rowCount()-from_row);
            for (int i=append_value, j=from_row; j<model->rowCount(); i++,j++)
            {
                tData tmp = *model->data(j);
                tmp.num = i+1;
                ((tModel*)tree_lists[ui->tabWidget->currentIndex()-2].first().view->model())->setData(i,&tmp);
            }
            DeleteClones(1,true,1,tree_lists[ui->tabWidget->currentIndex()-2].first().view);
            append_value = tree_lists[ui->tabWidget->currentIndex()-2].first().view->model()->rowCount() - append_value;
            ktotal += append_value+old_rows;*/
        }
    }

    GetTVLabel(ui->tabWidget->currentIndex())->setText(labelText);
    UpdateStats(append_value,1,ui->tabWidget->currentIndex(),ktotal);
    progress->setValue(progress->maximum());
    GetTVLabel(ui->tabWidget->currentIndex())->show();
    GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    file.close();
    tv->ResizeContext();
    if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
    for (int i=from_row, j=1; i<model->rowCount(); i++, j++)
        model->setData(model->index(i,0),from_row+j,Qt::EditRole);
    if (ui->tabWidget->currentIndex()==0)
    for (int cur_row=from_row; cur_row<model->rowCount(); cur_row++)
        cache.append(((tModel*)tv->model())->data(cur_row));
    if (ui->tabWidget->currentIndex()==1)
        current_tab->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>1)
        current_tabs[ui->tabWidget->currentIndex()-2]->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
}

void wf_mainwindow::ExportToCSV()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".csv"))
        fileName.append(".csv");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return;

    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(tv->model()->rowCount());
    int cur_col;
    for (int cur_row=0; cur_row<tv->model()->rowCount(); cur_row++)
    {
        file.write(QString("%1,").arg(tv->model()->data(tv->model()->index(cur_row,1)).toString()).toUtf8());
        for (cur_col=3; cur_col!=5;cur_col++)
            file.write(QString("%1,").arg(tv->model()->data(tv->model()->index(cur_row,cur_col)).toString()).toUtf8());
        file.write(tv->model()->data(tv->model()->index(cur_row,5)).toString().toUtf8());
        file.write("\n");
        progress->setValue(cur_row);
    }
    progress->setValue(progress->maximum());
    file.close();
}

void wf_mainwindow::ImportFromTXT()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.txt");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());
    tModel* model = (tModel*)tv->model();
    QAbstractItemModel* model2 = GetTV2Editor(NEGATIVE_TABLE)->model();

    QString row;
    int total_rows = 0;
    int irow;
    for (;!file.readLine().isNull();)
        total_rows++;
    if (!total_rows)
    {
        file.close();
        return;
    }
    file.seek(0);
    GetTVLabel(ui->tabWidget->currentIndex())->hide();
    QString labelText = GetTVLabel(ui->tabWidget->currentIndex())->text();
    if (ui->tabWidget->currentIndex()==1)
    {
        GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
        GetTVLabel(ui->tabWidget->currentIndex())->setMaximumWidth(275);
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Удаление отмеченных запросов/слов");
        GetTVLabel(ui->tabWidget->currentIndex())->show();
        DeleteWords(AB_NEGATIVE_TABLE);
        DeleteWords(NEGATIVE_TABLE);
    }
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(total_rows);
    const int from_row = model->rowCount();
    tv->model()->insertRows(from_row,total_rows);
    QSet<QString> words;
    QTextDocument document;
    if (ui->tabWidget->currentIndex()==1)
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Загрузка файла");
    for (int cur_row=0; !(row = file.readLine()).isEmpty();cur_row++)
    {
        row = row.toLower();
        document.setHtml(row.left(row.length()-1));
        QStringList w;
        w = document.toPlainText().split(' ');
        w.removeDuplicates();
        QString tmp = w.join(' ');
        model->setData(model->index(cur_row+from_row,1),tmp,Qt::EditRole);
        model->Init(cur_row+from_row);

        if (ui->tabWidget->currentIndex()==1)
            foreach(QString word, *model->Words(cur_row+from_row))
                words<<word;

        progress->setValue(cur_row);
    }
    if (ui->tabWidget->currentIndex()==1)
    {
        irow = model2->rowCount();
        model2->insertRows(model2->rowCount(),words.count());
        foreach (const QString word, words)
        {
            model2->setData(model2->index(irow,2),word,Qt::EditRole);
            irow++;
        }
        GetTVLabel(ui->tabWidget->currentIndex())->setText("Обновление вкладок");
        DeleteClones(2,false,NEGATIVE_TABLE);
        GetTV2Editor(NEGATIVE_TABLE)->ResizeContext();
        for (int i=0; i<GetTV2Editor(NEGATIVE_TABLE)->model()->rowCount(); i++)
            if (GetTV2Editor(NEGATIVE_TABLE)->model()->data(GetTV2Editor(NEGATIVE_TABLE)->model()->index(i,1)).toBool())
                for (int j=0; j<tree_list.count(); j++)
                        ((tModel*)tree_list[j].view->model())->PickWords(model2->data(model2->index(i,2)).toString());
        for (int j=0; j<tree_list.count(); j++)
        {
            UpdateWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(NEGATIVE_TABLE,tree_list[j].view);
            DeleteClones(1,false,1,tree_list[j].view);
            UpdateWords(AB_NEGATIVE_TABLE,tree_list[j].view);
            DeleteWords(AB_NEGATIVE_TABLE,tree_list[j].view);
        }
    }
    if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
        DeleteClones(1,true);
    int append_value = model->rowCount()-from_row;
    if (ui->tabWidget->currentIndex()==1)
    {
        if (current_tab->text(0)!="Все запросы")
        {
            append_value = tree_list.first().view->model()->rowCount();
            tree_list.first().view->model()->insertRows(append_value-1,model->rowCount()-from_row);
            for (int i=append_value, j=from_row; j<model->rowCount(); i++,j++)
            {
                tData tmp = *model->data(j);
                tmp.num = i+1;
                ((tModel*)tree_list.first().view->model())->setData(i,&tmp);
            }
            DeleteClones(1,true,1,tree_list.first().view);
            append_value = tree_list.first().view->model()->rowCount() - append_value;
        }
    }
    GetTVLabel(ui->tabWidget->currentIndex())->setText(labelText);
    UpdateStats(append_value,1);
    progress->setValue(progress->maximum());
    GetTVLabel(ui->tabWidget->currentIndex())->show();
    GetTVLabel(ui->tabWidget->currentIndex())->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    file.close();
    tv->ResizeContext();
    if (ui->tabWidget->currentIndex()==1 || ui->tabWidget->currentIndex()==0)
    for (int i=from_row, j=1; i<model->rowCount(); i++, j++)
        model->setData(model->index(i,0),from_row+j,Qt::EditRole);
    if (ui->tabWidget->currentIndex()==0)
    for (int cur_row=from_row; cur_row<model->rowCount(); cur_row++)
        cache.append(((tModel*)tv->model())->data(cur_row));
    if (ui->tabWidget->currentIndex()==1)
        current_tab->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>1)
        current_tabs[ui->tabWidget->currentIndex()-2]->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
}

void wf_mainwindow::ExportToTXT()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".txt"))
        fileName.append(".txt");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return;

    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(tv->model()->rowCount());
    for (int cur_row=0; cur_row<tv->model()->rowCount(); cur_row++)
    {
        file.write(tv->model()->data(tv->model()->index(cur_row,1)).toString().toUtf8());
        file.write("\n");
        progress->setValue(cur_row);
    }
    progress->setValue(progress->maximum());
    file.close();
}

/// ###########################################################################################################################
/// I/E words
/// ###########################################################################################################################

void wf_mainwindow::ImportWordsFromExcel()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.xlsx");
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
        file.close();
    else
        return;

    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    QXlsx::Document xlsx(fileName);
    MyView* tv = GetTV2Editor(ui->tabWidget_2->currentIndex());

    //const unsigned int total_rows = xlsx.dimension().rowCount();
    int total_rows =0 ;
    for (int i=1; !xlsx.read(i,1).isNull(); i++)
        total_rows++;

    if (!total_rows)
    {
        file.close();
        return;
    }

    progress->setMaximum(total_rows);
    const int from_row = tv->model()->rowCount();
    tv->model()->insertRows(from_row,total_rows);

    for (int cur_row=0; cur_row<total_rows; cur_row++)
    {
        progress->setValue(cur_row);
        tv->model()->setData(tv->model()->index(cur_row+from_row,2),xlsx.read(cur_row+1,1),Qt::EditRole);
        //tv->model()->setData(tv->model()->index(cur_row+from_row,1),xlsx.read(cur_row+1,2),Qt::EditRole);
    }
    int deleted_words = DeleteClones(2,true);
    UpdateStats(total_rows-deleted_words,2);
    progress->setValue(progress->maximum());
    UpdateWords();
    tv->ResizeContext();
}

void wf_mainwindow::ExportWordsToExcel()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".xlsx"))
        fileName.append(".xlsx");
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
        file.close();
    else
        return;

    QXlsx::Document xlsx;//(fileName);
    MyView* tv = GetTV2Editor(ui->tabWidget_2->currentIndex());

    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    progress->setMaximum(tv->model()->rowCount());
    for (int cur_row=0; cur_row<tv->model()->rowCount(); cur_row++)
    {
        xlsx.write(cur_row+1,1,tv->model()->data(tv->model()->index(cur_row,2)));
        //xlsx.write(cur_row+1,2,tv->model()->data(tv->model()->index(cur_row,1)));
        progress->setValue(cur_row);
    }
    progress->setValue(progress->maximum());
    xlsx.saveAs(fileName);
}

void wf_mainwindow::ImportWordsFromCSV()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.csv");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    MyView* tv = GetTV2Editor(ui->tabWidget_2->currentIndex());
    QString row;
    //QStringList list;
    int total_rows = 0;
    for (;!file.readLine().isNull();)
        total_rows++;
    if (!total_rows)
    {
        file.close();
        return;
    }

    file.seek(0);
    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    progress->setMaximum(total_rows);
    const int from_row = tv->model()->rowCount();
    //tv->model()->insertRows(from_row,total_rows);
    QSet<QString> string_set;
    for (int i=0; i<tv->model()->rowCount(); i++)
        string_set<<tv->model()->data(tv->model()->index(i,2)).toString();
    QList<QPair<QString,QString> > file_data;
    for (int cur_row=0; !(row = file.readLine()).isEmpty();cur_row++)
    {
        progress->setValue(cur_row);
        QStringList list = row.left(row.size()-1).split(",");
        file_data<<QPair<QString,QString>(row,list[0]);
    }
    QSet<QString> end_set;
    int count = 0;
    foreach (spair item, file_data)
    {
        end_set<<item.second;
        if (count==end_set.count())
            file_data.removeOne(item);
        count = end_set.count();
    }
    end_set.clear();
    count = string_set.count();
    for (int i=0; i<file_data.count(); i++)
    {
        string_set<<file_data[i].second;
        if (count!=string_set.count())
            end_set<<file_data[i].first;
        count = string_set.count();
    }
    QStringList end_list = end_set.toList();

    /*for (int cur_row=0; !(row = file.readLine()).isEmpty();cur_row++)
    {
        progress->setValue(cur_row);
        QStringList list = row.left(row.size()-1).split(",");
        tv->model()->setData(tv->model()->index(cur_row+from_row,2),list[0],Qt::EditRole);
        if (list.count()>1)
            tv->model()->setData(tv->model()->index(cur_row+from_row,1),list[1],Qt::EditRole);
        //tv->model()->setData(tv->model()->index(cur_row+from_row,2),row.left(row.size()-1).toLower(),Qt::EditRole);
    }
    int deleted_words = DeleteClones(2,true);
    UpdateStats(total_rows-deleted_words,2);*/
    int old_rows = tv->model()->rowCount();
    tv->model()->insertRows(tv->model()->rowCount(),end_list.count());
    for (int i=0, j=old_rows; i<end_list.count(); i++, j++)
    {
        QStringList list = end_list[i].left(end_list[i].size()-1).split(",");
        tv->model()->setData(tv->model()->index(j,2),list[0],Qt::EditRole);
        if (list.count()>1)
            tv->model()->setData(tv->model()->index(j,1),list[1],Qt::EditRole);
        tv->update(tv->model()->index(j,1));
    }
    UpdateStats(end_list.count(),2);
    progress->setValue(progress->maximum());
    UpdateWords();
    file.close();
    tv->ResizeContext();
}

void wf_mainwindow::ExportWordsToCSV()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".csv"))
        fileName.append(".csv");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return;

    MyView* tv = GetTV2Editor(ui->tabWidget_2->currentIndex());
    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    progress->setMaximum(tv->model()->rowCount());
    for (int cur_row=0; cur_row<tv->model()->rowCount(); cur_row++)
    {
        file.write(QString("%1,").arg(tv->model()->data(tv->model()->index(cur_row,2)).toString()).toUtf8());
        file.write(QString("%1\n").arg(tv->model()->data(tv->model()->index(cur_row,1)).toBool()).toUtf8());
        progress->setValue(cur_row);
    }
    progress->setValue(progress->maximum());
    file.close();
}

void wf_mainwindow::ImportWordsFromTXT()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.txt");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    MyView* tv = GetTV2Editor(ui->tabWidget_2->currentIndex());
    QString row;
    int total_rows = 0;
    for (;!file.readLine().isNull();)
        total_rows++;
    if (!total_rows)
    {
        file.close();
        return;
    }

    file.seek(0);
    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    progress->setMaximum(total_rows);
    const int from_row = tv->model()->rowCount();
    tv->model()->insertRows(from_row,total_rows);
    for (int cur_row=0; !(row = file.readLine()).isEmpty();cur_row++)
    {
        progress->setValue(cur_row);
        tv->model()->setData(tv->model()->index(cur_row+from_row,2),row.left(row.length()-1),Qt::EditRole);
    }
    int deleted_words = DeleteClones(2,true);
    UpdateStats(total_rows-deleted_words,2);
    progress->setValue(progress->maximum());
    file.close();
    UpdateWords();
    tv->ResizeContext();
}

void wf_mainwindow::ExportWordsToTXT()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".txt"))
        fileName.append(".txt");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return;

    MyView* tv = GetTV2Editor(ui->tabWidget_2->currentIndex());
    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    progress->setMaximum(tv->model()->rowCount());
    for (int cur_row=0; cur_row<tv->model()->rowCount(); cur_row++)
    {
        file.write(tv->model()->data(tv->model()->index(cur_row,2)).toString().toUtf8());
        file.write("\n");
        progress->setValue(cur_row);
    }
    progress->setValue(progress->maximum());
    file.close();
}
