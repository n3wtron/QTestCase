#include "copytests.h"
#include "ui_copytests.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

CopyTests::CopyTests(QWidget *parent, QSqlDatabase *db, QList<QSqlRecord> tests) :
        QDialog(parent),
        ui(new Ui::CopyTests)
{
    this->db=db;
    this->tests=tests;
    ui->setupUi(this);
    connect(ui->copyBtn,SIGNAL(clicked()),this,SLOT(copy()));
    refresh();
}


void CopyTests::refresh(){
    if (db->isOpen()){
        QSqlQuery typeQry("select distinct(type) from requirements");
        typeQry.exec();
        ui->cmbType->clear();

        while (typeQry.next()){
            ui->cmbType->addItem(typeQry.value(0).toString());
        }
    }
}

void CopyTests::copy(){
    if (db->isOpen()){
        QSqlQuery addQry;
        addQry.prepare("INSERT INTO tests (type,id_req,id_det,id,title,input,output,note) values (:type,:id_req,:id_det,:id,:title,:input,:output,:note)");
        addQry.bindValue(":type",ui->cmbType->currentText());
        addQry.bindValue(":id_req",ui->idReqEdt->value());
        addQry.bindValue(":id_det",ui->idDetEdt->value());
        QSqlRecord rec;
        for (int i=0;i<tests.size();i++){
            rec=tests.at(i);
            addQry.bindValue(":id",rec.value("id").toInt());
            addQry.bindValue(":title",rec.value("title").toString());
            addQry.bindValue(":input",rec.value("input").toString());
            addQry.bindValue(":output",rec.value("output").toString());
            addQry.bindValue(":note",rec.value("note").toString());
            if (!addQry.exec()){
                QMessageBox::critical(this,tr("Copia test"),tr("Copia fallita %1").arg(addQry.lastError().text()));
            }
        }
        QMessageBox::information(this,tr("Copia test"),tr("Copia Terminate"));
    }
}

CopyTests::~CopyTests()
{
    delete ui;
}

void CopyTests::changeEvent(QEvent *e)
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
