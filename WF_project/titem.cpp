#include "titem.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPainter>
#include <QKeyEvent>
#include <QApplication>
#include <QTimer>
#include <QMenu>
#include <QHeaderView>
#include "model.h"
#include "imodel.h"
#include <QDesktopServices>
#include <QUrl>

LineEdit::LineEdit(mLineType type,QWidget *parent):
    m_picking(false), m_type(type)
{
    setParent(parent);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setLineWrapMode(QTextEdit::NoWrap);
    setUndoRedoEnabled(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this,&LineEdit::customContextMenuRequested,[=](const QPoint& pos)
    {
        m_menu = createStandardContextMenu();
        QAction* act = new QAction("Поиск в Google",this);
        connect(act,SIGNAL(triggered()),this,SLOT(GoogleSearch()));
        m_menu->addAction(act);
        m_menu->popup(QCursor::pos());
    });
}

void LineEdit::GoogleSearch()
{
    QTextCursor cur = textCursor();
    cur.select(QTextCursor::WordUnderCursor);
    QDesktopServices::openUrl( QUrl(QString("http://www.google.com/search?q=%1").arg(cur.selectedText())) );
}

void LineEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    QTextEdit::mouseDoubleClickEvent(e);
    if (m_picking || m_type!=PROGRESSIVE_LINE || toPlainText().isEmpty())
        return;
    QTextCursor cur = textCursor();
    cur.select(QTextCursor::WordUnderCursor);
    emit WordWasDoubleClicked(cur.selectedText());
}

void LineEdit::mousePressEvent(QMouseEvent *e)
{
    QTextEdit::mousePressEvent(e);
    if (e->button()!=Qt::LeftButton)
    {
        e->ignore();
        return;
    }
    if (m_picking || m_type!=PROGRESSIVE_LINE)
        return;
    QTextCursor cur = textCursor();
    const int pos = textCursor().position();
    if (toPlainText().isEmpty())
        return;
    if (cur.charFormat().font().bold())
    {
        while (cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor) && cur.selectedText()!=" ") cur.clearSelection();
        if (cur.selectedText()==" ")
            cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
        else
            cur.clearSelection();
        while (cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor) && cur.selectedText().at(0)!=' ');
        if (cur.selectedText().isEmpty())
            return;
        if (cur.selectedText().at(0)==' ')
            cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
        if (cur.selectedText().isEmpty())
            return;
        m_picking = true;
        emit WordWasDeSelected(cur.selectedText());
        cur.insertHtml(cur.selectedText());
        cur.setPosition(pos);
        setTextCursor(cur);
        m_picking = false;
        return;
    }
    while (cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor) && cur.selectedText()!=" ") cur.clearSelection();
    if (cur.selectedText()==" ")
        cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
    else
        cur.clearSelection();
    while (cur.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor) && cur.selectedText().at(0)!=' ');
    if (cur.selectedText().isEmpty())
        return;
    if (cur.selectedText().at(0)==' ')
        cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
    if (cur.selectedText().isEmpty())
        return;
    m_picking = true;
    emit WordWasSelected(cur.selectedText());
    cur.insertHtml("<b><u>"+cur.selectedText()+"</b></u>");
    cur.setPosition(pos);
    setTextCursor(cur);
    m_picking = false;
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
     if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
     {
         clearFocus();
         return;
     }
     QTextEdit::keyPressEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent *e)
{
    QTextCursor cur(document());
    const int length = toPlainText().length();
    bool bold = false;
    html_text.clear();
    for (int i=0; i<length; i++)
    {
        cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
        if (cur.charFormat().font().bold() && !bold)
        {
            bold = true;
            html_text.append(cur.selectedText());
            html_text.insert(html_text.length()-1,"<b><u>");
            cur.clearSelection();
        }
        if (!cur.charFormat().font().bold() && bold)
        {
            bold = false;
            html_text.append(cur.selectedText());
            html_text.insert(html_text.length()-1,"</b></u>");
            cur.clearSelection();
        }
    }
    html_text.append(cur.selectedText());
    if (bold)   html_text.append("</b></u>");
    emit FocusLost(html_text);
    model->SetWords(row,toPlainText().split(' '));
    if (e->reason()!=Qt::PopupFocusReason)
        clearFocus();
    QTextEdit::focusOutEvent(e);
}

