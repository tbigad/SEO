#include "wf_mainwindow.h"
#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QLineEdit>

wf_mainwindow::wf_mainwindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::wf_mainwindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->horizontalLayoutWidget);
    tab_counter = 0;
    tab_counteres<<0<<0<<0<<0;
    tree_lists<<QList<tree_item>()<<QList<tree_item>()<<QList<tree_item>()<<QList<tree_item>();
    current_tabs<<new QTreeWidgetItem<<new QTreeWidgetItem<<new QTreeWidgetItem<<new QTreeWidgetItem;
    first_toggle = true;

    QMenu* ei_menu = new QMenu();   QAction* act;
    act = new QAction(QIcon(":/main/res/excel.png"),"Импортировать с excel",this);   ei_menu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ImportFromExcel()));
    act = new QAction(QIcon(":/main/res/csv.png"),"Импортировать с csv",this);       ei_menu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ImportFromCSV()));
    act = new QAction(QIcon(":/main/res/txt.png"),"Импортировать с txt",this);       ei_menu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ImportFromTXT()));
    ei_menu->addSeparator();
    act = new QAction(QIcon(":/main/res/excel.png"),"Экспортировать в excel",this);  ei_menu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ExportToExcel()));
    act = new QAction(QIcon(":/main/res/csv.png"),"Экспортировать в csv",this);      ei_menu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ExportToCSV()));
    act = new QAction(QIcon(":/main/res/txt.png"),"Экспортировать в txt",this);      ei_menu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ExportToTXT()));

    QMenu* ei_wordmenu = new QMenu();
    act = new QAction(QIcon(":/main/res/excel.png"),"Импортировать с excel",this);   ei_wordmenu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ImportWordsFromExcel()));
    act = new QAction(QIcon(":/main/res/csv.png"),"Импортировать с csv",this);       ei_wordmenu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ImportWordsFromCSV()));
    act = new QAction(QIcon(":/main/res/txt.png"),"Импортировать с txt",this);       ei_wordmenu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ImportWordsFromTXT()));
    ei_menu->addSeparator();
    act = new QAction(QIcon(":/main/res/excel.png"),"Экспортировать в excel",this);  ei_wordmenu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ExportWordsToExcel()));
    act = new QAction(QIcon(":/main/res/csv.png"),"Экспортировать в csv",this);      ei_wordmenu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ExportWordsToCSV()));
    act = new QAction(QIcon(":/main/res/txt.png"),"Экспортировать в txt",this);      ei_wordmenu->addAction(act);
    connect (act, SIGNAL(triggered()),this,SLOT(ExportWordsToTXT()));

    QList<iModel*> models;
    QList<iItem*> items;
    QList<MyView*> views;
    for (int i=0;i<5;i++)
    {
        QVBoxLayout* box = new QVBoxLayout;
        MyView* tv = new MyView(static_cast<mViewType>(5-i));
        iItem* item = new iItem;    iModel* model = new iModel;
        model->page_index = 4-i;
        tv->setModel(model);
        tv->setItemDelegate(item);
        models.append(model);   items.append(item); views.append(tv);
        connect(model,SIGNAL(RepaintIndex(const QModelIndex&)),tv,SLOT(update(QModelIndex)));
        connect(tv,SIGNAL(UpdateStates(int,int)),this,SLOT(UpdateStats(int,int)));
        connect(tv,&MyView::ClearTable,[=](mViewType to)
        {
            if (static_cast<int>(to)-1!=NEGATIVE_TABLE)
                GetTV2Label(static_cast<int>(to)-1)->setText("0 / 0 (100%)");
            else
                GetTV2Label(static_cast<int>(to)-1)->setText("0 / 0");
        });
        connect (tv,&MyView::FilterByWord, [=](const QString word)
        {   GetTVLine(ui->tabWidget->currentIndex())->setText(word);    });
        tv->resizeColumnsToContents();

        connect(tv,&MyView::ReloadFilter,[=]()
        {
            QString tmp = GetTV2Line(ui->tabWidget_2->currentIndex())->text();
            GetTV2Line(ui->tabWidget_2->currentIndex())->setText("");
            GetTV2Line(ui->tabWidget_2->currentIndex())->setText(tmp);
        });

        connect(item,&iItem::commitData,[=](QWidget*)
        { tv->ResizeContext(500); });
        connect(item,&iItem::WordWasEdited,[=](const QString& word, const int row)
        {
            int count=0;
            for (int i=0; i<GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount(); i++)
                if (((tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model())->Words(i)->contains(word))
                    count++;
            model->setData(model->index(row,3),count,Qt::EditRole);
        });

        QToolBar* bar = new QToolBar;   box->addWidget(bar);    box->addWidget(tv);

        QToolButton* b = new QToolButton;   bar->addWidget(b);
        b->setStyleSheet("QToolButton {background: url(:/main/res/Import-Export.png) top center no-repeat;}");
        b->setFixedWidth(79); b->setFixedHeight(30);
        connect(b,&QToolButton::clicked,[=]() { ei_wordmenu->popup(QCursor::pos()); });

        b = new QToolButton;    bar->addWidget(b);
        b->setStyleSheet("QToolButton {background: url(:/main/res/plus.png) top center no-repeat;}");
        b->setFixedWidth(79); b->setFixedHeight(30);
        /// #############################################################################
        /// ДОБАВЛЕНИЕ МАЛЕНЬКИХ СЛОВ ВРУЧНУЮ
        /// #############################################################################
        connect(b, &QToolButton::clicked, [=]()
        {
            QDialog* dialog = new QDialog;
            QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel ,dialog);
            connect(buttons->button(QDialogButtonBox::Ok),SIGNAL(clicked()),dialog,SLOT(accept()));
            buttons->button(QDialogButtonBox::Ok)->setText("Добавить");
            connect(buttons->button(QDialogButtonBox::Cancel),SIGNAL(clicked()),dialog,SLOT(reject()));
            buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");
            QPlainTextEdit* plain = new QPlainTextEdit;
            QVBoxLayout* layout = new QVBoxLayout(dialog);
            layout->addWidget(plain);
            dialog->setLayout(layout);
            dialog->layout()->addWidget(buttons);
            plain->setFocus();
            int ret = dialog->exec();
            if (ret==1 && !plain->toPlainText().isEmpty())
            {
                dialog->close();
                QStringList list = plain->toPlainText().split("\n");
                list.removeDuplicates();
                foreach (QString item, list)
                    if (item.isEmpty())
                        list.removeOne(item);
                int cur_row = tv->model()->rowCount();
                foreach (QModelIndex ind, tv->selectionModel()->selectedIndexes())
                    if (!tv->isRowHidden(ind.row()) && ind.column()==2)
                    {
                        cur_row = ind.row()+1;
                        break;
                    }
                QLineEdit* eline = GetTVLine(ui->tabWidget_2->currentIndex());
                const QString filter = eline->text();
                eline->setText("");
                QSet<QString> current_list;
                QList<QString> additional_list;
                for (int i=0; i<tv->model()->rowCount(); i++)
                    current_list<<tv->model()->data(tv->model()->index(i,2)).toString();
                int count=current_list.count();
                for (int i=0; i!=list.count(); i++)
                {
                    current_list<<list[i];
                    if (count!=current_list.count())
                        additional_list<<list[i];
                    count = current_list.count();
                }
                tv->model()->insertRows(cur_row,additional_list.count());
                for (int i=0; i!=additional_list.count(); cur_row++,i++)
                    tv->model()->setData(tv->model()->index(cur_row,2),additional_list[i]);
                UpdateStats(additional_list.count(),2);
                UpdateWords();
                eline->setText(filter);
                tv->ResizeContext();
            }
        });
        {
            QPushButton* b = new QPushButton;  bar->addWidget(b);
            b->setFlat(true);
            //b->setStyleSheet("QToolButton {background: url(:/main/res/refresh.png) top center no-repeat;}");
            b->setIcon(QIcon(":/main/res/refresh.png"));
            b->setFixedWidth(79); b->setFixedHeight(30);
            b->setIconSize(QSize(79,79));
            b->setEnabled(false);
            b->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
            connect(b,SIGNAL(clicked()),this,SLOT(DeleteWords()));
            if (i==4-NEGATIVE_TABLE)
                connect(b,&QToolButton::clicked,[=]()
                {   DeleteClones(1);    });
        }
        b = new QToolButton;
        if (i==4-NEGATIVE_TABLE)
            bar->addWidget(b);
        b->setCheckable(true);
        b->setStyleSheet("QToolButton {background: url(:/main/res/show_words.png) top center no-repeat;}");
        b->setFixedWidth(40); b->setFixedHeight(40);
        connect(b,&QToolButton::clicked,[=]()
        {
            MyView* ltv = GetTVEditor(ui->tabWidget->currentIndex());
            MyView* iltv = GetTV2Editor(ui->tabWidget_2->currentIndex());
            tModel* lmodel = (tModel*)ltv->model();
            iModel* ilmodel = (iModel*)iltv->model();
            QSet<QString> words;
            for (int i=0; i<lmodel->rowCount(); i++)
                foreach (QString s, *lmodel->Words(i))
                    words<<s;
            //if (b->isChecked())
                //ilmodel->setCurrentWords(words.toList());
            GetTV2Line(ui->tabWidget_2->currentIndex())->setText("");
            iltv->setUpdatesEnabled(false);
            int count;
            for (int i=0; i<ilmodel->rowCount(); i++)
            {
                if (b->isChecked())
                {
                    count = words.count();
                    words<<ilmodel->data(i)->word;
                    if (count==words.count())
                        iltv->showRow(i);
                    else
                    {
                        iltv->hideRow(i);
                        words.remove(ilmodel->data(i)->word);
                    }
                }
                else
                    iltv->showRow(i);
            }
            iltv->setUpdatesEnabled(true);
            b->setDown(true);
            b->setDown(false);
        });
        if (i==4-POSITIVE_TABLE)
        {
            b = new QToolButton;   bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/add.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(35);
            /*connect(b,&QToolButton::clicked,[=]()
            {
                tModel* smodel = (tModel*)GetTVEditor(1)->model();
                tModel* dmodel = (tModel*)GetTVEditor(2)->model();
                iModel* imodel = (iModel*)GetTV2Editor(POSITIVE_TABLE)->model();
                QSet<QString> dlist;
                int from_row = dmodel->rowCount();
                foreach (tree_item item, tree_lists[0])
                    for (int i=0; i<item.view->model()->rowCount(); i++)
                        dlist<<item.view->model()->data(item.view->model()->index(i,1)).toString();
                QSet<QString> slist;
                for (int i=0; i<imodel->rowCount(); i++)
                    if (imodel->data(i)->checked)
                        for (int j=0; j<smodel->rowCount(); j++)
                            slist<<smodel->data(j)->free_html+" "+imodel->data(i)->word;
                int count = dlist.count();
                QList<QString> end_list;
                QList<QString> sslist = slist.toList();
                for (int i=0; i<sslist.count(); i++)
                {
                    dlist<<sslist[i];
                    if (count!=dlist.count())
                        end_list<<sslist[i];
                    count = dlist.count();
                }
                dmodel->insertRows(from_row,end_list.count());
                for (int j=from_row, i=0; j<dmodel->rowCount(); j++, i++)
                    dmodel->setData(dmodel->index(j,1),end_list[i],Qt::EditRole);
            });*/
            connect(b,&QToolButton::clicked,[=]()
            {
                tModel* dmodel = (tModel*)GetTVEditor(2)->model();
                iModel* smodel = (iModel*)GetTV2Editor(POSITIVE_TABLE)->model();
                QStringList string_set;
                int old_rows = dmodel->rowCount();
                for (int i=0; i<dmodel->rowCount(); i++)
                    string_set<<dmodel->data(i)->free_html;
                string_set.removeDuplicates();
                //dmodel->Clear();
                QSet<QString> new_string_set;
                for (int i=0; i<string_set.count(); i++)
                    for (int j=0; j<smodel->rowCount(); j++)
                        if (smodel->data(j)->checked)
                            new_string_set<<string_set[i]+" "+smodel->data(j)->word;
                //new_string_set = string_set.toSet();
                string_set = MakeUniq(0,new_string_set);
                dmodel->insertRows(old_rows,string_set.count());
                //string_set = new_string_set.toList();
                for (int i=0, j=old_rows; i<string_set.count(); i++, j++)
                {
                    dmodel->setData(dmodel->index(j,1),string_set[i],Qt::EditRole);
                    dmodel->Init(j);
                }
                UpdateStats(string_set.count(),1,2);
                current_tabs[0]->setText(1,QString::number(GetTVEditor(2)->model()->rowCount()));
            });
        }

        QVBoxLayout* searchBox = new QVBoxLayout;  box->addLayout(searchBox);
        QHBoxLayout* filters = new QHBoxLayout;   // searchBox->addLayout(filters);

        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setSizePolicy(QSizePolicy::Policy::Minimum,QSizePolicy::Policy::Minimum);
        filters->setSizeConstraint(QLayout::SetMinimumSize);
        QWidget* wid = new QWidget;
        wid->setLayout(filters);
        scrollArea->setWidget(wid);
        searchBox->addWidget(scrollArea);

        for (int i=0; i<2; i++)
        {
            QVBoxLayout* search = new QVBoxLayout;
            filters->addLayout(search);
            QSpinBox* filter1 = new QSpinBox;
            filter1->setMaximum(100000);
            if (i==0)
                connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s2_0()));
            else
                connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s2_3()));
            search->addWidget(filter1);
            QSpinBox* filter2 = new QSpinBox;
            filter2->setMaximum(100000);
            if (i==0)
                connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s2_0()));
            else
                connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s2_3()));
            search->addWidget(filter2);
            filters->setSpacing(0);
        }

        {
            QVBoxLayout* empt = new QVBoxLayout;
            empt->addWidget(new QLabel);
            filters->addLayout(empt);

            QVBoxLayout* add_filter = new QVBoxLayout;
            QLineEdit* line = new QLineEdit;
            line->setClearButtonEnabled(true);
            add_filter->addWidget(line);
            connect(line,SIGNAL(textChanged(QString)),this,SLOT(s2_2()));
            QCheckBox* checker = new QCheckBox;
            add_filter->addWidget(checker);
            connect(checker,SIGNAL(toggled(bool)),this,SLOT(s2_2()));
            filters->insertLayout(1,add_filter);

            add_filter = new QVBoxLayout;
            QCheckBox* editor = new QCheckBox;
            add_filter->addWidget(editor);
            add_filter->setAlignment(editor,Qt::AlignHCenter);
            connect(editor,SIGNAL(clicked()),this,SLOT(s2_1()));
            filters->insertLayout(1,add_filter);
        }

        QHBoxLayout* searchl = new QHBoxLayout;     searchBox->addLayout(searchl);
        b = new QToolButton;                        searchl->addWidget(b);
        b->setIcon(QIcon(":/main/res/filter.png"));
        b->setAutoRaise(true);
        connect(b, &QToolButton::clicked,[=]()
        {
           if (GetTV2Filter(ui->tabWidget_2->currentIndex())->isHidden())
               ShowFilters(2);
           else
               HideFilters(2);
        });

        QLabel* label = new QLabel;                     searchl->addWidget(label);
        label->setPixmap(QPixmap(":/main/res/question.png").scaled(20,20,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));

        QLineEdit* line = new QLineEdit;  searchl->addWidget(line);
        line->setClearButtonEnabled(true);
        /// #############################################################################
        /// ФИЛЬТР МАЛЕНЬКИХ СЛОВ
        /// #############################################################################
        connect(line,&QLineEdit::textChanged,[=]()
        {
            line->setText(line->text().toLower());
            const QString& text = line->text();
            tv->setUpdatesEnabled(false);
            if (text.isEmpty())
                for (int i=0; i!=tv->model()->rowCount(); i++)
                    tv->showRow(i);
            else
                for (int i=0; i!=tv->model()->rowCount(); i++)
                    tv->model()->data(tv->model()->index(i,2)).toString()!=text ? tv->hideRow(i) : tv->showRow(i);
            tv->setUpdatesEnabled(true);
        });

        QHBoxLayout* status = new QHBoxLayout;
        if (i!=4-NEGATIVE_TABLE)
            status->addWidget(new QLabel("0 / 0 (100%)"));
        else
            status->addWidget(new QLabel("0 / 0"));
        status->addWidget(new QProgressBar);
        box->addLayout(status);

        QMenu* filter_menu = new QMenu();
        act = new QAction("№",this);                    act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Выберите",this);             act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Слова из запросов",this);    act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Количество слов",this);      act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        connect(filter_menu,&QMenu::triggered,[=]()
        {
            for (int i=0; i< filter_menu->actions().count();i++)
                filter_menu->actions()[i]->isChecked() ? tv->showColumn(i) : tv->hideColumn(i);
        });
        connect(((QAbstractButton*)tv->children()[tv->children().count()-9]),&QAbstractButton::clicked,[=]()
        { filter_menu->popup(QCursor::pos()); });

        QWidget* w = new QWidget; w->setLayout(box);
        switch (i)
        {
        case 4-NEGATIVE_TABLE: ui->tabWidget_2->insertTab(0,w,"Минус слова [-]"); break;
        case 4-POSITIVE_TABLE: ui->tabWidget_2->insertTab(0,w,"Доп. слова [+]"); break;
        case 4-AB_NEGATIVE_TABLE: ui->tabWidget_2->insertTab(0,w,"Запросы [-]"); break;
        case 4-NEGATIVE_BD_TABLE: ui->tabWidget_2->insertTab(0,w,"БД [-]"); break;
        case 4-POSITIVE_BD_TABLE: ui->tabWidget_2->insertTab(0,w,"БД [+]"); break;
        }
    }
    for (int i=0; i<5; i++)
        HideFilters(2,i);
    ui->tabWidget_2->setVisible(false);
    foreach (MyView* tv, views)
        connect(tv,&MyView::transfer,[=](const QString item, mViewType to)
        {
            iModel* to_model = (iModel*)views[5-static_cast<int>(to)]->model();
            if (!to_model->AddWord(item,false))
                to_model->SubWord(item);
            else
                UpdateStats(1,2,-1+static_cast<int>(to));
        });

    connect(items[4-NEGATIVE_TABLE],&iItem::WordWasPickedGlobal,[=]()
    {
        UpdateStats(0,2,NEGATIVE_TABLE);
    });

    connect(items[4-NEGATIVE_BD_TABLE],&iItem::WordWasPickedGlobal,[=]()
    {
        UpdateStats(0,2,NEGATIVE_BD_TABLE);
    });

    connect(items[4-POSITIVE_BD_TABLE],&iItem::WordWasPickedGlobal,[=]()
    {
        UpdateStats(0,2,POSITIVE_BD_TABLE);
    });

    foreach (iItem* item, items)
    {
        connect(item,&iItem::WordWasPickedGlobal,[=]()
        {
            GetRefreshButton()->setEnabled(true);
        });
    }

