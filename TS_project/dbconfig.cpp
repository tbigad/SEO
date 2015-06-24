#include "dbconfig.h"
#include "ui_dbconfig.h"
#include <QtSql/QSqlDatabase>
#include "mainwindow.h"

dbconfig::dbconfig(bool* is_connected,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbconfig), ex_state(is_connected)
{
    ui->setupUi(this);
}

dbconfig::~dbconfig()
{
    delete ui;
}

void dbconfig::on_b_exit_clicked()
{
    close();
}

QList<std::pair<QString, QString> > dbconfig::GetSettings()
{
    QList<std::pair<QString, QString> > list;
    list<<std::make_pair("db_name",ui->db_name->text());
    list<<std::make_pair("db_username",ui->username->text());
    list<<std::make_pair("db_password",ui->password->text());
    list<<std::make_pair("db_hostname",ui->hostname->text());
    list<<std::make_pair("db_port",ui->port->text());
    return list;
}

void dbconfig::SetSettings(QList<SS_pair> list)
{
    foreach (SS_pair data, list)
    {
        if (data.first=="db_name")
            ui->db_name->setText(data.second);
        if (data.first=="db_username")
            ui->username->setText(data.second);
        if (data.first=="db_password")
            ui->password->setText(data.second);
        if (data.first=="db_hostname")
            ui->hostname->setText(data.second);
        if (data.first=="db_port")
            ui->port->setText(data.second);
    }
    on_b_connect_clicked();
}

void dbconfig::on_b_connect_clicked()
{
    ui->l_state->setText("");
    bool error = false;
    if (ui->db_name->text().isEmpty())
    {
        ui->l_db_name->setStyleSheet("QLabel { color : red; }");
        error = true;
    }
    else
        ui->l_db_name->setStyleSheet("QLabel { color : #3C3C3C; }");
    if (ui->username->text().isEmpty())
    {
        ui->l_username->setStyleSheet("QLabel { color : red; }");
        error = true;
    }
    else
        ui->l_username->setStyleSheet("QLabel { color : #3C3C3C; }");
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setDatabaseName(ui->db_name->text());
    db.setUserName(ui->username->text());
    if (!ui->password->text().isEmpty())
        db.setPassword(ui->password->text());
    if (!ui->hostname->text().isEmpty())
        db.setHostName(ui->hostname->text());
    if (!ui->port->text().isEmpty())
        db.setPort(ui->port->text().toInt());
    if (!db.open())
        error = true;
    *ex_state = !error;
    if (error)
    {
        ui->l_state->setText("Не найдена бд с такими параметрами!");
        return;
    }
    else
    {
        ui->l_state->setText("БД успешно подключена!");
        close();
    }
}
