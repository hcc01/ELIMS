#include "testmanager.h"
#include "ui_testmanager.h"

TestManager::TestManager(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TestManager)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"样品编号","样品类型","检测项目","检测方法","流转时间"});
}

TestManager::~TestManager()
{
    delete ui;
}

void TestManager::initCMD()
{
    QString sql;
    //显示已经流转待分配任务的样品
    sql="select B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M, B.receiveTime "
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "where A.testor is null and B.receiveTime is not null and D.subpackage=0 "
          "group by B.sampleNumber,C.sampleType,M,B.receiveTime "
          "order by M , B.receiveTime;";
    DealFuc f=[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询分析任务时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        auto r=msg.result().toList();
        ui->tableView->clear();
        for(int i=1;i<r.count();i++){
            ui->tableView->append(r.at(i).toList());
        }
    };
    ui->pageCtrl->startSql(this,sql,1,{},f,50);
}