/// ############################################################################################################################################
/// БОЛЬШИЕ СЛОВА   ############################################################################################################################
/// ############################################################################################################################################
    for (int i=0; i<6; i++)
    {
        MyView* tv = new MyView(mViewType::DEFAULT_VIEW);   tItem* item;
        if (i==1 ||i==2)       item = new tItem(PROGRESSIVE_LINE,tv);
        else if (i==4)  item = new tItem(COLOR_LINE,tv);
        else            item = new tItem(SIMPLE_LINE,tv);
        tModel* model = new tModel(i==1 || i==2,tv);
        tv->setModel(model);
        tv->setItemDelegate(item);
        tv->resizeColumnsToContents();
        connect(tv,&MyView::UpdateStates,[=](int a, int b)
        {
           UpdateStats(a,b);
           if (i==1)
               UpdateWords();
        });
        connect(tv,&MyView::ReloadFilter,[=]()
        {
            QString tmp = GetTVLine(ui->tabWidget->currentIndex())->text();
            GetTVLine(ui->tabWidget->currentIndex())->setText("");
            GetTVLine(ui->tabWidget->currentIndex())->setText(tmp);
        });
        connect(tv,&MyView::ClearTable,[=](mViewType to)
        {
            GetTVLabel(ui->tabWidget->currentIndex())->setText("0 / 0 (100%)");
        });
        if (i==0)
            connect(tv,&MyView::RowsDeleted,[=](QList<int> dlist)
            {
                foreach (int i, dlist)
                    foreach (tData* cache_data, cache)
                        if (cache_data->num == model->data(i)->num)
                        {
                            cache.removeOne(cache_data);
                            break;
                        }
            });

        connect(model,SIGNAL(RepaintIndex(const QModelIndex&)),tv,SLOT(update(QModelIndex)));
        if (i==2)
        {
            connect(item,SIGNAL(WordWasPickedGlobal(const QString&)),models[4-POSITIVE_TABLE],SLOT(PickWords(const QString&)));
            connect(item,SIGNAL(WordWasUnpickedGlobal(const QString&)),models[4-POSITIVE_TABLE],SLOT(UnpickWords(const QString&)));
            connect(model, SIGNAL(WordAdded(const QString&,bool)),models[4-POSITIVE_TABLE],SLOT(AddWord(const QString&,bool)));
            connect(model, SIGNAL(WordDeleted(const QString&)),models[4-POSITIVE_TABLE],SLOT(SubWord(const QString&)));
            connect(items[4-POSITIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),model,SLOT(PickWords(const QString&,bool)));
        }
        connect(tv,&MyView::transfer,[=](const QString item, mViewType to)
        {
            iModel* to_model = (iModel*)views[5-static_cast<int>(to)]->model();
            if (!to_model->AddWord(item,false))
                to_model->SubWord(item);
            else
                UpdateStats(1,2,-1+static_cast<int>(to));
        });

        connect(item,&iItem::commitData,[=](QWidget*)
        { tv->ResizeContext(500); });

        QVBoxLayout* box = new QVBoxLayout; QWidget* w = new QWidget; w->setLayout(box);
        switch (i)
        {
            case 0:
                ui->tabWidget->insertTab(0,w,"Ядро запросов");
                break;
            case 1:
                ui->tabWidget->insertTab(1,w,"Редактор ядра");
                break;
            case 2:
                ui->tabWidget->insertTab(2,w,"Доп. слова [+]");
                break;
            case 3:
                ui->tabWidget->insertTab(3,w,"Интерпретация");
                break;
            case 4:
                ui->tabWidget->insertTab(4,w,"Сравнить запросы с ядром");
                break;
            case 5:
                ui->tabWidget->insertTab(5,w,"Запросы на парсинг");
                break;
        }

/// ############################################################################################################################################
/// БРАУЗЕР   ############################################################################################################################
/// ############################################################################################################################################

        QToolBar* bar = new QToolBar;
        box->addWidget(bar);
        if (i>1)
            //box->addWidget(tv);
        {
            QTreeWidget* browser = new QTreeWidget;
            browser->setSelectionMode(QAbstractItemView::SingleSelection);
            browser->viewport()->setAcceptDrops(true);
            browser->setDropIndicatorShown(true);
            browser->setDragDropMode(QAbstractItemView::InternalMove);
            browser->setDragEnabled(true);
            trees.append(browser);
            browser->setColumnCount(2);
            QSplitter* lbox = new QSplitter;
            tmp_boxes.append(lbox);
            box->addWidget(lbox);
            browser->headerItem()->setText(0,"Редактор");
            browser->headerItem()->setText(1,"Количество запросов");
            lbox->addWidget(browser);
            lbox->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);
            QMenu* tree_menu = new QMenu;
            browser->setContextMenuPolicy(Qt::CustomContextMenu);
            QAction* act = new QAction("Переименовать",browser);
            tree_menu->addAction(act);
            connect(act,&QAction::triggered,[=]()
            {
                if (browser->currentItem()->text(0) == "Все запросы")
                    return;
                QDialog* dialog = new QDialog;
                QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel ,dialog);
                connect(buttons->button(QDialogButtonBox::Ok),SIGNAL(clicked()),dialog,SLOT(accept()));
                buttons->button(QDialogButtonBox::Ok)->setText("Применить");
                connect(buttons->button(QDialogButtonBox::Cancel),SIGNAL(clicked()),dialog,SLOT(reject()));
                buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");
                QLineEdit* name_line = new QLineEdit;
                name_line->setText(browser->currentItem()->text(0));
                QVBoxLayout* layout = new QVBoxLayout(dialog);
                layout->addWidget(name_line);
                dialog->setLayout(layout);
                dialog->layout()->addWidget(buttons);
                name_line->setFocus();
                int ret = dialog->exec();
                if (ret==1 && !name_line->text().isEmpty())
                {
                    dialog->close();
                    browser->currentItem()->setText(0,name_line->text());
                    int max = 0;
                    foreach (tree_item item, tree_lists[i-2])
                        if (item.node->text(0).length()>max)
                            max = item.node->text(0).length();
                }
            });
            act = new QAction("Удалить",browser);
            tree_menu->addAction(act);
            connect(act,&QAction::triggered,[=]()
            {
                foreach (QTreeWidgetItem* item, browser->selectedItems())
                {
                    if (item->text(0)!="Все запросы")
                    {
                        foreach (QTreeWidgetItem* sitem, GetAllChildren(item))
                        {
                            foreach (tree_item litem, tree_lists[i-2])
                                if (litem.node==sitem)
                                    tree_lists[i-2].removeOne(litem);
                            delete sitem;
                        }
                        foreach (tree_item litem, tree_lists[i-2])
                            if (litem.node==item)
                                tree_lists[i-2].removeOne(litem);
                        delete item;
                        return;
                    }
                }
            });

            QAction* add_act = new QAction("Добавить",browser);
            tree_menu->addAction(add_act);
            m_is_first_pages.append(true);
            connect(add_act,&QAction::triggered,[=]()
            {
               AddTreeChildES(i-2);
            });
            add_act->trigger();

            connect(browser,&QTreeWidget::customContextMenuRequested,[=](const QPoint& pos)
            {
                if (browser->itemAt(pos) && browser->itemAt(pos)->text(0)!="+")
                    tree_menu->popup(QCursor::pos());
            });

            /*connect(browser, &QTreeWidget::itemClicked,[=](QTreeWidgetItem* item) mutable
            {
                if (ui->tabWidget_2->currentIndex()==NEGATIVE_TABLE)
                {
                    QToolBar* mtb = (QToolBar*)ui->tabWidget_2->widget(ui->tabWidget_2->currentIndex())->layout()->itemAt(0)->widget();
                    ((QToolButton*)mtb->widgetForAction(mtb->actions().at(3)))->setChecked(false);
                    GetTV2Line(ui->tabWidget_2->currentIndex())->setText("1");
                    GetTV2Line(ui->tabWidget_2->currentIndex())->setText("");
                }
            });*/
            connect(browser, &QTreeWidget::currentItemChanged,[=](QTreeWidgetItem* item, QTreeWidgetItem* p_item) mutable
            {
                if (item->text(0)=="+")
                    return;
                foreach (tree_item child, tree_lists[i-2])
                    if (item==child.node)
                    {
                        QList<int> f = tmp_boxes[i-2]->sizes();
                        int ci = f.count();
                        foreach (tree_item bchild, tree_lists[i-2])
                            if (bchild.node==current_tabs[i-2])
                            {
                                bchild.view->hide();
                                break;
                            }
                        lbox->insertWidget(0,child.view);
                        child.view->show();
                        if (tmp_boxes[i-2]->sizes().count()!=2)
                        {
                            if (ci==tmp_boxes[i-2]->sizes().count())
                                tmp_boxes[i-2]->setSizes(f);
                            else
                            {
                                f.insert(1,0);
                                tmp_boxes[i-2]->setSizes(f);
                            }
                        }

                        current_tabs[i-2] = child.node;
                        QStringList names;
                        QList<MyView*> views;
                        foreach (tree_item litem, tree_lists[i-2])
                            if (item!=litem.node)
                            {
                                /*names<<litem.node->text(0);
                                views<<litem.view;*/
                            }

                        QTreeWidgetItemIterator it(browser);
                        while (*it)
                        {
                            if ((*it)->text(0)!=item->text(0))
                            {
                                names<<(*it)->text(0);
                                foreach (tree_item litem, tree_lists[i-2])
                                    if (litem.node->text(0)==(*it)->text(0) && item!=litem.node)
                                    {

                                        views<<litem.view;
                                        break;
                                    }
                            }
                            ++it;
                        }
                        foreach (tree_item item, tree_lists[i-2])
                        {
                            item.view->SetBrowserInfo(names,views);
                            /*disconnect((tItem*)item.view->itemDelegate(),SIGNAL(WordWasPickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(PickWords(const QString&)));
                            disconnect((tItem*)item.view->itemDelegate(),SIGNAL(WordWasUnpickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(UnpickWords(const QString&)));
                            disconnect((tModel*)item.view->model(), SIGNAL(WordAdded(const QString&,bool)),models[4-NEGATIVE_TABLE],SLOT(AddWord(const QString&,bool)));
                            disconnect((tModel*)item.view->model(), SIGNAL(WordDeleted(const QString&)),models[4-NEGATIVE_TABLE],SLOT(SubWord(const QString&)));
                            disconnect(items[4-NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)item.view->model(),SLOT(PickWords(const QString&,bool)));
                            disconnect(items[4-AB_NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)item.view->model(),SLOT(PickQuery(const QString&,bool)));*/
                        }

                        foreach (tree_item litem, tree_lists[i-2])
                            if (current_tabs[i-2]==litem.node)
                            {
                                /*connect((tItem*)litem.view->itemDelegate(),SIGNAL(WordWasPickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(PickWords(const QString&)));
                                connect((tItem*)litem.view->itemDelegate(),SIGNAL(WordWasUnpickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(UnpickWords(const QString&)));
                                connect((tModel*)litem.view->model(), SIGNAL(WordAdded(const QString&,bool)),models[4-NEGATIVE_TABLE],SLOT(AddWord(const QString&,bool)));
                                connect((tModel*)litem.view->model(), SIGNAL(WordDeleted(const QString&)),models[4-NEGATIVE_TABLE],SLOT(SubWord(const QString&)));
                                connect(items[4-NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)litem.view->model(),SLOT(PickWords(const QString&,bool)));
                                connect(items[4-AB_NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)litem.view->model(),SLOT(PickQuery(const QString&,bool)));*/
                                break;
                            }
                        return;
                    }
            });

            emit browser->currentItemChanged(current_tabs[i-2],NULL);
            QList<int> sizes;
            sizes.append(lbox->width()*3/4);
            sizes.append(lbox->width()/4);
            lbox->setSizes(sizes);
        }
        else if (i!=0)
        {
            tmp_views = views;
            QTreeWidget* browser = new QTreeWidget;
            browser->setSelectionMode(QAbstractItemView::SingleSelection);
            browser->viewport()->setAcceptDrops(true);
            browser->setDropIndicatorShown(true);
            browser->setDragDropMode(QAbstractItemView::InternalMove);
            browser->setDragEnabled(true);
            tree = browser;
            tree->setColumnCount(2);
            QSplitter* lbox = new QSplitter;
            tmp_box = lbox;
            box->addWidget(lbox);
            browser->headerItem()->setText(0,"Редактор");
            browser->headerItem()->setText(1,"Количество запросов");
            lbox->addWidget(tree);
            lbox->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);
            QMenu* tree_menu = new QMenu;
            browser->setContextMenuPolicy(Qt::CustomContextMenu);
            QAction* act = new QAction("Переименовать",browser);
            tree_menu->addAction(act);
            connect(act,&QAction::triggered,[=]()
            {
                if (browser->currentItem()->text(0) == "Все запросы")
                    return;
                QDialog* dialog = new QDialog;
                QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel ,dialog);
                connect(buttons->button(QDialogButtonBox::Ok),SIGNAL(clicked()),dialog,SLOT(accept()));
                buttons->button(QDialogButtonBox::Ok)->setText("Применить");
                connect(buttons->button(QDialogButtonBox::Cancel),SIGNAL(clicked()),dialog,SLOT(reject()));
                buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");
                QLineEdit* name_line = new QLineEdit;
                name_line->setText(browser->currentItem()->text(0));
                QVBoxLayout* layout = new QVBoxLayout(dialog);
                layout->addWidget(name_line);
                dialog->setLayout(layout);
                dialog->layout()->addWidget(buttons);
                name_line->setFocus();
                int ret = dialog->exec();
                if (ret==1 && !name_line->text().isEmpty())
                {
                    dialog->close();
                    browser->currentItem()->setText(0,name_line->text());
                    int max = 0;
                    foreach (tree_item item, tree_list)
                        if (item.node->text(0).length()>max)
                            max = item.node->text(0).length();
                    //tree->setFixedWidth(max*13);
                }
            });
            act = new QAction("Удалить",browser);
            tree_menu->addAction(act);
            connect(act,&QAction::triggered,[=]()
            {
                foreach (QTreeWidgetItem* item, browser->selectedItems())
                {
                    if (item->text(0)!="Все запросы")
                    {
                        foreach (QTreeWidgetItem* sitem, GetAllChildren(item))
                        {
                            foreach (tree_item litem, tree_list)
                                if (litem.node==sitem)
                                    tree_list.removeOne(litem);
                            delete sitem;
                        }
                        foreach (tree_item litem, tree_list)
                            if (litem.node==item)
                                tree_list.removeOne(litem);
                        delete item;
                        return;
                    }
                }
            });

            QAction* add_act = new QAction("Добавить",browser);
            tree_menu->addAction(add_act);
            m_is_first_page = true;
            connect(add_act,SIGNAL(triggered()),this,SLOT(AddTreeChild()));
            add_act->trigger();

            connect(browser,&QTreeWidget::customContextMenuRequested,[=](const QPoint& pos)
            {
                if (browser->itemAt(pos) && browser->itemAt(pos)->text(0)!="+")
                    tree_menu->popup(QCursor::pos());
            });

            connect(browser, &QTreeWidget::itemClicked,[=](QTreeWidgetItem* item) mutable
            {
                if (item && item->text(0)=="+")
                {
                    browser->selectionModel()->setCurrentIndex(browser->model()->index(0,0),QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
                    add_act->trigger();
                    foreach (tree_item item, tree_list)
                        if (item.node->text(0)=="Все запросы")
                        {
                            item.node->setExpanded(true);
                            break;
                        }
                }
                else if (ui->tabWidget_2->currentIndex()==NEGATIVE_TABLE)
                {
                    QToolBar* mtb = (QToolBar*)ui->tabWidget_2->widget(ui->tabWidget_2->currentIndex())->layout()->itemAt(0)->widget();
                    ((QToolButton*)mtb->widgetForAction(mtb->actions().at(3)))->setChecked(false);
                    GetTV2Line(ui->tabWidget_2->currentIndex())->setText("1");
                    GetTV2Line(ui->tabWidget_2->currentIndex())->setText("");
                    //UpdateWords();
                }
            });
            connect(browser, &QTreeWidget::currentItemChanged,[=](QTreeWidgetItem* item, QTreeWidgetItem* p_item) mutable
            {
                if (item->text(0)=="+")
                    return;
                foreach (tree_item child, tree_list)
                    if (item==child.node)
                    {
                        QList<int> f = tmp_box->sizes();
                        int ci = f.count();
                        foreach (tree_item bchild, tree_list)
                            if (bchild.node==current_tab)
                            {
                                bchild.view->hide();
                                break;
                            }
                        //lbox->insertWidget(0,child.view,2);
                        lbox->insertWidget(0,child.view);
                        child.view->show();
                        if (tmp_box->sizes().count()!=2)
                        {
                            if (ci==tmp_box->sizes().count())
                                tmp_box->setSizes(f);
                            else
                            {
                                f.insert(1,0);
                                tmp_box->setSizes(f);
                            }
                        }

                        current_tab = child.node;
                        QStringList names;
                        QList<MyView*> views;
                        foreach (tree_item litem, tree_list)
                            if (item!=litem.node)
                            {
                                /*names<<litem.node->text(0);
                                views<<litem.view;*/
                            }

                        QTreeWidgetItemIterator it(browser);
                        while (*it)
                        {
                            if ((*it)->text(0)!=item->text(0))
                            {
                                names<<(*it)->text(0);
                                foreach (tree_item litem, tree_list)
                                    if (litem.node->text(0)==(*it)->text(0) && item!=litem.node)
                                    {

                                        views<<litem.view;
                                        break;
                                    }
                            }
                            ++it;
                        }
                        foreach (tree_item item, tree_list)
                        {
                            item.view->SetBrowserInfo(names,views);
                            disconnect((tItem*)item.view->itemDelegate(),SIGNAL(WordWasPickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(PickWords(const QString&)));
                            disconnect((tItem*)item.view->itemDelegate(),SIGNAL(WordWasUnpickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(UnpickWords(const QString&)));
                            disconnect((tModel*)item.view->model(), SIGNAL(WordAdded(const QString&,bool)),models[4-NEGATIVE_TABLE],SLOT(AddWord(const QString&,bool)));
                            disconnect((tModel*)item.view->model(), SIGNAL(WordDeleted(const QString&)),models[4-NEGATIVE_TABLE],SLOT(SubWord(const QString&)));
                            disconnect(items[4-NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)item.view->model(),SLOT(PickWords(const QString&,bool)));
                            disconnect(items[4-AB_NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)item.view->model(),SLOT(PickQuery(const QString&,bool)));
                        }

                        foreach (tree_item litem, tree_list)
                            if (current_tab==litem.node)
                            {
                                connect((tItem*)litem.view->itemDelegate(),SIGNAL(WordWasPickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(PickWords(const QString&)));
                                connect((tItem*)litem.view->itemDelegate(),SIGNAL(WordWasUnpickedGlobal(const QString&)),models[4-NEGATIVE_TABLE],SLOT(UnpickWords(const QString&)));
                                connect((tModel*)litem.view->model(), SIGNAL(WordAdded(const QString&,bool)),models[4-NEGATIVE_TABLE],SLOT(AddWord(const QString&,bool)));
                                connect((tModel*)litem.view->model(), SIGNAL(WordDeleted(const QString&)),models[4-NEGATIVE_TABLE],SLOT(SubWord(const QString&)));
                                connect(items[4-NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)litem.view->model(),SLOT(PickWords(const QString&,bool)));
                                connect(items[4-AB_NEGATIVE_TABLE],SIGNAL(WordWasPickedGlobal(const QString&,const bool)),(tModel*)litem.view->model(),SLOT(PickQuery(const QString&,bool)));
                                break;
                            }
                        return;
                    }
            });

            emit browser->currentItemChanged(current_tab,NULL);
            QList<int> sizes;
            sizes.append(lbox->width()*3/4);
            sizes.append(lbox->width()/4);
            lbox->setSizes(sizes);
        }
        else box->addWidget(tv);

        QToolButton* b = new QToolButton;   bar->addWidget(b);
        b->setFixedWidth(79); b->setFixedHeight(35);
        b->setStyleSheet("QToolButton {background: url(:/main/res/Import-Export.png) top center no-repeat;}");
        connect(b,&QToolButton::clicked,[=]() { ei_menu->popup(QCursor::pos()); });

        b = new QToolButton;                bar->addWidget(b);
        b->setStyleSheet("QToolButton {background: url(:/main/res/plus.png) top center no-repeat;}");
        b->setFixedWidth(79); b->setFixedHeight(30);
        /// #############################################################################
        /// ДОБАВЛЕНИЕ БОЛЬШИХ СЛОВ ВРУЧНУЮ
        /// #############################################################################
        connect(b, &QToolButton::clicked, [=]()
        {
            QDialog* dialog = new QDialog;
            QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel ,dialog);
            connect(buttons->button(QDialogButtonBox::Ok),SIGNAL(clicked()),dialog,SLOT(accept()));
            buttons->button(QDialogButtonBox::Ok)->setText("Добавить");
            connect(buttons->button(QDialogButtonBox::Cancel),SIGNAL(clicked()),dialog,SLOT(reject()));
            buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");
            QPlainTextEdit* plain = new QPlainTextEdit;
            QVBoxLayout* layout = new QVBoxLayout(dialog);
            layout->addWidget(plain);
            dialog->setLayout(layout);
            dialog->layout()->addWidget(buttons);
            plain->setFocus();
            int ret = dialog->exec();
            if (ret==1 && !plain->toPlainText().isEmpty())
            {
                dialog->close();
                QStringList list = plain->toPlainText().split("\n");
                list.removeDuplicates();
                MyView* tv1 = GetTVEditor(ui->tabWidget->currentIndex());
                foreach (QString item, list)
                    if (item.isEmpty())
                        list.removeOne(item);
                int cur_row = tv1->model()->rowCount();
                int from_row = cur_row;
                tModel* main_model = (tModel*)tv1->model();
                if (i==1)
                {
                    QSet<QString> string_set;
                    {
                        foreach (tree_item item, tree_list)
                        {
                            tModel* model = (tModel*)item.view->model();
                            for (int i=0; i<model->rowCount(); i++)
                                string_set<<model->data(i)->free_html;
                        }
                    }
                    int count = string_set.count();
                    foreach (QString line, list)
                    {
                        string_set<<line;
                        if (count==string_set.count())
                            list.removeOne(line);
                        count = string_set.count();
                    }
                }
                else if (i>0)
                {
                    QSet<QString> string_set;
                    {
                        foreach (tree_item item, tree_lists[i-2])
                        {
                            tModel* model = (tModel*)item.view->model();
                            for (int i=0; i<model->rowCount(); i++)
                                string_set<<model->data(i)->free_html;
                        }
                    }
                    int count = string_set.count();
                    foreach (QString line, list)
                    {
                        string_set<<line;
                        if (count==string_set.count())
                            list.removeOne(line);
                        count = string_set.count();
                    }
                }
                main_model->insertRows(cur_row,list.count());
                QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
                const QString filter = eline->text();
                eline->setText("");
                QSet<QString> words;
                foreach (QString item, list)
                {
                    tv1->model()->setData(tv1->model()->index(cur_row,1),item);
                    main_model->Init(cur_row);
                    if (i==0)
                        cache.append(main_model->data(cur_row));
                    foreach(QString word, *((tModel*)tv1->model())->Words(cur_row))
                        words<<word;
                    cur_row++;
                }
                if (i==1)
                {
                    iModel* model = (iModel*)GetTV2Editor(NEGATIVE_TABLE)->model();
                    cur_row = model->rowCount();
                    QSet<QString> current_list;
                    QList<QString> additional_list;
                    for (int i=0; i<model->rowCount(); i++)
                        current_list<<model->data(model->index(i,2),Qt::EditRole).toString();
                    int count=current_list.count();
                    foreach(QString word, words)
                    {
                        current_list<<word;
                        if (count!=current_list.count())
                            additional_list<<word;
                        count = current_list.count();
                    }
                    model->insertRows(cur_row,additional_list.count());
                    foreach(QString word, additional_list)
                    {
                        model->setData(model->index(cur_row,2),word,Qt::EditRole);
                        cur_row++;
                    }
                    UpdateStats(additional_list.count(),2,NEGATIVE_TABLE);
                    UpdateWords(NEGATIVE_TABLE);
                    if (tree_list[0].view!=tv1)
                    {
                        /*int to_remove=main_model->rowCount();
                        DeleteClones(1);
                        QList<QString> list;
                        for (int i = from_row; i<tv1->model()->rowCount(); i++)
                            list<<tv1->model()->data(tv1->model()->index(i,1)).toString();
                        int cur_row = tree_list[0].view->model()->rowCount();
                        tModel* main_model = (tModel*)tree_list[0].view->model();
                        main_model->insertRows(cur_row,list.count());
                        QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
                        const QString filter = eline->text();
                        eline->setText("");
                        QSet<QString> words;
                        foreach (QString item, list)
                        {
                            tree_list[0].view->model()->setData(tree_list[0].view->model()->index(cur_row,1),item);
                            main_model->Init(cur_row);
                            foreach(QString word, *((tModel*)tree_list[0].view->model())->Words(cur_row))
                                words<<word;
                            cur_row++;
                        }
                        eline->setText(filter);
                        main_model->removedRowsByProgram(to_remove - main_model->rowCount());
                        tree_list[0].view->ResizeContext();*/
                    }
                    QToolBar* mtb = (QToolBar*)ui->tabWidget_2->widget(AB_NEGATIVE_TABLE)->layout()->itemAt(0)->widget();
                    int tmp = ui->tabWidget_2->currentIndex();
                    ui->tabWidget_2->setCurrentIndex(AB_NEGATIVE_TABLE);
                    ((QToolButton*)mtb->widgetForAction(mtb->actions().at(2)))->click();
                    mtb = (QToolBar*)ui->tabWidget_2->widget(NEGATIVE_TABLE)->layout()->itemAt(0)->widget();
                    ui->tabWidget_2->setCurrentIndex(NEGATIVE_TABLE);
                    ((QToolButton*)mtb->widgetForAction(mtb->actions().at(2)))->click();
                    ui->tabWidget_2->setCurrentIndex(tmp);
                }
                else if (i>0)
                {
                    if (tree_lists[i-2][0].view!=tv1)
                    {
                        /*int to_remove=main_model->rowCount();
                        DeleteClones(i);
                        QList<QString> list;
                        for (int i = from_row; i<tv1->model()->rowCount(); i++)
                            list<<tv1->model()->data(tv1->model()->index(i,1)).toString();
                        int cur_row = tree_lists[i-2][0].view->model()->rowCount();
                        tModel* main_model = (tModel*)tree_lists[i-2][0].view->model();
                        main_model->insertRows(cur_row,list.count());
                        QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
                        const QString filter = eline->text();
                        eline->setText("");
                        QSet<QString> words;
                        foreach (QString item, list)
                        {
                            tree_lists[i-2][0].view->model()->setData(tree_lists[i-2][0].view->model()->index(cur_row,1),item);
                            main_model->Init(cur_row);
                            foreach(QString word, *((tModel*)tree_lists[i-2][0].view->model())->Words(cur_row))
                                words<<word;
                            cur_row++;
                        }
                        eline->setText(filter);
                        main_model->removedRowsByProgram(to_remove - main_model->rowCount());
                        tree_lists[i-2][0].view->ResizeContext();
                        tree_lists[i-2].first().node->setText(1,QString::number(tree_lists[i-2].first().view->model()->rowCount()));*/
                    }
                }

                eline->setText(filter);
                int to_remove=main_model->rowCount();
                DeleteClones(1,true,i);
                main_model->removedRowsByProgram(to_remove - main_model->rowCount());
                UpdateStats(list.count() - to_remove + main_model->rowCount(),1);
                tv1->ResizeContext();
                if (ui->tabWidget->currentIndex()==1)
                foreach (tree_item item, tree_list)
                    item.node->setText(1,QString::number(item.view->model()->rowCount()));
                else if (ui->tabWidget->currentIndex()>0)
                    foreach (tree_item item, tree_lists[ui->tabWidget->currentIndex()-2])
                        item.node->setText(1,QString::number(item.view->model()->rowCount()));
                /*if (i==1)
                    current_tab->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));
                else if (i>1)
                    current_tabs[ui->tabWidget->currentIndex()-2]->setText(1,QString::number(GetTVEditor(ui->tabWidget->currentIndex())->model()->rowCount()));*/
            }
        });

        if (i!=0)
        {
            if (i!=4)
            {
                b = new QToolButton;    bar->addWidget(b);
                b->setStyleSheet("QToolButton {background: url(:/main/res/cross.png) top center no-repeat;}");
                b->setFixedWidth(116); b->setFixedHeight(35);
                connect(b,SIGNAL(clicked()),this,SLOT(DeleteInterpretators()));
            }

            /*if (i!=1)
            {
                b = new QToolButton;                bar->addWidget(b);
                b->setStyleSheet("QToolButton {background: url(:/main/res/find_clones.png) top center no-repeat;}");
                b->setFixedWidth(90); b->setFixedHeight(30);
                connect(b,&QToolButton::clicked,[=]()
                { DeleteClones(1); });
            }*/
        }

        if (i==1)
        {
            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/calculate.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(30);
            connect(b,SIGNAL(clicked()),this,SLOT(Calculate()));
        }

        switch (i)
        {
        case 2:
        {
            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/add_words.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(30);
            connect(b,SIGNAL(clicked()),this,SLOT(AddQueryes()));
            break;
        }
        case 3:
        {
            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/copy_parsing.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(30);
            connect(b,&QToolButton::clicked, [=]()
            {
                foreach (tree_item item, tree_lists[1])
                {
                    if (item.parent==NULL)
                        continue;
                    delete item.view;
                    tree_lists[1].removeOne(item);
                }
                foreach (QTreeWidgetItem* item, GetAllChildren(tree_lists[1][0].node))
                    delete item;
                foreach (tree_item item, tree_lists[0])
                    if (item.node->text(0)!="Все запросы")
                        foreach (tree_item litem, tree_lists[1])
                            if (litem.node->text(0)==item.parent->text(0))
                            {
                                trees[1]->setCurrentItem(litem.node);
                                AddTreeChildES(1,item.node->text(0));
                            }
                tab_counteres[1]=1;
                tmp_boxes[1]->setSizes(QList<int>()<<400<<400);
                GetTVLabel(3)->setText("0 / 0 (100%)");
                ((tModel*)tree_lists[1].first().view->model())->Clear();
                foreach (tree_item item, tree_lists[0])
                    foreach (tree_item litem, tree_lists[1])
                        if (litem.node->text(0)==item.node->text(0))
                        {
                            litem.view->model()->insertRows(0,item.view->model()->rowCount());
                            for (int i=0; i<item.view->model()->rowCount(); i++)
                            {
                                litem.view->model()->setData(litem.view->model()->index(i,1),item.view->model()->data(item.view->model()->index(i,1)));
                                ((tModel*)litem.view->model())->Init(i);
                            }
                            litem.node->setText(1,QString::number(litem.view->model()->rowCount()));
                        }
                UpdateStats(tree_lists[1].first().view->model()->rowCount(),1);
                int total =0;
                foreach (tree_item oitem, tree_lists[1])
                    total+=oitem.view->model()->rowCount();
                GetTVLabel(3)->setText(QString("%1 / %1 (100%)").arg(total));
                return;
                tModel* dmodel = (tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
                tModel* smodel = (tModel*)GetTVEditor(2)->model();
                //dmodel->insertRows(dmodel->rowCount(),smodel->rowCount());
                QSet<QString> string_set;
                for (int i=0; i<smodel->rowCount(); i++)
                    string_set<<smodel->data(i)->query;
                QList<QString> list = MakeUniq(1,string_set);
                    //dmodel->setData(i,smodel->data(i));
                //UpdateStats(smodel->rowCount(),1);
                int old_row = dmodel->rowCount();
                dmodel->insertRows(dmodel->rowCount(),list.count());
                for (int i=old_row, j=0; i<dmodel->rowCount(); i++, j++)
                    dmodel->setData(dmodel->index(i,1),list[j],Qt::EditRole);
                UpdateStats(list.count(),1);
            });

            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/interpretation.png) top center no-repeat;}");
            b->setFixedWidth(116); b->setFixedHeight(30);
            connect(b,SIGNAL(clicked()),this,SLOT(MakeInterpretation()));
            break;
        }
        case 4:
        {
            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/compare.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(30);
            connect(b,SIGNAL(clicked()),this,SLOT(Compare()));
            break;
        }
        case 5:
        {
            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/copy_parsing.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(30);
            connect(b,&QToolButton::clicked, [=]()
            {
                foreach (tree_item item, tree_lists[3])
                {
                    if (item.parent==NULL)
                        continue;
                    delete item.view;
                    tree_lists[3].removeOne(item);
                }
                foreach (QTreeWidgetItem* item, GetAllChildren(tree_lists[3][0].node))
                    delete item;
                foreach (tree_item item, tree_lists[1])
                    if (item.node->text(0)!="Все запросы")
                        foreach (tree_item litem, tree_lists[3])
                            if (litem.node->text(0)==item.parent->text(0))
                            {
                                trees[3]->setCurrentItem(litem.node);
                                AddTreeChildES(3,item.node->text(0));
                            }
                ((tModel*)tree_lists[3].first().view->model())->Clear();
                tab_counteres[3]=1;
                tmp_boxes[3]->setSizes(QList<int>()<<400<<400);
                GetTVLabel(5)->setText("0 / 0 (100%)");
                foreach (tree_item item, tree_lists[1])
                    foreach (tree_item litem, tree_lists[3])
                        if (litem.node->text(0)==item.node->text(0))
                        {
                            litem.view->model()->insertRows(0,item.view->model()->rowCount());
                            for (int i=0; i<item.view->model()->rowCount(); i++)
                            {
                                litem.view->model()->setData(litem.view->model()->index(i,1),((tModel*)item.view->model())->data(i)->free_html);
                                ((tModel*)litem.view->model())->Init(i);
                            }
                            litem.node->setText(1,QString::number(litem.view->model()->rowCount()));
                        }
                UpdateStats(tree_lists[3].first().view->model()->rowCount(),1);
                int total =0;
                foreach (tree_item oitem, tree_lists[3])
                    total+=oitem.view->model()->rowCount();
                GetTVLabel(5)->setText(QString("%1 / %1 (100%)").arg(total));
                return; //старая реализация ниже
               tModel* dmodel = (tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
               tModel* smodel = (tModel*)GetTVEditor(3)->model();
               //dmodel->insertRows(dmodel->rowCount(),smodel->rowCount());
               QSet<QString> string_set;
               for (int i=0; i<smodel->rowCount(); i++)
                   string_set<<smodel->data(i)->query;
               QList<QString> list = MakeUniq(1,string_set);
                   //dmodel->setData(i,smodel->data(i));
               //UpdateStats(smodel->rowCount(),1);
               int old_row = dmodel->rowCount();
               dmodel->insertRows(dmodel->rowCount(),list.count());
               for (int i=old_row, j=0; i<dmodel->rowCount(); i++, j++)
                   dmodel->setData(dmodel->index(i,1),list[j],Qt::EditRole);
               UpdateStats(list.count(),1);
            });
            break;
        }
        }

        if (i!=0)
        {
            b = new QToolButton;    bar->addWidget(b);
            b->setStyleSheet("QToolButton {background: url(:/main/res/Tree.png) top center no-repeat;}");
            b->setFixedWidth(79); b->setFixedHeight(30);
            connect(b,SIGNAL(clicked()),this,SLOT(BuildTree()));
        }
        if (i==1)
        {
            QToolButton* b1 = new QToolButton;   bar->addWidget(b1);
            b1->setFixedWidth(30); b1->setFixedHeight(30);
            b1->setStyleSheet("QToolButton {background: url(:/main/res/black_plus.png) top center no-repeat;}");
            connect(b1,SIGNAL(clicked()),this,SLOT(AddTreeChild()));
        }
        else if (i>1)
        {
            QToolButton* b1 = new QToolButton;   bar->addWidget(b1);
            b1->setFixedWidth(30); b1->setFixedHeight(30);
            b1->setStyleSheet("QToolButton {background: url(:/main/res/black_plus.png) top center no-repeat;}");
            connect(b1,&QToolButton::clicked, [=]()
            {
                AddTreeChildES(i-2);
            });
        }

        QVBoxLayout* searchBox = new QVBoxLayout;  box->addLayout(searchBox);
        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setSizePolicy(QSizePolicy::Policy::Minimum,QSizePolicy::Policy::Minimum);
        QHBoxLayout* filters = new QHBoxLayout;    //searchBox->addLayout(filters);
        filters->setSizeConstraint(QLayout::SetMinimumSize);
        QWidget* wid = new QWidget;
        wid->setLayout(filters);
        scrollArea->setWidget(wid);
        searchBox->addWidget(scrollArea);
        for (int i=0; i<6; i++)
        {
            QVBoxLayout* search = new QVBoxLayout;
            filters->addLayout(search);
            QSpinBox* filter1 = new QSpinBox;
            filter1->setMaximum(100000);
            switch (i)
            {
            case 0: connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s1_0())); break;
            case 1: connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s1_3())); break;
            case 2: connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s1_4())); break;
            case 3: connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s1_5())); break;
            case 4: connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s1_6())); break;
            case 5: connect(filter1,SIGNAL(valueChanged(int)),this,SLOT(s1_7())); break;
            }


            search->addWidget(filter1);

            QSpinBox* filter2 = new QSpinBox;
            filter2->setMaximum(100000);
            switch (i)
            {
            case 0: connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s1_0())); break;
            case 1: connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s1_3())); break;
            case 2: connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s1_4())); break;
            case 3: connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s1_5())); break;
            case 4: connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s1_6())); break;
            case 5: connect(filter2,SIGNAL(valueChanged(int)),this,SLOT(s1_7())); break;
            }
            search->addWidget(filter2);
            filters->setSpacing(0);
        }
        {
            QVBoxLayout* empt = new QVBoxLayout;
            empt->addWidget(new QLabel);
            filters->addLayout(empt);

            QVBoxLayout* add_filter = new QVBoxLayout;
            QDateTimeEdit* editor = new QDateTimeEdit;
            editor->setDisplayFormat("dd.MM.yyyy");
            editor->setCalendarPopup(true);
            editor->setDateTime(QDateTime::currentDateTime());
            add_filter->addWidget(editor);
            connect(editor,SIGNAL(dateChanged(QDate)),this,SLOT(s1_2()));
            editor = new QDateTimeEdit;
            editor->setDisplayFormat("dd.MM.yyyy");
            editor->setCalendarPopup(true);
            editor->setDateTime(QDateTime::currentDateTime());
            add_filter->addWidget(editor);
            connect(editor,SIGNAL(dateChanged(QDate)),this,SLOT(s1_2()));
            filters->insertLayout(1,add_filter);

            add_filter = new QVBoxLayout;
            QLineEdit* line = new QLineEdit;
            line->setClearButtonEnabled(true);
            add_filter->addWidget(line);
            connect(line,SIGNAL(textChanged(QString)),this,SLOT(s1_1()));
            QCheckBox* checker = new QCheckBox;
            add_filter->addWidget(checker);
            connect(checker,SIGNAL(clicked()),this,SLOT(s1_1()));
            filters->insertLayout(1,add_filter);
        }

        HideFilters(1,i);
        QHBoxLayout* search = new QHBoxLayout;     searchBox->addLayout(search);
        b = new QToolButton;                       search->addWidget(b);
        b->setIcon(QIcon(":/main/res/filter.png"));
        b->setAutoRaise(true);
        connect(b, &QToolButton::clicked,[=]()
        {
           if (GetScroll(1,ui->tabWidget->currentIndex())->isHidden())
               ShowFilters(1);
           else
               HideFilters(1);
        });
        QLabel* label = new QLabel;                     search->addWidget(label);
        label->setPixmap(QPixmap(":/main/res/question.png").scaled(20,20,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));

        QLineEdit* line = new QLineEdit;  search->addWidget(line);
        line->setClearButtonEnabled(true);
        /// #############################################################################
        /// ФИЛЬТР БОЛЬШИХ СЛОВ
        /// #############################################################################
        connect(line,&QLineEdit::textChanged,[=]()
        {
            if (line->text()!=line->text().toLower())
            {
                line->setText(line->text().toLower());
                return;
            }
            s1();
        });

        QHBoxLayout* status = new QHBoxLayout;
        status->addWidget(new QLabel("0 / 0 (100%)"));
        status->addWidget(new QProgressBar);
        box->addLayout(status);

        QMenu* filter_menu = new QMenu();
        act = new QAction("№",this);                    act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Ключевые запросы",this);     act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Дата добавления",this);      act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Частотность [ ]",this);      act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Частотность [\" \"]",this);  act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        act = new QAction("Частотность [\"! \"]",this); act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
        connect(filter_menu,&QMenu::triggered,[=]()
        {
            for (int i=0; i< filter_menu->actions().count();i++)
                filter_menu->actions()[i]->isChecked() ? tv->showColumn(i) : tv->hideColumn(i);
        });

        connect(((QAbstractButton*)tv->children()[tv->children().count()-4]),&QAbstractButton::clicked,[=]()
        { filter_menu->popup(QCursor::pos()); });
    }

    ///Загрузка БД
    QString fileName = "sb.dbsce";
    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite))
        return;

    QString tmp;
    int total_rows = 0;
    for (;;total_rows++)
    {
        tmp = file.readLine();
        if (tmp.length()<2)
            break;
    }
    if (!total_rows)
    {
        file.close();
        return;
    }

    file.seek(0);
    iModel* pmodel = (iModel*)GetTV2Editor(POSITIVE_BD_TABLE)->model();
    iModel* nmodel = (iModel*)GetTV2Editor(NEGATIVE_BD_TABLE)->model();
    nmodel->insertRows(0,total_rows);
    for (int i=0; i<total_rows; i++)
    {
        tmp = file.readLine();
        nmodel->setData(nmodel->index(i,2),tmp.left(tmp.length()-1),Qt::EditRole);
    }

    file.readLine();
    QStringList positive;
    for (;;)
    {
        tmp = file.readLine();
        if (tmp.length()<2)
            break;
        positive<<tmp;
    }
    if (positive.isEmpty())
    {
        file.close();
        return;
    }

    pmodel->insertRows(0,positive.count());
    for (int i=0; i<positive.count(); i++)
        pmodel->setData(pmodel->index(i,2),positive[i].left(positive[i].length()-1),Qt::EditRole);
    file.close();

    UpdateStats(0,2,NEGATIVE_BD_TABLE);
    UpdateStats(0,2,POSITIVE_BD_TABLE);
}

