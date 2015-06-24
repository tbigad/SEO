#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QDialog>

namespace Ui {
class Authentication;
}

class Authentication : public QDialog
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = 0);
    ~Authentication();

private:
    Ui::Authentication *ui;
};

#endif // AUTHENTICATION_H
