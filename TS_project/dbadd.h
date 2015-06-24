#ifndef DBADD_H
#define DBADD_H

#include <QDialog>
typedef std::pair<QString,QString> SS_pair;

namespace Ui {
class dbadd;
}

class dbadd : public QDialog
{
    Q_OBJECT

public:
    explicit dbadd(QWidget *parent = 0);
    ~dbadd();

    QList<std::pair<QString, QString> > GetSettings();
    void SetSettings(QList<SS_pair> list);

private slots:
    void on_add_clicked();
    void on_cancel_clicked();

public:
    QString main_text;

private:
    Ui::dbadd *ui;
};

#endif // DBADD_H
