#include "addexecution.h"
#include "ui_addexecution.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QModelIndex>
#include <QBrush>
#include <QMap>

AddExecution::AddExecution(QWidget *parent,QSqlDatabase *db,QSqlRecord planRecord) :
        QDialog(parent),
        ui(new Ui::AddExecution)
{
    this->db=db;
    this->planRecord=planRecord;
    this->plansTestModel=NULL;
    ui->setupUi(this);
    connect(ui->addBtn,SIGNAL(clicked()),this,SLOT(addNext()));
    connect (ui->alreadyExecChk,SIGNAL(clicked()),this,SLOT(refresh()));
    connect (ui->id0Chk,SIGNAL(clicked()),this,SLOT(refresh()));
    refresh();
}


void AddExecution::refresh(){
    if (db->isOpen()){
        QString qryStr="select t.*,(select count(*) from plans_exec e where e.id_plan=tp.id_plan and e.id_req=tp.id_req and e.id_det=tp.id_det and e.id_test=tp.id_test) as count from tests as t inner join plans_tests tp on tp.id_test=t.id and tp.type=t.type and tp.id_req=t.id_req and tp.id_det=t.id_det where tp.id_plan=:idPlan";
        if (!ui->id0Chk->isChecked()){
            qryStr.append(" and t.id>0 ");
        }
        if (!ui->alreadyExecChk->isChecked()){
            qryStr.append(" group by t.type,t.id_req,t.id_det,t.id having count = 0 ");
        }
        QSqlQuery filterTest(qryStr);
        filterTest.bindValue(":idPlan",planRecord.value("id").toInt());
        filterTest.exec();
        ui->testTable->setModel(NULL);
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
        plansTestModel->setHeaderData(8,Qt::Horizontal,tr("Numero di esecuzioni"));
        ui->testTable->setModel(plansTestModel);

        QSqlQuery resultDistinct("Select DISTINCT(result) from plans_exec");
        ui->resultCmb->clear();
        resultDistinct.exec();
        while (resultDistinct.next()){
            ui->resultCmb->addItem(resultDistinct.value(0).toString());
        }
        ui->testTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    }
}


void AddExecution::addNext(){
    if (db->isOpen()){
        if (ui->testTable->currentIndex().isValid()){
            QSqlRecord currTest=plansTestModel->record(ui->testTable->currentIndex().row());

            QSqlQuery insExec("INSERT INTO plans_exec (id_plan,type,id_req,id_det,id_test,result,note,execution_date) values (:idPlan,:type,:idReq,:idDet,:idTest,:result,:note,:date)");
            insExec.bindValue(":idPlan",planRecord.value("id").toInt());
            insExec.bindValue(":type",currTest.value("type").toString());
            insExec.bindValue(":idReq",currTest.value("id_req").toInt());
            insExec.bindValue(":idDet",currTest.value("id_det").toInt());
            insExec.bindValue(":idTest",currTest.value("id").toInt());
            insExec.bindValue(":result",ui->resultCmb->currentText());
            insExec.bindValue(":note",ui->noteEdt->toPlainText());
            insExec.bindValue(":date",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            if (insExec.exec()){
                QMessageBox::information(this,tr("Inserimento Esecuzione"),tr("Esecuzione inserta con successo"));
                emit added();
                int crow=ui->testTable->currentIndex().row();
                refresh();
                ui->resultCmb->clear();
                ui->noteEdt->clear();
                if (ui->alreadyExecChk->isChecked()){
                    if (crow< plansTestModel->rowCount()-1){
                        ui->testTable->selectRow(crow+1);
                    }else{
                        ui->testTable->selectRow(crow);
                    }
                }else{
                    if (plansTestModel->rowCount()>0){
                        ui->testTable->selectRow(0);
                    }
                }
            }else{
                QMessageBox::critical(this,tr("Inserimento Esecuzione"),tr("Inserimento Esecuzione Fallita"));
            }
        }
    }
}

AddExecution::~AddExecution()
{
    delete ui;
}

void AddExecution::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
