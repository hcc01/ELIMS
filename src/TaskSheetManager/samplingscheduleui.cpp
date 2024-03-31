#include "samplingscheduleui.h"
#include "ui_samplingscheduleui.h"
#include"tasksheetui.h"
#include"cuser.h"
#include"tasksheetui.h"
#include"tasksheeteditor.h"
#include"samplegroupingdlg.h"
SamplingScheduleUI::SamplingScheduleUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::SamplingScheduleUI),
    m_reschedule(false)
{

    ui->setupUi(this);
    ui->taskToScheduledView->setHeader({"任务单号","项目名称","备注"});
    ui->taskScheduledView->setHeader({"任务单号","项目名称","计划开始时间","计划结束时间","带队","采样人员","备注","排单人"});
    ui->taskToScheduledView->addContextAction("排单",[this](){
        int row=ui->taskToScheduledView->selectedRow();
        if(row<0) return false;
        m_reschedule=false;
        if(!m_doScheduledlg){
            QMessageBox::information(nullptr,"error","m_doScheduledlg=0");
        }
        m_doScheduledlg->show();
    });
    ui->taskScheduledView->addContextAction("重新排单",[this](){
        int row=ui->taskScheduledView->selectedRow();
        if(row<0) return false;
        m_reschedule=true;
        m_doScheduledlg->show();
    });
    ui->taskScheduledView->addContextAction("打印标签",[this](){
        int row=ui->taskScheduledView->selectedRow();
        if(row<0) return ;
        SampleGroupingDlg dlg(this);
        dlg.init(ui->taskScheduledView->cellFlag(row,0).toInt(),(ui->taskScheduledView->value(row,4).toString()+"、"+ui->taskScheduledView->value(row,5).toString()).split("、"));
        dlg.exec();
    });

}

SamplingScheduleUI::~SamplingScheduleUI()
{
    delete ui;
}

void SamplingScheduleUI::initMod()
{
    QString sql;
    //创建排单表
    sql="CREATE TABLE IF NOT EXISTS samplingSchedul("
          "id int AUTO_INCREMENT primary key, "
          "taskNum varchar(16)  unique not null,"//任务单号
          "startDate  date, "          //
          "endDate date, "
            "actualStartDate  date,"//
            "actualEndDate date, "//
          "samplers varchar(255) ,"    //
          "samplerLeader varchar(32), "
           "scheduler varchar(32), "
          "schedulTime datetime, "
          "remark varchar(255), "
          "taskSheetID int ,"
          "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id) "
          ");";
    //创建采样任务序号表（用于标签）
    sql+="CREATE TABLE IF NOT EXISTS sampling_task_order("
           "id int AUTO_INCREMENT primary key, "
           "samplingDate date not null,"//采样日期
           "taskSheetID int, "
           "number int ,"
           "UNIQUE (samplingDate,number)"          //

           ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"tasknumber error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","初始化完成");
    });

}

void SamplingScheduleUI::initCMD()
{
    QString sql;


//    doSqlQuery(sql,f,1);
//    ui->pageCtrl->setSql(sql,1);
    sql="select name from users where position&?;";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新排单任务时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        m_allSamplers.clear();
        for(int i=1;i<r.count();i++){
            m_allSamplers.append(r.at(i).toList().at(0).toString());
        }

        m_doScheduledlg=new DoScheduleDlg(m_allSamplers,this);
        connect(m_doScheduledlg,&DoScheduleDlg::doSchedule,this,&SamplingScheduleUI::doSchedule);
        qDebug()<<"m_allSamplers"<<m_allSamplers;
        sqlFinished();
    },0,{CUser::Sampler});
    waitForSql();
    updateScheduledView();
    updateToScheduledView();
}

void SamplingScheduleUI::doSchedule(QString startDate, QString endDate, QString leader, QString samplers, QString remark)
{
    QString taskNum,sql;
    QJsonArray values;
    QModelIndexList indexs;
    if(!m_reschedule) indexs=ui->taskToScheduledView->selectedIndexes();
    else indexs=ui->taskScheduledView->selectedIndexes();
    bool ok=true;
    int taskSheetID;
    for(auto index:indexs){
        if(index.column()!=0) continue;
        if(!m_reschedule) {
            taskNum=ui->taskToScheduledView->value(index).toString();
            taskSheetID=ui->taskToScheduledView->cellFlag(index.row(),0).toInt();
        }
        else{

            taskNum=ui->taskScheduledView->value(index).toString();
            taskSheetID=ui->taskScheduledView->cellFlag(index.row(),0).toInt();
        }

        if(!m_reschedule) {
            sql="update test_task_info set taskStatus=? where id=?;insert into samplingSchedul(taskNum,startDate,endDate,samplerLeader,samplers,remark,schedulTime,scheduler,taskSheetID)"
              "values(?,?,?,?,?,?,now(),?,?);";
            values={TaskSheetUI::WAIT_SAMPLING,taskSheetID,taskNum,startDate,endDate,leader,samplers,remark,user()->name(),taskSheetID};
        }
        else{
            sql="update samplingSchedul set startDate=?,endDate=?,samplerLeader=?,samplers=?,remark=?,schedulTime=NOW(),scheduler=? where taskSheetID=?;";
            values={startDate,endDate,leader,samplers,remark,user()->name(),taskSheetID};
        }
        doSqlQuery(sql,[this, &ok](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"写入数据时出错：",msg.errorMsg());
                    sqlFinished();
                    ok=false;
                    return;
                }
                sqlFinished();
            },0,values);
        if(!ok) return;
    }
    updateScheduledView();
    updateToScheduledView();
}

