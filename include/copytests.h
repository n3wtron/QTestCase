#ifndef COPYTESTS_H
#define COPYTESTS_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QList>

namespace Ui {
    class CopyTests;
}

class CopyTests : public QDialog {
    Q_OBJECT
public:
    CopyTests(QWidget *parent, QSqlDatabase *db, QList<QSqlRecord> tests);
    ~CopyTests();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CopyTests *ui;
    QSqlDatabase *db;
    QList<QSqlRecord> tests;
    void refresh();
private slots:
    void copy();
};

#endif // COPYTESTS_H
