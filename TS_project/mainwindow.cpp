#include "mainwindow.h"
#include <QKeyEvent>
#include <QPushButton>
#include "WF_project/wf_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),

    count_mas(NULL), count_mas2(NULL), m_k(NULL), words_count(0), tpl(0), hand_cur(0),sort_order(0),
    pr_name("Новый проект"),
    off_cch(false), is_pr_saved(true), show_results(false), is_job_done(false), off_ct(false), m_is_html_pressed(false),
    db_dialog(new dbconfig(&is_db_connected)), dbuniq_dialog(new dbuniq), db_add_dialog(new dbadd)
{
    ui->setupUi(this);
    setWindowTitle("Новый проект");
    ui->centralWidget->setLayout(ui->horizontalLayout);
    ui->toolButton->setStyleSheet("QToolButton {background: #19B4E9; }");
    ui->m_show_results->setEnabled(false);
    ui->m_show_task->setEnabled(false);
    ui->m_stress->setChecked(true);
    ui->m_add_rus->setChecked(true);

    ui->tableWidget->setColumnCount(4);
    QStringList header_list;
    header_list.append(tr("Точное вхождение"));
    header_list.append(tr("Мин.кол.точных вх."));
    header_list.append(tr("Мин.кол.вх.словоформ"));
    header_list.append(tr("Словоформа 1"));
    ui->tableWidget->setHorizontalHeaderLabels(header_list);
    for (int col =0; col<ui->tableWidget->columnCount(); col++)
        ui->tableWidget->resizeColumnToContents(col);
    connect(ui->tableWidget->horizontalHeader(),
            SIGNAL(sectionDoubleClicked(int)),
            this,
            SLOT(table_sort(int)));
    ui->tableWidget->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget->horizontalHeader(),
            SIGNAL(customContextMenuRequested(QPoint)),
            this,
            SLOT(table_menu_event_col())    );
    ui->tableWidget->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget->verticalHeader(),
            SIGNAL(customContextMenuRequested(QPoint)),
            this,
            SLOT(table_menu_event_row())    );

    table_menu = new QMenu(ui->tableWidget);
    act_add_word = new QAction("Добавить слово",this);
    act_add_row = new QAction("Добавить строку",this);
    act_delete_row = new QAction("Удалить строку",this);
    act_add_col = new QAction("Добавить столбец",this);
    act_delete_col = new QAction("Удалить столбец",this);
    connect (act_add_word,SIGNAL(triggered()),this,SLOT(add_word()));
    connect (act_add_row,SIGNAL(triggered()),this,SLOT(add_row()));
    connect (act_delete_row,SIGNAL(triggered()),this,SLOT(delete_row()));
    connect (act_add_col,SIGNAL(triggered()),this,SLOT(add_col()));
    connect (act_delete_col,SIGNAL(triggered()),this,SLOT(delete_col()));
    table_menu->addAction(act_add_word);
    table_menu->addAction(act_add_row);
    table_menu->addAction(act_delete_row);
    table_menu->addAction(act_add_col);
    table_menu->addAction(act_delete_col);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(table_menu_event(const QPoint &)));

    ui->textEdit->setFocus();

    barl = new QLabel("0 символов, 0 без пробелов");
    this->statusBar()->addWidget(barl);

    mtb = new QToolBar();
    bu_1 = new QToolButton();              bu_2 = new QToolButton();
    bu_bold = new QToolButton();           bu_italic = new QToolButton();
    bu_underline = new QToolButton();      bu_alignment = new QToolButton();
    bu_alignment_left = new QToolButton(); bu_alignment_right = new QToolButton();
    bu_html = new QToolButton();           bu_db_uniqueness = new QToolButton();
    bu_db_connect = new QToolButton();     bu_spinbox = new QSpinBox();
    bu_db_add_dialog = new QToolButton();

    bu_1->setEnabled(false);
    bu_2->setEnabled(false);

    bu_1->setIcon(QIcon(":/main/res/Undo.png"));
    bu_2->setIcon(QIcon(":/main/res/Redo.png"));
    bu_bold->setIcon(QIcon(":/main/res/Bold.png"));
    bu_italic->setIcon(QIcon(":/main/res/Italic.png"));
    bu_underline->setIcon(QIcon(":/main/res/Underline.png"));
    bu_alignment->setIcon(QIcon(":/main/res/Alignment_text.png"));
    bu_alignment_left->setIcon(QIcon(":/main/res/Alignment_text_left.png"));
    bu_alignment_right->setIcon(QIcon(":/main/res/Alignment_text_right.png"));
    bu_html->setIcon(QIcon(":/main/res/html.png"));
    bu_db_uniqueness->setIcon(QIcon(":/main/res/uniqueness.png"));
    bu_db_connect->setIcon(QIcon(":/main/res/db_closed.png"));
    bu_db_add_dialog->setIcon(QIcon(":/main/res/db_add.png"));

    connect(bu_1,SIGNAL(clicked()),this,SLOT(do_undo()));
    connect(bu_2,SIGNAL(clicked()),this,SLOT(do_redo()));
    connect(bu_bold,SIGNAL(clicked()),this,SLOT(do_bold()));
    connect(bu_italic,SIGNAL(clicked()),this,SLOT(do_italic()));
    connect(bu_underline,SIGNAL(clicked()),this,SLOT(do_underline()));
    connect(bu_alignment,SIGNAL(clicked()),this,SLOT(do_align()));
    connect(bu_alignment_left,SIGNAL(clicked()),this,SLOT(do_align_left()));
    connect(bu_alignment_right,SIGNAL(clicked()),this,SLOT(do_align_right()));
    connect(bu_spinbox,SIGNAL(editingFinished()),this,SLOT(do_spinbox()));
    connect(bu_html,SIGNAL(clicked()),this,SLOT(do_html()));
    connect(db_dialog,&QDialog::finished,[=]()
    {
        if (is_db_connected)
        {
            mtb->actions().at(15)->setVisible(true);
            mtb->actions().at(16)->setVisible(true);
            bu_db_connect->setIcon(QIcon(":/main/res/db_opened.png"));
        }
        else
        {
            mtb->actions().at(15)->setVisible(false);
            mtb->actions().at(16)->setVisible(false);
            bu_db_connect->setIcon(QIcon(":/main/res/db_closed.png"));
        }
    } );

    connect(dbuniq_dialog,SIGNAL(finished(int)),this,SLOT(paint_shingles()));
    connect(bu_db_connect, &QPushButton::clicked,[=] () {db_dialog->show();} );

    connect(bu_db_uniqueness, &QPushButton::clicked,[=] ()
    {
        dbuniq_dialog->uniq_text=ui->textEdit->toPlainText();
        dbuniq_dialog->show();
    } );

    connect(bu_db_add_dialog, &QPushButton::clicked,[=] ()
    {
        db_add_dialog->main_text=ui->textEdit->toPlainText();
        db_add_dialog->show();
    } );

    mtb->addWidget(bu_1);           mtb->addWidget(bu_2);
    mtb->addSeparator();            mtb->addWidget(bu_bold);
    mtb->addWidget(bu_italic);      mtb->addWidget(bu_underline);
    mtb->addSeparator();            mtb->addWidget(bu_alignment_left);
    mtb->addWidget(bu_alignment);   mtb->addWidget(bu_alignment_right);
    mtb->addWidget(bu_spinbox);     mtb->addSeparator();
    mtb->addWidget(bu_html);        mtb->addSeparator();
    mtb->addWidget(bu_db_connect);  mtb->addWidget(bu_db_uniqueness);
    mtb->addWidget(bu_db_add_dialog);

    mtb->setMovable(false);
    mtb->actions().at(15)->setVisible(false);
    this->addToolBar(mtb);

    clear_staks();
    progress = new QProgressBar;
    progress->setValue(0);
    this->statusBar()->addWidget(progress);

    QList<int> sizes;
    sizes.append(ui->textEdit->width()*2);
    sizes.append(ui->textEdit->width()/4);
    ui->splitter->setSizes(sizes);
    connect(ui->textEdit, &QTextEdit::selectionChanged, [=]()
    {
        if (ui->textEdit->textCursor().selectedText().length())
        hand_text = ui->textEdit->textCursor().selectedText();
    });
    connect(ui->textEdit, &QTextEdit::cursorPositionChanged, [=]()
    {
        if (tpl == ui->textEdit->toPlainText().length())
            hand_cur=ui->textEdit->textCursor().position();
        if (ui->textEdit->textCursor().position()<ui->textEdit->toPlainText().length() && ui->textEdit->textCursor().position()>0 &&
                tpl-1!=ui->textEdit->toPlainText().length())
        {
            fake_chars.first = ui->textEdit->toPlainText().at(ui->textEdit->textCursor().position()-1);
            fake_chars.second = ui->textEdit->toPlainText().at(ui->textEdit->textCursor().position());
        }
    });

    QToolBar* main_bar = new QToolBar;
    main_bar->setOrientation(Qt::Vertical);
    QToolButton* WF_b = new QToolButton;
    WF_b->setIcon(QIcon(":/main/res/WF_main.png"));
    main_bar->addWidget(WF_b);
    QToolButton* auth_b = new QToolButton;
    auth_b->setIcon(QIcon(":/main/res/auth.png"));
    main_bar->addWidget(auth_b);

    connect(WF_b,SIGNAL(clicked()),this,SLOT(switch_to_pr()));

    ui->horizontalLayout->insertWidget(0,main_bar);
    on_display_horiz_triggered();
    LoadSettings();
    db_dialog->finished(0);
}

