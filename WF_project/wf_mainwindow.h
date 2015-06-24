#ifndef WF_MAINWINDOW_H
#define WF_MAINWINDOW_H

#include <QScrollArea>
#include <QMainWindow>
#include <QStatusBar>
#include <QProgressBar>
#include <QTableWidget>
#include <QLabel>
#include "titem.h"
#include "model.h"
#include "imodel.h"
#include <QSpinBox>
#include "ui_wf_mainwindow.h"
#include <QTreeWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QToolBar>
#include <QPushButton>

typedef QPair<int,int> di;
struct tree_item
{
    tree_item(QTreeWidgetItem* tnode,MyView* tview,QTreeWidgetItem* tparent)
    { node = tnode; view = tview; parent = tparent;  }
    friend bool operator ==(tree_item& a, const tree_item& b)
    {
        return a.node==b.node && a.view == b.view && a.parent == b.parent;
    }

    QTreeWidgetItem* node;
    MyView* view;
    QTreeWidgetItem* parent = NULL;
};
namespace Ui {
class wf_mainwindow;
}

class wf_mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit wf_mainwindow(QWidget *parent = 0);
    ~wf_mainwindow();

    enum wordsTables
    {
        AB_NEGATIVE_TABLE =0,
        NEGATIVE_TABLE,
        POSITIVE_TABLE,
        NEGATIVE_BD_TABLE,
        POSITIVE_BD_TABLE
    };

    int tab_counter;
    QTreeWidgetItem* current_tab;
    QList<tree_item> tree_list;
    bool m_is_first_page;

    QList<int> tab_counteres;
    QList<QList<tree_item> > tree_lists;
    QList<QTreeWidgetItem*> current_tabs;
    QList<bool> m_is_first_pages;

protected:
    void closeEvent(QCloseEvent *e);

public slots:

    QList<QString> MakeUniq(int index, QSet<QString> set);

    void ImportFromExcel();
    void ImportFromCSV();
    void ImportFromTXT();
    void ExportToExcel();
    void ExportToCSV();
    void ExportToTXT();

    void ImportWordsFromExcel();
    void ImportWordsFromCSV();
    void ImportWordsFromTXT();
    void ExportWordsToExcel();
    void ExportWordsToCSV();
    void ExportWordsToTXT();

    void DeleteWords(int index=-1, MyView* delete_from=NULL);

    void AddQueryes();
    void MakeInterpretation();
    void Compare();

    void BuildTree();
    void DeleteInterpretators();
    int DeleteClones(const int type, bool delete_byPr=false, int index=-1, MyView *delete_from=NULL);
    void Calculate();

    void UpdateStats(const int count, int type, int index=-1, int new_total=-1);
    void UpdateWords(int index=-1, MyView *update_from=NULL, QProgressBar* pr=NULL);

    void SearchWithFilters(int type, int ftype, int index=-1);

    QList<QTreeWidgetItem *> GetAllChildren(QTreeWidgetItem* item);

private slots:

    void AddTreeChild(QString name = QString());
    void AddTreeChildES(int index, QString name = QString());
    void on_tabWidget_tabBarClicked(int index);
    void on_save_pr_triggered();
    void on_tabWidget_2_tabBarClicked(int index);

    void s1();
    void s1_0();
    void s1_1();
    void s1_2();
    void s1_3();
    void s1_4();
    void s1_5();
    void s1_6();
    void s1_7();

    void s2();
    void s2_0();
    void s2_1();
    void s2_2();
    void s2_3();

    void on_open_pr_triggered();

