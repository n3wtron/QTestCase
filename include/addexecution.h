#ifndef ADDEXECUTION_H
#define ADDEXECUTION_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlDatabase>


namespace Ui {
    class AddExecution;
}

class AddExecution : public QDialog {
    Q_OBJECT
public:
    AddExecution(QWidget *,QSqlDatabase *,QSqlRecord planRecord);
    ~AddExecution();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AddExecution *ui;
    QSqlRecord planRecord;
    QSqlDatabase *db;
    QSqlQueryModel *plansTestModel;
private slots:
    void refresh();
    void addNext();
signals:
    void added();
};

#endif // ADDEXECUTION_H
