#include "testmanager.h"
#include "ui_testmanager.h"

TestManager::TestManager(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TestManager)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"样品编号","样品类型","检测项目","检测方法","流转时间"});
    ui->tableView->addContextAction("认领任务",[this](){
        auto indexs=ui->tableView->selectedIndexes();
        if(!indexs.count()) return;
        QString sql;
        QJsonArray values;
        for(auto index:indexs){
            if(index.column()!=0) continue;
            QStringList ids=ui->tableView->cellFlag(index.row(),0).toString().split(",");
            for(auto id:ids){
                sql+="update task_parameters set testor=? where id=? and testor is null;";
                values.append(user()->name());
                values.append(id.toInt());
            }
        }
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"更新数据库时出错:",msg.errorMsg());
                sqlFinished();
                return;
            }
            sqlFinished();
        });
        waitForSql();
    });
}

TestManager::~TestManager()
{
    delete ui;
}

void TestManager::initCMD()
{
    QString sql;
    //显示已经流转待分配任务的样品
    sql="select B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " DATE_FORMAT(B.receiveTime, '%m-%d %H:%i') as T ,GROUP_CONCAT(A.id SEPARATOR ',')"
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "where A.testor =? and A.finishedtime is null "
          "group by B.sampleNumber,C.sampleType,M,T "
          "order by M , T;";
//    sql="select A.sampleNumber, C.sampleType ,GROUP_CONCAT(DISTINCT B.parameterName SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M,"
//          " DATE_FORMAT(A.receiveTime, '%m-%d %H:%i') as T ,GROUP_CONCAT(B.id SEPARATOR ',') "
//          "from sampling_info as A "
//          "left join task_parameters as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleOrder=B.sampleGroup and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
//          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
//          "left join type_methods as D on D.taskSheetID=B.taskSheetID and D.testTypeID=B.testTypeID and D.parameterID=B.parameterID "
//          "left join test_methods as E on D.testMethodID=E.id "
//          "where B.testor is null and A.receiveTime is not null and D.subpackage=0 "
//          "group by A.sampleNumber,C.sampleType,M,T "
//          "order by M , T;";
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
            ui->tableView->setCellFlag(i-1,0,r.at(i).toList().last());//记录下ID
        }
    };
    ui->pageCtrl->startSql(this,sql,1,{},f,50);
}

void TestManager::on_myTask_clicked()
{
    QString sql;
    //显示已经分配未完成的
    sql="select B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " DATE_FORMAT(B.receiveTime, '%m-%d %H:%i') as T ,GROUP_CONCAT(A.id SEPARATOR ',')"
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "where A.testor =? and A.finishedtime is null "
          "group by B.sampleNumber,C.sampleType,M,T "
          "order by M , T;";
//    sql="select A.sampleNumber, C.sampleType ,GROUP_CONCAT(B.parameterName SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M,"
//          "from sampling_info as A "
//          "left join task_parameters as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleOrder=B.sampleGroup "
//          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
//          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
//          "left join test_methods as E on D.testMethodID=E.id "
//          "where A.testor =? and A.finishedtime is null "
//          "group by B.sampleNumber,C.sampleType,M,T "
//          "order by M , T;";
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
            ui->tableView->setCellFlag(i-1,0,r.at(i).toList().last());//记录下ID
        }
    };
    ui->pageCtrl->startSql(this,sql,1,{},f,50);
}


void TestManager::on_onSamplingBtn_clicked()
{

}

