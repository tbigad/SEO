#include "mainwindow.h"
#include "dbuniq.h"
#include "ui_dbuniq.h"

void MainWindow::paint_shingles()
{
    QList<QStringList> set = dbuniq_dialog->shingles_similar;
    if (set.isEmpty())
        return;
    QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?-1234567890";
    QTextCursor cur(ui->textEdit->document());
    QChar end_of_buf;
    fake_text = ui->textEdit->toHtml();
    progress->setMaximum(ui->textEdit->toPlainText().length());
    off_ct = true;
    QString word;
    QStringList founded_set;
    QTextCharFormat format;
    format.setBackground(QBrush(QColor("yellow")));
    int word_length = 0;
    while (cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor))
    {
        word_length++;
        progress->setValue(progress->value()+1);
        word = cur.selectedText().right(word_length);
        end_of_buf = word[word.length()-1];
        if (wrong_char.contains(end_of_buf) || end_of_buf.unicode()==8233)
        {
            word.remove(word.length()-1,1);
            if  (word.length()>3)
            {
                if (founded_set.length()<set[0].length())
                    founded_set<<word;
                else
                {
                    if (founded_set == set[0])
                    {
                        word = cur.selectedText();
                        cur.removeSelectedText();
                        cur.insertText(word,format);
                    }
                    else
                    {
                        founded_set.clear();
                        set.removeFirst();
                    }
                    cur.clearSelection();
                }
            }
            word_length=0;
        }
    }
    if (founded_set == set[0])
    {
        word = cur.selectedText();
        cur.removeSelectedText();
        cur.insertText(word,format);
    }
    off_ct = false;
}

int MainWindow::find_in_data(QString word,const int add)
{
    for (unsigned int row_c =0; row_c < words_count; row_c++)
        if ((m_k[row_c])->main_word==word)
        {
            (m_k[row_c])->met+=add;
            return 1;
        }
        else
        {
            low_task* iter_l=(m_k[row_c])->head;
            while (iter_l!=NULL)
            {
                if (iter_l->word==word)
                {
                    (m_k[row_c])->low_met+=add;
                    iter_l->met+=add;
                    return 2;
                }
                iter_l=iter_l->next;
            }
        }
    return -1;
}

