#ifndef ADDREQUIREMENT_H
#define ADDREQUIREMENT_H

#include <QDialog>
#include <QSqlDatabase>


namespace Ui {
    class AddRequirement;
}

class AddRequirement : public QDialog {
    Q_OBJECT
public:
    AddRequirement(QWidget *parent,QSqlDatabase *);
    ~AddRequirement();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AddRequirement *ui;
    QSqlDatabase *db;
private slots:
    void refreshType();
    void add();
signals:
    void added();
};

#endif // ADDREQUIREMENT_H
