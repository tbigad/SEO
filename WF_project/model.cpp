#include "model.h"
#include <QHeaderView>

tModel::tModel(bool editor,MyView* item, QObject *parent) :
    QAbstractTableModel(parent), icount(0)
{
    header_data << QString::fromUtf8("№") << QString::fromUtf8("Ключевые запросы")<< QString::fromUtf8("Дата добавления")<<
                   QString::fromUtf8("Частотность[]")<<QString::fromUtf8("Частотность[\"\"]")<<QString::fromUtf8("Частотность[!\"\"]")<<
                   QString::fromUtf8("Сумма[\"\"]")<<QString::fromUtf8("Сумма[!\"\"]");
    is_editor = true;
    m_view = item;
}

QVariant tModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::BackgroundRole)
        return m_view->verticalHeader()->visualIndex((index.row()))%2==0 ? QBrush(Qt::gray) : QBrush(Qt::white);

    if (role == Qt::DisplayRole || role == Qt::EditRole){
        if (index.column() == 0 )
            return list.at(index.row())->num;
        else if (index.column() == 1 )
            return list.at(index.row())->query;
        else if (index.column() == 2 )
            return list.at(index.row())->date;
        else if (index.column() == 3 )
            return list.at(index.row())->bfreq1 ? list.at(index.row())->freq1 : QVariant();
        else if (index.column() == 4 )
            return list.at(index.row())->bfreq2 ? list.at(index.row())->freq2 : QVariant();
        else if (index.column() == 5 )
            return list.at(index.row())->bfreq3 ? list.at(index.row())->freq3 : QVariant();
        else if (index.column() == 6 )
            return list.at(index.row())->sum1;
        else if (index.column() == 7 )
            return list.at(index.row())->sum2;

    }
    return QVariant();
}

tData* tModel::data(const int row) const
{
    return list.at(row);
}

bool tModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole && value.isValid())
        if(index.column()==0)
            list.at(index.row())->num = value.toInt();
        else if(index.column()==1)
            list.at(index.row())->query = value.toString();
        else if(index.column()==2)
            list.at(index.row())->date = value.toDateTime();
        else if(index.column()==3)
        {
            list.at(index.row())->freq1 = value.toInt();
            list.at(index.row())->bfreq1 = true;
        }
        else if(index.column()==4)
        {
            list.at(index.row())->freq2 = value.toInt();
            list.at(index.row())->bfreq2 = true;
        }
        else if(index.column()==5)
        {
            list.at(index.row())->freq3 = value.toInt();
            list.at(index.row())->bfreq3 = true;
        }
        else if(index.column()==6)
            list.at(index.row())->sum1 = value.toInt();
        else if(index.column()==7)
            list.at(index.row())->sum2 = value.toInt();
        else
            return false;
    else return false;

    return true;
}

void tModel::setData(const int row, const tData *value)
{
    *list.at(row) = *value;
    list.at(row)->query = list.at(row)->query.toLower();
    Init(row);
}

void tModel::Init(const int row)
{
    if (list.count()>row)
    if (list.at(row))
    SetWords(row,list.at(row)->query.split(' '),true);
}

void tModel::SpecialRemoveRows(QList<int> &dlist, QProgressBar *progress)
{
    int count=0;
    if (progress!=NULL)
    {
        progress->setValue(0);
        progress->setMaximum(dlist.count());
    }
    for (QList<int>::iterator iter=dlist.end(); iter!=dlist.begin();)
    {
        iter--;
        count++;
        if (iter==dlist.begin() || *iter-1!=*(iter-1))
        {
            beginRemoveRows(QModelIndex(),*iter,*iter+count-1);
            const QList<tData*>::iterator list_iter = list.begin() + *iter;
            list.erase(list_iter,list_iter+count);
            endRemoveRows();
            count=0;
        }
        if (progress!=NULL)
            progress->setValue(progress->value()+1);
    }
}

void tModel::Clear()
{
    removeRows(0,rowCount(),QModelIndex());
    icount=0;
}

void tModel::removedRowsByProgram(const int count)
{
    icount-=count;
}

bool tModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent,row,row+count-1);
    list.erase(list.begin()+row,list.begin()+row+count);
    endRemoveRows();
    return true;
}

int tModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return list.size();
}

int tModel::columnCount(const QModelIndex &parent)    const
{
    Q_UNUSED(parent);
    return 8;
}

QVariant tModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
           return QVariant();
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
       return header_data.at(section);
   return "  ";
}

Qt::ItemFlags tModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
        return Qt::ItemIsEnabled;
   if (index.column() == 0)
       return QAbstractItemModel::flags(index);
   return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool tModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent,row,row+count-1);
    for (int i=0; i<count; i++)
    {
        icount++;
        tData* it = new tData;
        it->num = icount;
        it->date = QDateTime::currentDateTime();
        list.append(it);
    }
    endInsertRows();
}

