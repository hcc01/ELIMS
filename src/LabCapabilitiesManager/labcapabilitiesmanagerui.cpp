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
    QString sql=  "create table IF NOT EXISTS test_field("
                  "id int AUTO_INCREMENT primary key, "
                  "testField varchar(32) unique,  "
                  "deleted TINYINT NOT NULL DEFAULT 0 );";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"test_field error",msg.result().toString());
            return;
        }
    });
    //检测类型
    sql=  "create table IF NOT EXISTS test_type("
                  "id int AUTO_INCREMENT primary key, "
                  "testFieldID int not null,  "
                  "testType varchar(32) unique, "
                  "deleted TINYINT NOT NULL DEFAULT 0 ,"
          "FOREIGN KEY (testFieldID) REFERENCES test_field (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"test_type error",msg.result().toString());
            return;
        }
    });
    //样品类型
    sql=  "create table IF NOT EXISTS sample_type ("
          "id int AUTO_INCREMENT primary key, "
          "testTypeID int not null,  "
          "sampleType varchar(32), "
          "deleted TINYINT NOT NULL DEFAULT 0 ,"
          "FOREIGN KEY (testTypeID) REFERENCES test_type (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"sample_type error",msg.result().toString());
            return;
        }
    });

    //检测参数表
    sql=  "CREATE TABLE IF NOT EXISTS detection_parameters("
                  "id INT NOT NULL AUTO_INCREMENT,"
                  "testFieldID int,"
                  " parameterName VARCHAR(255) NOT NULL,"
                  "additive TINYINT NOT NULL DEFAULT 0 ,"//是否为加和指标(包含子项目）
                  "PRIMARY KEY (id),"
                  " UNIQUE KEY unique_parameter_name(testFieldID,parameterName),"
                  " FOREIGN KEY (testFieldID) REFERENCES test_field(id) "
                  ");";
    sql+="CREATE TABLE IF NOT EXISTS detection_parameter_alias("
           "id INT NOT NULL AUTO_INCREMENT,"
           "parameterID int,"
           " alias VARCHAR(32) NOT NULL,"
           "PRIMARY KEY (id),"
           " UNIQUE KEY unique_parameter_name(parameterID,alias),"
           " FOREIGN KEY (parameterID) REFERENCES detection_parameters(id) "
           ");";
    sql+="CREATE TABLE IF NOT EXISTS detection_subparameters("
           "id INT NOT NULL AUTO_INCREMENT,"
           "parameterID int,"
           "subName VARCHAR(32) ,"
           "subparameterID int, "
           "PRIMARY KEY (id),"
           " UNIQUE KEY unique_parameter_name(parameterID,subName,subparameterID),"
           " FOREIGN KEY (parameterID) REFERENCES detection_parameters(id), "
           " FOREIGN KEY (subparameterID) REFERENCES detection_parameters(id) "
           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"创建检测参数表时错误",msg.result().toString());
            return;
        }
        qDebug()<<"检测项目表创建完成。";
    });
    //执行标准表
    sql=  "CREATE TABLE IF NOT EXISTS implementing_standards ("
          "id INT NOT NULL AUTO_INCREMENT,"
          "standardName VARCHAR(255)  NOT NULL,"
          " standardNum VARCHAR(32) NOT NULL,"
          "tableName VARCHAR(255) NOT NULL,"
          "testTypeID int NOT NULL,"
          "classNum VARCHAR(32) NOT NULL,"
          "PRIMARY KEY (id),"
          "deleted TINYINT NOT NULL DEFAULT 0, "
          " FOREIGN KEY (testTypeID) REFERENCES test_type(id),"
          "UNIQUE INDEX (standardNum,tableName, classNum) "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"implementing_standards error",msg.result().toString());
            return;
        }
    });
    //标准限值表
    sql=  "CREATE TABLE IF NOT EXISTS standard_limits("
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
            QMessageBox::information(this,"standard_limits error",msg.result().toString());
            return;
        }
    });
    //标准方法表
    sql=  "create table IF NOT EXISTS test_methods("
          "id int AUTO_INCREMENT primary key, "
          "methodName varchar(128) not null,  "//标准名称
          "methodNumber varchar(64) , "//标准编号
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
            QMessageBox::information(this,"create table test_methods error",msg.result().toString());
            return;
        }
    });
    //标准方法检测参数信息表
    sql=  "create table IF NOT EXISTS method_parameters ("
          "id int AUTO_INCREMENT primary key, "
          "methodID int not null,  "
          "parameterID int not null,  "
          "MDL double, "
          "unit varchar(8), "
          "LabMDL double,"
          "CMA TINYINT NOT NULL DEFAULT 0 ,"
          "deleted TINYINT NOT NULL DEFAULT 0 ,"
          "non_stdMethod TINYINT NOT NULL DEFAULT 0 ,"//非标方法
          "usedTimes int default 0,"
          "FOREIGN KEY (methodID) REFERENCES test_methods (id),"
          "UNIQUE (methodID,parameterID), "
          "FOREIGN KEY (parameterID) REFERENCES detection_parameters (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"create table method_parameters error",msg.result().toString());
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