/// ##################################################################################################
/// T Item class
/// ##################################################################################################


QWidget *tItem::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    if (index.column() == 1)
    {
        if (m_type==SIMPLE_LINE)
        {
            QLineEdit* editor =  new QLineEdit(parent);
            connect(editor,&QLineEdit::textChanged,[=]()
            {
                ((tModel*)index.model())->setData(index.model()->index(index.row(),index.column()),editor->text(),Qt::EditRole);
                ((tModel*)index.model())->SetWords(index.row(),editor->text().split(' '));
            });
            return editor;
        }
        else
        {
            LineEdit* editor = new LineEdit(m_type,parent);
            connect(editor,SIGNAL(WordWasSelected(QString)),index.model(),SLOT(PickWords(QString)));
            connect(editor,SIGNAL(WordWasSelected(QString)),this,SIGNAL(WordWasPickedGlobal(const QString&)));
            connect(editor,SIGNAL(WordWasDoubleClicked(const QString&)),this,SIGNAL(WordWasDoubleSelected(const QString&)));
            connect(editor,&LineEdit::WordWasDeSelected,[=](const QString& word)
            {
               ((tModel*)index.model())->PickWords(word,false);
               emit WordWasUnpickedGlobal(word);
            });
            connect(editor,&LineEdit::FocusLost, [=]()
            { editor->model->setData(editor->model->index(editor->row,1),editor->html_text,Qt::EditRole); });
            editor->model = (tModel*)index.model();
            editor->row = index.row();
            return editor;
        }
    }
    else if (index.column() == 2)
    {
        QDateTimeEdit *editor = new QDateTimeEdit(parent);
        editor->setDisplayFormat("dd.MM.yyyy");
        editor->setCalendarPopup(true);
        editor->setDateTime(QDateTime::currentDateTime());
        return editor;
    }
    else
    {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setMaximum(1000000);
        editor->setValue(index.model()->data(index, Qt::DisplayRole).toInt());
        return editor;
    }
}

void tItem::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        if (m_type==SIMPLE_LINE)
        {
            QLineEdit* ceditor = qobject_cast<QLineEdit *>(editor);
            if (ceditor)
                ceditor->setText(index.model()->data(index, Qt::EditRole).toString());
        }
        else
        {
            LineEdit* ceditor = qobject_cast<LineEdit *>(editor);
            if (ceditor)
            {
                ceditor->setText(index.model()->data(index, Qt::EditRole).toString());
                ceditor->model = (tModel*)index.model();
                ceditor->row = index.row();
            }
        }
    }
    else if (index.column() == 2)
    {
        QDateTimeEdit *ceditor = qobject_cast<QDateTimeEdit*>(editor);
        if (ceditor)
            ceditor->setDateTime(index.model()->data(index, Qt::EditRole).toDateTime());
    }
    else
    {
        QSpinBox *ceditor = qobject_cast<QSpinBox *>(editor);
        if (ceditor)
            ceditor->setValue(index.model()->data(index, Qt::EditRole).toInt());
    }
}

void tItem::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        if (m_type==SIMPLE_LINE)
        {
            QLineEdit *ceditor = qobject_cast<QLineEdit *>(editor);
            if (ceditor)
                model->setData(index, ceditor->text());
        }
        else
        {
            LineEdit *ceditor = qobject_cast<LineEdit *>(editor);
            if (ceditor)
                model->setData(index, ceditor->html_text);
        }
    }
    else if (index.column() == 2)
    {
        QDateTimeEdit *ceditor = qobject_cast<QDateTimeEdit *>(editor);
        if (ceditor)
            model->setData(index, ceditor->dateTime());
    }
    else
    {
        QSpinBox *ceditor = qobject_cast<QSpinBox *>(editor);
        if (ceditor)
            model->setData(index, ceditor->value());
    }
}

void tItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column()!=1)
    {
        QItemDelegate::paint(painter,option,index);
        return;
    }

    painter->fillRect(option.rect, m_view->verticalHeader()->visualIndex(index.row())%2==0 ? QBrush(Qt::gray) : QBrush(Qt::white));
    if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
    QTextDocument document;
    document.setHtml(index.data(Qt::DisplayRole).toString());
    painter->translate(option.rect.topLeft());
    document.drawContents(painter);
    painter->translate(-option.rect.topLeft());
}


/// ##################################################################################################
/// I Item class
/// ##################################################################################################

QWidget *iItem::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (index.column() == 1)
        return new QCheckBox(parent);
    else if (index.column() == 2)
        return new QLineEdit(parent);
    else
    {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setMaximum(1000000);
        return editor;
    }
}

void iItem::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        QCheckBox *ceditor = qobject_cast<QCheckBox *>(editor);
        if (ceditor)
            ceditor->setChecked(index.model()->data(index, Qt::EditRole).toBool());
    }
    else if (index.column() == 2)
    {
        QLineEdit *ceditor = qobject_cast<QLineEdit*>(editor);
        if (ceditor)
            ceditor->setText(index.model()->data(index, Qt::EditRole).toString());
    }
    else
    {
        QSpinBox *ceditor = qobject_cast<QSpinBox *>(editor);
        if (ceditor)
            ceditor->setValue(index.model()->data(index, Qt::EditRole).toInt());
    }
}

void iItem::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        QCheckBox *ceditor = qobject_cast<QCheckBox *>(editor);
        if (ceditor)
            model->setData(index, ceditor->isChecked());
    }
    else if (index.column() == 2)
    {
        QLineEdit *ceditor = qobject_cast<QLineEdit *>(editor);
        if (ceditor)
        {
            emit WordWasEdited(ceditor->text(),index.row());
            model->setData(index, ceditor->text());
        }
    }
    else
    {
        QSpinBox *ceditor = qobject_cast<QSpinBox *>(editor);
        if (ceditor)
            model->setData(index, ceditor->value());
    }
}

void iItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        QStyleOptionButton checkboxstyle;
        QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);
        checkboxstyle.rect = option.rect;
        checkboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/2 - checkbox_rect.width()/2);
        checkboxstyle.state = index.model()->data(index, Qt::DisplayRole).toBool() ?
                    QStyle::State_Enabled|QStyle::State_On : QStyle::State_Enabled|QStyle::State_Off;
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
    }
    else
        QItemDelegate::paint(painter, option, index);
}

bool iItem::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
     if (index.column() == 1)
     {
        if (event->type() == QEvent::MouseButtonPress)
        {
            model->setData(index,!model->data(index).toBool());
            emit WordWasPickedGlobal(index.model()->data(index.model()->index(index.row(),2)).toString(),model->data(index).toBool());
        }
        event->ignore();
        return false;
     }
     else
         QItemDelegate::editorEvent(event,model,option,index);
}

/// ##################################################################################################
/// My View
/// ##################################################################################################

