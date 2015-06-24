#ifndef TITEM_H
#define TITEM_H

#include <QDebug>
#include <QDateTimeEdit>
#include <QItemDelegate>
#include <QTextEdit>
#include <QTableView>

class tModel;
struct tData
{
    int num=0;
    QString query;
    QStringList words;
    QDateTime date;
    int freq1=0;    bool bfreq1 = false;
    int freq2=0;    bool bfreq2 = false;
    int freq3=0;    bool bfreq3 = false;
    int sum1=0;
    int sum2=0;
    int WordsMinLength=50;
    QString SimpleQuery; ///Для интерпретаторов, все слова слиты
    QString free_html; ///Запрос без html
};

struct iData
{
    int num=0;
    bool checked=false;
    QString word;
    int count=1;
};

enum mLineType
{
    SIMPLE_LINE = 0,
    PROGRESSIVE_LINE,
    COLOR_LINE
};

enum mViewType
{
    DEFAULT_VIEW = 0,
    AB_NEGATIVE_TABLE,
    NEGATIVE_TABLE,
    POSITIVE_TABLE,
    NEGATIVE_BD_TABLE,
    POSITIVE_BD_TABLE
};

/// ##################################################################################################
/// My View
/// ##################################################################################################

class MyView: public QTableView
{
    Q_OBJECT
public:
    explicit MyView(mViewType type);
public slots:
    void tSort(int col);
    void iSort(int col);
    void ResizeContext(int count=-1);
    void SetBrowserInfo(QStringList names,QList<MyView*> views);
signals:
    void transfer(const QString item, mViewType to);
    void UpdateStates(const int count, int type);
    void FilterByWord(const QString);
    void ReloadFilter();
    void BrowserInfoHere();
    void GetBrowserData();
    void RowsDeleted(QList<int>);
    void RowsWasDeleted();
    void ClearTable(mViewType to);
protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
private:
    QStringList names;
    QList<MyView*> views;
    QMenu* browser_menu;
    mViewType m_type;
    bool settings_tsort[7];
    bool settings_isort[4];
};


/// ##################################################################################################
/// Items
/// ##################################################################################################

class tItem : public QItemDelegate
{
    Q_OBJECT
public:

    tItem (mLineType type, MyView* view) { m_type = type; m_view = view; }
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

signals:
    void WordWasDoubleSelected(const QString&);
    void WordWasPickedGlobal(const QString&);
    void WordWasUnpickedGlobal(const QString&);
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    mLineType m_type;
private:
    MyView* m_view;
};

class iItem : public QItemDelegate
{
    Q_OBJECT
public:
    iItem(){};
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

public slots:

signals:
    void WordWasPickedGlobal(const QString&,const bool);
    void WordWasEdited(const QString& word, const int row) const;
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class LineEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit LineEdit(mLineType type,QWidget *parent=0);

    bool m_picking;
    tModel* model;
    int row;
    QString html_text;

public slots:
    void GoogleSearch();

protected:
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

signals:
    void WordWasSelected(QString word);
    void WordWasDeSelected(QString word);
    void FocusLost(const QString& word);
    void WordWasDoubleClicked(const QString& word);

private:
    QMenu* m_menu;
    mLineType m_type;
};

#endif // TITEM_H