void MainWindow::switch_to_pr()
{
    wf_mainwindow* pr = new wf_mainwindow();
    pr->showMaximized();
    close();
}

void MainWindow::do_bold()
{
    QTextCharFormat charisic = ui->textEdit->textCursor().charFormat();
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(8,
                      ui->textEdit->textCursor().position(),0,
                      ui->textEdit->textCursor().selectedText(),
                      "", charisic));
    if (charisic.fontWeight()==QFont::Normal)
        charisic.setFontWeight(QFont::Bold);
    else
        charisic.setFontWeight(QFont::Normal);
    ui->textEdit->textCursor().mergeCharFormat(charisic);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_italic()
{
    QTextCharFormat charisic = ui->textEdit->textCursor().charFormat();
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(8,
                      ui->textEdit->textCursor().position(),1,
                      ui->textEdit->textCursor().selectedText(),
                      "", charisic));
    if (charisic.fontItalic())
        charisic.setFontItalic(false);
    else
        charisic.setFontItalic(true);
    ui->textEdit->textCursor().mergeCharFormat(charisic);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_underline()
{
    QTextCharFormat charisic = ui->textEdit->textCursor().charFormat();
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(8,
                      ui->textEdit->textCursor().position(),2,
                      ui->textEdit->textCursor().selectedText(),
                      "", charisic));
    if (charisic.fontUnderline())
        charisic.setFontUnderline(false);
    else
        charisic.setFontUnderline(true);
    ui->textEdit->textCursor().mergeCharFormat(charisic);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_align()
{
    QTextBlockFormat textBlockFormat = ui->textEdit->textCursor().blockFormat();
    if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignLeft)
        textBlockFormat.setAlignment(Qt::AlignLeft);
    else if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignHCenter)
        textBlockFormat.setAlignment(Qt::AlignHCenter);
    else
        textBlockFormat.setAlignment(Qt::AlignRight);
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(9,
                      ui->textEdit->textCursor().position(),1,
                      "","",QTextCharFormat(),textBlockFormat));
    if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignHCenter)
        textBlockFormat.setAlignment(Qt::AlignLeft);
    else
        textBlockFormat.setAlignment(Qt::AlignHCenter);
    ui->textEdit->textCursor().mergeBlockFormat(textBlockFormat);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_align_left()
{
    QTextBlockFormat textBlockFormat = ui->textEdit->textCursor().blockFormat();
    if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignLeft)
        textBlockFormat.setAlignment(Qt::AlignLeft);
    else if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignHCenter)
        textBlockFormat.setAlignment(Qt::AlignHCenter);
    else
        textBlockFormat.setAlignment(Qt::AlignRight);
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(9,
                      ui->textEdit->textCursor().position(),2,
                      "","",QTextCharFormat(),textBlockFormat));
    textBlockFormat.setAlignment(Qt::AlignLeft);
    ui->textEdit->textCursor().mergeBlockFormat(textBlockFormat);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_align_right()
{
    QTextBlockFormat textBlockFormat = ui->textEdit->textCursor().blockFormat();
    if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignLeft)
        textBlockFormat.setAlignment(Qt::AlignLeft);
    else if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignHCenter)
        textBlockFormat.setAlignment(Qt::AlignHCenter);
    else
        textBlockFormat.setAlignment(Qt::AlignRight);
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(9,
                      ui->textEdit->textCursor().position(),3,
                      "","",QTextCharFormat(),textBlockFormat));
    if (ui->textEdit->textCursor().blockFormat().alignment()==Qt::AlignRight)
        textBlockFormat.setAlignment(Qt::AlignLeft);
    else
        textBlockFormat.setAlignment(Qt::AlignRight);
    ui->textEdit->textCursor().mergeBlockFormat(textBlockFormat);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_spinbox()
{
    QTextCharFormat charisic = ui->textEdit->textCursor().charFormat();
    if (charisic.fontPointSize()==0)
        charisic.setFontPointSize(12);
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(8,
                      ui->textEdit->textCursor().position(),3,
                      ui->textEdit->textCursor().selectedText(),
                      "", charisic));
    charisic.setFontPointSize(bu_spinbox->value());
    ui->textEdit->textCursor().mergeCharFormat(charisic);
    fake_text = ui->textEdit->toHtml();
}