MyView::MyView(mViewType type):
    m_type(type), browser_menu(NULL)
{
    verticalHeader()->setSectionsMovable(true);
    verticalHeader()->setDragEnabled(true);
    QMenu* table_menu = new QMenu();
    QAction* act = new QAction("Удалить строку",this);
    table_menu->addAction(act);
    connect(act,&QAction::triggered,[=]()
    {
        QList<int> dlist;
        foreach (QModelIndex ind, selectionModel()->selectedRows())
            if (!isRowHidden(ind.row()))
                dlist.append(ind.row());
        bool clear_table = dlist.count()==model()->rowCount();
        if (type==DEFAULT_VIEW)
        {
            emit RowsDeleted(dlist);
            ((tModel*)model())->SpecialRemoveRows(dlist,NULL);
            emit RowsWasDeleted();
            emit UpdateStates(0,1);
            if (clear_table)
            {
                ((tModel*)model())->removedRowsByProgram(dlist.count());
                emit ClearTable(m_type);
            }
        }
        else
        {
            ((iModel*)model())->SpecialRemoveRows(dlist,NULL);
            emit UpdateStates(0,2);
            if (clear_table)
                emit ClearTable(m_type);
        }

    });
    verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(verticalHeader(),&QHeaderView::customContextMenuRequested,[=]()
    { table_menu->popup(QCursor::pos()); });

    browser_menu = new QMenu();
    if (type==DEFAULT_VIEW)
    {
        act = new QAction("Отправить в Запросы [-]",this);       browser_menu->addAction(act);
        connect(act,&QAction::triggered, [=]()
        {
            foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                if (!isRowHidden(ind.row()) && ind.column()==1)
                    emit transfer(model()->data(ind).toString(),AB_NEGATIVE_TABLE);
        });
        connect(this,&QTableView::customContextMenuRequested,[=]()
        {
            if (currentIndex().column()==1)
                browser_menu->popup(QCursor::pos());
        });
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    table_menu = new QMenu();
    if (type!=DEFAULT_VIEW)
    {
        connect(this,&MyView::clicked,[=](QModelIndex ind)
        {  emit FilterByWord(model()->data(model()->index(currentIndex().row(),2)).toString());    });
        act = new QAction("Отметить выделенное",this);       table_menu->addAction(act);
        connect(act,&QAction::triggered, [=]()
        {
            foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                if (!isRowHidden(ind.row()) && ind.column()==2)
                {
                    model()->setData(model()->index(ind.row(),1),true);
                    emit ((iItem*)itemDelegate())->WordWasPickedGlobal(model()->data(ind).toString(),model()->data(model()->index(ind.row(),1)).toBool());
                }
        });
        act = new QAction("Снять выделение",this);       table_menu->addAction(act);
        connect(act,&QAction::triggered, [=]()
        {
            foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                if (!isRowHidden(ind.row()) && ind.column()==2)
                {
                    model()->setData(model()->index(ind.row(),1),false);
                    emit ((iItem*)itemDelegate())->WordWasPickedGlobal(model()->data(ind).toString(),model()->data(model()->index(ind.row(),1)).toBool());
                }
        });
        if (type!=NEGATIVE_TABLE)
        {
            act = new QAction("Отправить в минус слова [-]",this);   table_menu->addAction(act);
            connect(act,&QAction::triggered, [=]()
            {
                if (currentIndex().column()==2)
                {
                    foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                        if (!isRowHidden(ind.row()) && ind.column()==2)
                        {
                            model()->setData(model()->index(ind.row(),1),true);
                            emit transfer(model()->data(ind).toString(),NEGATIVE_TABLE);
                        }
                }
            });
        }
        if (type!=POSITIVE_TABLE)
        {
            act = new QAction("Отправить в доп. слова [+]",this);   table_menu->addAction(act);
            connect(act,&QAction::triggered, [=]()
            {
                if (currentIndex().column()==2)
                {
                    foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                        if (!isRowHidden(ind.row()) && ind.column()==2)
                        {
                            model()->setData(model()->index(ind.row(),1),true);
                            emit ((iItem*)itemDelegate())->WordWasPickedGlobal(model()->data(ind).toString(),model()->data(ind).toBool());
                            emit transfer(model()->data(ind).toString(),POSITIVE_TABLE);
                        }
                }
            });
        }
        if (type!=AB_NEGATIVE_TABLE)
        {
            act = new QAction("Отправить в Запросы [-]",this);       table_menu->addAction(act);
            connect(act,&QAction::triggered, [=]()
            {
                if (currentIndex().column()==2)
                {
                    foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                        if (!isRowHidden(ind.row()) && ind.column()==2)
                        {
                            model()->setData(model()->index(ind.row(),1),true);
                            emit ((iItem*)itemDelegate())->WordWasPickedGlobal(model()->data(ind).toString(),model()->data(ind).toBool());
                            emit transfer(model()->data(ind).toString(),AB_NEGATIVE_TABLE);
                        }
                }
            });
        }
        if (type!=NEGATIVE_BD_TABLE)
        {
            act = new QAction("Отправить в БД [-]",this);      table_menu->addAction(act);
            connect(act,&QAction::triggered, [=]()
            {
                if (currentIndex().column()==2)
                {
                    foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                        if (!isRowHidden(ind.row()) && ind.column()==2)
                        {
                            model()->setData(model()->index(ind.row(),1),true);
                            emit ((iItem*)itemDelegate())->WordWasPickedGlobal(model()->data(ind).toString(),model()->data(ind).toBool());
                            emit transfer(model()->data(ind).toString(),NEGATIVE_BD_TABLE);
                        }
                }
            });
        }
        if (type!=POSITIVE_BD_TABLE)
        {
            act = new QAction("Отправить в БД [+]",this);       table_menu->addAction(act);
            connect(act,&QAction::triggered, [=]()
            {
                if (currentIndex().column()==2)
                {
                    foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                        if (!isRowHidden(ind.row()) && ind.column()==2)
                        {
                            model()->setData(model()->index(ind.row(),1),true);
                            emit ((iItem*)itemDelegate())->WordWasPickedGlobal(model()->data(ind).toString(),model()->data(ind).toBool());
                            emit transfer(model()->data(ind).toString(),POSITIVE_BD_TABLE);
                        }
                }
            });
        }
    }
    if (type!=DEFAULT_VIEW)
    connect(this,&QTableView::customContextMenuRequested,[=]()
    {
        if (currentIndex().column()==2)
            table_menu->popup(QCursor::pos());
    });

    horizontalHeader()->setSortIndicatorShown(true);
    if (type==DEFAULT_VIEW)
        connect(horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(tSort(int)));
    else
        connect(horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(iSort(int)));
    for (int i=0; i<7; i++)
        settings_tsort[i]=true;
    for (int i=0; i<4; i++)
        settings_isort[i]=true;

}

typedef QPair<int,int> ipair;
typedef QPair<int,QDateTime> dtpair;
typedef QPair<int,QString> spair;
bool compi(const ipair &a, const ipair &b) {
    return a.second > b.second;
}
bool compdt(const dtpair &a, const dtpair &b) {
    return a.second > b.second;
}
bool r = false;
bool comps(const spair &a, const spair &b) {
    /*if (r)
    {
        if (a.second.at(0)=='<')
            return true;
        else if (b.second.at(0)=='<')
            return false;
        else if (a.second.at(0)=='\"')
            return false;
        else
            return a.second > b.second;
    }
    else*/
        return a.second > b.second;
}

void MyView::tSort(int col)
{
    tModel* mod = (tModel*)model();
    switch (col)
    {
    case 1:
    {
        QList<spair> list;
        QList <tData> reserve;
        if (settings_tsort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            list<<spair(i,mod->data(mod->index(i,col),Qt::EditRole).toString());
            reserve<<*mod->data(i);
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                list<<spair(i,mod->data(mod->index(i,col),Qt::EditRole).toString());
                reserve<<*mod->data(i);
            }

        QTextDocument document;
        if (((tItem*)itemDelegate())->m_type==COLOR_LINE)
            for (int i=0; i<list.count(); i++)
                if (list.at(i).second.length()>0 && list.at(i).second.at(0)=='<')
                {
                    document.setHtml(list.at(i).second);
                    list[i].second = "!!"+document.toPlainText();
                }
        qSort(list.begin(), list.end(), comps);
        if (settings_tsort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            mod->setData(i,&reserve.at(list[i].first));
            for (int j=0; j<model()->columnCount(); j++)
                update(model()->index(i,j));
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                mod->setData(i,&reserve.at(list[i].first));
                for (int j=0; j<model()->columnCount(); j++)
                    update(model()->index(i,j));
            }
        break;
    }
    case 2:
    {
        QList<dtpair> list;
        QList <tData> reserve;
        if (settings_tsort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            list<<dtpair(i,mod->data(mod->index(i,col),Qt::EditRole).toDateTime());
            reserve<<*mod->data(i);
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                list<<dtpair(i,mod->data(mod->index(i,col),Qt::EditRole).toDateTime());
                reserve<<*mod->data(i);
            }
        qSort(list.begin(), list.end(), compdt);
        if (settings_tsort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            mod->setData(i,&reserve.at(list[i].first));
            for (int j=0; j<model()->columnCount(); j++)
                update(model()->index(i,j));
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                mod->setData(i,&reserve.at(list[i].first));
                for (int j=0; j<model()->columnCount(); j++)
                    update(model()->index(i,j));
            }
        break;
    }
    case 0:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    {
        QList<ipair> list;
        QList <tData> reserve;
        if (settings_tsort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            list<<ipair(i,mod->data(mod->index(i,col),Qt::EditRole).toInt());
            reserve<<*mod->data(i);
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                list<<ipair(i,mod->data(mod->index(i,col),Qt::EditRole).toInt());
                reserve<<*mod->data(mod->rowCount()-i-1);
            }
        qSort(list.begin(), list.end(), compi);
        if (settings_tsort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            mod->setData(i,&reserve.at(list[i].first));
            for (int j=0; j<model()->columnCount(); j++)
                update(model()->index(i,j));
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                mod->setData(i,&reserve.at(list[mod->rowCount()-i-1].first));
                for (int j=0; j<model()->columnCount(); j++)
                    update(model()->index(i,j));
            }
        break;
    }
    }
    settings_tsort[col]=!settings_tsort[col];
    horizontalHeader()->setSortIndicator(col,settings_tsort[col] ? Qt::AscendingOrder : Qt::DescendingOrder);
    emit ReloadFilter();
}

void MyView::iSort(int col)
{
    iModel* mod = (iModel*)model();
    switch (col)
    {
    case 2:
    {
        QList<spair> list;
        QList <iData> reserve;
        if (settings_isort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            list<<spair(i,mod->data(mod->index(i,col),Qt::EditRole).toString());
            reserve<<*mod->data(i);
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                list<<spair(i,mod->data(mod->index(i,col),Qt::EditRole).toString());
                reserve<<*mod->data(i);
            }
        qSort(list.begin(), list.end(), comps);
        if (settings_isort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            mod->setData(i,&reserve.at(list[i].first));
            for (int j=0; j<model()->columnCount(); j++)
                update(model()->index(i,j));
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                mod->setData(i,&reserve.at(list[i].first));
                for (int j=0; j<model()->columnCount(); j++)
                    update(model()->index(i,j));
            }
        break;
    }
    case 1:
    {
        QList<ipair> list;
        QList <iData> reserve;
        if (settings_isort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            list<<ipair(i,mod->data(mod->index(i,col),Qt::EditRole).toInt());
            reserve<<*mod->data(i);
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                list<<ipair(i,mod->data(mod->index(i,col),Qt::EditRole).toInt());
                reserve<<*mod->data(mod->rowCount()-i-1);
            }
        qSort(list.begin(), list.end(), compi);
        if (settings_isort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            mod->setData(i,&reserve.at(list[i].first));
            for (int j=0; j<model()->columnCount(); j++)
                update(model()->index(i,j));
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                mod->setData(i,&reserve.at(list[mod->rowCount()-i-1].first));
                for (int j=0; j<model()->columnCount(); j++)
                    update(model()->index(i,j));
            }
        break;
    }
    case 0:
    case 3:
    {
        QList<ipair> list;
        QList <iData> reserve;
        if (settings_isort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            list<<ipair(i,mod->data(mod->index(i,col),Qt::EditRole).toInt());
            reserve<<*mod->data(i);
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                list<<ipair(i,mod->data(mod->index(i,col),Qt::EditRole).toInt());
                reserve<<*mod->data(i);
            }
        qSort(list.begin(), list.end(), compi);
        if (settings_isort[col])
        for (int i=0; i<mod->rowCount(); i++)
        {
            mod->setData(i,&reserve.at(list[i].first));
            for (int j=0; j<model()->columnCount(); j++)
                update(model()->index(i,j));
        }
        else
            for (int i=mod->rowCount()-1; i>-1; i--)
            {
                mod->setData(i,&reserve.at(list[i].first));
                for (int j=0; j<model()->columnCount(); j++)
                    update(model()->index(i,j));
            }
        break;
    }
    }
    settings_isort[col]=!settings_isort[col];
    horizontalHeader()->setSortIndicator(col,settings_isort[col] ? Qt::AscendingOrder : Qt::DescendingOrder);
    emit ReloadFilter();
}

void MyView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_type!=DEFAULT_VIEW)
        if (currentIndex().column()==1)
            return;
    QTableView::mouseDoubleClickEvent(e);
}