void MainWindow::on_m_proccessing_triggered()
{
    if (show_results)
        table_show_task();
    delete_task_data();

    if (!ui->m_add_rus->isChecked())
    {
        for (size_t i=0; i<founded_words.size(); i++)
            if (ui->tableWidget->item(founded_words[i].first,founded_words[i].second))
                ui->tableWidget->item(founded_words[i].first,founded_words[i].second)->setText("");
        founded_words.clear();
    }

    words_count = 0;
    int broken =0;
    m_k = new mini_task*[ui->tableWidget->rowCount()];

    for (int row_c =0; row_c<ui->tableWidget->rowCount(); row_c++)
    {
        if (!ui->tableWidget->item(row_c,0) || ui->tableWidget->item(row_c,0)->text()=="")
        {
            broken++;
            continue;
        }
        mini_task* new_task = new mini_task;
        words_count++;
        new_task->main_word = ui->tableWidget->item(row_c,0)->text().toLower();
        new_task->met=0;
        new_task->low_met=0;
        bool first = true;
        new_task->head=NULL;
        for (int col_c = 3; ui->tableWidget->item(row_c,col_c); col_c++)
        {
            if (ui->tableWidget->item(row_c,col_c)->text()=="")
                continue;
            low_task* new_low_task = new low_task;
            new_low_task->word = ui->tableWidget->item(row_c,col_c)->text().toLower();
            new_low_task->met = 0;
            if (std::find(founded_words.begin(),founded_words.end(),std::make_pair(row_c,col_c))!=founded_words.end())
                new_low_task->founded=true;
            if (first)
            {
                first=false;
                new_low_task->next = NULL;
                new_task->head = new_low_task;
            }
            else
            {
                low_task* iter = new_task->head;
                new_low_task->next=NULL;
                while (iter->next!=NULL)
                    iter = iter->next;
                iter->next = new_low_task;
            }
        }
        m_k[row_c-broken] = new_task;
    }
    if (words_count==0)
        return;

    count_mas = new int[ui->tableWidget->rowCount()];
    count_mas2 = new int[ui->tableWidget->rowCount()];

    for (int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        if (ui->tableWidget->item(i,1))
            count_mas[i]=ui->tableWidget->item(i,1)->text().toInt();
        else
            count_mas[i]=0;
        if (ui->tableWidget->item(i,2))
            count_mas2[i]=ui->tableWidget->item(i,2)->text().toInt();
        else
            count_mas2[i]=0;
    }

    off_cch = true;
    progress->setValue(0);
    bool found=false;
    int rus_f;
    QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?";
    QString word = "";

    if (!ui->m_stress->isChecked())
    {
        const QString widget_text = ui->textEdit->toPlainText().toLower();
        QString::const_iterator iter = widget_text.begin();
        progress->setMaximum(widget_text.length());
        while (iter!=widget_text.end())
        {
            progress->setValue(progress->value()+1);
            if (wrong_char.contains((*(iter))))
            {
                if (word!="")
                {
                    found=false;
                    for (unsigned int row_c =0; row_c < words_count; row_c++)
                        if ((m_k[row_c])->main_word==word)
                        {
                            (m_k[row_c])->met++;
                            found=true;
                            break;
                        }
                        else
                        {
                            low_task* iter_l=(m_k[row_c])->head;
                            while (iter_l!=NULL)
                            {
                                if (iter_l->word==word)
                                {
                                    (m_k[row_c])->low_met++;
                                    iter_l->met++;
                                    found=true;
                                    break;
                                }
                                iter_l=iter_l->next;
                            }
                        }
                }
                if (ui->m_add_rus->isChecked()==true && !found && word!="" && (rus_f=russian_test(word))!=-1)
                {
                    low_task* iter = (m_k[rus_f])->head;
                    low_task* new_low_task = new low_task;
                    low_task* help_fiter = iter;
                    int su=1;
                    while (iter!=NULL)
                    {
                        su++;
                        if (!iter->next)
                        {
                            help_fiter = iter;
                            break;
                        }
                        iter=iter->next;
                    }
                    if (help_fiter == NULL)
                    {
                        (m_k[rus_f])->head = new_low_task;
                    }
                    else
                        help_fiter->next = new_low_task;
                    new_low_task->word = word;
                    new_low_task->met=1;
                    new_low_task->next = NULL;
                    new_low_task->founded=true;
                    founded_words.push_back(std::make_pair(rus_f,su+2));
                    (m_k[rus_f])->low_met++;
                    if (ui->tableWidget->columnCount()<su+3)
                    {
                        ui->tableWidget->setColumnCount(su+3);
                        char buf_int[15];
                        sprintf(buf_int, "%d", ui->tableWidget->columnCount()-3);
                        ui->tableWidget->setHorizontalHeaderItem(ui->tableWidget->columnCount()-1,new QTableWidgetItem(tr("Словоформа ") + buf_int));
                    }
                    ui->tableWidget->setItem(rus_f,su+2,new QTableWidgetItem(word));
                }
                word="";
            }
            else
                word+=(*(iter));
            iter++;
        }
        found = false;
        if (word!="")
        {
            for (unsigned int row_c =0; row_c < words_count; row_c++)
                if ((m_k[row_c])->main_word==word)
                {
                    (m_k[row_c])->met++;
                    found = true;
                    break;
                }
                else
                {
                    low_task* iter_l=(m_k[row_c])->head;
                    while (iter_l!=NULL)
                    {
                        if (iter_l->word==word)
                        {
                            (m_k[row_c])->low_met++;
                            iter_l->met++;
                            found = true;
                            break;
                        }
                        iter_l=iter_l->next;
                    }
                }
            if (ui->m_add_rus->isChecked()==true && !found && (rus_f=russian_test(word))!=-1)
            {
                low_task* iter = (m_k[rus_f])->head;
                low_task* new_low_task = new low_task;
                low_task* help_fiter = iter;
                int su=1;
                while (iter!=NULL)
                {
                    su++;
                    if (!iter->next)
                    {
                        help_fiter = iter;
                        break;
                    }
                    iter=iter->next;
                }
                if (help_fiter == NULL)
                {
                    (m_k[rus_f])->head = new_low_task;
                }
                else
                    help_fiter->next = new_low_task;
                new_low_task->word = word;
                new_low_task->met=1;
                new_low_task->next = NULL;
                new_low_task->founded=true;
                (m_k[rus_f])->low_met++;
                if (ui->tableWidget->columnCount()<su+3)
                {
                    ui->tableWidget->setColumnCount(su+3);
                    char buf_int[15];
                    sprintf(buf_int, "%d", ui->tableWidget->columnCount()-3);
                    ui->tableWidget->setHorizontalHeaderItem(ui->tableWidget->columnCount()-1,new QTableWidgetItem(tr("Словоформа ") + buf_int));
                }
                ui->tableWidget->setItem(rus_f,su+2,new QTableWidgetItem(word));
            }
        }
    }
    else
    {
        QTextCursor cur(ui->textEdit->document());
        QChar end_of_buf;
        fake_text = ui->textEdit->toHtml();
        progress->setMaximum(ui->textEdit->toPlainText().length());
        off_ct = true;
        QTextCharFormat format;
        QFont font_format;
        QString nature_word;
        while (cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor))
        {
            progress->setValue(progress->value()+1);
            word = cur.selectedText();
            end_of_buf = word[word.length()-1];
            if (wrong_char.contains(end_of_buf) || end_of_buf.unicode()==8233)
            {
                word.remove(word.length()-1,1);
                nature_word = word;
                word = word.toLower();
                found = false;
                for (unsigned int row_c =0; row_c < words_count; row_c++)
                    if ((m_k[row_c])->main_word==word)
                    {
                        cur.removeSelectedText();
                        format.setForeground(QBrush(QColor("red")));
                        font_format.setBold(true);
                        font_format.setUnderline(true);
                        format.setFont(font_format);
                        cur.insertText(nature_word,format);
                        cur.insertText(end_of_buf);
                        (m_k[row_c])->met++;
                        found = true;
                        break;
                    }
                    else
                    {
                        low_task* iter_l=(m_k[row_c])->head;
                        while (iter_l!=NULL)
                        {
                            if (iter_l->word==word)
                            {
                                cur.removeSelectedText();
                                format.setForeground(QBrush(QColor("blue")));
                                font_format.setBold(true);
                                font_format.setUnderline(false);
                                format.setFont(font_format);
                                cur.insertText(nature_word,format);
                                cur.insertText(end_of_buf);
                                found = true;
                                (m_k[row_c])->low_met++;
                                iter_l->met++;
                                break;
                            }
                            iter_l=iter_l->next;
                        }
                    }
                if (ui->m_add_rus->isChecked()==true && !found && word!="" && (rus_f=russian_test(word))!=-1)
                {
                    low_task* iter = (m_k[rus_f])->head;
                    low_task* new_low_task = new low_task;
                    low_task* help_fiter = iter;
                    int su=1;
                    while (iter!=NULL)
                    {
                        su++;
                        if (!iter->next)
                        {
                            help_fiter = iter;
                            break;
                        }
                        iter=iter->next;
                    }
                    if (help_fiter == NULL)
                    {
                        (m_k[rus_f])->head = new_low_task;
                    }
                    else
                        help_fiter->next = new_low_task;
                    new_low_task->word = word;
                    new_low_task->met=1;
                    new_low_task->next = NULL;
                    new_low_task->founded=true;
                    founded_words.push_back(std::make_pair(rus_f,su+2));
                    (m_k[rus_f])->low_met++;
                    if (ui->tableWidget->columnCount()<su+3)
                    {
                        ui->tableWidget->setColumnCount(su+3);
                        char buf_int[15];
                        sprintf(buf_int, "%d", ui->tableWidget->columnCount()-3);
                        ui->tableWidget->setHorizontalHeaderItem(ui->tableWidget->columnCount()-1,new QTableWidgetItem(tr("Словоформа ") + buf_int));
                    }
                    ui->tableWidget->setItem(rus_f,su+2,new QTableWidgetItem(word));
                }
                cur.clearSelection();
            }
        }
        word = cur.selectedText().toLower();
        found = false;
        for (unsigned int row_c =0; row_c < words_count; row_c++)
            if ((m_k[row_c])->main_word==word.toLower())
            {
                cur.removeSelectedText();
                cur.insertHtml("<b><u><font color='red'>" + word + "</font></b></u>" + end_of_buf);
                (m_k[row_c])->met++;
                found = true;
                break;
            }
            else
            {
                low_task* iter_l=(m_k[row_c])->head;
                while (iter_l!=NULL)
                {
                    if (iter_l->word==word.toLower())
                    {
                        cur.removeSelectedText();
                        cur.insertHtml("<b><font color='blue'>" + word + "</font></b>" + end_of_buf);
                        found = true;
                        (m_k[row_c])->low_met++;
                        iter_l->met++;
                        break;
                    }
                    iter_l=iter_l->next;
                }
            }
        if (ui->m_add_rus->isChecked()==true && !found && word!="" && (rus_f=russian_test(word))!=-1)
        {
            low_task* iter = (m_k[rus_f])->head;
            low_task* new_low_task = new low_task;
            low_task* help_fiter = iter;
            int su=1;
            while (iter!=NULL)
            {
                su++;
                if (!iter->next)
                {
                    help_fiter = iter;
                    break;
                }
                iter=iter->next;
            }
            if (help_fiter == NULL)
            {
                (m_k[rus_f])->head = new_low_task;
            }
            else
                help_fiter->next = new_low_task;
            new_low_task->word = word;
            new_low_task->met=1;
            new_low_task->next = NULL;
            new_low_task->founded=true;
            founded_words.push_back(std::make_pair(rus_f,su+2));
            (m_k[rus_f])->low_met++;
            if (ui->tableWidget->columnCount()<su+3)
            {
                ui->tableWidget->setColumnCount(su+3);
                char buf_int[15];
                sprintf(buf_int, "%d", ui->tableWidget->columnCount()-3);
                ui->tableWidget->setHorizontalHeaderItem(ui->tableWidget->columnCount()-1,new QTableWidgetItem(tr("Словоформа ") + buf_int));
            }
            ui->tableWidget->setItem(rus_f,su+2,new QTableWidgetItem(word));
        }
        tpl = ui->textEdit->toPlainText().length();
        refresh_ur(false);
        oper_stack.insert(oper_stack.begin(),1,
                  oper_info(1,1,1,
                  fake_text,ui->textEdit->toHtml()  ));
        fake_text = ui->textEdit->toHtml();
    }
    off_ct = false;
    table_show_results();
    off_cch = false;
    ui->m_show_results->setEnabled(true);
    ui->m_show_task->setEnabled(true);
    setWindowTitle(pr_name+'*');
    is_pr_saved = false;
    ui->m_Save_pr_as->setEnabled(true);
    is_job_done = true;
    ui->m_proccessing->setChecked(true);
}