void MainWindow::do_html()
{
    QString old_text = ui->textEdit->toHtml();
    off_ct = true;
    if (m_is_html_pressed)
    {
        bu_html->setDown(false);
        m_is_html_pressed = false;
        ui->textEdit->setText(m_html_fake_text);
    }
    else
    {
        bu_html->setDown(true);
        m_is_html_pressed = true;
        m_html_fake_text = ui->textEdit->toHtml();
        QTextCursor cur (ui->textEdit->document());
        cur.setPosition(0);
        QChar header[] = {'1','2','2','3','3','3','4','4','4','4','5','5','5','5','5','6'};
        int cur_header = 0;
        QTextBlockFormat tbf;
        QString adjust;
        QTextCharFormat format;
        QTextBlockFormat blockformat;
        bool list = false;
        tbf = cur.blockFormat();
        progress->setValue(0);
        progress->setMaximum(ui->textEdit->document()->blockCount());
        while (cur.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor))
        {
            progress->setValue(progress->value()+1);
            adjust.clear();
            if (list)
            {
                if (cur.selectedText().left(3).contains('-'))
                    adjust += "<li>" + cur.selectedText().left(cur.selectedText().length()-1) + "</li>\n";
                else
                {
                    adjust += "</ul>\n";
                    list = false;
                }
            }
            if (tbf.alignment()==Qt::AlignHCenter)
            {
                adjust += QString ("<h%1>%2</h%1>\n").arg(header[cur_header]).arg(cur.selectedText().left(cur.selectedText().length()-1));
                if (cur_header!=15)
                    cur_header++;
            }
            else if(!list)
            {
                adjust += "<p>" + cur.selectedText().left(cur.selectedText().length()-1) + "</p>\n";
                if (cur.selectedText().right(8).contains(':'))
                {
                    list = true;
                    adjust += "<ul>\n";
                }
            }
            tbf = cur.blockFormat();
            cur.removeSelectedText();
            cur.setBlockFormat(blockformat);
            cur.insertText(adjust,format);
        }
        progress->setValue(progress->value()+1);
        cur.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        if (list && cur.selectedText().left(3).contains('-'))
            adjust = "<li>" + cur.selectedText().left(cur.selectedText().length()-1) + "</li>\n</ui>";
        else
            if (tbf.alignment()==Qt::AlignHCenter)
                adjust += QString ("<h%1>%2</h%1>").arg(header[cur_header]).arg(cur.selectedText().left(cur.selectedText().length()-1));
            else
                adjust = "<p>" + cur.selectedText().left(cur.selectedText().length()-1) + "</p>";
        cur.removeSelectedText();
        cur.setBlockFormat(blockformat);
        cur.insertText(adjust,format);
        ui->textEdit->setPlainText(ui->textEdit->toPlainText().replace(QString("<p></p>"),QString("")));
        tpl = ui->textEdit->toPlainText().length();
        barl->setText(QString("%1 символов, %2 без пробелов").arg(tpl).arg(tpl - ui->textEdit->toPlainText().count(' ')));
        off_ct = false;
    }
    refresh_ur(false);
    oper_stack.insert(oper_stack.begin(),1,
                      oper_info(10,0,0,
                      old_text,ui->textEdit->toHtml()));
}

