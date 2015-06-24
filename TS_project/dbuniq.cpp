#include "dbuniq.h"
#include "ui_dbuniq.h"
#include <QtSql/QtSql>
#include <QTextEdit>
#include <QWindow>
#include <QLayout>
dbuniq::dbuniq(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbuniq)
{
    ui->setupUi(this);
    ui->w_progress->hide();
    ui->edit_field->hide();
}

dbuniq::~dbuniq()
{
    delete ui;
}

QList<std::pair<QString, QString> > dbuniq::GetSettings()
{
    QList<std::pair<QString, QString> > list;
    list<<std::make_pair("uniq_shingl",ui->spin_length->text());
    list<<std::make_pair("uniq_table",ui->edit_tablename->text());
    list<<std::make_pair("uniq_search_type",ui->w_radi_number->isChecked() ? "0" : "1");
    list<<std::make_pair("uniq_column",ui->spin_field->text());
    list<<std::make_pair("uniq_field_name",ui->edit_field->text());
    return list;
}

void dbuniq::SetSettings(QList<SS_pair> list)
{
    foreach (SS_pair data, list)
    {
        if (data.first=="uniq_shingl")
            ui->spin_length->setValue(data.second.toInt());
        if (data.first=="uniq_table")
            ui->edit_tablename->setText(data.second);
        if (data.first=="uniq_search_type")
            if (data.second=="0")
            {
                ui->w_radi_number->setChecked(true);
                on_w_radi_number_clicked();
            }
            else
            {
                ui->w_radio_name->setChecked(true);
                on_w_radio_name_clicked();
            }
        if (data.first=="uniq_column")
            ui->spin_field->setValue(data.second.toInt());
        if (data.first=="uniq_field_name")
            ui->edit_field->setText(data.second);
    }
}

void dbuniq::on_b_do_clicked()
{
    if (ui->edit_tablename->text().isEmpty() || (ui->w_radio_name->isChecked() && ui->edit_field->text().isEmpty()))
    {
        ui->l_status->setText("Заполните все поля.");
        return;
    }
    QSqlQuery query ("SELECT * FROM " + ui->edit_tablename->text());
    query.next();
    if (!((ui->w_radi_number->isChecked() && query.value(ui->spin_field->value()).toString()!=QString::null) ||
            (ui->w_radio_name && query.record().indexOf(ui->edit_field->text())!=-1)))
    {
        ui->l_status->setText("Указанное поле/табл. не найдено.");
        return;
    }

    QList<QStringList> main_list = shingles(uniq_text,ui->spin_length->value());
    if (uniq_text.isEmpty() || main_list.isEmpty())
    {
        ui->l_status->setText("Нечего проверять на уникальность..");
        ui->w_progress->hide();
        return;
    }
    ui->l_status->setText("Делю на шинглы основной текст...");
    ui->w_progress->show();

    QList<QList<QStringList> > lists;
    int ind_text = 0;
    bool number_type = ui->w_radi_number->isChecked();
    ui->l_status->setText("Делю на шинглы основной текст...");
    do
    {
        ind_text++;
        ui->l_status->setText(QString("Делю текст %1 на шинглы...").arg(ind_text));
        number_type ? lists<<shingles(query.value(ui->spin_field->value()).toString(),ui->spin_length->value()) :
                      lists<<shingles(query.value(query.record().indexOf(ui->edit_field->text())).toString(),ui->spin_length->value());
    }
    while (query.next());
    ind_text = 0;
    ui->w_progress->setValue(0);
    ui->w_progress->setMaximum(lists.length());
    ui->l_status->setText("Проверяю на уникальность...");
    shingles_similar.clear();
    foreach (QList<QStringList> val, lists)
    {
        ui->w_progress->setValue(ui->w_progress->value()+1);
        if (val.isEmpty())
        {
            lists.removeOne(val);
            continue;
        }
        foreach (QStringList main_val, main_list)
            foreach (QStringList low_val, val)
                if (main_val == low_val)
                {
                    ind_text++;
                    if (!shingles_similar.contains(low_val))
                        shingles_similar << low_val;
                }
    }
    if (ind_text>0)
        ui->l_status->setText(QString("Найдено %1 совпадений.").arg(ind_text));
    else
        ui->l_status->setText("Не найдено совпадений. Текст уникален.");
    ui->w_progress->hide();
}

void dbuniq::on_b_exit_clicked()
{
    close();
    ui->l_status->setText("");
}

void dbuniq::on_w_radio_name_clicked()
{
    ui->spin_field->hide();
    ui->edit_field->show();
}

void dbuniq::on_w_radi_number_clicked()
{
    ui->spin_field->show();
    ui->edit_field->hide();
}
