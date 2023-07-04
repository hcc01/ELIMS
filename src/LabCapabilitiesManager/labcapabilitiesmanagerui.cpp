#include "labcapabilitiesmanagerui.h"
#include "ui_labcapabilitiesmanagerui.h"
#include<QMessageBox>
#include"testtypeeditor.h"
LabCapabilitiesManagerUI::LabCapabilitiesManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::LabCapabilitiesManagerUI)
{
    ui->setupUi(this);
    connect(&m_testTypeEdt,&TestTypeEditor::doSql,this,&TabWidgetBase::doSqlQuery);
}

LabCapabilitiesManagerUI::~LabCapabilitiesManagerUI()
{
    delete ui;
}

void LabCapabilitiesManagerUI::dealProcess(const ProcessNoticeCMD &)
{

}

void LabCapabilitiesManagerUI::initMod()
{
    //检测领域
    QString sql=  "create table test_field("
                  "id int AUTO_INCREMENT primary key, "
                  "testField varchar(32) unique,  "
                  "deleted TINYINT NOT NULL DEFAULT 0 );";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //检测类型
    sql=  "create table test_type("
                  "id int AUTO_INCREMENT primary key, "
                  "testFieldID int not null,  "
                  "testType varchar(32) unique, "
                  "deleted TINYINT NOT NULL DEFAULT 0 ,"
          "FOREIGN KEY (testFieldID) REFERENCES test_field (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //样品类型
    sql=  "create table sample_type("
          "id int AUTO_INCREMENT primary key, "
          "testTypeID int not null,  "
          "sampleType varchar(32) unique, "
          "deleted TINYINT NOT NULL DEFAULT 0 ,"
          "FOREIGN KEY (testTypeID) REFERENCES test_type (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
//    doSqlQuery(sql,[&](const QSqlReturnMsg& msg){
//        if(msg.error()){
//            qDebug()<<"error.";
//            QMessageBox::information(this,"error",msg.result().toString());
//        }
//        else{
//             QMessageBox::information(this,"","初始化建表成功。");
//        }
//               });
//    doSqlQuery(sql);
}

void LabCapabilitiesManagerUI::on_testTypeEditBtn_clicked()
{
    m_testTypeEdt.init();
    m_testTypeEdt.show();
}

