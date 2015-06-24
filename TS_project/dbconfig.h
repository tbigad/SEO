#ifndef DBCONFIG_H
#define DBCONFIG_H

#include <QDialog>
#include <QToolButton>
typedef std::pair<QString,QString> SS_pair;
namespace Ui {
class dbconfig;
}

class dbconfig : public QDialog
{
    Q_OBJECT

public:
    explicit dbconfig(bool *is_connected, QWidget *parent = 0);
    ~dbconfig();

    QList<std::pair<QString, QString> > GetSettings();
    void SetSettings(QList<SS_pair >);

private slots:
    void on_b_exit_clicked();
    void on_b_connect_clicked();

private:
    Ui::dbconfig *ui;
    bool* ex_state;
};

#endif // DBCONFIG_H