void SamplingScheduleUI::updateScheduledView()
{
    QString sql;
    sql="select A.taskNum, inspectedProject, startDate, endDate, samplerLeader, samplers, A.remark,scheduler ,A.taskSheetID from "
          "samplingSchedul as A left join test_task_info as B on  A.taskSheetID=B.id where B.deleted=0 and B.taskStatus<? order by startDate ;";
    ui->pageCtrl2->startSql(this,sql,1,{TaskSheetUI::TESTING},[this](const QSqlReturnMsg&msg){
        QList<QVariant>r=msg.result().toList();
        ui->taskScheduledView->clear();
        for(int i=1;i<r.count();i++){
            qDebug()<<r.at(i).toList();
            ui->taskScheduledView->append({r.at(i).toList()});
            ui->taskScheduledView->setCellFlag(i-1,0,r.at(i).toList().last());
        }
    });
}

void SamplingScheduleUI::updateToScheduledView()
{
    QString sql=QString("select taskNum, inspectedProject,otherRequirements ,id from test_task_info where taskStatus=%1 and deleted =0").arg(TaskSheetUI::SCHEDULING);
    DealFuc f=[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新排单任务时出错：",msg.errorMsg());
            return;
        }
        QList<QVariant>table=msg.result().toList();
        ui->taskToScheduledView->clear();
        for(int i=1;i<table.count();i++){
            ui->taskToScheduledView->append(table.at(i).toList());
            ui->taskToScheduledView->setCellFlag(i-1,0,table.at(i).toList().last().toInt());
        }
    };

    ui->pageCtrl->startSql(this,sql,1,{},f);
}

void SamplingScheduleUI::on_taskToScheduledView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()){
        return;
    }
    QString taskNum=ui->taskToScheduledView->value(index.row(),0).toString();
    TaskSheetEditor* sheet=new TaskSheetEditor(this,TaskSheetEditor::ViewMode);
    sheet->load(taskNum);
    sheet->show();

}


void SamplingScheduleUI::on_taskScheduledView_doubleClicked(const QModelIndex &index)
{
    QString taskNum=ui->taskScheduledView->value(index.row(),0).toString();
    TaskSheetEditor* sheet=new TaskSheetEditor(this,TaskSheetEditor::ViewMode);
    sheet->load(taskNum);
    sheet->show();
}


void SamplingScheduleUI::on_refleshBtn_clicked()
{
    initCMD();
}


void SamplingScheduleUI::on_myTaskBtn_clicked(bool checked)
{
    if(!checked) return;
    QString sql;
    sql=QString(
        "select samplingSchedul.taskNum, inspectedProject, startDate, endDate, samplerLeader, samplers, samplingSchedul.remark,scheduler "
        "from samplingSchedul "
        "left join test_task_info on  samplingSchedul.taskNum=test_task_info.taskNum "
        "where (samplers like ? or samplerLeader=?) and taskStatus=? and test_task_info.deleted=0 "
        "order by startDate asc;");
    sql="select A.taskNum, inspectedProject, startDate, endDate, samplerLeader, samplers, A.remark,scheduler ,A.taskSheetID "
          "from samplingSchedul as A "
          "left join test_task_info as B on  A.taskSheetID=B.id "
          "where B.deleted=0 and (samplers like ? or samplerLeader=?) and B.taskStatus<=? "
          "order by startDate ;";
    ui->pageCtrl2->startSql(this,sql,1,{QString("%%1%").arg(user()->name()),user()->name(),TaskSheetUI::SAMPLING},[this](const QSqlReturnMsg&msg){
        QList<QVariant>table=msg.result().toList();
        ui->taskScheduledView->clear();
        for(int i=1;i<table.count();i++){
            ui->taskScheduledView->append(table.at(i).toList());
        }
    });
}


void SamplingScheduleUI::on_myScheduleBtn_clicked(bool checked)
{
    if(!checked) return;
    QString sql;
    sql=QString("select samplingSchedul.taskNum, inspectedProject, startDate, endDate, samplerLeader, samplers, samplingSchedul.remark,scheduler from "
                        "samplingSchedul left join test_task_info on  samplingSchedul.taskNum=test_task_info.taskNum where scheduler =? and test_task_info.deleted=0 order by schedulTime desc ;");

    ui->pageCtrl2->startSql(this,sql,1,{user()->name()},[this](const QSqlReturnMsg&msg){
        QList<QVariant>table=msg.result().toList();
        ui->taskScheduledView->clear();
        for(int i=1;i<table.count();i++){
            ui->taskScheduledView->append(table.at(i).toList());
        }
    });
}


void SamplingScheduleUI::on_allBtn_clicked(bool checked)
{
    if(!checked) return;
    updateScheduledView();
}