void tModel::PickQuery(const QString& word, bool picked)
{
    int i=0;
    if (word.isEmpty())
        return;
    if (picked)
    foreach (tData* item, list)
    {
        if (item->free_html==word)
        {
            item->query = "<b><u>"+word+"</b></u>";
            emit RepaintIndex(index(i,1));
        }
        i++;
    }
    else
        foreach (tData* item, list)
        {
            if (item->free_html==word)
            {
                item->query = word;
                emit RepaintIndex(index(i,1));
            }
            i++;
        }
    submit();
}

void tModel::PickWords(const QString& word, bool picked)
{
    int i=0;
    if (word.isEmpty())
        return;
    if (picked)
    foreach (tData* item, list)
    {
        int fi = -1;
        while ((fi = item->query.indexOf(word[0],fi+1))!=-1)
            if ((fi==0 || item->query.at(fi-1)==' ') &&
                    item->query.mid(fi,word.length()) == word &&
                    (word.length()+fi==item->query.length() || item->query.at(word.length()+fi)==' '))
            {
                item->query.replace(fi,word.length(),"<b><u>" + word + "</b></u>");
                fi+=word.length();
                emit RepaintIndex(index(i,1));
            }
        i++;
    }
    else
        foreach (tData* item, list)
        {
            int fi = -1;
            while ((fi = item->query.indexOf(word[0],fi+1))!=-1)
                if (fi!=0 && item->query.at(fi-1)=='>' &&
                        item->query.mid(fi,word.length()) == word &&
                        item->query.at(word.length()+fi)=='<')
                {
                    item->query.replace(fi-6,word.length()+14,word);
                    fi+=word.length();
                    emit RepaintIndex(index(i,1));
                }
            i++;
        }
    submit();
}

int tModel::MakeUnique(QProgressBar* progress)
{
    QSet<QString> dset;
    QList<int> dlist;
    int count = 0;
    int row=0;
    progress->setValue(0);
    progress->setMaximum(list.count());
    foreach (tData* item, list)
    {
        dset<<item->free_html;
        if (dset.count()==count)
            dlist<<row;
        row++;
        count = dset.count();
        progress->setValue(progress->value()+1);
    }
    SpecialRemoveRows(dlist,progress);
    return dlist.count();
}

void tModel::SetWords(int row, const QStringList& words, bool init)
{
    if (!init)
    {
        foreach (QString word, words)
            if (!list.at(row)->words.contains(word))
                emit WordAdded(word,init);
        foreach (QString word, list.at(row)->words)
            if (!words.contains(word))
                emit WordDeleted(word);
    }
    QStringList twords;
    foreach (QString word, words)
    {
        word.replace("<b><u>","");
        twords<<word.replace("</b></u>","");
    }
    list.at(row)->words = twords;
    list.at(row)->words.sort();

    foreach (QString word, list.at(row)->words)
        if (word.isEmpty())
            list.at(row)->words.removeOne(word);
    if (list.at(row)->words.isEmpty())
        return;
    list.at(row)->query.replace("  "," ");
    while (!list.at(row)->query.isEmpty() && list.at(row)->query.at(list.at(row)->query.length()-1)==' ')
        list.at(row)->query.remove(list.at(row)->query.length()-1,1);
    list.at(row)->WordsMinLength = words.at(0).length();
    list.at(row)->SimpleQuery.clear();
    foreach (QString item,list.at(row)->words)
    {
        if (item.length()<list.at(row)->WordsMinLength)
            list.at(row)->WordsMinLength = item.length();
        list.at(row)->SimpleQuery+=item;
    }

    if (is_editor)
    {
        QTextDocument document;
        document.setHtml(list.at(row)->query);

        list.at(row)->free_html = document.toPlainText();//list.at(row)->query;
        /*list.at(row)->free_html.replace("</b></u>","");
        list.at(row)->free_html.replace("<b><u>","");*/
    }
    else
        list.at(row)->free_html = list.at(row)->query;
    QString reform_text, reform_tmp;
    int reform_from, reform_len;
    QStringList daughter;
    if (row<rowCount() && !init)
    for (int i=row+1, iter=1; i<rowCount(); i++, iter++)
    {
        bool f=true;
        daughter = list.at(i)->query.split(' ');
        foreach (QString word, list.at(row)->query.split(' '))
            if (!daughter.contains(word))
                f=false;
        if (!f)
            return;
        reform_text = list.at(i)->query;
        for (int biter=list.at(row)->query.split(' ').count()-1; biter>-1; biter--)
        {
            reform_from = reform_text.indexOf(list.at(row)->query.split(' ').at(biter));
            reform_len = list.at(row)->query.split(' ').at(biter).size();
            reform_tmp = reform_text.mid(reform_from,reform_len);
            reform_text.replace(reform_from,reform_len,"");
            reform_text.insert(0,QString("%1 ").arg(reform_tmp));
        }
        reform_text.replace("  "," ");
        list.at(i)->query = reform_text;
        emit RepaintIndex(index(i,1));
    }
}

QStringList *tModel::Words(const int row) const
{
    return &list.at(row)->words;
}

void tModel::FixICount(const int from, int count)
{
    if (count==-1)
        count = list.count();
    for (int i=from, step=1; i<count+from; i++, step++)
        list[i]->num = icount-count+step;
}
