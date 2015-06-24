#include "imodel.h"

bool iModel::loading = false;

iModel::iModel(QObject *parent) :
    QAbstractTableModel(parent), icount(0)
{
    header_data<<QString::fromUtf8("№")<<QString::fromUtf8("Выберите")<<QString::fromUtf8("Слова из запросов")<<QString::fromUtf8("Количество слов");
}

QVariant iModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole){
        if (index.column() == 0 )
            return list.at(index.row())->num;
        else if (index.column() == 1 )
            return list.at(index.row())->checked;
        else if (index.column() == 2 )
            return list.at(index.row())->word;
        else if (index.column() == 3 )
            return list.at(index.row())->count;
    }
    return QVariant();
}

iData* iModel::data(const int row) const
{
    return list.at(row);
}

bool iModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole && value.isValid())
        if(index.column()==0)
            list.at(index.row())->num = value.toInt();
        else if(index.column()==1)
            list.at(index.row())->checked = value.toBool();
        else if(index.column()==2)
        {
            QString val = value.toString().toLower();
            if (val.isEmpty())  return false;
            val.replace("\'","");
            val.replace("\"","");
            val.replace("\*","");
            list.at(index.row())->word = val;
        }
        else if(index.column()==3)
            list.at(index.row())->count = value.toInt();
        else
            return false;
    else return false;

    return true;
}

iData* iModel::find(const QString& word)
{
    foreach (iData* item, list)
        if (item->word==word)
            return item;
    return NULL;
}

int iModel::MakeUnique(QProgressBar* progress)
{
    QSet<QString> dset;
    QList<int> dlist;
    int count = 0;
    int row=0;
    progress->setValue(0);
    progress->setMaximum(list.count());
    QMap<QString, int> map;
    foreach (iData* item, list)
    {
        dset<<item->word;
        if (dset.count()==count)
        {
            if (item->checked)
                dlist<<map.value(item->word);
            else
                dlist<<row;
        }
        else map.insert(item->word,row);
        row++;
        count = dset.count();
        progress->setValue(progress->value()+1);
    }
    SpecialRemoveRows(dlist,progress);
    return dlist.count();
}

void iModel::setData(const int row, const iData *value)
{
    *list.at(row) = *value;
    list.at(row)->word = list.at(row)->word.toLower();
}

int iModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return list.size();
}

int iModel::columnCount(const QModelIndex &parent)    const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant iModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
           return QVariant();
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
       return header_data.at(section);
   return "  ";
}

Qt::ItemFlags iModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
        return Qt::ItemIsEnabled;
   if (index.column() == 0)
       return QAbstractItemModel::flags(index);
   return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool iModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent,row,row+count-1);
    for (int i=0; i<count; i++)
    {
        icount++;
        iData* it = new iData;
        it->num = icount;
        list.insert(row+i,it);
    }
    endInsertRows();
    return true;
}

void iModel::PickWords(const QString& word)
{
    int i=0;
    foreach (iData* item, list)
    {
        if (item->word==word)
        {
            item->checked = true;
            submit();
            emit RepaintIndex(index(i,1));
            break;
        }
        i++;
    }
}

void iModel::UnpickWords(const QString& word)
{
    int i=0;
    foreach (iData* item, list)
    {
        if (item->word==word)
        {
            item->checked = false;
            submit();
            emit RepaintIndex(index(i,1));
            break;
        }
        i++;
    }
}

bool iModel::AddWord(const QString& word,bool init)
{
    if (word.isEmpty())
        return false;
    int i=0;
    foreach (iData* item, list)
    {
        if (item->word==word)
        {
            item->count++;
            submit();
            emit RepaintIndex(index(i,3));
            return false;
        }
        i++;
    }
    if (!init)
    {
        insertRow(rowCount());
        setData(index(rowCount()-1,2),word,Qt::EditRole);
    }
    return true;
}

void iModel::SubWord(const QString& word)
{
    if (word.isEmpty())
        return;
    int i=0;
    foreach (iData* item, list)
    {
        if (item->word==word)
        {
            item->count--;
            submit();
            emit RepaintIndex(index(i,3));
            //if (item->count==0)
                //removeRows(i,1,QModelIndex());
            break;
        }
        i++;
    }
}

void iModel::Clear()
{
    if ((page_index==3 || page_index==4) && loading)
        return;
    removeRows(0,rowCount(),QModelIndex());
    icount=0;
}

bool iModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if ((page_index==3 || page_index==4) && loading)
        return true;
    beginRemoveRows(parent,row,row+count-1);
    list.erase(list.begin()+row,list.begin()+row+count);
    endRemoveRows();
    return true;
}

void iModel::SpecialRemoveRows(QList<int> &dlist, QProgressBar *progress)
{
    if ((page_index==3 || page_index==4) && loading)
        return;
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
            const QList<iData*>::iterator list_iter = list.begin() + *iter;
            list.erase(list_iter,list_iter+count);
            endRemoveRows();
            count=0;
        }
        if (progress!=NULL)
            progress->setValue(progress->value()+1);
    }
}

void iModel::setCurrentWords(QList<QString> words)
{
    m_cwords = words;
}

void iModel::removedRowsByProgram(const int count)
{
    icount-=count;
}
