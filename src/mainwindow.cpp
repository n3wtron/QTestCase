#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlRecord>
#include "addrequirement.h"
#include "addtest.h"
#include "addexecution.h"
#include "copytests.h"
#include <QSqlQuery>
#include <QDebug>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QStringList>
#include <QList>
#include <QHeaderView>
#include <QProgressDialog>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    requirementsModel=NULL;
    testsModel=NULL;
    plansModel=NULL;
    allTestsModel=NULL;
    plansTestModel=NULL;
    execModel=NULL;
    ui->setupUi(this);

    connect(ui->action_New,SIGNAL(triggered()),this,SLOT(newDb()));
    connect(ui->action_Close,SIGNAL(triggered()),this,SLOT(closeDb()));
    connect(ui->action_Open,SIGNAL(triggered()),this,SLOT(openDb()));

    connect(ui->addRequirementsBtn,SIGNAL(clicked()),this,SLOT(openAddRequirements()));
    connect(ui->delRequirementBtn,SIGNAL(clicked()),this,SLOT(deleteRequirement()));

    connect(ui->actionAdd_Test,SIGNAL(triggered()),this,SLOT(openAddTests()));
    connect(ui->actionAdd_Requirement,SIGNAL(triggered()),this,SLOT(openAddRequirements()));
    connect(ui->delTestBtn,SIGNAL(clicked()),this, SLOT(delTest()));
    connect(ui->requirementsTable,SIGNAL(clicked(QModelIndex)),this,SLOT(refreshTests()));

    connect(ui->addPlanBtn,SIGNAL(clicked()),this,SLOT(addPlan()));
    connect(ui->actionAdd_Plan,SIGNAL(triggered()),this,SLOT(addPlan()));
    connect(ui->delPlanBtn,SIGNAL(clicked()),this,SLOT(delPlan()));

    connect(ui->plansTable,SIGNAL(clicked(QModelIndex)),this,SLOT(refreshTestsPlan()));
    connect(ui->addPlanTestBtn,SIGNAL(clicked()),this,SLOT(associateTest()));
    connect(ui->delPlanTestBtn,SIGNAL(clicked()),this,SLOT(deAssociateTest()));

    connect(ui->execBtn,SIGNAL(clicked()),this,SLOT(openAddExec()));
    connect(ui->delExecBtn,SIGNAL(clicked()),this,SLOT(delExec()));

    connect(ui->testFilterChk,SIGNAL(clicked()),this,SLOT(refreshTests()));
    connect(ui->actionCopy_Test,SIGNAL(triggered()),this,SLOT(openCopyTests()));

    connect(ui->refreshExecBtn,SIGNAL(clicked()),this,SLOT(refreshExec()));

    //CSV
    connect(ui->actionImport_Requirements,SIGNAL(triggered()),this,SLOT(importRequirements()));
    connect(ui->actionExport_Requirements,SIGNAL(triggered()),this,SLOT(exportRequirements()));
    connect(ui->actionImport_Test,SIGNAL(triggered()),this,SLOT(importTests()));
    connect(ui->actionExport_Test,SIGNAL(triggered()),this,SLOT(exportTests()));
    connect(ui->actionExport_Execs,SIGNAL(triggered()),this,SLOT(exportExecs()));

}

void MainWindow::initDb(QString fileName){
    db=QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);
    if (! db.open() ){
        QMessageBox::critical(this,tr("Connessione DB"),tr("Impossibile connettersi al db"));
        return ;
    }
    dbFileName=fileName;
    refreshTitle();
    refreshRequirements();
    refreshTests();
    refreshPlans();
}

void MainWindow::newDb(){
    QString fileName=QFileDialog::getSaveFileName(this,tr("Apri Database"),QDir::currentPath(),"sqlite (*.sqlite)");
    if (!fileName.isEmpty()){
        if(QFile::exists(fileName)){
            QFile::remove(fileName);
        }
        QFile baseDb(QApplication::applicationDirPath()+"/db/base.sqlite");
        if(!baseDb.copy(fileName)){
            QMessageBox::critical(this,tr("Nuovo Db"),tr("Impossibile creare il nuovo database"));
        }else{
            initDb(fileName);
        }
    }
}

