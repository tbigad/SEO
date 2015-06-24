#include "dbadd.h"
#include "ui_dbadd.h"
#include <QtSql/QtSql>

dbadd::dbadd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbadd)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);
}

dbadd::~dbadd()
{
    delete ui;
}

QList<std::pair<QString, QString> > dbadd::GetSettings()
{
    QList<std::pair<QString, QString> > list;
    list<<std::make_pair("add_table_name",ui->table_name->text());
    list<<std::make_pair("add_table_id",ui->table_id->text());
    return list;
}

void dbadd::SetSettings(QList<SS_pair> list)
{
    foreach (SS_pair data, list)
    {
        if (data.first=="add_table_name")
            ui->table_name->setText(data.second);
        if (data.first=="add_table_id")
            ui->table_id->setValue(data.second.toInt());
    }
}

void dbadd::on_add_clicked()
{
    if (ui->table_name->text().isEmpty() ||  ui->table_id->text().isEmpty())
    {
        ui->state->setText("Заполните все поля.");
        return;
    }
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query (QString("select table_name from information_schema.tables where table_name ='%1' and table_schema='%2'")
                     .arg(ui->table_name->text())
                     .arg(db.databaseName()));
    query.next();
    if (query.value(0).isNull())
    {
        ui->state->setText("Таблицы не существует!");
        return;
    }
    query.exec(QString("SELECT * FROM %1 where id=%2").arg(ui->table_name->text()).arg(ui->table_id->value()));
    query.next();
    if (!query.value(0).isNull())
    {
        ui->state->setText("ID занят!");
        return;
    }
    query.prepare(QString("insert into %1 values(%2,'%3')")
               .arg(ui->table_name->text())
               .arg(ui->table_id->value())
               .arg(main_text));
    query.exec();

    ui->state->setText("Текст добавлен в бд!");
    close();
}

void dbadd::on_cancel_clicked()
{
    close();
}
