#include "mainwindow.h"

void MainWindow::table_menu_event(const QPoint &pos)
{
    Q_UNUSED(pos);
    int visible =0;
    act_delete_row->setVisible(false);
    act_add_col->setVisible(false);
    act_delete_col->setVisible(false);
    if (ui->tableWidget->selectedItems().count()==1 &&
            ui->tableWidget->selectedItems().at(0)->text()!="" &&
            (ui->tableWidget->selectedItems().at(0)->column()!=1  &&
            ui->tableWidget->selectedItems().at(0)->column()!=2))
        act_add_word->setVisible(true);
    else
    {
        act_add_word->setVisible(false);
        visible++;
    }
    if (ui->tableWidget->rowCount()==0)
        act_add_row->setVisible(true);
    else
     {
        act_add_row->setVisible(false);
        visible++;
    }
    if (visible<2)
        table_menu->popup(QCursor::pos());
}

void MainWindow::table_menu_event_col()
{
    if (show_results || ui->tableWidget->selectionModel()->selectedColumns().count()==0 || ui->tableWidget->selectionModel()->selectedColumns().at(0).column()<3)
        return;
    act_add_word->setVisible(false);
    act_add_row->setVisible(false);
    act_delete_row->setVisible(false);
    if (ui->tableWidget->selectionModel()->selectedColumns().at(0).column()==3 && ui->tableWidget->columnCount()==4)
        act_delete_col->setVisible(false);
    else
        act_delete_col->setVisible(true);
    act_add_col->setVisible(true);
    table_menu->popup(QCursor::pos());
}

void MainWindow::table_menu_event_row()
{
    if (show_results || ui->tableWidget->selectionModel()->selectedRows().count()==0)
        return;
    act_add_word->setVisible(false);
    act_add_row->setVisible(true);
    act_delete_row->setVisible(true);
    act_add_col->setVisible(false);
    act_delete_col->setVisible(false);
    table_menu->popup(QCursor::pos());
}

void MainWindow::add_word()
{
    if (!ui->tableWidget->currentItem())
        return;
    QTextCursor cur(ui->textEdit->document());
    cur.setPosition(ui->textEdit->textCursor().position());

    int curp = cur.position();
    int curd = ui->tableWidget->currentItem()->text().length();
    off_ct = true;

    if (cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor))
        if (cur.selectedText() != " ")
        {
            ui->textEdit->insertPlainText(" ");
            curd++;
        }

    QString text;
    text = show_results ? ui->tableWidget->currentItem()->text().left(ui->tableWidget->currentItem()->text().indexOf(' ')) :
                          ui->tableWidget->currentItem()->text();
    if (is_job_done)
        if (ui->tableWidget->currentItem()->column()==0)
        {
            m_k[ui->tableWidget->currentItem()->row()]->met++;
            if (ui->m_stress->isChecked())
                text = "<b><u><font color='red'>" + text + "</font></b></u>";
        }
        else
        {
            low_task* iter = m_k[ui->tableWidget->currentItem()->row()]->head;
            for (int i=0; i<ui->tableWidget->currentItem()->column()-3;i++)
                iter = iter->next;
            m_k[ui->tableWidget->currentItem()->row()]->low_met++;
            iter->met++;
            if (ui->m_stress->isChecked())
                text = "<b><font color='blue'>" + text + "</font></b>";
        }

    ui->textEdit->insertHtml(text);
    tpl = ui->textEdit->toPlainText().length();
    if (show_results)
        table_show_results();
    off_ct = false;

    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,oper_info(7,
                                                     curd,
                                                     curp,
                                                     ui->tableWidget->currentItem()->text()));
}

void MainWindow::add_row()
{
    if (ui->tableWidget->rowCount()==0)
        ui->tableWidget->setRowCount(1);
    else
        ui->tableWidget->insertRow(ui->tableWidget->currentRow());

    refresh_ur();
    if (ui->tableWidget->currentRow()>-1)
        oper_stack.insert(oper_stack.begin(),1,oper_info(3,ui->tableWidget->currentRow()-1));
    else
        oper_stack.insert(oper_stack.begin(),1,oper_info(3,0));
}

