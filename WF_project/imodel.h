#ifndef IMODEL_H
#define IMODEL_H

#include <QAbstractTableModel>
#include <QProgressBar>
#include "titem.h"

class iModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit iModel(QObject *parent = 0);

    QList<iData *> list;
    QStringList header_data;

    QVariant data(const QModelIndex &index, int role) const;
    iData *data(const int row) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void setData(const int row, const iData *value);

    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setCurrentWords(QList<QString> words);

    bool removeRows(int row, int count, const QModelIndex &parent);
    void SpecialRemoveRows(QList<int>& dlist,QProgressBar* progress);
    void removedRowsByProgram(const int count);
    void Clear();

    int MakeUnique(QProgressBar *progress);

    iData* find(const QString& word);

    bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
    int page_index;
    static bool loading;
signals:
    void RepaintIndex(const QModelIndex&);

public slots:
    void SubWord(const QString &word);
    bool AddWord(const QString &word, bool init);
    void PickWords(const QString &word);
    void UnpickWords(const QString &word);

private:
    int icount;
    QList<QString> m_cwords;

};

#endif // MODEL_H