void MainWindow::openDb(){
    if (db.isOpen()){
        int resp = QMessageBox::question(this,tr("Apri Database"),tr("Vuoi chiudere il database attuale?"),QMessageBox::Yes,QMessageBox::No);
        if (resp==QMessageBox::Yes){
            closeDb();
        }else{
            return;
        }
    }else{
        QString fileName=QFileDialog::getOpenFileName(this,tr("Apri Database"),QDir::currentPath(),"sqlite (*.sqlite)");
        if (!fileName.isEmpty()){
            initDb(fileName);
        }
    }
}

void MainWindow::closeDb(){
    if (db.isOpen()){
        db.close();
        dbFileName.clear();
        refreshTitle();
        refreshRequirements();
        refreshTests();
        refreshPlans();
    }
}

void MainWindow::refreshTitle(){
    if (db.isOpen()){
        setWindowTitle(QApplication::applicationName()+" "+dbFileName);
    }else{
        setWindowTitle(QApplication::applicationName());
    }
}

void MainWindow::refreshRequirements(){
    ui->requirementsTable->setModel(NULL);
    if (db.isOpen()){
        free(requirementsModel);
        requirementsModel = new QSqlTableModel();
        requirementsModel->setTable("requirements");
        requirementsModel->select();

        requirementsModel->setHeaderData(0,Qt::Horizontal,tr("Type"));
        requirementsModel->setHeaderData(1,Qt::Horizontal,tr("ID"));
        requirementsModel->setHeaderData(2,Qt::Horizontal,tr("ID_DET"));
        requirementsModel->setHeaderData(3,Qt::Horizontal,tr("Titolo"));
        requirementsModel->setHeaderData(4,Qt::Horizontal,tr("Dettaglio"));
        ui->requirementsTable->setModel(requirementsModel);
        ui->requirementsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    }
}

void MainWindow::openAddRequirements(){
    if (db.isOpen()){
        AddRequirement *win = new AddRequirement(this,&db);
        connect(win,SIGNAL(added()),this,SLOT(refreshRequirements()));
        win->show();
    }
}

void MainWindow::deleteRequirement(){
    if (ui->requirementsTable->currentIndex().isValid()){
        int res=QMessageBox::question(this,tr("Elimina Requisito"),tr("Si e' sicuri di voler cancellare il requisito?"),QMessageBox::Yes,QMessageBox::No);
        if (res==QMessageBox::Yes){
            QSqlRecord rec=requirementsModel->record(ui->requirementsTable->currentIndex().row());
            QSqlQuery delQry;

            delQry.prepare("delete from requirements where type=:type and id=:id and id_det=:id_det");
            delQry.bindValue(":type",rec.value("type").toString());
            delQry.bindValue(":id",rec.value("id").toInt());
            delQry.bindValue(":id_det",rec.value("id_det").toInt());
            if (delQry.exec()){
                QMessageBox::information(this,tr("Elimina Requisito"),tr("Requisito Cancellato con successo"));
                refreshRequirements();
            }else{
                QMessageBox::critical(this,tr("Elimina Requisito"),tr("Cancellazione Requisito FALLITA"));
            }
        }
    }
}

void MainWindow::refreshTests(){
    ui->testsTable->setModel(NULL);
    if (db.isOpen()){
        free(testsModel);
        testsModel = new QSqlTableModel();
        testsModel->setTable("tests");
        testsModel->select();
        testsModel->setHeaderData(0,Qt::Horizontal,tr("Type"));
        testsModel->setHeaderData(1,Qt::Horizontal,tr("ID_REQ"));
        testsModel->setHeaderData(2,Qt::Horizontal,tr("ID_DET"));
        testsModel->setHeaderData(3,Qt::Horizontal,tr("ID"));
        testsModel->setHeaderData(4,Qt::Horizontal,tr("Titolo"));
        testsModel->setHeaderData(5,Qt::Horizontal,tr("Input"));
        testsModel->setHeaderData(6,Qt::Horizontal,tr("Output"));
        testsModel->setHeaderData(7,Qt::Horizontal,tr("Note"));
        if (ui->requirementsTable->currentIndex().isValid() && ui->testFilterChk->isChecked()){
            QSqlRecord rec = requirementsModel->record(ui->requirementsTable->currentIndex().row());
            QString filter=QString("type='%1' and id_req=%2 and id_det=%3")
                           .arg(rec.value("type").toString())
                           .arg(rec.value("id").toInt())
                           .arg(rec.value("id_det").toInt());
            testsModel->setFilter(filter);
        }
        ui->testsTable->setModel(testsModel);
        ui->testsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    }
}