private:

    bool first_toggle;
    inline QAction* GetRefreshButton(int index=-1)
    {
        if (index==-1)
            index = ui->tabWidget_2->currentIndex();
        QToolBar* bar = (QToolBar*)ui->tabWidget_2->widget(index)->layout()->itemAt(0)->widget();
        bar->actions().at(2)->setEnabled(false);
        return bar->actions()[2];
    }

    inline MyView* GetInnerEditor(int index)
    //{ return (MyView*)(((QTabWidget*)ui->tabWidget->widget(1)->layout()->itemAt(1)->widget())->widget(index)); }
    { foreach (tree_item item, tree_list)
            if (item.node==current_tab)
                return item.view;
    }
    inline MyView* GetTVEditor(int index)
    { if (index==1)// return (MyView*)ui->tabWidget->widget(index)->layout()->itemAt(1)->widget();

        {
            foreach (tree_item item, tree_list)
                if (item.node==current_tab)
                    return item.view;
        }
        else if (index>1)
        {
            foreach (tree_item item, tree_lists[index-2])
                if (item.node==current_tabs[index-2])
                    return item.view;
        }
        else
            return (MyView*)ui->tabWidget->widget(index)->layout()->itemAt(1)->widget();
    }

    inline MyView* GetTV2Editor(int index)
    { return (MyView*)ui->tabWidget_2->widget(index)->layout()->itemAt(1)->widget(); }

    inline QProgressBar* GetTVProgress(int index)
    { return (QProgressBar*)ui->tabWidget->widget(index)->layout()->itemAt(3)->layout()->itemAt(1)->widget(); }
    inline QProgressBar* GetTV2Progress(int index)
    { return (QProgressBar*)ui->tabWidget_2->widget(index)->layout()->itemAt(3)->layout()->itemAt(1)->widget(); }

    inline QLabel* GetTVLabel(int index)
    { return (QLabel*)ui->tabWidget->widget(index)->layout()->itemAt(3)->layout()->itemAt(0)->widget(); }
    inline QLabel* GetTV2Label(int index)
    { return (QLabel*)ui->tabWidget_2->widget(index)->layout()->itemAt(3)->layout()->itemAt(0)->widget(); }

    inline QLineEdit* GetTVLine(int index)
    { return (QLineEdit*)ui->tabWidget->widget(index)->layout()->itemAt(2)->layout()->itemAt(1)->layout()->itemAt(2)->widget(); }
    inline QLineEdit* GetTV2Line(int index)
    { return (QLineEdit*)ui->tabWidget->widget(index)->layout()->itemAt(2)->layout()->itemAt(1)->layout()->itemAt(2)->widget(); }

    QScrollArea* GetScroll(int type, int index)
    { return type==1 ? (QScrollArea*)ui->tabWidget->widget(index)->layout()->itemAt(2)->layout()->itemAt(0)->widget() : (QScrollArea*)ui->tabWidget_2->widget(index)->layout()->itemAt(2)->layout()->itemAt(0)->widget(); }

    QLayout* GetFilterLayout(int type,int index)
    { return ui->tabWidget->widget(index)->layout()->itemAt(2)->layout(); }

    void HideFilters(int type, int index=-1)
    {
        if (index==-1)  index= type==1 ? ui->tabWidget->currentIndex() : ui->tabWidget_2->currentIndex();
        if (type==1) for (int i=0; i<9;i++) for (int j=0; j<2; j++)
            {
                if (i<8)
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()->hide();
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()
                        ->setFixedWidth(GetTVEditor(index)->columnWidth(i));
                    GetScroll(type,index)->hide();
                }
                else
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(0)->widget()->hide();
                    GetScroll(type,index)->hide();
                }
            }
        if (type==2) for (int i=0; i<5;i++) for (int j=0; j<2; j++)
            if (i<4)
            {
                if (i!=1)
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()->hide();
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()
                        ->setFixedWidth(GetTV2Editor(index)->columnWidth(i));
                    GetScroll(type,index)->hide();
                }
                else
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(0)->widget()->hide();
                    GetScroll(type,index)->hide();
                }
            }
            else
            {
                GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(0)->widget()->hide();
                GetScroll(type,index)->hide();
            }
    }
    void ShowFilters(int type, int index=-1)
    {
        if (index==-1)  index= type==1 ? ui->tabWidget->currentIndex() : ui->tabWidget_2->currentIndex();
        if (type==1) for (int i=0; i<9;i++) for (int j=0; j<2; j++)
            {
                if (i<8)
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()->show();
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()
                        ->setFixedWidth(GetTVEditor(index)->columnWidth(i));
                    GetScroll(type,index)->show();
                }
                else
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(0)->widget()->show();
                    GetScroll(type,index)->show();
                }
            }
        if (type==2) for (int i=0; i<5;i++) for (int j=0; j<2; j++)
            if (i<4)
            {
                if (i!=1)
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()->show();
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(j)->widget()
                        ->setFixedWidth(GetTV2Editor(index)->columnWidth(i));
                    GetScroll(type,index)->show();
                }
                else
                {
                    GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(0)->widget()->show();
                    GetScroll(type,index)->show();
                }
            }
            else
            {
                GetScroll(type,index)->widget()->layout()->itemAt(i)->layout()->itemAt(0)->widget()->show();
                GetScroll(type,index)->show();
            }
    }

    inline QWidget* GetTVFilter(int index, int row=0, int col=0)
    { return GetScroll(1,index)->widget()->layout()->itemAt(col)->layout()->itemAt(row)->widget(); }
    inline QWidget* GetTV2Filter(int index, int row=0, int col=0)
    { return GetScroll(2,index)->widget()->layout()->itemAt(col)->layout()->itemAt(row)->widget(); }

    QList<MyView*> tmp_views;

    QTreeWidget* tree;
    QSplitter* tmp_box;
    QList<QTreeWidget*> trees;
    QList<QSplitter*> tmp_boxes;

    QList<tData*> cache;
    Ui::wf_mainwindow *ui;
};

#endif // WF_MAINWINDOW_H