int MainWindow::russian_test(QString word)
{
    QString rus_alp ="йцукенгшщзхъфывапролджэячсмитьбюё";

    QString test;
    std::vector<std::pair<int,int> > cont_of_happy;
    std::vector<std::pair<int,int> > help_vector;
    int success_count;
    int word_ind;
    bool line_of_s;
    int old_ind;
    QString lword,rword;

    for (unsigned int i = 0; i < words_count; i++)
    {
        test = m_k[i]->main_word;
        if (abs(word.length()-test.length())>5)
            continue;
        if (test.length()<word.length())
        {
            lword = test;
            rword = word;
        }
        else
        {
            lword = word;
            rword = test;
        }
        success_count=0;
        line_of_s=false;
        cont_of_happy.clear();
        help_vector.clear();
        old_ind=0;
        word_ind=0;
        for (int letter = 0; letter<lword.length(); letter++)
        {
            if (rus_alp.contains(lword[letter]))
            {
                old_ind = word_ind;
                if ((word_ind=rword.indexOf(lword[letter],old_ind))!=-1)
                {
                    if ((old_ind+1==word_ind && success_count>0) || (success_count==0))
                    {
                        if (!line_of_s)
                            success_count=1;
                        else
                            success_count++;
                        line_of_s = true;
                        help_vector.push_back(std::make_pair(word_ind,letter));
                    }
                    else
                    {
                        success_count=1;
                        line_of_s = true;
                        if (help_vector.size()>=cont_of_happy.size())
                        {
                            cont_of_happy.clear();
                            cont_of_happy = help_vector;
                        }
                        help_vector.clear();
                    }
                }
                else
                    if ((word_ind=rword.indexOf(lword[letter]))!=-1)
                        {
                            success_count=1;
                            line_of_s = true;
                            if (help_vector.size()>=cont_of_happy.size())
                            {
                                cont_of_happy.clear();
                                cont_of_happy = help_vector;
                            }
                            help_vector.clear();
                            help_vector.push_back(std::make_pair(word_ind,letter));
                        }
                    else
                    {
                        word_ind=0;
                        line_of_s = false;
                        success_count=0;
                        if (help_vector.size()>cont_of_happy.size())
                        {
                            cont_of_happy.clear();
                            cont_of_happy = help_vector;
                        }
                        help_vector.clear();
                    }
            }
            else
                break;
        }
        if (help_vector.size()>cont_of_happy.size())
        {
            cont_of_happy.clear();
            cont_of_happy = help_vector;
        }
        success_count = cont_of_happy.size();
        if (success_count<4)
            continue;
        QStringList prefixes;
        prefixes << "в"<<"с"<<"у"<<"о"<<
                    "су"<<"из"<<"ис"<<"вз"<<"вс"<<"от"<<"со"<<"об"<<"от"<<"за"<<"на"<<"до"<<"по"<<"за"<<"вы"<<"не"<<
                    "съ"<<"въ"<<
                    "без"<<"бес"<<"вос"<<"взо"<<"вза"<<"все"<<"еже"<<"изо"<<"ино"<<"наи"<<"над"<<"низ"<<"нис"<<"обо"<<
                    "ото"<<"пре"<<"под"<<"пра"<<"при"<<"про"<<"раз"<<"рас"<<"роз"<<"рос";
        line_of_s=false;
        switch (cont_of_happy[0].second)
        {
        case 0: line_of_s=true; break;
        case 1: foreach (QString elem,prefixes) {if (elem==lword.left(1)) line_of_s=true; if (elem.length()>1) break;} break;
        case 2: foreach (QString elem,prefixes) {if (elem==lword.left(2)) line_of_s=true; if (elem.length()>2) break;} break;
        case 3: foreach (QString elem,prefixes) {if (elem.length()<3) continue; if (elem==lword.left(3)) line_of_s=true;} break;
        }
        if (!line_of_s)
            continue;
        line_of_s=false;
        switch (cont_of_happy[0].first)
        {
        case 0: line_of_s=true; break;
        case 1: foreach (QString elem,prefixes) {if (elem==rword.left(1)) line_of_s=true; if (elem.length()>1) break;} break;
        case 2: foreach (QString elem,prefixes) {if (elem==rword.left(2)) line_of_s=true; if (elem.length()>2) break;} break;
        case 3: foreach (QString elem,prefixes) {if (elem.length()<3) continue; if (elem==rword.left(3)) line_of_s=true;} break;
        }
        if (!line_of_s)
            continue;
        QStringList postfixes;
        postfixes <<"а"<<"я"<<"о"<<"е"<<"и"<<"ы"<<"ю"<<
                    "ий"<<"ия"<<"ию"<<"ии"<<"ом"<<"ем"<<"ах"<<"ях"<<"ям"<<"ов"<<"ам"<<"ев"<<"ый"<<"ая"<<
                    "ое"<<"ые"<<"ых"<<"ым"<<"ой"<<"ую"<<"ие"<<"их"<<"им"<<"ей"<<"яя"<<"ий"<<"ее"<<"юю"<<
                    "ть"<<"ся"<<
                    "иев"<<"иям"<<"ием"<<"ами"<<"ями"<<"иях"<<"ого"<<"ому"<<"ими"<<"его"<<"ыми"<<"ься"<<"ать";
        line_of_s=false;
        switch (rword.length()-cont_of_happy[success_count-1].first-1)
        {
        case 0: line_of_s=true; break;
        case 1: foreach (QString elem,postfixes) {if (elem==rword.right(1)) line_of_s=true; if (elem.length()>1) break;} break;
        case 2: foreach (QString elem,postfixes) {if (elem==rword.right(2)) line_of_s=true; if (elem.length()>2) break;} break;
        case 3: foreach (QString elem,postfixes) {if (elem.length()<3) continue; if (elem==rword.right(3)) line_of_s=true;} break;
        }
        if (!line_of_s)
            continue;
        line_of_s=false;
        switch (lword.length()-cont_of_happy[success_count-1].second-1)
        {
        case 0: line_of_s=true; break;
        case 1: foreach (QString elem,postfixes) {if (elem==lword.right(1)) line_of_s=true; if (elem.length()>1) break;} break;
        case 2: foreach (QString elem,postfixes) {if (elem==lword.right(2)) line_of_s=true; if (elem.length()>2) break;} break;
        case 3: foreach (QString elem,postfixes) {if (elem.length()<3) continue; if (elem==lword.right(3)) line_of_s=true;} break;
        }
        if (!line_of_s)
            continue;
        return i;
    }
    return -1;
}

