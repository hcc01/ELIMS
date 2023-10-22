#include "persnaldatamanagerui.h"
#include "qcryptographichash.h"
#include "ui_persnaldatamanagerui.h"

PersnalDataManagerUI::PersnalDataManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::PersnalDataManagerUI)
{
    ui->setupUi(this);
}

PersnalDataManagerUI::~PersnalDataManagerUI()
{
    delete ui;
}

void PersnalDataManagerUI::initMod()
{

}

void PersnalDataManagerUI::initCMD()
{
    QString sql;
    sql=QString("select phone, EducationDegree, Title from users where name='%1'").arg(user()->name());
    ui->nameEdit->setText(user()->name());
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"初始化时错误：",msg.result().toString());
            return;
        }
        QList<QVariant> r=msg.result().toList().at(1).toList();
        ui->phoneEdit->setText(r.first().toString());
        ui->EducationDegreeEdit->setText(r.at(1).toString());
        ui->TytleEdit->setText(r.at(2).toString());
    });

}

void PersnalDataManagerUI::on_informationOkbtn_clicked()
{
    QString sql;
    sql=QString("update users set phone=?,EducationDegree=?, Title=? where name=? ");
    QJsonArray values={ui->phoneEdit->text(),ui->EducationDegreeEdit->text(),ui->TytleEdit->text(),user()->name()};
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"修改信息时错误：",msg.result().toString());
            return;
        }
        QMessageBox::information(nullptr,"","信息更新成功！");
    },0,values);
}


void PersnalDataManagerUI::on_passwordOkbtn_clicked()
{
    QString sql;
    if(ui->newPwEdit->text().isEmpty()){
        QMessageBox::information(nullptr,"","新密码不能为空！");
        return;
    }
    if(ui->originPwEdit->text().isEmpty()){
        QMessageBox::information(nullptr,"","初始密码不能为空！");
        return;
    }
    if(ui->newPwEdit->text()!=ui->confirmPwEdit->text()){
        QMessageBox::information(nullptr,"","两次密码不一致！");
        return;
    }
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(ui->originPwEdit->text().toLocal8Bit());
    QString psword=QString(hash.result().toHex());
    sql=QString("select * from sys_employee_login where name=? and password=?;");
    QJsonArray values={user()->name(),psword};

    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"修改信息时错误：",msg.result().toString());
            return;
        }
        QList<QVariant> r=msg.result().toList();
        if(r.count()<2){
            QMessageBox::information(nullptr,"","初始密码错误！");
            return;
        }
        QString sql;
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(ui->newPwEdit->text().toLocal8Bit());
        QString psword=QString(hash.result().toHex());
        sql=QString("update sys_employee_login set password=? where name=? ");
        QJsonArray values={user()->name(),psword};
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"修改密码时错误：",msg.result().toString());
                return;
            }
            QMessageBox::information(nullptr,"","密码更新成功！");
        },0,values);

    },0,values);
}

