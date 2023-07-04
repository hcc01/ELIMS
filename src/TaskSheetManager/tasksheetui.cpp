#include "tasksheetui.h"
#include "ui_tasksheetui.h"
#include"tasksheeteditor.h"
#include<QMessageBox>
TaskSheetUI::TaskSheetUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TaskSheetUI)
{
    ui->setupUi(this);
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

//    //任务单信息
//    sql+="CREATE TABLE IF NOT EXISTS test_task_info("
//           "taskNum varchar(32) primary key, "//任务单号
//           "contactNum varchar(32), "//合同编号
//           "clientID int"               //委托单位ID
//           "inspectedEentityID int, "   //受检单位ID
//           "inspectedProject  VARCHAR(255), "    //项目名称，
//           "projectAdds  VARCHAR(255), "         //项目地址
//           "taskStatus  VARCHAR(255), "          //任务状态（用于状态查询）
//           "testPeriod int, "           //检测周期
//           "sampleDisposal int, "           //留样约定
//           "reprotCopies int, "          //报告份数
//           "testMethodc int," //检测方法说明（来源)
//            "subpackageDesc VARCHAR(255),"//分包说明
//           "otherRequirements VARCHAR(255),"//其它要求
//           "remarks VARCHAR(255),"//备注
//           "FOREIGN KEY (clientID) REFERENCES client_info (id), "
//           "FOREIGN KEY (inspectedEentityID) REFERENCES client_info (id)"
//           ");";
//    //检测点位信息
//    sql+="CREATE TABLE IF NOT EXISTS sampling_site_info("
//           "id int AUTO_INCREMENT primary key, "
//           "taskNum varchar(32) , "//任务单号
//           "contactNum varchar(32), "//合同编号
//           "planSamplingSiteName varchar(32), "               //计划采样点位名称
//           "testTimes int, "   //检测次数
//           "testDays int, "    //检测天数，
//           "projectAdds  VARCHAR(255), "         //项目地址
//           "taskStatus  VARCHAR(255), "          //任务状态（用于状态查询）
//           "inletTo int, "           //如果是进口，此处保存对应的出口点位ID
//           "remarks VARCHAR(255),"//备注
//           //以上是合同评审时需要确认的信息（任务单下单时填写的信息）
//           //以下是现场采样时需要确认或记录的信息
//           "samplingSiteName varchar(32), "               //现场采样点位名称
//           "samplingPhoto1 BLOB,"
//           "samplingPhoto2 BLOB,"
//           "samplingPhoto3 BLOB,"
//           "samplingPhoto4 BLOB,"
//           "samplingPhoto5 BLOB,"//每个点位可以保存5张现场照片
//           "samplingSiteInfo JSON,"//每个点位的采样记录信息
//           "FOREIGN KEY (taskNum) REFERENCES test_task_info (taskNum), "
//           ");";
//    //点位检测项目信息
//    sql+="CREATE TABLE IF NOT EXISTS analyte_info("
//           "id  int AUTO_INCREMENT primary key, "//任务单号
//           "samplingSiteID int, "//合同编号
//           "clientID int"               //委托单位ID
//           "inspectedEentityID int, "   //受检单位ID
//           "inspectedProject  VARCHAR(255), "    //项目名称，
//           "projectAdds  VARCHAR(255), "         //项目地址
//           "taskStatus  VARCHAR(255), "          //任务状态（用于状态查询）
//           "testPeriod int, "           //检测周期
//           "sampleDisposal int, "           //留样约定
//           "reprotCopies int, "          //报告份数
//           "testMethodc int," //检测方法说明（来源)
//           "subpackageDesc VARCHAR(255),"//分包说明
//           "otherRequirements VARCHAR(255),"//其它要求
//           "remarks VARCHAR(255),"//备注
//           "FOREIGN KEY (clientID) REFERENCES client_info (id), "
//           "FOREIGN KEY (inspectedEentityID) REFERENCES client_info (id)"
//           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","初始化完成");
    });
}

void TaskSheetUI::on_newSheetBtn_clicked()
{
    TaskSheetEditor* sheet=new TaskSheetEditor(this);
    connect(sheet,&TaskSheetEditor::doSql,this,&TaskSheetUI::doSqlQuery);
    sheet->show();
    sheet->init();
}

