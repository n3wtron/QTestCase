#ifndef ADDTEST_H
#define ADDTEST_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlTableModel>

namespace Ui {
    class AddTest;
}

class AddTest : public QDialog {
    Q_OBJECT
public:
    AddTest(QWidget *parent,QSqlDatabase *db, QSqlRecord *rec = NULL);
    ~AddTest();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AddTest *ui;
    QSqlDatabase *db;
    void refreshData();
private slots:
    void add();
    void setMaxId();
    void filterType();
    void filterReq();
signals:
    void added();
};

#endif // ADDTEST_H