wf_mainwindow::~wf_mainwindow()
{
    QString fileName = "sb.dbsce";
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    iModel* pmodel = (iModel*)GetTV2Editor(POSITIVE_BD_TABLE)->model();
    iModel* nmodel = (iModel*)GetTV2Editor(NEGATIVE_BD_TABLE)->model();
    for (int i=0; i<nmodel->rowCount(); i++)
    {
        file.write(nmodel->data(nmodel->index(i,2),Qt::EditRole).toString().toUtf8());
        file.write("\n");
    }
    file.write("\n");
    for (int i=0; i<pmodel->rowCount(); i++)
    {
        file.write(pmodel->data(pmodel->index(i,2),Qt::EditRole).toString().toUtf8());
        file.write("\n");
    }
    file.close();
    delete ui;
}

void wf_mainwindow::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);
    this->~wf_mainwindow();
}

void wf_mainwindow::on_tabWidget_tabBarClicked(int index)
{
    if (index==1)
    {
        ui->tabWidget_2->setCurrentIndex(NEGATIVE_TABLE);
        ui->tabWidget->setCurrentIndex(1);
        ui->tabWidget_2->setVisible(true);
        if (cache.count()==0)
            return;
        QLabel* label = GetTVLabel(1);
        int ktotal = GetTVLabel(1)->text().mid(label->text().indexOf('/')+2,label->text().indexOf(' ',label->text().indexOf('/')+2)-label->text().indexOf('/')-1).toInt();
        tModel* model = (tModel*)GetTVEditor(index)->model();
        QProgressBar* progress = GetTVProgress(1);
        bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
        HideFilters(1);
        QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
        const QString filter = eline->text();
        eline->setText("");
        int cur_row = model->rowCount();
        QList<tData*> cache_wo_duplicates;
        QSet<QString> string_set;
        int count = 0;
        foreach (tData* item, cache)
        {
            string_set<<item->free_html;
            if (count!=string_set.count())
                cache_wo_duplicates<<item;
            count = string_set.count();
        }
        string_set.clear();
        count = 0;
        {
            foreach (tree_item item, tree_list)
            {
                tModel* model = (tModel*)item.view->model();
                for (int i=0; i<model->rowCount(); i++)
                    string_set<<model->data(i)->free_html;
            }
        }
        count = string_set.count();
        foreach (tData* item, cache_wo_duplicates)
        {
            string_set<<item->free_html;
            if (count==string_set.count())
                cache_wo_duplicates.removeOne(item);
            count = string_set.count();
        }
        cache = cache_wo_duplicates;
        if (cache.count()!=0)
        {
            progress->setMaximum(cache.count());
            progress->setValue(0);
        }
        ktotal+=cache.count();
        model->insertRows(cur_row,cache.count());
        iModel* model2 = (iModel*)GetTV2Editor(NEGATIVE_TABLE)->model();
        QSet<QString> iwords;
        for (int i=0; i<model2->rowCount(); i++)
            iwords<<model2->data(i)->word;
        QSet<QString> words;
        foreach (tData* item, cache)
        {
            model->setData(model->index(cur_row,1),item->query,Qt::EditRole);
            if (item->bfreq1)
                model->setData(model->index(cur_row,3),item->freq1,Qt::EditRole);
            if (item->bfreq2)
                model->setData(model->index(cur_row,4),item->freq2,Qt::EditRole);
            if (item->bfreq3)
                model->setData(model->index(cur_row,5),item->freq3,Qt::EditRole);
            model->setData(model->index(cur_row,6),item->sum1,Qt::EditRole);
            model->setData(model->index(cur_row,7),item->sum2,Qt::EditRole);
            model->Init(cur_row);
            foreach(QString word, *model->Words(cur_row))
            {
                word.replace("\'","");
                word.replace("\"","");
                word.replace("\*","");
                words<<word;
            }
            cur_row++;
            progress->setValue(progress->value()+1);
        }
        cur_row = model2->rowCount();
        count = iwords.count();
        QList<QString> wwords = words.toList();
        foreach (QString item, wwords)
        {
            iwords<<item;
            if (count==iwords.count())
                wwords.removeOne(item);
            count = iwords.count();
        }
        model2->insertRows(model2->rowCount(),wwords.count());
        progress = GetTV2Progress(NEGATIVE_TABLE);
        progress->setMaximum(wwords.count());
        progress->setValue(0);
        QStringList lwords = wwords;
        qSort(lwords);
        foreach (const QString word, lwords)
        {
            model2->setData(model2->index(cur_row,2),word,Qt::EditRole);
            cur_row++;
            progress->setValue(progress->value()+1);
        }
        if (!lwords.isEmpty())
        {
            UpdateStats(words.count(),2,NEGATIVE_TABLE);
            //UpdateWords(NEGATIVE_TABLE);
        }
        if (!filters_hidden)
            ShowFilters(1);
        GetTV2Editor(NEGATIVE_TABLE)->ResizeContext();
        GetTVEditor(1)->ResizeContext();
        progress->setValue(progress->value()+1);
        for (int i=0; i<GetTV2Editor(NEGATIVE_TABLE)->model()->rowCount(); i++)
            if (GetTV2Editor(NEGATIVE_TABLE)->model()->data(GetTV2Editor(NEGATIVE_TABLE)->model()->index(i,1)).toBool())
                for (int j=0; j<tree_list.count(); j++)
                        ((tModel*)tree_list[j].view->model())->PickWords(model2->data(model2->index(i,2),Qt::EditRole).toString());
        DeleteWords(NEGATIVE_TABLE,tree_list.first().view);
        DeleteClones(1);
        //DeleteClones(2);
        UpdateWords(AB_NEGATIVE_TABLE,tree_list.first().view);
        DeleteWords(AB_NEGATIVE_TABLE,tree_list.first().view);
        if (cache.count()!=0)
            UpdateStats(0,1,1,ktotal);
        cache.clear();
        eline->setText(filter);
    }
    else    if (index==2)
    {
        ui->tabWidget_2->setCurrentIndex(POSITIVE_TABLE);
        ui->tabWidget->setCurrentIndex(2);
        ui->tabWidget_2->setVisible(true);
        /*if (first_toggle)
        {
            foreach (tree_item item, tree_list)
                if (item.node->text(0)!="Все запросы")
                    foreach (tree_item litem, tree_lists[0])
                        if (litem.node->text(0)==item.parent->text(0))
                        {
                            trees[0]->setCurrentItem(litem.node);
                            AddTreeChildES(0,item.node->text(0));
                        }
            tmp_boxes[0]->setSizes(QList<int>()<<400<<400);
        }*/
        first_toggle = false;

    }
    else    ui->tabWidget_2->setVisible(false);
}