void MainWindow::do_undo()
{
    off_cch = true;
    off_ct = true;
    if (show_results)
        table_show_task();
    off_pr();
    bu_2->setEnabled(true);
    switch (oper_stack[oper_iter].ind)
    {
    case 1:
    {
        QTextCursor cur (ui->textEdit->document());
        cur.setPosition(oper_stack[oper_iter].row);
        switch (oper_stack[oper_iter].col)
        {
        case 0:
            cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
            cur.removeSelectedText();
            break;
        case 1:
            ui->textEdit->clear();
            cur.insertHtml(oper_stack[oper_iter].word);
            break;
        case 2:
            cur.insertText(oper_stack[oper_iter].word, QTextCharFormat ());
            break;
        default:
            break;
        }
        tpl = ui->textEdit->toPlainText().length();
        barl->setText(QString::number(tpl) + tr(" символов, ") + QString::number(tpl-ui->textEdit->toPlainText().count(' ')) + tr(" без пробелов"));
        break;
    }
    case 2:
    {
        if (ui->tableWidget->item(oper_stack[oper_iter].row,oper_stack[oper_iter].col))
            fake_memory = ui->tableWidget->item(oper_stack[oper_iter].row,oper_stack[oper_iter].col)->text();
        else
            fake_memory.clear();
        ui->tableWidget->setItem(oper_stack[oper_iter].row,oper_stack[oper_iter].col,new QTableWidgetItem(cells_memory[memory_iter]));
        cells_memory[memory_iter] = fake_memory;
        memory_iter++;
        break;
    }
    case 3:
    {
        ui->tableWidget->removeRow(oper_stack[oper_iter].row);
        break;
    }
    case 4:
    {
        ui->tableWidget->insertRow(oper_stack[oper_iter].row-1);
        for (int i=0; i<ui->tableWidget->columnCount();i++)
            ui->tableWidget->setItem(oper_stack[oper_iter].row-1,i,new QTableWidgetItem(oper_stack[oper_iter].str_list.at(i)));
        break;
    }
    case 5:
    {
        ui->tableWidget->removeColumn(oper_stack[oper_iter].row+1);
        break;
    }
    case 6:
    {
        ui->tableWidget->insertColumn(oper_stack[oper_iter].row);
        for (int i=0; i<ui->tableWidget->rowCount();i++)
            ui->tableWidget->setItem(i,oper_stack[oper_iter].row,new QTableWidgetItem(oper_stack[oper_iter].str_list.at(i)));
        for (int i=4; i<ui->tableWidget->columnCount();i++)
        {
            ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(tr("Словоформа ") + QString::number(i-2)));
            ui->tableWidget->resizeColumnToContents(i);
        }
        break;
    }
    case 7:
    {
        QTextCursor cur(ui->textEdit->document());
        cur.setPosition(oper_stack[oper_iter].col);
        tpl = ui->textEdit->toPlainText().length();
        for (int i=0; i<oper_stack[oper_iter].row; i++)
        {
            tpl--;
            cur.deleteChar();
        }
        break;
    }
    case 8:
    {
        QTextCursor cur (ui->textEdit->textCursor());
        cur.setPosition(oper_stack[oper_iter].row);
        if (! ((cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,oper_stack[oper_iter].word.length())) && (cur.selectedText()==oper_stack[oper_iter].word)) )
        {
            cur.setPosition(oper_stack[oper_iter].row);
            cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,oper_stack[oper_iter].word.length());
        }
        ui->textEdit->setTextCursor(cur);
        QTextCharFormat charisic = ui->textEdit->textCursor().charFormat();
        switch (oper_stack[oper_iter].col)
        {
        case 0:
        {
            charisic.setFontWeight(oper_stack[oper_iter].format.fontWeight());
            break;
        }
        case 1:
        {
            charisic.setFontItalic(oper_stack[oper_iter].format.fontItalic());
            break;
        }
        case 2:
        {
            charisic.setFontUnderline(oper_stack[oper_iter].format.fontUnderline());
            break;
        }
        case 3:
        {
            charisic.setFontPointSize(oper_stack[oper_iter].format.fontPointSize());
            break;
        }
        }
        ui->textEdit->textCursor().mergeCharFormat(charisic);
        break;
    }
    case 9:
    {
        ui->textEdit->textCursor().setPosition(oper_stack[oper_iter].row);
        QTextCursor cur(ui->textEdit->document());
        cur.setPosition(oper_stack[oper_iter].row);
        ui->textEdit->setTextCursor(cur);
        ui->textEdit->textCursor().mergeBlockFormat(oper_stack[oper_iter].block_format);
        break;
    }
    case 10:
    {
        ui->textEdit->setText(oper_stack[oper_iter].word);
        if (m_is_html_pressed)
        {
            bu_html->setDown(false);
            m_is_html_pressed = false;
        }
        else
        {
            bu_html->setDown(true);
            m_is_html_pressed = true;
        }
        break;
    }
    }
    oper_iter++;
    off_cch = false;
    off_ct = false;
    if (oper_iter>oper_stack.size()-1)
        bu_1->setEnabled(false);
}

