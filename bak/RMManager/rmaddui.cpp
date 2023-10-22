#include "rmaddui.h"
#include "ui_rmaddui.h"
#include <QJsonObject>
#include<QMessageBox>
#include<QDebug>
RMAddUI::RMAddUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RMAddUI)
{
    ui->setupUi(this);
}

RMAddUI::~RMAddUI()
{
    delete ui;
}

void RMAddUI::on_btAdd_clicked()
{
    if(ui->lineEdit_conc->text().isEmpty()||ui->lineEdit_name->text().isEmpty()||ui->lineEdit_number->text().isEmpty()){
        QMessageBox::warning(this,"错误","带*的项目不能为空");
        return;
    }
    QString sql=QString("insert into rm_info(rm_number,name,lot_number,spec,validity,solvent,conc,unit,uncertainty,manufactor,mark,store_pos,amount,add_date) "
    "values ('%1','%2','%3','%4','%5','%6',%7,'%8','%9','%10','%11','%12',%13,'%14')").arg(ui->lineEdit_number->text()).arg(ui->lineEdit_name->text()).arg(ui->lineEdit_lotNumber->text())
            .arg(ui->lineEdit_spec->text()).arg(ui->dateEdit_validity->date().toString("yyyy-MM-dd")).arg(ui->lineEdit_solvent->text())
            .arg(ui->lineEdit_conc->doubleValue()).arg(ui->lineEdit_unit->text()).arg(ui->lineEdit_Uncertainty->text())
            .arg(ui->lineEdit_manufactor->text()).arg(ui->lineEdit_mark->text()).arg(ui->lineEdit_storePos->text())
            .arg(ui->spinBox_amount->value()).arg(QDate::currentDate().toString("yyyy-MM-dd"));
   // qDebug()<<sql;
//    return;
//    QJsonObject rmData;
//    rmData["rm_number"]=ui->lineEdit_number->text();
//    rmData["name"]=ui->lineEdit_name->text();
//    rmData["lot_number"]=ui->lineEdit_lotNumber->text();
//    rmData["spec"]=ui->lineEdit_spec->text();
//    rmData["validity"]=ui->dateEdit_validity->date().toString("yyyy-MM-dd");
//    rmData["solvent"]=ui->lineEdit_solvent->text();
//    rmData["conc"]=ui->lineEdit_conc->doubleValue();
//    rmData["unit"]=ui->lineEdit_unit->text();
//    rmData["uncertainty"]=ui->lineEdit_Uncertainty->text();
//    rmData["manufactor"]=ui->lineEdit_manufactor->text();
//    rmData["mark"]=ui->lineEdit_mark->text();
//    rmData["store_pos"]=ui->lineEdit_storePos->text();
//    rmData["amount"]=ui->spinBox_amount->value();
    emit doQuery(sql);
}

void RMAddUI::on_btFinished_clicked()
{
    accept();
}