void MainWindow::on_m_stress_triggered()
{
    off_ct = true;
    if (ui->m_stress->isChecked())
        on_m_proccessing_triggered();
    else
    {
        QTextCursor cur(ui->textEdit->document());
        QString word;
        QChar end_of_buf;
        QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?";
        fake_text = ui->textEdit->toHtml();
        progress->setMaximum(ui->textEdit->toPlainText().length());
        progress->setValue(0);
        while (cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor))
        {
            progress->setValue(progress->value()+1);
            word = cur.selectedText();
            end_of_buf = word[word.length()-1];
            if (wrong_char.contains(end_of_buf) || end_of_buf.unicode()==8233)
            {
                QTextCursor cur2 (ui->textEdit->document());
                cur2.setPosition(cur.position());
                cur2.movePosition(QTextCursor::PreviousCharacter);
                if (cur2.charFormat().foreground().color()!=QColor("red") && cur2.charFormat().foreground().color()!=QColor("blue"))
                {
                    cur.clearSelection();
                    word.clear();
                    continue;
                }
                word = cur.selectedText();
                cur.removeSelectedText();
                cur.insertText(word,QTextCharFormat());
                cur.clearSelection();
            }
        }
        if (!cur.selectedText().isEmpty())
        {
            QTextCursor cur2 (ui->textEdit->document());
            if (cur2.charFormat().foreground().color()==QColor("red") || cur2.charFormat().foreground().color()==QColor("blue"))
            {
                word = cur.selectedText();
                cur.removeSelectedText();
                cur.insertText(word,QTextCharFormat());
            }
        }
        tpl = ui->textEdit->toPlainText().length();
        refresh_ur(false);
        oper_stack.insert(oper_stack.begin(),1,
                  oper_info(1,1,1,
                  fake_text,ui->textEdit->toHtml()  ));
        fake_text = ui->textEdit->toHtml();
    }
    off_ct = false;
}


QList<QStringList> dbuniq::shingles(QString text,int shi_count)
{
    ui->w_progress->setValue(0);
    ui->w_progress->setMaximum(text.length());
    QString::iterator iter = text.begin();
    QString wrong_char = "?!., ;:\'\"\\/+><@#~`$%^&*(){}|\n\r\t\a\?-1234567890";
    QString word;
    QList<QStringList> list;
    QStringList slist;
    while (iter!=text.end())
    {
        if (wrong_char.indexOf(*iter)!=-1)
        {
            if (word.length()>3)
                if (slist.length()>shi_count-1)
                {
                    list<<slist;
                    slist.clear();
                    slist<<word;
                }
                else
                    slist<<word;
            word.clear();
        }
        else
            word += *(iter);
        ui->w_progress->setValue(ui->w_progress->value()+1);
        iter++;
    }
    if (!word.isEmpty() && word.length()>3)
        if (slist.length()>shi_count-1)
        {
            list<<slist;
            slist.clear();
            slist<<word;
        }
        else
            slist<<word;
    if (!slist.isEmpty())
        list<<slist;
    return list;
}
