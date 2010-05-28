#include "include/addtest.h"
#include "ui_addtest.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

AddTest::AddTest(QWidget *parent,QSqlDatabase *db, QSqlRecord *rec) :
        QDialog(parent),
        ui(new Ui::AddTest)
{
    ui->setupUi(this);
    connect(ui->addBtn,SIGNAL(clicked()),this,SLOT(add()));

    this->db=db;
    refreshData();
    connect(ui->typeCmb,SIGNAL(currentIndexChanged(int)),this,SLOT(filterType()));
    connect(ui->reqCmb,SIGNAL(currentIndexChanged(int)),this,SLOT(filterReq()));
    connect(ui->detCmb,SIGNAL(currentIndexChanged(int)),this,SLOT(setMaxId()));
    if (rec!=NULL && db->isOpen()){
        ui->typeCmb->setCurrentIndex(ui->typeCmb->findText(rec->value("type").toString()));
        ui->reqCmb->setCurrentIndex(ui->reqCmb->findText(rec->value("id").toString()));
        ui->detCmb->setCurrentIndex(ui->detCmb->findText(rec->value("id_det").toString()));
    }

}

void AddTest::refreshData(){
    if (db->isOpen()){
        QSqlQuery refreshQry("select DISTINCT(type) as type from requirements",*this->db);
        refreshQry.exec();
        while (refreshQry.next()){
            ui->typeCmb->addItem(refreshQry.record().value("type").toString());
        }
        filterType();
        filterReq();
        setMaxId();
    }
}

void AddTest::setMaxId(){
    if (db->isOpen()){
        QSqlQuery maxId;
        maxId.prepare("select MAX(id) from tests where type=:type and id_req=:id_req and id_det=:id_det");
        maxId.bindValue(":type",ui->typeCmb->currentText());
        maxId.bindValue(":id_req",ui->reqCmb->currentText().toInt());
        maxId.bindValue(":id_det",ui->detCmb->currentText().toInt());
        maxId.exec();
        maxId.first();
        ui->idTestEdt->setValue(maxId.value(0).toInt()+1);
    }
}

void AddTest::filterType(){
    if (db->isOpen()){
        QSqlQuery reqsQry("select DISTINCT(id) from requirements where type=:type");
        reqsQry.bindValue(":type",ui->typeCmb->currentText());
        reqsQry.exec();
        ui->reqCmb->clear();
        while (reqsQry.next()){

            ui->reqCmb->addItem(reqsQry.value(0).toString());
        }
    }
}

void AddTest::filterReq(){
    if (db->isOpen()){
        QSqlQuery detsQry("select id_det from requirements where type=:type and id=:id");
        detsQry.bindValue(":type",ui->typeCmb->currentText());
        detsQry.bindValue(":id",ui->reqCmb->currentText().toInt());
        detsQry.exec();
        ui->detCmb->clear();
        while (detsQry.next()){
            ui->detCmb->addItem(detsQry.value(0).toString());
        }
    }
}

void AddTest::add(){
    if (db->isOpen()){
        QSqlQuery addQry;
        addQry.prepare("INSERT INTO tests (type,id_req,id_det,id,title,input,output,note) values (:type,:id_req,:id_det,:id,:title,:input,:output,:note)");
        addQry.bindValue(":type",ui->typeCmb->currentText());
        addQry.bindValue(":id_req",ui->reqCmb->currentText().toInt());
        addQry.bindValue(":id_det",ui->detCmb->currentText().toInt());
        addQry.bindValue(":id",ui->idTestEdt->value());
        addQry.bindValue(":title",ui->titleEdt->text());
        addQry.bindValue(":input",ui->inputEdt->toPlainText());
        addQry.bindValue(":output",ui->outputEdt->toPlainText());
        addQry.bindValue(":note",ui->noteEdt->toPlainText());
        if (addQry.exec()){
            QMessageBox::information(this,tr("Inserimento Test"),tr("Test inserito con successo"));
            ui->idTestEdt->setValue(ui->idTestEdt->value()+1);
            emit added();
        }else{
            QMessageBox::critical(this,tr("Inserimento Test"),tr("Inserimento test Fallito"));
        }
    }
}


AddTest::~AddTest()
{
    delete ui;
}

void AddTest::changeEvent(QEvent *e)
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
