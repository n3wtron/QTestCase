#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void refreshRequirements();
    void refreshTests();
    void refreshPlans();
    void refreshTestsPlan();
    void associateTest();
    void deAssociateTest();
    void refreshExec();
    void addPlan();
    void delPlan();
    void delExec();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel *requirementsModel;
    QSqlTableModel *testsModel;
    QSqlTableModel *plansModel;
    QSqlTableModel *allTestsModel;
    QSqlQueryModel *plansTestModel;
    QSqlTableModel *execModel;
    void initDb(QString);
    QString dbFileName;
private slots:
    void openAddRequirements();
    void deleteRequirement();
    void openAddTests();
    void openAddExec();
    void importRequirements();
    void exportRequirements();
    void exportExecs();
    void openCopyTests();
    void delTest();
    void openDb();
    void closeDb();
    void newDb();
    void refreshTitle();

    void importTests();
    void exportTests();

};

#endif // MAINWINDOW_H
