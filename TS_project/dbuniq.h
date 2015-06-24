#ifndef DBUNIQ_H
#define DBUNIQ_H

#include <QDialog>
typedef std::pair<QString,QString> SS_pair;
namespace Ui {
class dbuniq;
}

class dbuniq : public QDialog
{
    Q_OBJECT

public:
    explicit dbuniq(QWidget *parent = 0);
    ~dbuniq();

    QList<std::pair<QString, QString> > GetSettings();
    void SetSettings(QList<SS_pair> list);

private slots:
    void on_b_do_clicked();
    void on_b_exit_clicked();
    void on_w_radio_name_clicked();
    void on_w_radi_number_clicked();
    QList<QStringList> shingles(QString, int shi_count);    

public:
    QString uniq_text;
    QList<QStringList> shingles_similar;

private:
    Ui::dbuniq *ui;
};

#endif // DBUNIQ_H