void MainWindow::openAddTests(){
    if (db.isOpen()){
        AddTest *win;
        if (ui->requirementsTable->currentIndex().isValid()){
            QSqlRecord rec=requirementsModel->record(ui->requirementsTable->currentIndex().row());
            win=new AddTest(this,&db,&rec);
        }else{
            win=new AddTest(this,&db,NULL);
        }
        connect(win,SIGNAL(added()),this,SLOT(refreshTests()));
        win->show();
    }
}

void MainWindow::delTest(){
    if (db.isOpen()){
        if (ui->testsTable->currentIndex().isValid()){
            int resp=QMessageBox::question(this,tr("Cancella Test"),tr("Cancellare il test?"),QMessageBox::Yes,QMessageBox::No);
            if (resp==QMessageBox::Yes){
                QSqlRecord rec=testsModel->record(ui->testsTable->currentIndex().row());
                QSqlQuery delTestQry("delete from tests where type=:type and id_req=:idReq and id_det=:idDet and id=:id");
                delTestQry.bindValue(":type",rec.value("type").toString());
                delTestQry.bindValue(":idReq",rec.value("id_req").toInt());
                delTestQry.bindValue(":idDet",rec.value("id_det").toInt());
                delTestQry.bindValue(":id",rec.value("id").toInt());
                if (delTestQry.exec()){
                    QMessageBox::information(this,tr("Cancella Test"),tr("Cancellazione avvenuta con successo"));
                    refreshTests();
                }else{
                    QMessageBox::critical(this,tr("Cancella Test"),tr("Cancellazione fallita %1").arg(delTestQry.lastError().text()));
                }
            }
        }
    }
}

void MainWindow::refreshPlans(){
    ui->plansTable->setModel(NULL);
    if (db.isOpen()){
        free(plansModel);
        plansModel = new QSqlTableModel();
        plansModel->setEditStrategy(QSqlTableModel::OnFieldChange);
        plansModel->setTable("plans");
        plansModel->select();
        plansModel->setHeaderData(0,Qt::Horizontal,tr("ID"));
        plansModel->setHeaderData(1,Qt::Horizontal,tr("Date"));
        plansModel->setHeaderData(2,Qt::Horizontal,tr("Approvato da"));
        plansModel->setHeaderData(3,Qt::Horizontal,tr("Approvato"));
        ui->plansTable->setModel(plansModel);
        ui->plansTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        refreshTestsPlan();
    }
}

void MainWindow::addPlan(){
    if (db.isOpen()){
        if (plansModel==NULL){
            refreshPlans();
        }
        if (plansModel!=NULL){
            plansModel->insertRow(plansModel->rowCount());
        }
    }
}

void MainWindow::delPlan(){
    if (db.isOpen()){
        if (ui->plansTable->currentIndex().isValid()){
            int resp=QMessageBox::question(this,tr("Elimina Piano"),tr("Vuoi cancellare il piano selezionato?"),QMessageBox::Yes,QMessageBox::No);
            if (resp==QMessageBox::Yes){
                QSqlQuery delQry("delete from plans where id=:id");
                delQry.bindValue(":id",plansModel->record(ui->plansTable->currentIndex().row()).value("id").toString());
                if(delQry.exec()){
                    QMessageBox::information(this,tr("Elimina Piano"),tr("Piano Eliminato con successo"));
                    refreshPlans();
                }else{
                    QMessageBox::critical(this,tr("Elimina Piano"),tr("Eliminazione piano fallita"));
                }
            }
        }
    }
}