void MainWindow::do_redo()
{
    off_cch = true;
    off_ct = true;
    oper_iter--;
    if (show_results)
        table_show_task();
    off_pr();
    switch (oper_stack[oper_iter].ind)
    {
    case 1:
    {
        QTextCursor cur (ui->textEdit->document());
        cur.setPosition(oper_stack[oper_iter].row-1);
        switch (oper_stack[oper_iter].col)
        {
        case 0:
            cur.insertText(oper_stack[oper_iter].word, QTextCharFormat ());
            break;
        case 1:
            ui->textEdit->clear();
            cur.insertHtml(oper_stack[oper_iter].new_word);
            break;
        case 2:
            cur.setPosition(oper_stack[oper_iter].row+1);
            cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
            cur.removeSelectedText();
            break;
        }
        tpl = ui->textEdit->toPlainText().length();
        barl->setText(QString::number(tpl) + tr(" символов, ") + QString::number(tpl-ui->textEdit->toPlainText().count(' ')) + tr(" без пробелов"));
        break;
    }
    case 2:
    {
        memory_iter--;
        if (ui->tableWidget->item(oper_stack[oper_iter].row,oper_stack[oper_iter].col))
            fake_memory = ui->tableWidget->item(oper_stack[oper_iter].row,oper_stack[oper_iter].col)->text();
        else
            fake_memory.clear();
        ui->tableWidget->setItem(oper_stack[oper_iter].row,oper_stack[oper_iter].col,new QTableWidgetItem(cells_memory[memory_iter]));
        cells_memory[memory_iter] = fake_memory;
        break;
    }
    case 3:
    {
        ui->tableWidget->insertRow(oper_stack[oper_iter].row);
        break;
    }
    case 4:
    {
        ui->tableWidget->removeRow(oper_stack[oper_iter].row-1);
        break;
    }
    case 5:
    {
        ui->tableWidget->insertColumn(oper_stack[oper_iter].row+1);
        for (int i=4; i<ui->tableWidget->columnCount();i++)
            ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(tr("Словоформа ") + QString::number(i-2)));
        ui->tableWidget->resizeColumnToContents(oper_stack[oper_iter].row+1);
        break;
    }
    case 6:
    {
        ui->tableWidget->removeColumn(oper_stack[oper_iter].row);
        for (int i=4; i<ui->tableWidget->columnCount();i++)
        {
            ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(tr("Словоформа ") + QString::number(i-2)));
            ui->tableWidget->resizeColumnToContents(i);
        }
        break;
    }
    case 7:
    {
        QTextCursor cur(ui->textEdit->document());
        cur.setPosition(oper_stack[oper_iter].col);
        if (cur.position()!=0)
        {
            if (cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor))
            {
                cur.movePosition(QTextCursor::NextCharacter);
                if (cur.selectedText() != " ")
                {
                    tpl++;
                    cur.insertText(" ");
                }
            }
        }
        cur.insertText(oper_stack[oper_iter].word);
        break;
    }
    case 8:
    {
        QTextCursor cur (ui->textEdit->textCursor());
        cur.setPosition(oper_stack[oper_iter].row);
        if (! ((cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,oper_stack[oper_iter].word.length())) && (cur.selectedText()==oper_stack[oper_iter].word)) )
        {
            cur.setPosition(oper_stack[oper_iter].row);
            cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,oper_stack[oper_iter].word.length());
        }
        ui->textEdit->setTextCursor(cur);
        QTextCharFormat charisic = ui->textEdit->textCursor().charFormat();
        switch (oper_stack[oper_iter].col)
        {
        case 0:
        {
            if (charisic.fontWeight()!=QFont::Bold)
                charisic.setFontWeight(QFont::Bold);
            else
                charisic.setFontWeight(QFont::Normal);
            //charisic.setFontWeight(oper_stack[oper_iter].format.fontWeight());
            break;
        }
        case 1:
        {
            if (charisic.fontItalic())
                charisic.setFontItalic(false);
            else
                charisic.setFontItalic(true);
            break;
        }
        case 2:
        {
            if (charisic.fontUnderline())
                charisic.setFontUnderline(false);
            else
                charisic.setFontUnderline(true);
            break;
        }
        case 3:
        {
            charisic.setFontPointSize(oper_stack[oper_iter].format.fontPointSize());
            break;
        }
        }
        ui->textEdit->textCursor().mergeCharFormat(charisic);
        break;
    }
    case 9:
    {
        QTextBlockFormat textBlockFormat = ui->textEdit->textCursor().blockFormat();
        QTextCursor cur (ui->textEdit->document());
        cur.setPosition(oper_stack[oper_iter].row);
        switch (oper_stack[oper_iter].col)
        {
        case 1:
        {
            if (cur.blockFormat().alignment()==Qt::AlignHCenter)
                textBlockFormat.setAlignment(Qt::AlignLeft);
            else
                textBlockFormat.setAlignment(Qt::AlignHCenter);
            break;
        }
        case 2:
        {
            if (cur.blockFormat().alignment()==Qt::AlignRight)
                textBlockFormat.setAlignment(Qt::AlignLeft);
            else
                textBlockFormat.setAlignment(Qt::AlignRight);
            break;
        }
        case 3:
        {
            textBlockFormat.setAlignment(Qt::AlignLeft);
            break;
        }
        }
        ui->textEdit->setTextCursor(cur);
        ui->textEdit->textCursor().mergeBlockFormat(textBlockFormat);
    }
    case 10:
    {
        if (m_is_html_pressed)
        {
            bu_html->setDown(false);
            m_is_html_pressed = false;
        }
        else
        {
            bu_html->setDown(true);
            m_is_html_pressed = true;
        }
        ui->textEdit->setText(oper_stack[oper_iter].new_word);
        break;
    }
    }
    off_cch = false;
    off_ct = false;
    bu_1->setEnabled(true);
    if (oper_iter==0)
        bu_2->setEnabled(false);
}

