#include "samplecirculationui.h"
#include "tasksheetui.h"
#include "ui_samplecirculationui.h"

SampleCirculationUI::SampleCirculationUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::SampleCirculationUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"任务单号","委托单位","受检单位","项目名称"});
}

SampleCirculationUI::~SampleCirculationUI()
{
    delete ui;
}

void SampleCirculationUI::initCMD()
{
    QString sql;
    sql="select taskNum,clientName, inspectedEentityName, inspectedProject from test_task_info where creator=? and deleted!=1 and taskStatus=?;";
    ui->pageCtrl->startSql(this,sql,1,{user()->name(),TaskSheetUI::WAIT_SAMPLING},[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询任务单信息出错：",msg.errorMsg());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        ui->tableView->clear();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            ui->tableView->append(row);
        }
    });


}

void SampleCirculationUI::on_sampleReceiveBtn_clicked()
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return ;
    bool error=false;
    taskNum=ui->tableView->value(row,0).toString();
    //更新任务单状态
    doSqlQuery("UPDATE test_task_info set taskStatus=? where taskNum=?",[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"updateTaskStatus error",msg.result().toString());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{TaskSheetUI::TESTING,taskNum});
    waitForSql();
    if(error) return;
    //写入交接表
    doSqlQuery("insert into  sample_circulate (taskNum, receiveTime) values(?,now());",[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"写入交接表出错：",msg.result().toString());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{taskNum});
    waitForSql();
    if(error) return;
    initCMD();
}