void MainWindow::refreshTestsPlan(){
    ui->plansTestsTable->setModel(NULL);
    ui->allTestsTable->setModel(NULL);
    if (db.isOpen()){
        if (ui->plansTable->currentIndex().isValid()){
            QSqlRecord rec=plansModel->record(ui->plansTable->currentIndex().row());
            QSqlQuery filterTest("select t.* from tests as t inner join plans_tests tp on tp.id_test=t.id and tp.type=t.type and tp.id_req=t.id_req and tp.id_det=t.id_det where tp.id_plan=:idPlan");
            filterTest.bindValue(":idPlan",rec.value("id").toInt());
            filterTest.exec();
            free(plansTestModel);
            plansTestModel=new QSqlQueryModel();
            plansTestModel->setQuery(filterTest);
            plansTestModel->setHeaderData(0,Qt::Horizontal,tr("Type"));
            plansTestModel->setHeaderData(1,Qt::Horizontal,tr("ID_REQ"));
            plansTestModel->setHeaderData(2,Qt::Horizontal,tr("ID_DET"));
            plansTestModel->setHeaderData(3,Qt::Horizontal,tr("ID"));
            plansTestModel->setHeaderData(4,Qt::Horizontal,tr("Titolo"));
            plansTestModel->setHeaderData(5,Qt::Horizontal,tr("Input"));
            plansTestModel->setHeaderData(6,Qt::Horizontal,tr("Output"));
            plansTestModel->setHeaderData(7,Qt::Horizontal,tr("Note"));
            ui->plansTestsTable->setModel(plansTestModel);
            ui->plansTestsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

            //ALL TESTS
            free(allTestsModel);
            allTestsModel = new QSqlTableModel();
            allTestsModel->setTable("tests");
            allTestsModel->select();
            allTestsModel->setHeaderData(0,Qt::Horizontal,tr("Type"));
            allTestsModel->setHeaderData(1,Qt::Horizontal,tr("ID_REQ"));
            allTestsModel->setHeaderData(2,Qt::Horizontal,tr("ID_DET"));
            allTestsModel->setHeaderData(3,Qt::Horizontal,tr("ID"));
            allTestsModel->setHeaderData(4,Qt::Horizontal,tr("Titolo"));
            allTestsModel->setHeaderData(5,Qt::Horizontal,tr("Input"));
            allTestsModel->setHeaderData(6,Qt::Horizontal,tr("Output"));
            allTestsModel->setHeaderData(7,Qt::Horizontal,tr("Note"));
            ui->allTestsTable->setModel(allTestsModel);
            ui->allTestsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            //REFRESH EXECUTIONS
            refreshExec();
        }
    }
}

void MainWindow::associateTest(){
    if (db.isOpen()){
        if (!ui->allTestsTable->selectedIndexes().isEmpty() && ui->plansTable->currentIndex().isValid()){
            QSqlRecord planRec = plansModel->record(ui->plansTable->currentIndex().row());
            QSqlQuery assQry("insert into plans_tests (id_plan,type,id_req,id_det,id_test) values (:idPlan,:type,:idReq,:idDet,:idTest)");
            assQry.bindValue(":idPlan",planRec.value("id").toInt());
            QSqlQuery selQry("select count (*) from plans_tests where id_plan=:idPlan and type=:type and id_req=:idReq and id_det=:idDet and id_test=:idTest");
            selQry.bindValue(":idPlan",planRec.value("id").toInt());
            qDebug()<<ui->allTestsTable->selectedIndexes().size();
            int ncol=allTestsModel->columnCount();
            int crow=-1;
            QProgressDialog progrDialog(this);
            progrDialog.setMaximum(ui->allTestsTable->selectedIndexes().size()/ncol);
            progrDialog.setMinimum(0);
            progrDialog.show();
            db.transaction();
            for (int i=0;i<ui->allTestsTable->selectedIndexes().size()/ncol;i++){
                if (ui->allTestsTable->selectedIndexes().at(i).row()!=crow){
                    progrDialog.setValue(i);
                    crow=ui->allTestsTable->selectedIndexes().at(i).row();
                    QSqlRecord testRec = allTestsModel->record(crow);

                    assQry.bindValue(":type",testRec.value("type").toString());
                    assQry.bindValue(":idReq",testRec.value("id_req").toInt());
                    assQry.bindValue(":idDet",testRec.value("id_det").toInt());
                    assQry.bindValue(":idTest",testRec.value("id").toInt());

                    selQry.bindValue(":type",testRec.value("type").toString());
                    selQry.bindValue(":idReq",testRec.value("id_req").toInt());
                    selQry.bindValue(":idDet",testRec.value("id_det").toInt());
                    selQry.bindValue(":idTest",testRec.value("id").toInt());

                    selQry.exec();
                    selQry.first();
                    if (selQry.value(0).toInt()==1){
                        QMessageBox::critical(this,tr("Associazione Piano Test"),tr("gia presente"));
                    }else{
                        if (!assQry.exec()){
                            QMessageBox::critical(this,tr("Associazione Piano Test"),tr("Associazione Fallita %1").arg(assQry.lastError().text()));
                            qDebug()<<testRec;
                        }
                    }
                }
                db.commit();
            }
            progrDialog.close();
            refreshTestsPlan();
        }
    }
}

