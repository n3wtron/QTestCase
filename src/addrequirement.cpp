#include "addrequirement.h"
#include "ui_addrequirement.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>


AddRequirement::AddRequirement(QWidget *parent, QSqlDatabase *db) :
    QDialog(parent),
    ui(new Ui::AddRequirement)
{
    ui->setupUi(this);
    connect(ui->addBtn,SIGNAL(clicked()),this,SLOT(add()));
    this->db=db;
    refreshType();
}

void AddRequirement::refreshType(){
    if (db->isOpen()){
        QSqlQuery qry("select distinct(type) from requirements",*db);
        while (qry.next()){
            ui->cmbType->addItem(qry.value(0).toString());
        }
    }
}

void AddRequirement::add(){
    if (db->isOpen()){
        QSqlQuery addQry;
        addQry.prepare("INSERT INTO requirements (type,id,id_det,title,detail) VALUES(:type,:id,:id_det,:title,:detail)");
        addQry.bindValue(":type",ui->cmbType->currentText());
        addQry.bindValue(":id",ui->idEdt->value());
        addQry.bindValue(":id_det",ui->idDetEdt->value());
        addQry.bindValue(":title",ui->titleEdt->text());
        addQry.bindValue(":detail",ui->descEdt->toPlainText());
        if ( addQry.exec() ){
            QMessageBox::information(this,tr("Aggiungi Requisito"),tr("Inserimento requisito avvenuto con successo"));
            ui->idDetEdt->setValue(ui->idDetEdt->value()+1);
            refreshType();
            emit added();
        }else{
            QMessageBox::critical(this,tr("Aggiungi Requisito"),tr("Inserimento requisito in ERRORE %1").arg(addQry.lastError().text()));
        }
    }else{
            QMessageBox::critical(this,tr("Aggiungi Requisito"),tr("Database chiuso"));
    }
}

AddRequirement::~AddRequirement()
{
    delete ui;
}

void AddRequirement::changeEvent(QEvent *e)
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