void MainWindow::delete_row()
{
    for (int i=0, count=ui->tableWidget->selectionModel()->selectedRows().count(); i<count; i++)
    {
        refresh_ur();
        QStringList list;
        for (int i=0; i<ui->tableWidget->columnCount(); i++)
            if (ui->tableWidget->item(ui->tableWidget->selectionModel()->selectedRows().at(0).row(),i))
                list << ui->tableWidget->item(ui->tableWidget->selectionModel()->selectedRows().at(0).row(),i)->text();
        else
                list << "";
        oper_stack.insert(oper_stack.begin(),1,oper_info(4,ui->tableWidget->selectionModel()->selectedRows().at(0).row()+1,
                                                         0,"","",QTextCharFormat(),QTextBlockFormat(),list));
        ui->tableWidget->removeRow(ui->tableWidget->selectionModel()->selectedRows().at(0).row());
    }
}

void MainWindow::add_col()
{
    if (ui->tableWidget->rowCount()==0)
        return;
    ui->tableWidget->insertColumn(ui->tableWidget->currentColumn()+1);
    for (int i=4; i<ui->tableWidget->columnCount();i++)
    {
        char buf_int[15];
        sprintf(buf_int, "%d", i-2);
        ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(tr("Словоформа ") + buf_int));
    }
    ui->tableWidget->resizeColumnToContents(ui->tableWidget->currentColumn()+1);

    refresh_ur();
    oper_stack.insert(oper_stack.begin(),1,oper_info(5,ui->tableWidget->currentColumn()));
}

void MainWindow::delete_col()
{
    for (int i=0, count=ui->tableWidget->selectionModel()->selectedColumns().count(); i<count; i++)
    {
        refresh_ur();
        QStringList list;
        for (int i=0; i<ui->tableWidget->rowCount(); i++)
            if (ui->tableWidget->item(i,ui->tableWidget->selectionModel()->selectedColumns().at(0).column()))
                list << ui->tableWidget->item(i,ui->tableWidget->selectionModel()->selectedColumns().at(0).column())->text();
        else
                list << "";
        oper_stack.insert(oper_stack.begin(),1,oper_info(6,ui->tableWidget->selectionModel()->selectedColumns().at(0).column(),
                                                         0,"","",QTextCharFormat(),QTextBlockFormat(),list));
        ui->tableWidget->removeColumn(ui->tableWidget->selectionModel()->selectedColumns().at(0).column());
    }
    for (int i=4; i<ui->tableWidget->columnCount();i++)
    {
        char buf_int[15];
        sprintf(buf_int, "%d", i-2);
        ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(tr("Словоформа ") + buf_int));
    }
}

void MainWindow::table_sort(int col)
{
    off_cch = true;
    cells_memory.clear();
    memory_iter=0;
    int erase_me=0;
    foreach (oper_info val, oper_stack)
    {
        if (val.ind>1 && val.ind!=7)
        {
            oper_stack.erase(oper_stack.begin()+erase_me);
            if (oper_iter>erase_me)
                oper_iter--;
            erase_me--;
        }
        erase_me++;
    }
    if (oper_iter==0)
        bu_2->setEnabled(false);
    if (oper_stack.empty())
        bu_1->setEnabled(false);
    QString fake_text[ui->tableWidget->rowCount()];
    int fake_count[ui->tableWidget->rowCount()];
    int fake_count2[ui->tableWidget->rowCount()];
    for (unsigned int row=0; ui->tableWidget->item(row,0); row++)
        fake_text[row] = ui->tableWidget->item(row,0)->text();
    Qt::SortOrder order;
    if (sort_order==0)
    {
        sort_order=1;
        order=Qt::AscendingOrder;
    }
    else
    {
        sort_order=0;
        order=Qt::DescendingOrder;
    }

    ui->tableWidget->sortItems(col,order);
    off_cch = false;

    if (!is_job_done)
        return;

    mini_task* buf_mk[ui->tableWidget->rowCount()];

    for (int i=0; i<ui->tableWidget->rowCount(); i++)
        for (int row=0; row < ui->tableWidget->rowCount(); row++)
            if (ui->tableWidget->item(i,0) && ui->tableWidget->item(i,0)->text()==fake_text[row])
            {
                buf_mk[i] = m_k[row];
                fake_count[i] = count_mas[row];
                fake_count2[i] = count_mas2[row];
                break;
            }

    for (unsigned int i=0; i<words_count; i++)
    {
        m_k[i] = buf_mk[i];
        count_mas[i] = fake_count[i];
        count_mas2[i] = fake_count2[i];
    }

    is_pr_saved = false;
    ui->m_Save_pr_as->setEnabled(true);
    ui->m_proccessing->setChecked(false);
    setWindowTitle(pr_name+'*');
}

void MainWindow::on_tableWidget_clicked(const QModelIndex &index)
{
    if (show_results)
        return;

    if (ui->tableWidget->item(index.row(),index.column()))
        fake_memory = ui->tableWidget->item(index.row(),index.column())->text();
    else
        fake_memory.clear();
}