void wf_mainwindow::on_tabWidget_2_tabBarClicked(int index)
{
    ui->tabWidget_2->setCurrentIndex(index);
    //UpdateWords(index);
    GetTV2Editor(index)->ResizeContext();
}

void wf_mainwindow::DeleteWords(int index, MyView* delete_from)
{
    bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
    HideFilters(1);
    QLineEdit* eline1 = GetTVLine(ui->tabWidget->currentIndex());
    const QString filter1 = eline1->text();
    eline1->setText("");
    QLineEdit* eline2 = GetTVLine(ui->tabWidget_2->currentIndex());
    const QString filter2 = eline2->text();
    eline2->setText("");

    MyView* tv = GetTVEditor(ui->tabWidget->currentIndex());
    if (delete_from)
        tv = delete_from;
    if (index==-1)
        index=ui->tabWidget_2->currentIndex();
    GetRefreshButton(index)->setEnabled(false);
    UpdateWords(index);
    MyView* tv2 = GetTV2Editor(index);
    QProgressBar* progress = GetTV2Progress(ui->tabWidget_2->currentIndex());
    progress->setValue(0);
    progress->setMaximum(tv2->model()->rowCount());
    QString word;
    QString line;
    int count;
    for (int i=0; i<tv2->model()->rowCount(); i++)
    {
        if (tv2->model()->data(tv2->model()->index(i,1)).toBool())
        {
            word = tv2->model()->data(tv2->model()->index(i,2)).toString();
            count = tv2->model()->data(tv2->model()->index(i,3)).toInt();
            if (count!=0)
            if (!word.isEmpty())
            for (int bi=0; bi<tv->model()->rowCount(); bi++)
            {
                line = tv->model()->data(tv->model()->index(bi,1)).toString();
                if (count==0)
                    break;
                if (index==AB_NEGATIVE_TABLE)
                {
                    if ((((tModel*)tv->model())->data(bi))->free_html==word)
                    {
                        count--;
                        tv->model()->removeRow(bi);
                        UpdateStats(0,1);
                        break;
                    }
                    continue;
                }
                if (((tModel*)tv->model())->Words(bi)->contains(word))
                    count--;
                int fi = -1;
                while ((fi = line.indexOf(word[0],fi+1))!=-1)
                    if (fi!=0 && line.at(fi-1)=='>' &&
                            line.mid(fi,word.length()) == word &&
                            line.at(word.length()+fi)=='<')
                    {
                        int offset_left = 0, offset_right = 0;
                        if (fi>6 && line[fi-7]==' ')
                            offset_left = 1;
                        if (fi+word.length()+8<line.length() && line[fi+word.length()+8]==' ')
                        {
                            offset_right = 1;
                            if (line.length()>fi+word.length()+8)
                                offset_left = 0;
                        }
                        line.replace(fi-6-offset_left,word.length()+14+offset_right+offset_left,"");
                        if (line.isEmpty())
                        {
                            tv->model()->removeRow(bi);
                            bi--;
                            UpdateStats(0,1);
                        }
                        else
                        {
                            tv->model()->setData(tv->model()->index(bi,1),line);
                            ((tModel*)tv->model())->Init(bi);
                            tv->update(tv->model()->index(bi,1));
                        }
                    }
            }
            tv2->model()->setData(tv2->model()->index(i,3),0,Qt::EditRole);
        }
        progress->setValue(progress->value()+1);
    }

    /// ВЫПОЛНИТЬ СМЕЩЕНИЕ СЛОВ С НУЛЕВЫМ КОЛИЧЕСТВОМ ВНИЗ
    QList<iData> reserve;
    QList<int> to_delete;
    for (int row = 0; row<tv2->model()->rowCount(); row++)
        if (tv2->model()->data(tv2->model()->index(row,1)).toBool())
        {
            to_delete<<row;
            reserve<<*((iModel*)tv2->model())->data(row);
        }
    ((iModel*)tv2->model())->SpecialRemoveRows(to_delete,progress);
    tv2->model()->insertRows(tv2->model()->rowCount(),reserve.count());
    for (int i=0; i<reserve.count(); i++)
        ((iModel*)tv2->model())->setData(tv2->model()->rowCount()-reserve.count()+i,&reserve[i]);
    ((iModel*)tv2->model())->removedRowsByProgram(reserve.count());
    eline1->setText(filter1);
    eline2->setText(filter2);
    if (!filters_hidden)
        ShowFilters(1);
    for (int i=0; i<5; i++)
        if (i!=index)
            UpdateWords(i,NULL,progress);
    current_tab->setText(1,QString::number(GetTVEditor(1)->model()->rowCount()));
}