void MainWindow::deAssociateTest(){
    if (db.isOpen()){
        if (ui->plansTestsTable->selectedIndexes().size()>0 && ui->plansTable->currentIndex().isValid()){
            int ncol=plansTestModel->columnCount();
            int crow=-1;
            QSqlRecord planRec = plansModel->record(ui->plansTable->currentIndex().row());
            for (int i=0;i<ui->plansTestsTable->selectedIndexes().size()/ncol;i++){
                if (ui->plansTestsTable->selectedIndexes().at(i).row()!=crow){
                    crow=ui->plansTestsTable->selectedIndexes().at(i).row();
                    QSqlRecord testRec = plansTestModel->record(crow);
                    QSqlQuery delQry("delete from plans_tests where type=:type and id_plan=:idPlan and id_req=:idReq and id_det=:idDet and id_test=:idTest" );
                    delQry.bindValue(":type",testRec.value("type").toString());
                    delQry.bindValue(":idPlan",planRec.value("id").toInt());
                    delQry.bindValue(":idReq",testRec.value("id_req").toInt());
                    delQry.bindValue(":idDet",testRec.value("id_det").toInt());
                    delQry.bindValue(":idTest",testRec.value("id").toInt());

                    if(!delQry.exec()){
                        QMessageBox::critical(this,tr("DeAssociazione Piano Test"),tr("DeAssociazione Fallita %1").arg(delQry.lastError().text()));
                    }
                }
            }//fine for
            refreshTestsPlan();
        }
    }
}

void MainWindow::refreshExec(){
    if (db.isOpen()){
        if(ui->plansTable->currentIndex().isValid()){
            QSqlRecord planRec = plansModel->record(ui->plansTable->currentIndex().row());
            ui->execTable->setModel(NULL);
            free(execModel);
            execModel=new QSqlTableModel();
            execModel->setTable("plans_exec");
            execModel->select();
            execModel->setHeaderData(0,Qt::Horizontal,tr("ID"));
            execModel->setHeaderData(1,Qt::Horizontal,tr("ID_PLAN"));
            execModel->setHeaderData(2,Qt::Horizontal,tr("Type"));
            execModel->setHeaderData(3,Qt::Horizontal,tr("ID_REQ"));
            execModel->setHeaderData(4,Qt::Horizontal,tr("ID_DET"));
            execModel->setHeaderData(5,Qt::Horizontal,tr("ID_TEST"));
            execModel->setHeaderData(6,Qt::Horizontal,tr("Date"));
            execModel->setHeaderData(7,Qt::Horizontal,tr("Result"));
            execModel->setHeaderData(8,Qt::Horizontal,tr("Note"));
            ui->execTable->setModel(execModel);
            ui->execTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        }
    }
}

void MainWindow::openAddExec(){
    if (db.isOpen()){
        if(ui->plansTable->currentIndex().isValid()){
            QSqlRecord planRec = plansModel->record(ui->plansTable->currentIndex().row());
            AddExecution *win=new AddExecution(this,&db,planRec);
            win->setModal(true);
            win->show();
        }
    }
}

void MainWindow::delExec(){
    if (db.isOpen()){
        if(ui->execTable->currentIndex().isValid()){
            QSqlRecord execRecord=execModel->record(ui->execTable->currentIndex().row());
            int res=QMessageBox::question(this,tr("Eliminazione Esecuzione"),tr("Conferma Eliminazione esecuzione"),QMessageBox::Yes,QMessageBox::No);
            if (res==QMessageBox::Yes){
                QSqlQuery delQry("delete from plans_exec where id=:id");
                delQry.bindValue(":id",execRecord.value("id").toInt());
                if (delQry.exec()){
                    QMessageBox::information(this,tr("Eliminazione Esecuzione"),tr("Eliminazione esecuzione avvenuta con successo"));
                    refreshExec();
                }else{
                    QMessageBox::critical(this,tr("Eliminazione Esecuzione"),tr("Eliminazione execuzione Fallita"));
                }
            }
        }
    }
}