void MyView::ResizeContext(int count)
{
    if (count==-1)
        count = model()->rowCount();
    int maxes[8];
    maxes[0]=5;
    maxes[1]=20;
    maxes[2]=18;
    maxes[3]=15;
    maxes[4]=16;
    maxes[5]=17;
    maxes[6]=12;
    maxes[7]=12;
    QTextDocument document;
    QString free_html_text;
    if (m_type==DEFAULT_VIEW)
    {
        for (int i=0; i<model()->rowCount(); i++)
        {
            document.setHtml(model()->data(model()->index(i,1)).toString());
            free_html_text = document.toPlainText();
            //if (model()->data(model()->index(i,1)).toString().length()>maxes[1])
            if (free_html_text.length()>maxes[1])
                maxes[1] = model()->data(model()->index(i,1)).toString().length();
        }
        if (QString::number(model()->rowCount()).length()+1>maxes[0])
            maxes[0] = QString::number(model()->rowCount()).length()+1;
        for (int i=0; i<8; i++)
            setColumnWidth(i,maxes[i]*9);
    }
    else
    {
        maxes[1]=12;
        maxes[2]=19;
        maxes[3]=17;
        for (int i=0; i<model()->rowCount(); i++)
        {
            if (model()->data(model()->index(i,2)).toString().length()>maxes[2])
                maxes[2] = model()->data(model()->index(i,2)).toString().length();
        }
        if (QString::number(model()->rowCount()).length()+1>maxes[0])
            maxes[0] = QString::number(model()->rowCount()).length()+1;
        for (int i=0; i<4; i++)
            setColumnWidth(i,maxes[i]*9);
    }
}

