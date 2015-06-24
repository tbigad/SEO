#include "wf_mainwindow.h"
#include "QFileDialog"
#include "QFile"
#include <xlsxdocument.h>
#include <QMessageBox>
#include "QtCore/QSettings"

void wf_mainwindow::on_save_pr_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName;
    fileName = dialog.getSaveFileName(this,tr("Сохранить"),"/home","*");
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".scew"))
        fileName.append(".scew");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(0);
    for (int i=0; i<8; i++)
    {
        if (i==0)
            progress->setMaximum(progress->maximum()+GetTVEditor(i)->model()->rowCount());
        else if (i==1)
            foreach (tree_item item, tree_list)
                progress->setMaximum(progress->maximum()+item.view->model()->rowCount());
        else if (i>1 && i<6)
            foreach (tree_item item, tree_lists[i-2])
                progress->setMaximum(progress->maximum()+item.view->model()->rowCount());
        else if (i>5)
            progress->setMaximum(progress->maximum()+GetTV2Editor(i-6)->model()->rowCount());
    }
    progress->setValue(0);

    for (int page=0; page<6; page++)
    {
        if (page==0)
        {
            tModel* model = (tModel*)GetTVEditor(page)->model();
            file.write(GetTVLabel(page)->text().toUtf8());
            file.write("\n");
            file.write(QString::number(model->rowCount()).toUtf8());
            file.write("\n");
            for (int row =0; row<model->rowCount(); row++)
            {
                for (int col=0; col<model->columnCount(QModelIndex()); col++)
                {
                    if (col==3)
                        if (!model->data(row)->bfreq1)
                        {
                            file.write(",");
                            continue;
                        }
                    if (col==4)
                        if (!model->data(row)->bfreq2)
                        {
                            file.write(",");
                            continue;
                        }
                    if (col==5)
                        if (!model->data(row)->bfreq3)
                        {
                            file.write(",");
                            continue;
                        }
                    file.write(model->data(model->index(row,col),Qt::EditRole).toString().toUtf8());
                    file.write(",");
                }
                progress->setValue(progress->value()+1);
                file.write("\n");
            }
            file.write("\n");
        }
        else if (page==1)
        {
            file.write(GetTVLabel(page)->text().toUtf8());
            file.write("\n");
            foreach (tree_item item, tree_list)
            {
                if (item.parent)
                    file.write(item.parent->text(0).toUtf8());
                else
                    file.write("NULL");
                file.write("\n");
                file.write(item.node->text(0).toUtf8());
                file.write("\n");
                tModel* model = (tModel*)item.view->model();
                file.write(QString::number(model->rowCount()).toUtf8());
                file.write("\n");
                for (int row =0; row<model->rowCount(); row++)
                {
                    for (int col=0; col<model->columnCount(QModelIndex()); col++)
                    {
                        if (col==3)
                            if (!model->data(row)->bfreq1)
                            {
                                file.write(",");
                                continue;
                            }
                        if (col==4)
                            if (!model->data(row)->bfreq2)
                            {
                                file.write(",");
                                continue;
                            }
                        if (col==5)
                            if (!model->data(row)->bfreq3)
                            {
                                file.write(",");
                                continue;
                            }
                        file.write(model->data(model->index(row,col),Qt::EditRole).toString().toUtf8());
                        file.write(",");
                    }
                    progress->setValue(progress->value()+1);
                    file.write("\n");
                }
                file.write("\n");
            }
            file.write("\nPAGE_CLOSED\n");
        }
            else
            {
                file.write(GetTVLabel(page)->text().toUtf8());
                file.write("\n");
                foreach (tree_item item, tree_lists[page-2])
                {
                    if (item.parent)
                        file.write(item.parent->text(0).toUtf8());
                    else
                        file.write("NULL");
                    file.write("\n");
                    file.write(item.node->text(0).toUtf8());
                    file.write("\n");
                    tModel* model = (tModel*)item.view->model();
                    file.write(QString::number(model->rowCount()).toUtf8());
                    file.write("\n");
                    for (int row =0; row<model->rowCount(); row++)
                    {
                        for (int col=0; col<model->columnCount(QModelIndex()); col++)
                        {
                            if (col==3)
                                if (!model->data(row)->bfreq1)
                                {
                                    file.write(",");
                                    continue;
                                }
                            if (col==4)
                                if (!model->data(row)->bfreq2)
                                {
                                    file.write(",");
                                    continue;
                                }
                            if (col==5)
                                if (!model->data(row)->bfreq3)
                                {
                                    file.write(",");
                                    continue;
                                }
                            file.write(model->data(model->index(row,col),Qt::EditRole).toString().toUtf8());
                            file.write(",");
                        }
                        progress->setValue(progress->value()+1);
                        file.write("\n");
                    }
                    file.write("\n");
                }
                file.write("\nPAGE_CLOSED\n");
            }
    }

    for (int page=0; page<3; page++)
    {
        iModel* model = (iModel*)GetTV2Editor(page)->model();
        file.write(QString::number(model->rowCount()).toUtf8());
        file.write("\n");
        for (int row =0; row<model->rowCount(); row++)
        {
            for (int col=0; col<model->columnCount(QModelIndex()); col++)
            {
                file.write(model->data(model->index(row,col),Qt::EditRole).toString().toUtf8());
                file.write(",");
            }
            progress->setValue(progress->value()+1);
            progress->setValue(progress->value()+1);
            file.write("\n");
        }
        file.write("\n");
        file.write(GetTV2Label(page)->text().toUtf8());
        file.write("\n");
    }
    file.flush();
    file.close();