void MainWindow::importRequirements(){
    if (db.isOpen()){
        QString fileName=QFileDialog::getOpenFileName(this,tr("Apri file CSV dei requisiti"),QDir::currentPath(),QString("csv file(*.csv)"));
        if (!fileName.isEmpty()){
            QFile csvFile(fileName);
            QSqlQuery insQry("INSERT INTO requirements (type,id,id_det,title,detail) values (:type,:id,:idDet,:title,:detail)");
            if( csvFile.open(QIODevice::ReadOnly)){
                QStringList values;
                QString line;
                while (!csvFile.atEnd()){
                    line = csvFile.readLine();
                    values = line.split(";");
                    insQry.bindValue(":type",values.at(0));
                    insQry.bindValue(":id",((QString)values.at(1)).toInt());
                    insQry.bindValue(":idDet",((QString)values.at(2)).toInt());
                    insQry.bindValue(":title",values.at(3));
                    if (values.size()==5){
                        insQry.bindValue(":detail",(QString)values.at(4));
                    }else{
                        insQry.bindValue(":detail","");
                    }
                    if (!insQry.exec()){
                        QMessageBox::critical(this,tr("Import CSV"),tr("Errore SQL %1").arg(insQry.lastError().text()));
                        break;
                    }
                }
                csvFile.close();
                refreshRequirements();
            }else{
                QMessageBox::critical(this,tr("Apri file CSV dei requisiti"),tr("Impossibile aprire il file"));
            }
        }
    }
}

void MainWindow::exportRequirements(){
    if (db.isOpen()){
        QString fileName=QFileDialog::getSaveFileName(this,tr("Salva file CSV dei requisiti"),QDir::currentPath(),QString("csv file(*.csv)"));
        if (!fileName.isEmpty()){
            QFile csvFile(fileName);
            if(csvFile.open(QIODevice::WriteOnly)){
                QSqlQuery reqQry("SELECT type,id,id_det,title,detail from requirements order by type,id,id_det");
                reqQry.exec();
                QString line;
                while (reqQry.next()){
                    line=QString("%1;%2;%3;%4;%5\n")
                         .arg(reqQry.value(0).toString())
                         .arg(reqQry.value(1).toString())
                         .arg(reqQry.value(2).toString())
                         .arg(reqQry.value(3).toString().replace("\n"," "))
                         .arg(reqQry.value(4).toString().replace("\n"," "));
                    csvFile.write(line.toUtf8());
                }
                csvFile.close();
                QMessageBox::information(this,tr("Esporta requisiti"),tr("Esportazione completata"));
            }
            else{
                QMessageBox::critical(this,tr("Salva file CSV dei requisiti"),tr("Impossibile aprire il file"));
            }
        }
    }
}

void MainWindow::importTests(){
    if (db.isOpen()){
        QString fileName=QFileDialog::getOpenFileName(this,tr("Apri file CSV dei test case"),QDir::currentPath(),QString("csv file(*.csv)"));
        if (!fileName.isEmpty()){
            QFile csvFile(fileName);
            QSqlQuery insQry("INSERT INTO tests (type,id_req,id_det,id,title,input,output,note) values (:type,:idReq,:idDet,:id,:title,:input,:output,:note)");
            if( csvFile.open(QIODevice::ReadOnly)){
                QStringList values;
                QString line;
                while (!csvFile.atEnd()){
                    line = csvFile.readLine();
                    values = line.split(";");
                    if (values.size()>4){
                        insQry.bindValue(":type",values.at(0));
                        insQry.bindValue(":idReq",((QString)values.at(1)).toInt());
                        insQry.bindValue(":idDet",((QString)values.at(2)).toInt());
                        insQry.bindValue(":id",((QString)values.at(3)).toInt());
                        insQry.bindValue(":title",values.at(4));
                    }
                    if (values.size()>5){
                        insQry.bindValue(":input",(QString)values.at(5));
                    }else{
                        insQry.bindValue(":input","");
                    }
                    if (values.size()>6){
                        insQry.bindValue(":output",(QString)values.at(6));
                    }else{
                        insQry.bindValue(":output","");
                    }
                    if (values.size()>7){
                        insQry.bindValue(":note",(QString)values.at(7));
                    }else{
                        insQry.bindValue(":note","");
                    }
                    if (!insQry.exec()){
                        QMessageBox::critical(this,tr("Import CSV"),tr("Errore SQL %1 %2 ").arg(insQry.lastError().text()).arg(line));

                    }
                }
                csvFile.close();
                refreshRequirements();
            }else{
                QMessageBox::critical(this,tr("Apri file CSV dei test case"),tr("Impossibile aprire il file"));
            }
        }
    }
}