MainWindow::~MainWindow()
{
    SaveSettings();
    delete_task_data();
    delete ui;
}

void MainWindow::delete_task_data()
{
    if (count_mas==NULL)
        return;
    delete []count_mas;
    delete []count_mas2;
    count_mas = NULL;
    for (unsigned int i=0; i<words_count; i++)
    {
        if ((m_k[i])->head!=NULL)
            while (1)
            {
                low_task* fiter = (m_k[i])->head;
                low_task* help_fiter;
                while (fiter->next)
                {
                    if (!fiter->next->next)
                        help_fiter = fiter;
                    fiter=fiter->next;
                }
                if (fiter == (m_k[i])->head)
                {
                    delete fiter;
                    (m_k[i])->head = NULL;
                    break;
                }
                help_fiter->next =NULL;
                delete fiter;
            }
    }
    words_count=0;
    delete []m_k;
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (ui->tableWidget->hasFocus())
        if (e->key()==Qt::Key_Down || e->key()==Qt::Key_Up || e->key()==Qt::Key_Left || e->key()==Qt::Key_Right || e->key()==Qt::Key_Tab)
            if (show_results)
                return;
            else
                if (ui->tableWidget->item(ui->tableWidget->currentRow(),ui->tableWidget->currentColumn()))
                    fake_memory = ui->tableWidget->item(ui->tableWidget->currentRow(),ui->tableWidget->currentColumn())->text();
                else
                    fake_memory.clear();
    if (ui->textEdit->hasFocus())
    {
        if(e->modifiers() & Qt::ControlModifier && e->nativeVirtualKey()==1747 )
            ui->textEdit->copy();
        if(e->modifiers() & Qt::ControlModifier && e->nativeVirtualKey()==1758 )
            ui->textEdit->cut();
        if(e->modifiers() & Qt::ControlModifier && e->nativeVirtualKey()==1741)
            ui->textEdit->paste();
        if(e->modifiers() & Qt::ControlModifier && e->nativeVirtualKey()==1734)
            ui->textEdit->selectAll();
    }
}

