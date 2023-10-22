#include "tasksheetui.h"
#include "ui_tasksheetui.h"
#include"tasksheeteditor.h"
#include<QMessageBox>
#include<qexcel.h>
TaskSheetUI::TaskSheetUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TaskSheetUI)
{
    ui->setupUi(this);
    ui->tableView->addContextAction("编辑",[this](){
        QString taskNum;
        int row=ui->tableView->selectedRow();
        if(row<0) return;
        taskNum=ui->tableView->value(row,0).toString();
        TaskSheetEditor* sheet=new TaskSheetEditor(this,this);
        sheet->init();
        sheet->load(taskNum);
        sheet->show();
    });
//    ui->tableView->addContextAction("导出EXCEL",[this](){
//        QString taskNum;
//        int row=ui->tableView->selectedRow();
//        if(row<0) return;
//        taskNum=ui->tableView->value(row,0).toString();
//    });
}

TaskSheetUI::~TaskSheetUI()
{
    delete ui;
}

void TaskSheetUI::dealProcess(const ProcessNoticeCMD &)
{

}

void TaskSheetUI::initMod()
{
    QString sql;
    //客户信息表，同时也是受检单位表
    sql="CREATE TABLE IF NOT EXISTS client_info("
           "id int AUTO_INCREMENT primary key, "
           "clientName varchar(32) NOT NULL, "
           "address varchar(32) NOT NULL, "
           "remark VARCHAR(255),"
           "deleted TINYINT NOT NULL DEFAULT 0); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //客户联系人表
    sql="CREATE TABLE IF NOT EXISTS client_contacts("
          "id int AUTO_INCREMENT primary key, "
          "clientID INT, "
          "name varchar(32) NOT NULL, "
          "phoneNum varchar(32), "
          "post varchar(32), "
          "eMail varchar(32), "
          "remark VARCHAR(255), "
          "deleted TINYINT NOT NULL DEFAULT 0,"
          "FOREIGN KEY (clientID) REFERENCES client_info (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });

    //客户历史点位表
    sql="CREATE TABLE IF NOT EXISTS inspected_site("
          "id int AUTO_INCREMENT primary key, "
          "clientID INT, "
          "name varchar(64) NOT NULL, "
          "testTypeID int, "
          "coordinate varchar(32), "
          "deleted TINYINT NOT NULL DEFAULT 0,"
          "FOREIGN KEY (testTypeID) REFERENCES test_type (id),"
          "FOREIGN KEY (clientID) REFERENCES client_info (id)); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });
    //任务单信息
    sql="CREATE TABLE IF NOT EXISTS test_task_info("
           "id int AUTO_INCREMENT primary key, "
           "taskNum varchar(32) unique not null, "//任务单号
           "contractNum varchar(32), "//合同编号
           "salesRepresentative int,"//业务员ID
           "clientID int,"               //委托单位ID
           "clientName VARCHAR(255),"
           "clientAddr VARCHAR(255),"
           "clientContact varchar(32), "
           "clientPhone varchar(32),"
           "inspectedEentityID int, "   //受检单位ID
           "inspectedEentityName VARCHAR(255),"
           "inspectedEentityContact VARCHAR(32),"
           "inspectedEentityPhone VARCHAR(32),"
           "inspectedProject  VARCHAR(255), "    //项目名称，
           "projectAddr  VARCHAR(255), "         //项目地址
           "reportPurpos varchar(32),"
           "reportPeriod int, "           //检测周期
           "sampleDisposal varchar(32), "           //留样约定
           "reprotCopies int, "          //报告份数
           "methodSource int," //检测方法说明（来源)
            "subpackage TINYINT NOT NULL DEFAULT 1,"//是否允许分包说明
           "otherRequirements VARCHAR(255),"//其它要求
           "remarks VARCHAR(255),"//备注
           "taskStatus  int, "          //任务状态（enum, 用于状态查询）
           "creator  varchar(32),"//创建人
          "createDate datetime, "//创建时间
          "FOREIGN KEY (clientID) REFERENCES client_info (id), "
           "FOREIGN KEY (inspectedEentityID) REFERENCES client_info (id)"
           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"test_task_info error",msg.result().toString());
            return;
        }
    });
    //任务单状态表，记录任务单的各个环节
    sql="CREATE TABLE IF NOT EXISTS task_status("
          "id int AUTO_INCREMENT primary key, "
          "taskSheetID int not null,"//任务单ID
           "taskStatus  int, "          //任务状态
          "operator VARCHAR(10), "//操作人员
          "operateTime datetime, "//操作时间
          "operateComment VARCHAR(255), "    //操作说明，
          "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id) "
          //           "FOREIGN KEY (limitValueID) REFERENCES standard_limits (id) "//放弃使用外键，自行控制。因为当执行标准为空时，无法传输空值
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"site_monitoring_info error",msg.result().toString());
            return;
        }
    });

    //点位监测项目信息
    sql="CREATE TABLE IF NOT EXISTS site_monitoring_info("
           "id int AUTO_INCREMENT primary key, "
           "taskSheetID int not null,"//任务单ID
           "testTypeID int not null , "//检测类型ID
           "testType varchar(32), "
          "sampleType varchar(32), "
          "samplingSiteName varchar(32), "//采样点位名称
           "samplingFrequency int not null default 1, "//监测频次
           "samplingPeriod int not null default 1, "    //监测周期，
           "limitValueID int, "         //限值ID
           "remark  VARCHAR(255), "          //备注
           "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id), "
           "FOREIGN KEY (testTypeID) REFERENCES test_type (id) "