void MyView::SetBrowserInfo(QStringList names, QList<MyView *> views)
{
    if (browser_menu==NULL)
    {
        browser_menu= new QMenu();
        connect(this,&QTableView::customContextMenuRequested,[=]()
        {
            if (currentIndex().column()==1)
                browser_menu->popup(QCursor::pos());
        });

    }
    browser_menu->clear();
    QAction* act = new QAction("Отправить в Запросы [-]",this);       browser_menu->addAction(act);
    connect(act,&QAction::triggered, [=]()
    {
        foreach (QModelIndex ind, selectionModel()->selectedIndexes())
            if (!isRowHidden(ind.row()) && ind.column()==1)
                emit transfer(model()->data(ind).toString(),AB_NEGATIVE_TABLE);
    });
    for (int i=0; i<names.count(); i++)
    {
        act = new QAction(QString("Отправить в %1").arg(names[i]),this);
        browser_menu->addAction(act);
        connect(act,&QAction::triggered, [=]()
        {
            QList<int> rows_to_delete;
            QList<tData> rows_to_copy;
            foreach (QModelIndex ind, selectionModel()->selectedIndexes())
                if (!isRowHidden(ind.row()) && ind.column()==1)
                {
                    rows_to_delete.append(ind.row());
                    rows_to_copy.append(*((tModel*)model())->data(ind.row()));
                }
            ((tModel*)model())->SpecialRemoveRows(rows_to_delete,NULL);
            views[i]->model()->insertRows(views[i]->model()->rowCount(),rows_to_copy.count());
            for (int j=0; j<rows_to_copy.count(); j++)
                ((tModel*)views[i]->model())->setData(views[i]->model()->rowCount()-rows_to_copy.count()+j,&rows_to_copy[j]);
        });
    }
}