void MainWindow::off_pr(bool changed)
{
    is_job_done = false;
    progress->setValue(0);
    show_results = false;
    ui->m_show_results->setEnabled(false);
    ui->m_show_task->setEnabled(false);
    ui->m_show_results->setChecked(false);
    ui->m_show_task->setChecked(false);
    is_pr_saved = !changed;
    if (!changed)
        ui->m_Save_pr_as->setEnabled(true);
    ui->m_proccessing->setChecked(!changed);
    if (changed)
        setWindowTitle(pr_name+'*');
    else
    {
        setWindowTitle(pr_name);
        clear_staks();
    }
}

void MainWindow::refresh_ur(bool make_off)
{
    if (make_off)
        off_pr();
    oper_stack.erase(oper_stack.begin(),oper_stack.begin()+oper_iter);
    cells_memory.erase(cells_memory.begin(),cells_memory.begin()+memory_iter);
    oper_iter=0;
    memory_iter=0;
    bu_1->setEnabled(true);
    bu_2->setEnabled(false);
    ui->m_Save_pr_as->setEnabled(true);
}

void MainWindow::clear_staks()
{
    oper_stack.clear();
    cells_memory.clear();
    oper_iter=0;
    memory_iter=0;
    bu_1->setEnabled(false);
    bu_2->setEnabled(false);
}

void MainWindow::on_splitter_splitterMoved(int, int)
{
    if (ui->splitter->orientation()==Qt::Horizontal && ui->tableWidget->width()==0)
        ui->toolButton->setArrowType(Qt::LeftArrow);
    else
        if (ui->splitter->orientation()==Qt::Horizontal)
            ui->toolButton->setArrowType(Qt::RightArrow);
        else
            if (ui->splitter->orientation()==Qt::Vertical && ui->tableWidget->height()==0)
                ui->toolButton->setArrowType(Qt::UpArrow);
            else
                ui->toolButton->setArrowType(Qt::DownArrow);
}
