#ifndef MODEL_H
#define MODEL_H

#include <QAbstractTableModel>
#include "titem.h"
#include <QProgressBar>

class tModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit tModel(bool editor, MyView* item, QObject *parent = 0);

    QList<tData *> list;
    QStringList header_data;

    void Init (const int row);
    QVariant data(const QModelIndex &index, int role) const;
    tData *data(const int row) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void setData(const int row, const tData *value);

    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void SetWords(int row, const QStringList &words, bool init=false);
    QStringList* Words(const int row) const;

    bool removeRows(int row, int count, const QModelIndex &parent);
    void SpecialRemoveRows(QList<int> &dlist, QProgressBar* progress);
    void removedRowsByProgram(const int count);
    void Clear();

    int MakeUnique(QProgressBar *progress);
    void FixICount(const int from, int count=-1);

signals:
    void RepaintIndex(const QModelIndex& row);
    void WordAdded(const QString& word,bool init);
    void WordDeleted(const QString& word);

public slots:
    void PickWords(const QString &word, bool=true);
    void PickQuery(const QString &word, bool=true);
    //void UnpickWords(const QString &word, bool=true);

public:
    bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());

private:
    MyView* m_view;
    int icount;
    bool is_editor;
};

#endif // MODEL_H