void wf_mainwindow::AddQueryes()
{
    foreach (tree_item item, tree_lists[0])
    {
        if (item.parent==NULL)
            continue;
        delete item.view;
        tree_lists[0].removeOne(item);
    }
    foreach (QTreeWidgetItem* item, GetAllChildren(tree_lists[0][0].node))
        delete item;
    foreach (tree_item item, tree_list)
        if (item.node->text(0)!="Все запросы")
            foreach (tree_item litem, tree_lists[0])
                if (litem.node->text(0)==item.parent->text(0))
                {
                    trees[0]->setCurrentItem(litem.node);
                    AddTreeChildES(0,item.node->text(0));
                }
    tab_counteres[0]=1;
    tmp_boxes[0]->setSizes(QList<int>()<<400<<400);
    GetTVLabel(2)->setText("0 / 0 (100%)");
    ((tModel*)tree_lists[0].first().view->model())->Clear();
    foreach (tree_item item, tree_list)
        foreach (tree_item litem, tree_lists[0])
            if (litem.node->text(0)==item.node->text(0))
            {
                litem.view->model()->insertRows(0,item.view->model()->rowCount());
                for (int i=0; i<item.view->model()->rowCount(); i++)
                {
                    litem.view->model()->setData(litem.view->model()->index(i,1),item.view->model()->data(item.view->model()->index(i,1)));
                    ((tModel*)litem.view->model())->Init(i);
                }
                litem.node->setText(1,QString::number(litem.view->model()->rowCount()));
            }
    UpdateStats(tree_lists[0].first().view->model()->rowCount(),1);
    int total=0;
    foreach (tree_item oitem, tree_lists[0])
        total+=oitem.view->model()->rowCount();
    GetTVLabel(2)->setText(QString("%1 / %1 (100%)").arg(total));
    return;


    bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
    HideFilters(1);
    QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
    const QString filter = eline->text();
    eline->setText("");

    QAbstractItemModel* model1 = GetTVEditor(1)->model();
    QAbstractItemModel* model2 = GetTV2Editor(POSITIVE_TABLE)->model();
    QAbstractItemModel* model3 = GetTVEditor(2)->model();
    int row3 = model3->rowCount();
    QString line;
    //model3->insertRows(model3->rowCount(),model1->rowCount()+model1->rowCount()*model2->rowCount());
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setMaximum(model1->rowCount());
    progress->setValue(0);
    int row_to_delete=0;
    QSet<QString> string_set;
    for (int row1=0; row1<model1->rowCount(); row1++)
    {
        for (int row2=0; row2<model2->rowCount(); row2++)
            if (model2->data(model2->index(row2,1)).toBool())
                string_set<<((tModel*)model1)->data(row1)->free_html+" "+model2->data(model2->index(row2,2)).toString();
        line.clear();
        /*foreach (const QString word, *((tModel*)model1)->Words(row1))
            line.append(QString("%1 ").arg(word));*/
        //model3->setData(model3->index(row3,1),line);
        //((tModel*)model3)->Init(row3);
        row3++;
        /*for (int row2=0; row2<model2->rowCount(); row2++)
        {
            if (!model2->data(model2->index(row2,1)).toBool())
            {
                row_to_delete++;
                continue;
            }
            //model3->setData(model3->index(row3,1),line+' '+model2->data(model2->index(row2,2)).toString());
            //((tModel*)model3)->Init(row3);
            row3++;
        }*/
        progress->setValue(progress->value()+1);
    }

    QList<QString> list = MakeUniq(0,string_set);
    int old_row = model3->rowCount();
    model3->insertRows(model3->rowCount(),list.count());
    for (int i=old_row, j=0; i<model3->rowCount(); i++, j++)
        model3->setData(model3->index(i,1),list[j]);
    //model3->removeRows(model3->rowCount()-row_to_delete,row_to_delete);
    //UpdateStats(model1->rowCount()+model1->rowCount()*model2->rowCount(),1);
    UpdateStats(list.count(),1);
    GetTVEditor(2)->ResizeContext();
    eline->setText(filter);
    //DeleteClones(1,true);
    if (!filters_hidden)
        ShowFilters(1);
}

