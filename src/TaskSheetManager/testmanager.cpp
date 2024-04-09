#include "testmanager.h"
#include "ui_testmanager.h"

TestManager::TestManager(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TestManager)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"项目名称","样品编号","样品类型","检测项目","检测方法","流转时间","样品数量"});
    ui->tableView->addContextAction("领样测试",[this](){
        on_startTestBtn_clicked();
    });
    ui->tableView->addContextAction("提交数据",[this](){
        on_submitBtn_clicked();
    });
}

TestManager::~TestManager()
{
    delete ui;
}

void TestManager::initCMD()
{
    QString sql;
    ui->tableView->setHeader({"项目名称","样品编号","样品类型","检测项目","检测方法","流转时间","样品数量"});
    //显示已经流转待分配任务的样品
    sql="select F.inspectedProject ,B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName order by A.id SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " DATE_FORMAT(B.receiveTime, '%m-%d %H:%i') as T ,GROUP_CONCAT(A.id SEPARATOR ',')"
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "left join test_task_info as F on A.taskSheetID=F.id "
          "where A.testor is null  and B.receiveTime is not null and D.subpackage=0  "
          "group by F.inspectedProject ,B.sampleNumber,C.sampleType,M,T "
          "order by M , T;";

    sql="select inspectedProject, SUBSTRING(sampleNumber, 1, 8) as num,sampleType , paras, M,DATE_FORMAT(receiveTime, '%m-%d') as T ,count(*),GROUP_CONCAT(ids SEPARATOR ',') "
          "from (select F.inspectedProject ,B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName order by A.id SEPARATOR '、') as paras, GROUP_CONCAT(A.id SEPARATOR ',') as ids,CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " B.receiveTime "
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "left join test_task_info as F on A.taskSheetID=F.id "
          "where A.testor is null  and B.receiveTime is not null and D.subpackage=0  "
          "group by F.inspectedProject ,B.sampleNumber,C.sampleType,M,B.receiveTime "
          "order by M , B.receiveTime) as X "
          "group by inspectedProject,  num,sampleType , paras, M, T;";
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
    ui->pageCtrl->startSql(this,sql,1,{user()->name()},f,50);
}

void TestManager::on_myTask_clicked()
{
    QString sql;
    ui->tableView->setHeader({"项目名称","样品编号","样品类型","检测项目","检测方法","流转时间","样品数量"});
    //显示已经分配未完成的
    sql="select F.inspectedProject ,B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName order by A.id SEPARATOR '、'), CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " DATE_FORMAT(B.receiveTime, '%m-%d %H:%i') as T ,GROUP_CONCAT(A.id SEPARATOR ',')"
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "left join test_task_info as F on A.taskSheetID=F.id "
          "where A.testor =? and A.finishedtime is null "
          "group by F.inspectedProject ,B.sampleNumber,C.sampleType,M,T "
          "order by M , T;";
    sql="select inspectedProject, SUBSTRING(sampleNumber, 1, 8) as num,sampleType , paras, M,DATE_FORMAT(receiveTime, '%m-%d') as T ,count(*),GROUP_CONCAT(ids SEPARATOR ',') "
          "from (select F.inspectedProject ,B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName order by A.id SEPARATOR '、') as paras, GROUP_CONCAT(A.id SEPARATOR ',') as ids,CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " B.receiveTime "
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "left join test_task_info as F on A.taskSheetID=F.id "
          "where A.testor =? and A.finishedtime is null "
          "group by F.inspectedProject ,B.sampleNumber,C.sampleType,M,B.receiveTime "
          "order by M , B.receiveTime) as X "
          "group by inspectedProject,  num,sampleType , paras, M, T;";
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
    ui->pageCtrl->startSql(this,sql,1,{user()->name()},f,50);
}