//           "FOREIGN KEY (limitValueID) REFERENCES standard_limits (id) "//放弃使用外键，自行控制。因为当执行标准为空时，无法传输空值
           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"site_monitoring_info error",msg.result().toString());
            return;
        }
    });


    //方法评审表
    sql="CREATE TABLE IF NOT EXISTS task_methods("
           "id int AUTO_INCREMENT primary key, "//
           "monitoringInfoID int not null, "//监测信息ID
           "taskSheetID int not null,"//任务单ID
           "testTypeID int not null , "//检测类型ID
           "parameterID int not null, "               //检测参数ID
           "parameterName VARCHAR(16) not null, "   //检测参数名称
           "testMethodID  int , "    //检测方法ID，此部分往下为后续方法评审时保存数据
           "testMethodName  VARCHAR(100), "         //检测方法名称
           "fieldTesting  TINYINT NOT NULL DEFAULT 0, "          //是否现场测试
           "sampleGroup VARCHAR(32), "           //样品组
           "subpackage TINYINT NOT NULL DEFAULT 0, "          //是否分包
           "subpackageDesc VARCHAR(255),"//分包说明
            "CMA TINYINT NOT NULL DEFAULT 0,"//是否在资质范围内
           "FOREIGN KEY (monitoringInfoID) REFERENCES site_monitoring_info (id), "
           "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id), "
           "FOREIGN KEY (testTypeID) REFERENCES test_type (id), "
           "FOREIGN KEY (parameterID) REFERENCES detection_parameters (id), "
           "FOREIGN KEY (testMethodID) REFERENCES method_parameters (id)"
           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"task_methods error",msg.result().toString());
            return;
        }
    });
    //任务单自动编号计数表
    sql="CREATE TABLE IF NOT EXISTS tasknumber("
           "taskdate varchar(10) primary key, "//
           "taskcount int not null "//
           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"tasknumber error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","初始化完成");
    });
}

void TaskSheetUI::initCMD()
{
    ui->tableView->setHeader({"任务单号","录单员","业务员" ,"委托单位","受检单位","项目名称","当前状态"});

    QString sql=QString("SELECT taskNum, creator, salesRepresentative, clientName, inspectedEentityName, inspectedProject, taskStatus from test_task_info where creator='%1' ORDER BY createDate DESC;").arg(user()->name());
    if(user()->name()=="admin") sql="SELECT taskNum, creator, salesRepresentative, clientName, inspectedEentityName, inspectedProject, taskStatus from test_task_info ORDER BY createDate DESC;";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"获取任务单信息失败",msg.result().toString());
                return;
            }
            QList<QVariant> r=msg.result().toList();
            for(int i=1;i<r.count();i++){
                QList<QVariant>row=r.at(i).toList();
                ui->tableView->append(row);
            }
        },1);
}


void TaskSheetUI::on_newSheetBtn_clicked()
{
    TaskSheetEditor* sheet=new TaskSheetEditor(this);
//    connect(sheet,&TaskSheetEditor::doSql,this,&TaskSheetUI::doSqlQuery);
    sheet->show();
    sheet->init();
}