void MainWindow::table_show_results()
{
    int picked = ui->tableWidget->currentRow();
    ui->tableWidget->clearContents();
    labels1 = new QLabel*[words_count];
    labels2 = new QLabel*[words_count];
    for (unsigned int i=0; i<words_count; i++)
    {
        labels1[i] = new QLabel();
        labels2[i] = new QLabel();
        ui->tableWidget->setItem(i,0,new QTableWidgetItem((m_k[i])->main_word));
        char cur_index[15];
        low_task* iter = (m_k[i])->head;
        for (unsigned int j=3; iter!=NULL; j++, iter = iter->next)
        {
            sprintf(cur_index, "%d", iter->met);
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(iter->word));
            if (iter->met==0)
            {
                ui->tableWidget->item(i,j)->setBackgroundColor("red");
                ui->tableWidget->item(i,j)->setText(ui->tableWidget->item(i,j)->text() + " (не найдено)");
                continue;
            }
            ui->tableWidget->item(i,j)->setText(ui->tableWidget->item(i,j)->text() + " ("+cur_index + ")");
            if (iter->founded)
                ui->tableWidget->item(i,j)->setBackgroundColor("green");
        }
        QString arrow;
        if (count_mas[i]-((m_k[i])->met)>0)
            arrow = "<html><b><font color='red'>&darr;</font></b></html> ";
        else
            if (count_mas[i]-((m_k[i])->met)!=0)
                arrow = "<html><b><font color='green'>&uarr;</font></b></html> ";

        labels1[i]->setText(QString::number(abs(count_mas[i]-m_k[i]->met)));
        labels1[i]->setText(arrow+labels1[i]->text() +" <b><font color='red'>("+QString::number(m_k[i]->met)+")</b></font>");
        ui->tableWidget->setCellWidget(i,1,labels1[i]);

        arrow.clear();
        if (count_mas2[i]-((m_k[i])->low_met)>0)
            arrow = "<html><b><font color='red'>&darr;</font></b></html> ";
        else
            if (count_mas2[i]-((m_k[i])->low_met)!=0)
                arrow = "<html><b><font color='green'>&uarr;</font></b></html> ";

        labels2[i]->setText(QString::number(abs(count_mas2[i]-m_k[i]->low_met)));
        labels2[i]->setText(arrow+labels2[i]->text() +" <b><font color='blue'>("+QString::number(m_k[i]->low_met)+")</b></font>");
        ui->tableWidget->setCellWidget(i,2,labels2[i]);
    }
    ui->m_show_results->setChecked(true);
    ui->m_show_task->setChecked(false);
    show_results = true;
    ui->tableWidget->selectRow(picked);
    ui->tableWidget->setRowCount(words_count);
    for (int col =0; col<ui->tableWidget->columnCount(); col++)
        ui->tableWidget->resizeColumnToContents(col);
}

void MainWindow::table_show_task()
{
    int picked = ui->tableWidget->currentRow();
    ui->tableWidget->clearContents();
    for (unsigned int i=0; i<words_count; i++)
    {
        ui->tableWidget->setItem(i,0,new QTableWidgetItem((m_k[i])->main_word));
        low_task* iter = (m_k[i])->head;
        for (unsigned int j=3; iter!=NULL; j++, iter = iter->next)
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(iter->word));
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(QString::number(count_mas[i])));
        ui->tableWidget->setItem(i,2,new QTableWidgetItem(QString::number(count_mas2[i])));
    }
    ui->m_show_results->setChecked(false);
    ui->m_show_task->setChecked(true);
    show_results = false;
    ui->tableWidget->selectRow(picked);
    ui->tableWidget->setRowCount(words_count);
    for (int col =0; col<ui->tableWidget->columnCount(); col++)
        ui->tableWidget->resizeColumnToContents(col);
}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
    if (off_cch || show_results)
        return;
    ui->tableWidget->resizeColumnToContents(column);

    std::vector<std::pair<int,int> >::iterator rus_iter;
    if ((rus_iter=std::find(founded_words.begin(),founded_words.end(),std::make_pair(row,column)))!=founded_words.end())
        founded_words.erase(rus_iter);
    refresh_ur();
    oper_stack.insert(oper_stack.begin(),1,oper_info(2,row,column));
    cells_memory.insert(cells_memory.begin(),1,fake_memory);
    fake_memory = ui->tableWidget->item(row,column)->text();
}