void TestManager::on_onSamplingBtn_clicked()
{
    QString sql;
    ui->tableView->setHeader({"项目名称","样品编号","样品类型","检测项目","检测方法","采样时间","样品数量"});
    sql="select inspectedProject, SUBSTRING(sampleNumber, 1, 8) as num,sampleType , paras, M,SUBSTRING(sampleNumber, 2, 6) as T ,count(*),GROUP_CONCAT(ids SEPARATOR ',') "
          "from (select F.inspectedProject ,B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName order by A.id SEPARATOR '、') as paras, GROUP_CONCAT(A.id SEPARATOR ',') as ids,CONCAT(E.methodName,' ', E.methodNumber) as M,"
          " B.receiveTime "
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_methods as E on D.testMethodID=E.id "
          "left join test_task_info as F on A.taskSheetID=F.id "
          "where B.sampleNumber is not null  and B.receiveTime is null and D.subpackage=0  "
          "group by F.inspectedProject ,B.sampleNumber,C.sampleType,M,B.receiveTime "
          ") as X "
          "group by inspectedProject,  num,sampleType , paras, M, T "
          "order by M , T;";
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


void TestManager::on_UnassignedBtn_clicked()
{
    initCMD();
}


void TestManager::on_AssignedBtn_clicked()
{
    QString sql;
    ui->tableView->setHeader({"项目名称","样品编号","样品类型","检测项目","责任人","流转时间","样品数量"});
    //显示已经流转待分配任务的样品
    sql="select inspectedProject, SUBSTRING(sampleNumber, 1, 8) as num,sampleType , paras, testor,DATE_FORMAT(receiveTime, '%m-%d') as T ,count(*),GROUP_CONCAT(ids SEPARATOR ',') "
          "from (select F.inspectedProject ,B.sampleNumber,C.sampleType ,GROUP_CONCAT(A.parameterName order by A.id SEPARATOR '、') as paras, GROUP_CONCAT(A.id SEPARATOR ',') as ids,D.testMethodID ,A.testor,"
          " B.receiveTime "
          "from task_parameters as A "
          "left join sampling_info as B on A.monitoringInfoID=B.monitoringInfoID and A.sampleGroup=B.sampleOrder and A.Period=B.samplingPeriod and A.Frequency=B.samplingRound "
          "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
          "left join type_methods as D on D.taskSheetID=A.taskSheetID and D.testTypeID=A.testTypeID and D.parameterID=A.parameterID "
          "left join test_task_info as F on A.taskSheetID=F.id "
          "where A.finishedtime is null  and A.testor is not null  "
          "group by F.inspectedProject ,B.sampleNumber,C.sampleType,B.receiveTime,D.testMethodID ,A.testor "
          ") as X "
          "group by inspectedProject,  num,sampleType , paras, testor, T"
          " order by T;";

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
    ui->pageCtrl->startSql(this,sql,1,{user()->name()},f,50);
}


void TestManager::on_startTestBtn_clicked()
{
        if(!ui->UnassignedBtn->isChecked()) return;
        auto indexs=ui->tableView->selectedIndexes();
        if(!indexs.count()) return;
        QString sql;
        QJsonArray values;
        for(auto index:indexs){
            if(index.column()!=0) continue;
            QStringList ids=ui->tableView->cellFlag(index.row(),0).toString().split(",");
            for(auto id:ids){
                sql+="update task_parameters set testor=? , startTime=now() where id=? and testor is null;";
                values.append(user()->name());
                values.append(id.toInt());
            }
        }
        connectDB(CMD_START_Transaction);
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                releaseDB(CMD_ROLLBACK_Transaction);
                QMessageBox::information(nullptr,"更新数据库时出错:",msg.errorMsg());
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,values);
        waitForSql();
        releaseDB(CMD_COMMIT_Transaction);
}


void TestManager::on_submitBtn_clicked()
{
        if(!ui->myTask->isChecked()) return;
        auto indexs=ui->tableView->selectedIndexes();
        if(!indexs.count()) return;
        QString sql;
        QJsonArray values;
        for(auto index:indexs){
            if(index.column()!=0) continue;
            QStringList ids=ui->tableView->cellFlag(index.row(),0).toString().split(",");
            for(auto id:ids){
                sql+="update task_parameters set finishedtime=now() where id=? ;";
                values.append(id.toInt());
            }
        }
        connectDB(CMD_START_Transaction);
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                releaseDB(CMD_ROLLBACK_Transaction);
                QMessageBox::information(nullptr,"更新数据库时出错:",msg.errorMsg());
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,values);
        waitForSql();
        releaseDB(CMD_COMMIT_Transaction);
}

