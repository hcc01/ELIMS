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

    //检测参数表
    sql=  "CREATE TABLE detection_parameters ("
                  "id INT NOT NULL AUTO_INCREMENT,"
                  "testFieldID int,"
                  " parameter_name VARCHAR(255) NOT NULL,"
                  "uniqueMark VARCHAR(255),"
                  "alias VARCHAR(255),"
                  "abbreviation VARCHAR(50),"
                  "subparameter VARCHAR(255),"
                  "PRIMARY KEY (id),"
                  " UNIQUE KEY unique_parameter_name(testFieldID,parameter_name,uniqueMark),"
                  " FOREIGN KEY (testFieldID) REFERENCES test_field(id) "
                  ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //执行标准表
    sql=  "CREATE TABLE implementing_standards ("
          "id INT NOT NULL AUTO_INCREMENT,"
          "standardName VARCHAR(255)  NOT NULL,"
          " standardNum VARCHAR(32) NOT NULL,"
          "tableName VARCHAR(255) NOT NULL,"
          "testFieldID int NOT NULL,"
          "classNum VARCHAR(32) NOT NULL,"
          "PRIMARY KEY (id),"
          "deleted TINYINT NOT NULL DEFAULT 0, "
          " FOREIGN KEY (testTypeID) REFERENCES test_type(id),"
          "UNIQUE INDEX (standardNum,tableName, classNum) "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //标准限值表
    sql=  "CREATE TABLE standard_limits ("
          "id INT NOT NULL AUTO_INCREMENT,"
          "standardID int,"
          "parameterName VARCHAR(32)  NOT NULL,"
          " parameterID int,"
          "trade VARCHAR(255) DEFAULT '',"
          "higher double,"
          "lower double,"
          "unit varchar(16)  DEFAULT '',"
          "PRIMARY KEY (id),"
          " FOREIGN KEY (standardID) REFERENCES implementing_standards(id), "
          " FOREIGN KEY (parameterID) REFERENCES detection_parameters(id) "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //标准方法表
    sql=  "create table test_methods("
          "id int AUTO_INCREMENT primary key, "
          "methodName varchar(64) not null,  "//标准名称
          "methodNumber varchar(32) , "//标准编号
          "testFieldID int not null,"
          "coverage int not null, "//适用范围
          "testingMode int NOT NULL DEFAULT 0 , "//测试方式，0-实验室，1-现场，2-都可测（对于都可测的项目，将默认使用现场测试，并在合同评审时确认。）
          "minAmount int, "//最小样品量（用于水样采样体积的估算)
          "sampleGroup varchar(32) , "//样品分组，同一组的样品只采一个。
          "sampleMedium  varchar(32) ,"//采样介质
          "seriesConnection tinyint not null default 0, "//是否串联
          "mediumPrepare tinyint not null default 0, "//采样介质准备，是否需要实验室准备。
          "preservatives varchar(64) ,"//固定剂
          "storageCondition  varchar(32) ,"//保存条件
          "stability varchar(16), "//时效性
          "samplingFlow varchar(16), "//采样流量
          "samplingDuration varchar(16), "//采样时间
          "samplingRequirements varchar(255), "//采样要求
          "blankControl varchar(16),"//空白要求
          "parallelControl varchar(16),"//平行要求
          "curveCalibration varchar(16),"//曲线校核要求
          "spikeControl varchar(16),"//加标要求
          "version INT DEFAULT 1 ,"
          "UNIQUE (methodName,methodNumber,coverage)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //标准方法检测参数信息表
    sql=  "create table method_parameters("
          "id int AUTO_INCREMENT primary key, "
          "methodID int not null,  "
          "parameterID int not null,  "
          "MDL double, "
          "unit varchar(8), "
          "LabMDL double,"
          "CMA TINYINT NOT NULL DEFAULT 0 ,"
          "deleted TINYINT NOT NULL DEFAULT 0 ,"
          "usedTimes int default 0,"
          "FOREIGN KEY (methodID) REFERENCES test_methods (id),"
          "UNIQUE (methodID,parameterID), "
          "FOREIGN KEY (parameterID) REFERENCES detection_parameters (id)); ";
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