return;
    int current_inner_page = 0;
    for (int page=0; page<2; page++)
    {
        if (page==1 && current_inner_page==0)
        {
            file.write(GetTVLabel(page)->text().toUtf8());
            file.write("\n");
        }
        tModel* model;
        if (page==1)
        {
            file.write(tree_list[current_inner_page].node->text(0).toUtf8());
            file.write("\n");
            if (tree_list[current_inner_page].parent!=NULL)
                file.write(tree_list[current_inner_page].parent->text(0).toUtf8());
            else
                file.write("NULL");
            file.write("\n");
            model = (tModel*)tree_list[current_inner_page].view->model();
            current_inner_page++;
        }
        else
            model = (tModel*)GetTVEditor(page)->model();
        for (int row =0; row<model->rowCount(); row++)
        {
            for (int col=0; col<model->columnCount(QModelIndex()); col++)
            {
                if (col==3)
                    if (!model->data(row)->bfreq1)
                    {
                        file.write(",");
                        continue;
                    }
                if (col==4)
                    if (!model->data(row)->bfreq2)
                    {
                        file.write(",");
                        continue;
                    }
                if (col==5)
                    if (!model->data(row)->bfreq3)
                    {
                        file.write(",");
                        continue;
                    }
                file.write(model->data(model->index(row,col),Qt::EditRole).toString().toUtf8());
                file.write(",");
            }
            progress->setValue(progress->value()+1);
            file.write("\n");
        }
        if (page!=1)
        {
            file.write("\n");
            file.write(GetTVLabel(page)->text().toUtf8());
        }
        file.write("\n");
        if (page==0)
            file.write("\n");
        if (current_inner_page<tree_list.count() && page==1)
            page--;
        else if (page==1)
            file.write("PAGES_CLOSED\n");
    }
    for (int page=0; page<3; page++)
    {
        iModel* model = (iModel*)GetTV2Editor(page)->model();
        for (int row =0; row<model->rowCount(); row++)
        {
            for (int col=0; col<model->columnCount(QModelIndex()); col++)
            {
                file.write(model->data(model->index(row,col),Qt::EditRole).toString().toUtf8());
                file.write(",");
            }
            progress->setValue(progress->value()+1);
            file.write("\n");
        }
        file.write("\n");
        file.write(GetTV2Label(page)->text().toUtf8());
        file.write("\n");
    }
    file.flush();
    file.close();
}

