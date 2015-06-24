#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QToolButton>
#include <QToolBar>
#include <QProgressBar>
#include <QCheckBox>
#include <QSpinBox>

#include "dbconfig.h"
#include "dbuniq.h"
#include "dbadd.h"

#include <QTextCharFormat>
#include <QProcess>
#include <vector>
#include <string>
#include <QDebug>
#include "ui_mainwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    //Project
    void on_m_Open_task_triggered();
    void on_m_Save_r_as_triggered();
    void on_m_New_pr_triggered();
    bool on_m_Save_pr_as_triggered();
    void on_m_Open_pr_triggered();
    void on_m_Open_file_triggered();
    void on_m_Save_file_triggered();
    void switch_to_pr();

    //Visual
    void on_display_horiz_triggered();
    void on_m_hide_table_triggered();
    void on_m_Quit_triggered();
    void on_toolButton_clicked();
    void on_splitter_splitterMoved(int pos, int index);

    //Load//Save
    void console_end();
    void SaveSettings();
    void LoadSettings();

    //Programm
    void keyReleaseEvent(QKeyEvent *);
    void do_bold();
    void do_italic();
    void do_underline();
    void do_undo();
    void do_redo();
    void do_align();
    void do_align_left();
    void do_align_right();
    void do_spinbox();
    void do_html();
    void on_m_proccessing_triggered();
    void paint_shingles();

    // WIDGETS
    //TABLE
    void table_sort(int);
    void table_menu_event(const QPoint &);
    void table_menu_event_col();
    void table_menu_event_row();
    void on_tableWidget_cellChanged(int row, int column);
    void on_tableWidget_clicked(const QModelIndex &index);
    void add_row();
    void delete_row();
    void add_col();
    void delete_col();
    void add_word();
    void on_m_show_results_triggered();
    void on_m_show_task_triggered();

    //TEXT_EDIT
    void on_textEdit_textChanged();

    void on_m_stress_triggered();

private:
    Ui::MainWindow *ui;

    //Диалоги
    dbconfig* db_dialog;
    dbuniq* dbuniq_dialog;
    dbadd* db_add_dialog;
    //Проект
    bool is_pr_saved, is_job_done, show_results, off_cch, off_ct;
    QProcess* console;
    void delete_task_data();
    int russian_test(QString);

    void off_pr(bool changed=true);
    void refresh_ur(bool make_off=true);
    void clear_staks();

    struct oper_info
    {
        oper_info(int ind_f = 0,int row_f=0,int col_f=0, QString word_f="", QString new_word_f="",
                  QTextCharFormat format_f=QTextCharFormat(),
                  QTextBlockFormat block_format_f=QTextBlockFormat(),
                  QStringList str_list_f=QStringList()):
            ind(ind_f), row(row_f), col(col_f), word(word_f), new_word(new_word_f),
            format(format_f), block_format(block_format_f), str_list(str_list_f)
        {}
        int ind;
        int row;
        int col;
        QString word;
        QString new_word;
        QTextCharFormat format;
        QTextCharFormat format2;
        QTextBlockFormat block_format;
        QStringList str_list;
    };

    //DATABASE
    bool is_db_connected;
    //UNDO/REDO
    std::vector<oper_info> oper_stack;
    int oper_iter;
    //TABLE
    int find_in_data (QString word, const int add=0);
    std::vector<QString> cells_memory;
    int memory_iter;
    QString fake_memory;
    //TEXTEDIT
    QString fake_text;
    QString hand_text;
    int hand_cur;
    std::pair<QChar,QChar> fake_chars;
    //TOOLBAR
    QToolButton* bu_1;
    QToolButton* bu_2;
    QToolButton* bu_bold;
    QToolButton* bu_italic;
    QToolButton* bu_underline;
    QToolButton* bu_alignment;
    QToolButton* bu_alignment_left;
    QToolButton* bu_alignment_right;
    QToolButton* bu_html;
    QToolButton* bu_db_uniqueness;
    QToolButton* bu_db_connect;
    QToolButton* bu_db_add_dialog;
    QSpinBox* bu_spinbox;
    QToolBar* mtb;
    bool m_is_html_pressed;
    QString m_html_fake_text;

    //Панель состояния
    QLabel* barl;
    QProgressBar* progress;
    //Текстовый редактор
    int tpl;

    //Таблица
    QLabel** labels1;
    QLabel** labels2;
    QMenu* table_menu;
    QAction* act_add_word;
    QAction* act_add_row;
    QAction* act_add_col;
    QAction* act_delete_row;
    QAction* act_delete_col;
    int sort_order;
    void table_show_task();
    void table_show_results();

    //Данные
    std::vector<std::pair<int,int> > founded_words;
    unsigned int words_count;
    int* count_mas;
    int* count_mas2;
    QString pr_name;
    struct low_task
    {
        QString word="";
        int met;
        low_task* next;
        bool founded=false;
    };
    struct mini_task
    {
        QString main_word="";
        int met;
        int low_met;
        low_task* head;
    };
    mini_task** m_k;

    ///ограничение на запуск
    int lim;
};

#endif // MAINWINDOW_H