QList<QString> wf_mainwindow::MakeUniq(int index,QSet<QString> set)
{
    QList<QString> list = set.toList();
    QSet<QString> dset;
    foreach (tree_item item, tree_lists[index])
        for (int i=0; i<item.view->model()->rowCount(); i++)
            dset<<item.view->model()->data(item.view->model()->index(i,1)).toString();
    int count = dset.count();
    QList<QString> end_list;
    for (int i=0; i<list.count(); i++)
    {
        dset<<list[i];
        if (count!=dset.count())
            end_list<<list[i];
        count = dset.count();
    }
    return end_list;
}

void wf_mainwindow::DeleteInterpretators()
{
    bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
    HideFilters(1);
    QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
    const QString filter = eline->text();
    eline->setText("");

    tModel* model = (tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
    QSet<QString> dset;
    QList<int> dlist;
    int count = 0;
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setValue(0);
    progress->setMaximum(model->rowCount());
    for (int row=0; row!=model->rowCount(); row++)
    {
        dset<<model->data(row)->SimpleQuery;
        if (dset.count()==count)
            dlist<<row;
        count = dset.count();
        progress->setValue(progress->value()+1);
    }
    model->SpecialRemoveRows(dlist,progress);
    UpdateStats(0,1);
    if (ui->tabWidget->currentIndex()==1)
        UpdateWords();
    eline->setText(filter);
    if (!filters_hidden)
        ShowFilters(1);
    if (ui->tabWidget->currentIndex()==1)
    foreach (tree_item item, tree_list)
        item.node->setText(1,QString::number(item.view->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>0)
        foreach (tree_item item, tree_lists[ui->tabWidget->currentIndex()-2])
            item.node->setText(1,QString::number(item.view->model()->rowCount()));

}

void wf_mainwindow::BuildTree()
{
    tModel* model = (tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
    QList<QList<int> > tree;
    std::vector<int> new_tree (model->rowCount());
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    progress->setValue(0);
    progress->setMaximum(model->rowCount()*3);
    for (int row=0 ; row<model->rowCount() ; row++)
    {
        while (model->Words(row)->count()>tree.count())
            tree.append(QList<int>());
        tree[model->Words(row)->count()-1].append(row);
        progress->setValue(progress->value()+1);
    }

    QStringList* parent;
    QStringList* daughter;
    QString reform_text, reform_tmp;
    int reform_from, reform_len;
    int vector_counter=0;
    int root_pos;
    QList<int> root;
    QList<QList<int> > roots;
    for (int level = 0; level<tree.count(); level++)
    {
        foreach (int row, tree.at(level))
        {
            root_pos = vector_counter;
            new_tree[vector_counter] = row;
            vector_counter++;
            progress->setValue(progress->value()+1);
            parent = model->Words(row);
            for (int ilevel = level+1; ilevel<tree.count(); ilevel++)
            {
                foreach (int irow, tree[ilevel])
                {
                    bool f = false;
                    daughter = model->Words(irow);
                    foreach (QString word, *parent)
                    {
                        if (!daughter->contains(word))
                        {
                            f=true;
                            break;
                        }
                    }
                    if (!f)
                    {
                        QStringList pos;
                        reform_text = model->data(model->index(irow,1),Qt::EditRole).toString();
                        for (int i=parent->count()-1; i>-1; i--)
                        {
                            reform_from = reform_text.indexOf(model->data(model->index(row,1),Qt::EditRole).toString().split(' ').at(i));
                            reform_len = model->data(model->index(row,1),Qt::EditRole).toString().split(' ').at(i).size();
                            reform_tmp = reform_text.mid(reform_from,reform_len);
                            reform_text.replace(reform_from,reform_len,"");
                            reform_text.insert(0,QString("%1 ").arg(reform_tmp));
                            reform_text.replace("  "," ");
                            reform_tmp.replace("<b><u>","");
                            reform_tmp.replace("</b></u>","");
                            pos.push_front(reform_tmp);
                        }

                        model->setData(model->index(irow,1),reform_text,Qt::EditRole);
                        foreach (QString word, *daughter)
                            if (!pos.contains(word))
                                pos.append(word);
                        *daughter = pos;

                        progress->setValue(progress->value()+1);
                        new_tree[vector_counter] = irow;
                        tree[ilevel].removeOne(irow);
                        root<<irow;
                        vector_counter++;
                    }
                }
            }
            if (!root.isEmpty())
            roots<<root;
            QList<int> advanced_tree;
            while (!roots.empty())
            {
                int inserted =0;
                advanced_tree<<roots.first()[0];
                parent = model->Words(roots.first()[0]);
                root.clear();
                int ppos = roots.first()[0];
                roots.first().removeAt(0);
                if (roots.first().count()==0)
                {
                    roots.pop_front();
                    continue;
                }
                for (int i=0; i<roots.first().count(); i++)
                {
                    if (model->Words(roots.first()[i])->count()<=parent->count())
                        continue;
                    bool f = false;
                    daughter = model->Words(roots.first()[i]);
                    foreach (QString word, *parent)
                    {
                        if (!daughter->contains(word))
                        {
                            f=true;
                            break;
                        }
                    }
                    if (!f)
                    {
                        QString pa_q = model->data(model->index(ppos,1),Qt::EditRole).toString();
                        pa_q.replace("  "," ");
                        if (pa_q[pa_q.length()-1]==' ')
                            pa_q = pa_q.left(pa_q.length()-1);
                        reform_text = model->data(model->index(roots.first()[i],1),Qt::EditRole).toString();
                        for (int j=parent->count()-1; j>-1; j--)
                        {
                            //reform_from = reform_text.indexOf(model->data(model->index(ppos,1),Qt::EditRole).toString().split(' ').at(j));
                            //reform_len = model->data(model->index(ppos,1),Qt::EditRole).toString().split(' ').at(j).size();
                            reform_from = reform_text.indexOf(QString(pa_q.split(' ').at(j)+' '));
                            if (reform_from==-1)
                                reform_from = reform_text.indexOf(' ' + pa_q.split(' ').at(j))+1;
                            reform_len = pa_q.split(' ').at(j).size();
                            reform_tmp = reform_text.mid(reform_from,reform_len);
                            reform_text.replace(reform_from,reform_len,"");
                            reform_text.insert(0,QString("%1 ").arg(reform_tmp));
                            reform_text.replace("  "," ");
                            reform_tmp.replace("<b><u>","");
                            reform_tmp.replace("</b></u>","");
                        }

                        model->setData(model->index(roots.first()[i],1),reform_text,Qt::EditRole);
                        root<<roots.first()[i];
                        roots.first().removeAt(i);
                        i--;
                        inserted++;
                    }
                }

                if (inserted!=0)
                {
                    if (roots.first().count()==0)
                        roots.pop_front();
                    roots.push_front(root);
                }
            }
            for (int i=root_pos+1, j=0; i<vector_counter; i++, j++)
                new_tree[i] = advanced_tree[j];
        }
    }
    QList <tData> reserve;
    for (int i=0; i<model->rowCount(); i++)
        reserve<<*model->data(i);
    for (int i=0; i<model->rowCount(); i++)
    {
        model->setData(i,&reserve.at(new_tree[i]));
        model->Words(i)->sort();
        progress->setValue(progress->value()+1);
    }
    for (int i = 0; i!=model->rowCount(); i++)
        for (int j=0; j!=8; j++)
            GetTVEditor(ui->tabWidget->currentIndex())->update(model->index(i,j));
}

inline void Recursion(int pos, QStringList in, QStringList &out)
{
    if (pos==in.count()-1)
    {
        out.append(in.join(' '));
        return;
    }
    for (int i=pos; i<in.count(); i++)
    {
        in.swap(pos,i);
        Recursion(pos+1,in,out);
        in.swap(i,pos);
    }
}

void wf_mainwindow::MakeInterpretation()
{
    tModel* dmodel = (tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
    //tModel* smodel = (tModel*)GetTVEditor(ui->tabWidget->currentIndex()-1)->model();
    QSet<QString> list_set;
    QList<QString> list;
    for (int i=0; i<dmodel->rowCount(); i++)
        list_set<<dmodel->data(i)->query;

    list = list_set.toList();

    bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
    HideFilters(1);
    QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
    const QString filter = eline->text();
    eline->setText("");

    QStringList query;
    QStringList::iterator iter;
    QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
    //progress->setMaximum(smodel->rowCount());
    progress->setMaximum(list.count());
    progress->setValue(0);
    int coun=0;
    int lcoun;
    //for (int rows=0 ; rows<smodel->rowCount(); rows++)
    for (int rows=0 ; rows<list.count(); rows++)
    {
        lcoun=1;
        //for (int i=2; i<smodel->Words(rows)->count()+1; i++)
        for (int i=2; i<list.at(rows).split(' ').count()+1; i++)
            lcoun*=i;
        coun+=lcoun;
    }
    const int prev_count = dmodel->rowCount();
    int next_gen = dmodel->rowCount();
    //dmodel->insertRows(dmodel->rowCount(),coun-dmodel->rowCount());
    //UpdateStats(dmodel->rowCount(),1);
    //for (int rows=0 ; rows<smodel->rowCount(); rows++)

    QSet<QString> end_list;
    for (int rows=0 ; rows<list.count(); rows++)
    {
        //Recursion(0,*smodel->Words(rows),query);
        Recursion(0,list.at(rows).split(' '),query);
        query.removeFirst();
        iter=query.begin();
        for (int rowd=next_gen; rowd<query.count() + next_gen; rowd++)
        //for (int rowd=dmodel->rowCount()-query.count(); rowd<dmodel->rowCount(); rowd++)
        {
            /*dmodel->setData(dmodel->index(rowd,1),*iter,Qt::EditRole);
            dmodel->Init(rowd);*/
            end_list<<*iter;
            iter++;
        }
        next_gen+=query.count();
        progress->setValue(progress->value()+1);
        query.clear();
    }
    QList<QString> end_end_list = MakeUniq(1,end_list);
    int old_row = dmodel->rowCount();
    dmodel->insertRows(dmodel->rowCount(),end_end_list.count());
    for (int i=old_row, j=0; i<dmodel->rowCount(); i++, j++)
    {
        dmodel->setData(dmodel->index(i,1),end_end_list[j],Qt::EditRole);
        dmodel->Init(i);
    }
    eline->setText(filter);
    if (!filters_hidden)
        ShowFilters(1);
    /*if (current_tabs[1]->text(0)=="Все запросы")
        UpdateStats(end_end_list.count(),1);*/
    //DeleteClones(1,true);
    UpdateStats(dmodel->rowCount()-old_row,1);
    current_tabs[1]->setText(1,QString::number(GetTVEditor(3)->model()->rowCount()));
}

void wf_mainwindow::Compare()
{
    foreach (tree_item item, tree_lists[2])
    {
        if (item.parent==NULL)
            continue;
        delete item.view;
        tree_lists[2].removeOne(item);
    }
    foreach (QTreeWidgetItem* item, GetAllChildren(tree_lists[2][0].node))
        delete item;
    foreach (tree_item item, tree_lists[1])
        if (item.node->text(0)!="Все запросы")
            foreach (tree_item litem, tree_lists[2])
                if (litem.node->text(0)==item.parent->text(0))
                {
                    trees[2]->setCurrentItem(litem.node);
                    AddTreeChildES(2,item.node->text(0));
                }
    tab_counteres[2]=1;
    tmp_boxes[2]->setSizes(QList<int>()<<400<<400);
    GetTVLabel(4)->setText("0 / 0 (100%)");
    foreach (tree_item item, tree_lists[1])
        foreach (tree_item litem, tree_lists[2])
            if (litem.node->text(0)==item.node->text(0))
            {
                litem.view->model()->insertRows(0,item.view->model()->rowCount());
                for (int i=0; i<item.view->model()->rowCount(); i++)
                    litem.view->model()->setData(litem.view->model()->index(i,1),item.view->model()->data(item.view->model()->index(i,1)));
            }
    UpdateStats(tree_lists[2].first().view->model()->rowCount(),1);

    bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
    HideFilters(1);
    QLineEdit* eline = GetTVLine(ui->tabWidget->currentIndex());
    const QString filter = eline->text();
    eline->setText("");

    foreach (tree_item oitem, tree_lists[2])
    {
        tModel* dmodel = (tModel*)oitem.view->model();//(tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
        tModel* smodel1 = (tModel*)GetTVEditor(0)->model();
        tModel* smodel2;
        foreach( tree_item poitem, tree_lists[1])
            if (poitem.node->text(0)==oitem.node->text(0))
                //(tModel*)GetTVEditor(ui->tabWidget->currentIndex()-1)->model();
            {
                smodel2 = (tModel*)poitem.view->model();
                break;
            }
        //DeleteClones(1);

        tData* val;
        tData tmp;
        std::vector<int> compared;
        int uncmp=0;
        dmodel->removeRows(0,dmodel->rowCount(),QModelIndex());
        dmodel->insertRows(0,smodel2->rowCount());

        QProgressBar* progress = GetTVProgress(ui->tabWidget->currentIndex());
        progress->setMaximum(smodel2->rowCount());
        progress->setValue(0);
        QSet<QString> string_set;
        QSet<QString> founded_set;
        for (int i=0; i<smodel2->rowCount(); i++)
            string_set<<smodel2->data(i)->free_html;
        int count = smodel2->rowCount();
        for (int i=0; i<smodel1->rowCount(); i++)
        {
            string_set<<smodel1->data(i)->free_html;
            if (count==string_set.count())
                founded_set<<smodel1->data(i)->free_html;
            count = string_set.count();
        }
        count = founded_set.count();
        int up_to_down = 0;
        int down_to_up = dmodel->rowCount()-1;
        for (int i=0; i<smodel2->rowCount(); i++)
        {
            val = smodel2->data(i);
            founded_set<<smodel2->data(i)->free_html;
            if (count==founded_set.count())
            {
                tmp = *val;
                tmp.query.insert(0,"<font color='red'>");
                dmodel->setData(up_to_down,&tmp);
                up_to_down++;
            }
            else
            {
                tmp = *val;
                dmodel->setData(down_to_up,&tmp);
                down_to_up--;
            }
            count = founded_set.count();
            progress->setValue(progress->value()+1);
        }
    }
    tModel* dmodel = (tModel*)GetTVEditor(ui->tabWidget->currentIndex())->model();
    for (int i=0; i<dmodel->rowCount(); i++)
    {
        dmodel->RepaintIndex(dmodel->index(i,0));
        dmodel->RepaintIndex(dmodel->index(i,1));
    }
    /*if (dmodel->rowCount()>0)
        UpdateStats(dmodel->rowCount(),1);*/
    eline->setText(filter);
    if (!filters_hidden)
        ShowFilters(1);
    int total =0;
    foreach (tree_item oitem, tree_lists[2])
        total+=oitem.view->model()->rowCount();
    GetTVLabel(4)->setText(QString("%1 / %1 (100%)").arg(total));
    if (ui->tabWidget->currentIndex()==1)
    foreach (tree_item item, tree_list)
        item.node->setText(1,QString::number(item.view->model()->rowCount()));
    else if (ui->tabWidget->currentIndex()>0)
        foreach (tree_item item, tree_lists[ui->tabWidget->currentIndex()-2])
            item.node->setText(1,QString::number(item.view->model()->rowCount()));
}

void wf_mainwindow::UpdateStats(const int count, int type, int index, int new_total)
{
    QLabel* label;
    if (type==1)
    {
        if (index==-1)
            index = ui->tabWidget->currentIndex();
        label = GetTVLabel(index);
    }
    else
    {
        if (index==-1)
            index = ui->tabWidget_2->currentIndex();
        label = GetTV2Label(index);
    }
    if (type==2 && (index==NEGATIVE_TABLE || index==NEGATIVE_BD_TABLE || index==POSITIVE_BD_TABLE))
    {
        int checked = 0;
        for (int i=0; i<GetTV2Editor(index)->model()->rowCount(); i++)
            if (GetTV2Editor(index)->model()->data(GetTV2Editor(index)->model()->index(i,1)).toBool())
                checked++;
        label->setText(QString("%2 / %1").arg(GetTV2Editor(index)->model()->rowCount()).arg(checked));
        return;
    }
    int total = label->text().mid(label->text().indexOf('/')+2,label->text().indexOf(' ',label->text().indexOf('/')+2)-label->text().indexOf('/')-1).toInt();
    MyView* tv = type==1 ? GetTVEditor(index) : GetTV2Editor(index);
    if (type==1 && index==1)
        tv = tree_list.first().view;
    else if (type==1 && index>1)
        tv = tree_lists[index-2].first().view;
    int now =0;
    if (type==1 && index==1)
        foreach (tree_item item, tree_list)
            now += item.view->model()->rowCount();
    else if (type==1 && index>1)
        foreach (tree_item item, tree_lists[index-2])
            now += item.view->model()->rowCount();
    else
        now = tv->model()->rowCount();
    total+=count;
    if (new_total!=-1)
        total = new_total;
    if (total>0)
        label->setText(QString("%1 / %2 (%3%)").arg(now).arg(total).arg(100*now/total));
}

int wf_mainwindow::DeleteClones(const int type, bool delete_byPr, int index, MyView* delete_from)
{
    QLineEdit* eline;
    QString filter;
    MyView* tv;
    if (type==1)
    {
        if (index==-1)
            index = ui->tabWidget->currentIndex();
        eline = GetTVLine(index);
        tv = GetTVEditor(index);
        if (delete_from)
            tv = delete_from;
    }
    else
    {
        if (index==-1)
            index = ui->tabWidget_2->currentIndex();
        eline = GetTV2Line(index);
        tv = GetTV2Editor(index);
    }

    bool filters_hidden = GetTVFilter(ui->tabWidget->currentIndex())->isHidden();
    HideFilters(1);
    filter = eline->text();
    eline->setText("");
    QProgressBar* progress;
    if (type==1)
        progress = GetTVProgress(index);
    else
        progress = GetTV2Progress(index);

    int dcount;
    if (type==1)
        dcount = ((tModel*)tv->model())->MakeUnique(progress);
    else
        dcount = ((iModel*)tv->model())->MakeUnique(progress);

    UpdateStats(0,type);
    if (type==1 && index==1)
        UpdateWords();
    eline->setText(filter);
    if (!filters_hidden)
        ShowFilters(1);
    if (delete_byPr)
    {
        UpdateStats(-dcount,type);
        if (type==1)
        {
            ((tModel*)tv->model())->removedRowsByProgram(dcount);
            UpdateStats(0,type);
        }
        else
            ((iModel*)tv->model())->removedRowsByProgram(dcount);
    }
    return dcount;
}

void wf_mainwindow::Calculate()
{
    int index = ui->tabWidget->currentIndex();
    tModel* tmodel= (tModel*)GetTVEditor(index)->model();
    QProgressBar* progress = GetTVProgress(index);
    progress->setValue(0);
    progress->setMaximum(tmodel->rowCount());

    QString parent;
    QStringList parent_text;
    QStringList daughter_text;
    bool is_daughter=false;
    int sum1;
    int sum2;
    int parent_row;

    for (int i=0; i<tmodel->rowCount(); i++)
    {
        daughter_text = index==1 ? tmodel->data(i)->free_html.split(' ') :
                                   tmodel->data(i)->query.split(' ');
        if (tmodel->data(i)->words.count()>=parent_text.count())
        {
            if (parent_text.count()!=0)
                is_daughter = true;
            for (int j=0; j<parent_text.count(); j++)
                if (parent_text[j]!=daughter_text[j])
                {
                    is_daughter=false;
                    break;
                }
        }
        if (is_daughter)
        {
            sum1 -= tmodel->data(tmodel->index(i,4),Qt::EditRole).toInt();
            sum2 -= tmodel->data(tmodel->index(i,5),Qt::EditRole).toInt();
            is_daughter=false;
        }
        else
        {
            if (!parent.isEmpty())
            {
                tmodel->setData(tmodel->index(parent_row,6),sum1,Qt::EditRole);
                tmodel->setData(tmodel->index(parent_row,7),sum2,Qt::EditRole);
                GetTVEditor(index)->update(tmodel->index(parent_row,6));
                GetTVEditor(index)->update(tmodel->index(parent_row,7));
            }
            parent = index==1 ? tmodel->data(i)->free_html : tmodel->data(i)->query;
            parent_text = parent.split(' ');
            sum1 = tmodel->data(i)->freq2;
            sum2 = tmodel->data(i)->freq3;
            parent_row = i;
        }
        progress->setValue(progress->value()+1);
    }
    if (!parent.isEmpty())
    {
        tmodel->setData(tmodel->index(parent_row,6),sum1,Qt::EditRole);
        tmodel->setData(tmodel->index(parent_row,7),sum2,Qt::EditRole);
        GetTVEditor(index)->update(tmodel->index(parent_row,6));
        GetTVEditor(index)->update(tmodel->index(parent_row,7));
    }
}

void wf_mainwindow::UpdateWords(int index, MyView* update_from, QProgressBar* pr)
{
    if (index==-1)
        index = ui->tabWidget_2->currentIndex();
    iModel* imodel= (iModel*)GetTV2Editor(index)->model();
    tModel* tmodel= (tModel*)GetTVEditor(1)->model();
    if (update_from)
        tmodel = (tModel*)update_from->model();
    QProgressBar* progress = GetTV2Progress(index);
    if (pr)
        progress = pr;
    progress->setValue(0);
    QSet<QString> words;
    QMap<QString,int> map;
    int count=0;
    if (index!=AB_NEGATIVE_TABLE)
    {
        progress->setMaximum(tmodel->rowCount()+imodel->rowCount());
        for (int i=0; i<tmodel->rowCount(); i++)
        {
            foreach (QString word, tmodel->data(i)->words)
            {
                words<<word;
                map.insert(word,map.value(word)+1);
                count = words.count();
            }
            progress->setValue(progress->value()+1);
        }
        for (int i=0; i<imodel->rowCount(); i++)
        {
            if (!imodel->data(i)->word.contains(' '))
                imodel->setData(imodel->index(i,3),map.value(imodel->data(i)->word),Qt::EditRole);
            else
            {
                int tmp_count = 0;
                for (int j=0; j<tmodel->rowCount(); j++)
                    if (tmodel->data(j)->free_html.contains(QRegularExpression(QString('* '+imodel->data(i)->word+' '))) ||
                        tmodel->data(j)->free_html.contains(QRegularExpression(QString('* '+imodel->data(i)->word))) ||
                        tmodel->data(j)->free_html.contains(QRegularExpression(QString(imodel->data(i)->word+' '))) ||
                        tmodel->data(j)->free_html==imodel->data(i)->word)
                        tmp_count++;
                imodel->setData(imodel->index(i,3),tmp_count,Qt::EditRole);
            }
            GetTV2Editor(index)->update(imodel->index(i,3));
            progress->setValue(progress->value()+1);

        }
    }
    else
    {
        progress->setMaximum(imodel->rowCount());
        for (int i=0; i<imodel->rowCount(); i++)
        {
            imodel->setData(imodel->index(i,3),0,Qt::EditRole);
            for (int j=0; j<tmodel->rowCount(); j++)
                if (tmodel->data(j)->free_html==imodel->data(i)->word)
                {
                    imodel->setData(imodel->index(i,3),1,Qt::EditRole);
                    GetTV2Editor(index)->update(imodel->index(i,3));
                    break;
                }
            progress->setValue(progress->value()+1);
        }
    }
}

void wf_mainwindow::SearchWithFilters(int type, int ftype, int index)
{
    QLineEdit* line;
    MyView* tv;
    if (index==-1)
    {
        if (type==1)
            index=ui->tabWidget->currentIndex();
        else
            index=ui->tabWidget_2->currentIndex();
    }

    if (type==1)
    {
        line = GetTVLine(index);
        tv = GetTVEditor(index);
    }
    else
    {
        line = GetTV2Line(index);
        tv = GetTV2Editor(index);
    }

    tModel* model = (tModel*)tv->model();
    iModel* imodel = (iModel*)tv->model();
    tv->setUpdatesEnabled(false);
    bool need_show;
    if (type==1)
    for (int i=0; i!=tv->model()->rowCount(); i++)
    {
        need_show=true;
        if (ftype==-1)
        if (GetTVFilter(index)->isHidden())
        {
            foreach (QString word, line->text().split(' '))
                if (!word.isEmpty())
                    if (word.length() < model->data(i)->WordsMinLength || !model->Words(i)->contains(word))
                    {
                        need_show = false;
                        break;
                    }
        }
        if (need_show)
            if (!GetScroll(type,index)->isHidden())
            {
                if (ftype==0)
                if (model->data(i)->num < ((QSpinBox*)GetTVFilter(index,0,0))->value() || model->data(i)->num > ((QSpinBox*)GetTVFilter(index,1,0))->value())
                    need_show=false;
                if (ftype==1)
                if (((QCheckBox*)GetTVFilter(index,1,1))->isChecked())
                {
                    foreach (QString word, ((QLineEdit*)GetTVFilter(index,0,1))->text().split(' '))
                        if (!word.isEmpty())
                            if (!model->data(i)->free_html.contains(word))
                            {
                                need_show = false;
                                break;
                            }
                }
                else
                {
                    foreach (QString word, ((QLineEdit*)GetTVFilter(index,0,1))->text().split(' '))
                        if (!word.isEmpty())
                            if (word.length() < model->data(i)->WordsMinLength || !model->Words(i)->contains(word))
                            {
                                need_show = false;
                                break;
                            }
                }
                if (ftype==2)
                if (model->data(i)->date < ((QDateTimeEdit*)GetTVFilter(index,0,2))->dateTime()
                        || model->data(i)->date > ((QDateTimeEdit*)GetTVFilter(index,1,2))->dateTime())
                    need_show=false;
                if (ftype==3)
                if (model->data(i)->freq1 < ((QSpinBox*)GetTVFilter(index,0,3))->value() || model->data(i)->freq1 > ((QSpinBox*)GetTVFilter(index,1,3))->value())
                    need_show=false;
                if (ftype==4)
                if (model->data(i)->freq2 < ((QSpinBox*)GetTVFilter(index,0,4))->value() || model->data(i)->freq2 > ((QSpinBox*)GetTVFilter(index,1,4))->value())
                    need_show=false;
                if (ftype==5)
                if (model->data(i)->freq3 < ((QSpinBox*)GetTVFilter(index,0,5))->value() || model->data(i)->freq3 > ((QSpinBox*)GetTVFilter(index,1,5))->value())
                    need_show=false;
                if (ftype==6)
                if (model->data(i)->sum1 < ((QSpinBox*)GetTVFilter(index,0,6))->value() || model->data(i)->sum1 > ((QSpinBox*)GetTVFilter(index,1,6))->value())
                    need_show=false;
                if (ftype==7)
                if (model->data(i)->sum2 < ((QSpinBox*)GetTVFilter(index,0,7))->value() || model->data(i)->sum2 > ((QSpinBox*)GetTVFilter(index,1,7))->value())
                    need_show=false;
            }
        tv->setRowHidden(i,!need_show);
    }
    else
    {
        for (int i=0; i!=tv->model()->rowCount(); i++)
        {
            need_show=true;
            if (ftype==-1)
                if (line->text()!=imodel->data(i)->word)
                    need_show = false;
            if (need_show)
                if (!GetScroll(type,index)->isHidden())
                {
                    if (ftype==0)
                    if (imodel->data(i)->num < ((QSpinBox*)GetTV2Filter(index,0,0))->value() || imodel->data(i)->num > ((QSpinBox*)GetTV2Filter(index,1,0))->value())
                        need_show=false;
                    if (ftype==1)
                    if (imodel->data(i)->checked != (((QCheckBox*)GetTV2Filter(index,0,1))->isChecked()))
                        need_show=false;
                    if (ftype==2)
                    if (!((QLineEdit*)GetTV2Filter(index,0,2))->text().isEmpty())
                    {
                        if (((QCheckBox*)GetTV2Filter(index,1,2))->isChecked())
                        {
                            if (((QLineEdit*)GetTV2Filter(index,0,2))->text()!=" ")
                            {
                                foreach (QString word, ((QLineEdit*)GetTV2Filter(index,0,2))->text().split(' '))
                                    if (!word.isEmpty())
                                        if (!imodel->data(i)->word.contains(word))
                                        {
                                            need_show = false;
                                            break;
                                        }
                            }
                            else
                                if (!imodel->data(i)->word.contains(" "))
                                    need_show = false;
                        }
                        else
                        {
                            if (imodel->data(i)->word!=((QLineEdit*)GetTV2Filter(index,0,2))->text())
                                need_show = false;
                        }
                    }
                    if (ftype==3)
                    if (imodel->data(i)->count < ((QSpinBox*)GetTV2Filter(index,0,3))->value() || imodel->data(i)->count >= ((QSpinBox*)GetTV2Filter(index,1,3))->value())
                        need_show=false;
                }
            tv->setRowHidden(i,!need_show);
        }
    }
    tv->setUpdatesEnabled(true);
}

void wf_mainwindow::s1()
{   SearchWithFilters(1,-1);    }
void wf_mainwindow::s2()
{   SearchWithFilters(2,-1);    }

void wf_mainwindow::s1_0()
{   SearchWithFilters(1,0);    }
void wf_mainwindow::s1_1()
{   SearchWithFilters(1,1);    }
void wf_mainwindow::s1_2()
{   SearchWithFilters(1,2);    }
void wf_mainwindow::s1_3()
{   SearchWithFilters(1,3);    }
void wf_mainwindow::s1_4()
{   SearchWithFilters(1,4);    }
void wf_mainwindow::s1_5()
{   SearchWithFilters(1,5);    }
void wf_mainwindow::s1_6()
{   SearchWithFilters(1,6);    }
void wf_mainwindow::s1_7()
{   SearchWithFilters(1,7);    }


void wf_mainwindow::s2_0()
{   SearchWithFilters(2,0);    }
void wf_mainwindow::s2_1()
{   SearchWithFilters(2,1);    }
void wf_mainwindow::s2_2()
{   SearchWithFilters(2,2);    }
void wf_mainwindow::s2_3()
{   SearchWithFilters(2,3);    }

QList<QTreeWidgetItem*> wf_mainwindow::GetAllChildren (QTreeWidgetItem* item)
{
    QList<QTreeWidgetItem*> result;
    foreach (QTreeWidgetItem* litem, item->takeChildren())
    {
        result<<GetAllChildren(litem);
        result<<litem;
    }
    return result;
}

void wf_mainwindow::AddTreeChild(QString name)
{
    MyView* ltv = new MyView(mViewType::DEFAULT_VIEW);
    QTreeWidgetItem* item;
    if (m_is_first_page)
    {
        item = new QTreeWidgetItem(tree);
        item->setText(0,"Все запросы");
        current_tab = item;
        tree_list<<tree_item(item,ltv,NULL);
        connect(ltv,&MyView::ClearTable,[=](mViewType to)
        {
            GetTVLabel(ui->tabWidget->currentIndex())->setText("0 / 0 (100%)");
        });
    }
    else
    {
        item = new QTreeWidgetItem(tree->currentItem());
        if (name.isEmpty())
            item->setText(0,QString("Редактор %1").arg(tab_counter+1));
        else
            item->setText(0,name);
        tree_list<<tree_item(item,ltv,tree->currentItem());
    }
    connect(ltv,&MyView::RowsWasDeleted,[=]()
    {
        current_tab->setText(1,QString::number(GetTVEditor(1)->model()->rowCount()));
    });
    item->setText(1,"0");
    tModel* lmodel = new tModel(true,ltv);
    tItem* litem;
    litem = new tItem(PROGRESSIVE_LINE,ltv);
    ltv->setModel(lmodel);
    connect(litem,&tItem::WordWasDoubleSelected,[=](const QString& word)
    {
        GetTVLine(ui->tabWidget->currentIndex())->setText(word);
    });
    ltv->setItemDelegate(litem);
    ltv->resizeColumnsToContents();
    connect(ltv,&MyView::UpdateStates,[=](int a, int b)
    {
       UpdateStats(a,b);
       UpdateWords();
    });
    QMenu* filter_menu = new QMenu();
    QAction* act = new QAction("№",this);                    act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Ключевые запросы",this);              act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Дата добавления",this);               act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Частотность [ ]",this);               act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Частотность [\" \"]",this);           act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Частотность [\"! \"]",this);          act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    connect(filter_menu,&QMenu::triggered,[=]()
    {
        for (int i=0; i< filter_menu->actions().count();i++)
            filter_menu->actions()[i]->isChecked() ? ltv->showColumn(i) : ltv->hideColumn(i);
    });
    connect(((QAbstractButton*)ltv->children()[ltv->children().count()-4]),&QAbstractButton::clicked,[=]()
    { filter_menu->popup(QCursor::pos()); });
    connect(ltv,&MyView::ReloadFilter,[=]()
    {
        QString tmp = GetTVLine(ui->tabWidget->currentIndex())->text();
        GetTVLine(ui->tabWidget->currentIndex())->setText("");
        GetTVLine(ui->tabWidget->currentIndex())->setText(tmp);
    });
    connect(lmodel,SIGNAL(RepaintIndex(const QModelIndex&)),ltv,SLOT(update(QModelIndex)));
    connect(litem,&iItem::commitData,[=](QWidget*)
    { ltv->ResizeContext(500); });
    connect(ltv,&MyView::transfer,[=](const QString item, mViewType to)
    {
        iModel* to_model = (iModel*)tmp_views[5-static_cast<int>(to)]->model();
        QTextDocument document;
        document.setHtml(item);
        if (!to_model->AddWord(document.toPlainText(),false))
            to_model->SubWord(document.toPlainText());
        else
            UpdateStats(1,2,-1+static_cast<int>(to));
    });
    tab_counter++;
    if (m_is_first_page)
        //tmp_box->insertWidget(0,ltv,2);
        tmp_box->insertWidget(0,ltv);
    /*int max = 0;
    foreach (tree_item item, tree_list)
        if (item.node->text(0).length()>max)
            max = item.node->text(0).length();
    tree->setFixedWidth(max*13);*/
    foreach (tree_item item, tree_list)
        item.node->setExpanded(true);
    m_is_first_page = false;
}

void wf_mainwindow::AddTreeChildES(int index,QString name)
{
    MyView* ltv = new MyView(mViewType::DEFAULT_VIEW);
    QTreeWidgetItem* item;
    if (m_is_first_pages[index])
    {
        item = new QTreeWidgetItem(trees[index]);
        item->setText(0,"Все запросы");
        current_tabs[index] = item;
        tree_lists[index]<<tree_item(item,ltv,NULL);
        connect(ltv,&MyView::ClearTable,[=](mViewType to)
        {
            GetTVLabel(ui->tabWidget->currentIndex())->setText("0 / 0 (100%)");
        });
    }
    else
    {
        item = new QTreeWidgetItem(trees[index]->currentItem());
        if (name.isEmpty())
            item->setText(0,QString("Редактор %1").arg(tab_counteres[index]+1));
        else
            item->setText(0,name);
        tree_lists[index]<<tree_item(item,ltv,trees[index]->currentItem());
    }
    connect(ltv,&MyView::RowsWasDeleted,[=]()
    {
        current_tabs[index]->setText(1,QString::number(GetTVEditor(index+2)->model()->rowCount()));
    });

    item->setText(1,"0");
    tModel* lmodel = new tModel(index==2,ltv);
    tItem* litem;
    litem = new tItem(index==4 ? COLOR_LINE : SIMPLE_LINE,ltv);
    ltv->setModel(lmodel);
    /*connect(litem,&tItem::WordWasDoubleSelected,[=](const QString& word)
    {
        GetTVLine(ui->tabWidget->currentIndex())->setText(word);
    });*/
    ltv->setItemDelegate(litem);
    ltv->resizeColumnsToContents();
    connect(ltv,&MyView::UpdateStates,[=](int a, int b)
    {
       UpdateStats(a,b);
       //UpdateWords();
    });
    QMenu* filter_menu = new QMenu();
    QAction* act = new QAction("№",this);                    act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Ключевые запросы",this);              act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Дата добавления",this);               act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Частотность [ ]",this);               act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Частотность [\" \"]",this);           act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    act = new QAction("Частотность [\"! \"]",this);          act->setCheckable(true); act->setChecked(true); filter_menu->addAction(act);
    connect(filter_menu,&QMenu::triggered,[=]()
    {
        for (int i=0; i< filter_menu->actions().count();i++)
            filter_menu->actions()[i]->isChecked() ? ltv->showColumn(i) : ltv->hideColumn(i);
    });
    connect(((QAbstractButton*)ltv->children()[ltv->children().count()-4]),&QAbstractButton::clicked,[=]()
    { filter_menu->popup(QCursor::pos()); });
    connect(ltv,&MyView::ReloadFilter,[=]()
    {
        QString tmp = GetTVLine(ui->tabWidget->currentIndex())->text();
        GetTVLine(ui->tabWidget->currentIndex())->setText("");
        GetTVLine(ui->tabWidget->currentIndex())->setText(tmp);
    });
    connect(lmodel,SIGNAL(RepaintIndex(const QModelIndex&)),ltv,SLOT(update(QModelIndex)));
    connect(litem,&iItem::commitData,[=](QWidget*)
    { ltv->ResizeContext(500); });
    connect(ltv,&MyView::transfer,[=](const QString item, mViewType to)
    {
        /*iModel* to_model = (iModel*)tmp_views[5-static_cast<int>(to)]->model();
        QTextDocument document;
        document.setHtml(item);
        if (!to_model->AddWord(document.toPlainText(),false))
            to_model->SubWord(document.toPlainText());
        else
            UpdateStats(1,2,-1+static_cast<int>(to));*/
    });
    tab_counteres[index]++;
    if (m_is_first_pages[index])
        tmp_boxes[index]->insertWidget(0,ltv);
    foreach (tree_item item, tree_lists[index])
        item.node->setExpanded(true);
    m_is_first_pages[index] = false;
}