void MainWindow::exportTests(){
    if (db.isOpen()){
        QString fileName=QFileDialog::getSaveFileName(this,tr("Salva file CSV dei test case"),QDir::currentPath(),QString("csv file(*.csv)"));
        if (!fileName.isEmpty()){
            QFile csvFile(fileName);
            if(csvFile.open(QIODevice::WriteOnly)){
                QSqlQuery reqQry("SELECT type,id_req,id_det,id,title,input,output,note from tests order by type,id_req,id_det,id");
                reqQry.exec();
                QString line;
                while (reqQry.next()){
                    line=QString("%1;%2;%3;%4;%5;%6;%7;%8\n")
                         .arg(reqQry.value(0).toString())
                         .arg(reqQry.value(1).toString())
                         .arg(reqQry.value(2).toString())
                         .arg(reqQry.value(3).toString().replace("\n"," "))
                         .arg(reqQry.value(4).toString().replace("\n"," "))
                         .arg(reqQry.value(5).toString().replace("\n"," "))
                         .arg(reqQry.value(6).toString().replace("\n"," "))
                         .arg(reqQry.value(7).toString().replace("\n"," "));
                    csvFile.write(line.toUtf8());
                }
                csvFile.close();
                QMessageBox::information(this,tr("Esporta test"),tr("Esportazione completata"));
            }
            else{
                QMessageBox::critical(this,tr("Salva file CSV dei test case"),tr("Impossibile aprire il file"));
            }
        }
    }
}

void MainWindow::exportExecs(){
    if (db.isOpen()){
        if (ui->plansTable->currentIndex().isValid()){

            QString fileName=QFileDialog::getSaveFileName(this,tr("Salva file CSV delle esecuzioni"),QDir::currentPath(),QString("csv file(*.csv)"));
            if (!fileName.isEmpty()){
                QFile csvFile(fileName);
                if(csvFile.open(QIODevice::WriteOnly)){
                    QSqlRecord planRec = plansModel->record(ui->plansTable->currentIndex().row());
                    QSqlQuery reqQry("SELECT pe.type,pe.id_req,pe.id_det,pe.id_test,t.title,t.input,t.output,pe.result,pe.note from plans_exec pe inner join tests t on t.type=pe.type and t.id_req=pe.id_req and t.id_det=pe.id_det and t.id=pe.id_test where id_plan=:idPlan order by pe.type,pe.id_req,pe.id_det,pe.id_test");
                    reqQry.bindValue(":idPlan",planRec.value("id").toInt());
                    if(reqQry.exec()){
                        QString line;
                        while (reqQry.next()){
                            line=QString("%1.%2.%3;%4;%5;;%6;%7;%8;;%9\n")
                                 .arg(reqQry.value(0).toString())
                                 .arg(reqQry.value(1).toString())
                                 .arg(reqQry.value(2).toString())
                                 .arg(reqQry.value(3).toString().replace("\n"," "))
                                 .arg(reqQry.value(4).toString().replace("\n"," "))
                                 .arg(reqQry.value(5).toString().replace("\n"," "))
                                 .arg(reqQry.value(6).toString().replace("\n"," "))
                                 .arg(reqQry.value(7).toString().replace("\n"," "))
                                 .arg(reqQry.value(8).toString().replace("\n"," "));
                            csvFile.write(line.toUtf8());
                        }

                        csvFile.close();
                        QMessageBox::information(this,tr("Esporta esecuzioni"),tr("Esportazione completata"));
                    }else{
                        QMessageBox::critical(this,tr("Esporta esecuzioni"),tr("SQL Error: %1").arg(reqQry.lastError().text()));
                    }
                }
                else{
                    QMessageBox::critical(this,tr("Salva file CSV delle esecuzion"),tr("Impossibile aprire il file"));
                }
            }
        }
    }
}

void MainWindow::openCopyTests(){
    if (db.isOpen()){
        int ncol=testsModel->columnCount();
        int crow=-1;
        QList<QSqlRecord> tests;
        for (int i=0;i<ui->testsTable->selectedIndexes().size()/ncol;i++){
            if (ui->testsTable->selectedIndexes().at(i).row()!=crow){
                crow=ui->testsTable->selectedIndexes().at(i).row();
                tests.append(testsModel->record(crow));
            }
        }
        CopyTests *win=new CopyTests(this,&db,tests);
        win->setModal(true);
        win->show();
    }
}

MainWindow::~MainWindow()
{
    if (db.isOpen()){
        db.close();
    }
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