void wf_mainwindow::on_open_pr_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = dialog.getOpenFileName(this,tr("Открыть"),"/home","*.scew");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    iModel::loading = true;
    int total_rows=0;
    QStringList list;
    int type=0;
    QString row;
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    for (;!file.readLine().isNull();)
        total_rows++;
    progress->setMaximum(total_rows);
    file.seek(0);
    progress->setValue(0);
    foreach (tree_item item, tree_list)
    {
        if (item.parent==NULL)
            continue;
        delete item.view;
        tree_list.removeOne(item);
    }
    foreach (QTreeWidgetItem* item, GetAllChildren(tree_list[0].node))
        delete item;
    QString name;
    QString parent;
    int page=0;
    int state=0;
    first_toggle = false;
    for (;!(row = file.readLine()).isEmpty();)
    {
        progress->setValue(progress->value()+1);
        if (page==6)
            break;
        if (state==0)
        {
            GetTVLabel(page)->setText(row);
            state = 1;
            continue;
        }
        else
        {
            if (page==0)
            {
                tModel* model = (tModel*)GetTVEditor(page)->model();
                QVariant var = row.left(row.length()-1);
                model->insertRows(0,var.toInt());
                for (int i=0; i<model->rowCount(); i++)
                {
                    row = file.readLine();
                    progress->setValue(progress->value()+1);
                    list = row.left(row.length()-1).split(',');
                    for (int col=0; col<8; col++)
                    {
                        if (col==3 || col==4 || col==5)
                            if (list[col].isEmpty())
                                continue;
                        model->setData(model->index(i,col),list[col],Qt::EditRole);
                        ((tModel*)model)->Init(i);
                    }
                }
                state = 0;
                progress->setValue(progress->value()+1);
                page++;
                file.readLine();
                if (model->rowCount()==0)
                    file.readLine();
            }
            else if (page==1)
            {
                state =2;
                do
                {
                    progress->setValue(progress->value()+1);
                    if (state==3 && row=="\n")
                    {
                        state=2;
                        continue;
                    }
                    if (state==2 && row=="\n")
                        continue;
                    if (row=="PAGE_CLOSED\n")
                        break;
                    tModel* model;
                    if (state==2)
                    {
                        state=3;
                        parent = row.left(row.length()-1);
                        row = file.readLine();
                        progress->setValue(progress->value()+1);
                        name = row.left(row.length()-1);
                        if (parent=="NULL")
                            model = (tModel*)GetTVEditor(page)->model();
                        else
                        {
                            foreach (tree_item item, tree_list)
                                if (item.node->text(0)==parent)
                                    tree->setCurrentItem(item.node);
                            AddTreeChild(name);
                            foreach (tree_item item, tree_list)
                                if (item.node->text(0)==name)
                                {
                                    model = (tModel*)item.view->model();
                                    break;
                                }
                        }
                    }
                    row = file.readLine();
                    progress->setValue(progress->value()+1);
                    QVariant var = row.left(row.length()-1);
                    model->insertRows(0,var.toInt());
                    for (int i=0; i<model->rowCount(); i++)
                    {
                        row = file.readLine();
                        progress->setValue(progress->value()+1);
                        list = row.left(row.length()-1).split(',');
                        for (int col=0; col<8; col++)
                        {
                            if (col==3 || col==4 || col==5)
                                if (list[col].isEmpty())
                                    continue;
                            model->setData(model->index(i,col),list[col],Qt::EditRole);
                            ((tModel*)model)->Init(i);
                        }
                    }
                }
                while (!(row = file.readLine()).isEmpty());
                state = 0;
                page++;
            }
            else if (page>1)
            {
                state =2;
                do
                {
                    progress->setValue(progress->value()+1);
                    if (state==3 && row=="\n")
                    {
                        state=2;
                        continue;
                    }
                    if (state==2 && row=="\n")
                        continue;
                    if (row=="PAGE_CLOSED\n")
                        break;
                    tModel* model;
                    if (state==2)
                    {
                        state=3;
                        parent = row.left(row.length()-1);
                        row = file.readLine();
                        progress->setValue(progress->value()+1);
                        name = row.left(row.length()-1);
                        if (parent=="NULL")
                            model = (tModel*)GetTVEditor(page)->model();
                        else
                        {
                            foreach (tree_item item, tree_lists[page-2])
                                if (item.node->text(0)==parent)
                                    trees[page-2]->setCurrentItem(item.node);
                            AddTreeChildES(page-2,name);
                            foreach (tree_item item, tree_lists[page-2])
                                if (item.node->text(0)==name)
                                {
                                    model = (tModel*)item.view->model();
                                    break;
                                }
                        }
                    }
                    progress->setValue(progress->value()+1);
                    row = file.readLine();
                    QVariant var = row.left(row.length()-1);
                    model->insertRows(0,var.toInt());
                    for (int i=0; i<model->rowCount(); i++)
                    {
                        progress->setValue(progress->value()+1);
                        row = file.readLine();
                        list = row.left(row.length()-1).split(',');
                        for (int col=0; col<8; col++)
                        {
                            if (col==3 || col==4 || col==5)
                                if (list[col].isEmpty())
                                    continue;
                            model->setData(model->index(i,col),list[col],Qt::EditRole);
                            ((tModel*)model)->Init(i);
                        }
                    }
                }
                while (!(row = file.readLine()).isEmpty());
                state = 0;
                page++;
            }
        }
    }

    iModel::loading = true;

    page =0;
    iModel* imodel;
    state =0;
    do
    {
        progress->setValue(progress->value()+1);
        if (page==3)
            break;
        if (state==0)
        {
            imodel = (iModel*)GetTV2Editor(page)->model();
            QVariant var = row.left(row.length()-1);
            imodel->insertRows(0,var.toInt());
            state = 1;
            continue;
        }
        else
        {
            imodel = (iModel*)GetTV2Editor(page)->model();
            for (int i=0; i<imodel->rowCount(); i++)
            {
                list = row.left(row.length()-1).split(',');
                for (int col=0; col<4; col++)
                    imodel->setData(imodel->index(i,col),list[col],Qt::EditRole);
                row = file.readLine();
                progress->setValue(progress->value()+1);
            }
            row = file.readLine();
            progress->setValue(progress->value()+1);
            GetTV2Label(page)->setText(row.left(row.length()-1));
            page++;
            state = 0;
        }

    } while (!(row = file.readLine()).isEmpty());

    iModel::loading = false;
    file.close();
    foreach (tree_item item, tree_list)
        item.node->setText(1,QString::number(item.view->model()->rowCount()));
    for (int i=0; i<4;i++)
        foreach (tree_item item, tree_lists[i])
            item.node->setText(1,QString::number(item.view->model()->rowCount()));
    return;

    /*
    foreach (tree_item item, tree_list)
        item.node->setText(1,QString::number(item.view->model()->rowCount()));
    current_tab = tree_list.first().node;
    QStringList names;
    QList<MyView*> views;
    foreach (tree_item litem, tree_list)
        if (tree_list.first().node!=litem.node)
        {
            names<<litem.node->text(0);
            views<<litem.view;
        }
    tree_list.first().view->SetBrowserInfo(names,views);
    tree->setCurrentIndex(tree->model()->index(0,0));
    return;
    /*bool tmp_pages=true;
    for (;!(row = file.readLine()).isEmpty();)
    {
        if (row == "\n")
        {
            if (type==0)
            {
                if (page==1)
                {
                    if (parent!="NULL")
                    {
                        foreach (tree_item item, tree_list)
                            if (item.node->text(0)==parent)
                                tree->setCurrentItem(item.node);
                        AddTreeChild(name);
                        foreach (tree_item item, tree_list)
                            if (item.node->text(0)==name)
                            {
                                item.view->model()->insertRows(0,total_rows-2);
                                progress->setMaximum(progress->maximum()+total_rows-2);
                                break;
                            }
                    }
                    else
                    {
                        ((tModel*)tree_list[0].view->model())->Clear();
                        tree_list[0].view->model()->insertRows(0,total_rows-2);
                        progress->setMaximum(progress->maximum()+total_rows-2);
                    }
                    page--;
                    name.clear();
                    parent.clear();
                }
                else
                {
                    ((tModel*)GetTVEditor(page)->model())->Clear();
                    GetTVEditor(page)->model()->insertRows(0,total_rows);
                    progress->setMaximum(progress->maximum()+total_rows);
                }
            }
            else
            {
                if (page<3)
                {
                    ((iModel*)GetTV2Editor(page)->model())->Clear();
                    GetTV2Editor(page)->model()->insertRows(0,total_rows);
                    progress->setMaximum(progress->maximum()+total_rows);
                }
            }
            //progress->setMaximum(progress->maximum()+total_rows);
            page++;
            if (page>2 && type==1)
                break;
            if (page>5)
            {
                page=0;
                type=1;
            }
            if (page==1 && tmp_pages)
            {
                tmp_pages = false;
                row = file.readLine();
                row = file.readLine();
                row = file.readLine();
            }
            else if ((page!=1 && type==0) || type==1)
                row = file.readLine();
            total_rows=0;
        }
        else
        {
            if (page==1)
            {
                if (name.isEmpty())
                    name = row.left(row.length()-1);
                else if (parent.isEmpty())
                    parent = row.left(row.length()-1);
                if (name=="PAGES_CLOSED")
                {
                    name.clear();
                    page++;
                    total_rows=-1;
                }
            }
            total_rows++;
        }
    }
    file.seek(0);
    page=0;
    type=0;
    //QStringList list;
    tmp_pages = true;
    bool KOSTIL = false;
    bool KOSTIL2 = false;
    name.clear();
    parent.clear();

    for (int i=0; !(row = file.readLine()).isEmpty(); i++)
    {
        if (page>5)
        {
            page=0;
            type=1;
        }
        if (row=="\n")
        {
            if (page==1 && type==0)
                page--;
            else
                row = file.readLine();
            if (type==0)
            {
                if (tmp_pages && KOSTIL)
                {
                    tmp_pages=false;
                    row = file.readLine();
                    //row = file.readLine();
                    GetTVLabel(page+1)->setText(row.left(row.length()-1));
                }
                else
                {
                    if (!KOSTIL || KOSTIL2)
                        GetTVLabel(page)->setText(row.left(row.length()-1));
                    KOSTIL = true;
                }
                name.clear();
                parent.clear();
            }
            else
                GetTV2Label(page)->setText(row.left(row.length()-1));
            page++;
            i=-1;
            continue;
        }
        else if (row=="PAGES_CLOSED\n")
        {
            page++;
            KOSTIL2 = true;
            i=-1;
            continue;
        }
        QAbstractTableModel* model;
        if (type==0)
            if (page==1)
            {
                if (name.isEmpty())
                {
                    name = row.left(row.length()-1);
                    parent = file.readLine();
                    row = file.readLine();
                }
                foreach (tree_item item, tree_list)
                {

                    if (item.node->text(0)==name)
                    {
                        model = (QAbstractTableModel*)item.view->model();
                        break;
                    }
                }
            }
            else
                model = (QAbstractTableModel*)GetTVEditor(page)->model();
        else
            model = (QAbstractTableModel*)GetTV2Editor(page)->model();
        if (row=="\n" && page==1)
        {
            name.clear();
            parent.clear();
            i=-1;
            continue;
        }
        list = row.left(row.length()-1).split(',');
        if (!((page==POSITIVE_BD_TABLE || page==NEGATIVE_BD_TABLE) && type==1))
        for (int col=0; col<list.count(); col++)
        {
            if (col==3 || col==4 || col==5)
                if (list[col].isEmpty())
                    continue;
            model->setData(model->index(i,col),list[col],Qt::EditRole);
            if (type==0)
                ((tModel*)model)->Init(i);
        }
        progress->setValue(progress->value()+1);
    }
    iModel::loading = false;
    file.close();
    foreach (tree_item item, tree_list)
        item.node->setText(1,QString::number(item.view->model()->rowCount()));
    current_tab = tree_list.first().node;
    QStringList names;
    QList<MyView*> views;
    foreach (tree_item litem, tree_list)
        if (tree_list.first().node!=litem.node)
        {
            names<<litem.node->text(0);
            views<<litem.view;
        }
    tree_list.first().view->SetBrowserInfo(names,views);
    tree->setCurrentIndex(tree->model()->index(0,0));*/
}
