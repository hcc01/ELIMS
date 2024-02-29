
#include "reportmanagerui.h"
#include "tasksheetui.h"
#include "ui_reportmanagerui.h"
#include"itemsselectdlg.h"
ReportManagerUI::ReportManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::ReportManagerUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"任务单号","报告编号","委托单位","项目名称","检测类型","当前状态"});
}

ReportManagerUI::~ReportManagerUI()
{
    delete ui;
}

void ReportManagerUI::initCMD()
{
    QString sql;
    switch(ui->sortTypeBox->currentIndex()){
    case 0://未完成
    {
        sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status from task_methods as A "
              "left join test_task_info as B on A.taskSheetID=B.id "
              "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
              "left join report_status as D on A.reportNum=D.reportNum "
//                      "left join (select "//后面要选择样品交接时间并排序
                      "where B.taskStatus>=%1 and (D.status is null or D.status<=%2) and B.deleted=0").arg(TaskSheetUI::TESTING).arg(TaskSheetUI::REPORT_REVIEW3);
        if(!(user()->position()&(CUser::LabManager|CUser::LabSupervisor))){
            sql+=QString(" and B.creator='%1'").arg(user()->name());
        }
        sql+=" group by B.taskNum, A.reportNum, B.clientName, B.inspectedProject,D.status";
    }
    break;
    case 1://待归档
    {
        sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT C.sampleType SEPARATOR '/'),D.status from task_methods as A "
                      "left join test_task_info as B on A.taskSheetID=B.id "
                      "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
                      "left join report_status as D on A.reportNum=D.reportNum "
                      "where D.status=%1 and D.status is not null and B.deleted=0").arg(TaskSheetUI::REPORT_ARCHIVING);
        if(!(user()->position()&(CUser::LabManager|CUser::LabSupervisor))){
            sql+=QString(" and B.creator='%1'").arg(user()->name());
        }
         sql+=" group by B.taskNum, A.reportNum, B.clientName, B.inspectedProject,D.status";
    }
    break;
    case 2://已完成
    {
        sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT C.sampleType SEPARATOR '/'),D.status from task_methods as A "
                      "left join test_task_info as B on A.taskSheetID=B.id "
                      "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
                      "left join report_status as D on A.reportNum=D.reportNum "
                      "where D.status>=%1 and D.status is not null and B.deleted=0").arg(TaskSheetUI::REPORT_ARCHIVING);
        if(!(user()->position()&(CUser::LabManager|CUser::LabSupervisor))){
            sql+=QString(" and B.creator='%1'").arg(user()->name());
        }
        sql+=" group by B.taskNum, A.reportNum, B.clientName, B.inspectedProject,D.status";
    }
    break;
    }
    ui->pageCtrl->startSql(this,sql,1,{},[this](const QSqlReturnMsg&msg){
        ui->tableView->clear();
        QList<QVariant>r=msg.result().toList();
        QList<QVariant>row;
        for(int i=1;i<r.count();i++){
            row=r.at(i).toList();
            row[5]=TaskSheetUI::getStatusName(row.at(5).toInt());
             ui->tableView->append(row);

        }

    });
    return;
    //以下作废
    //流程节点在报告编制和归档之间的任务单
    QList<QVariant>tasks;//tasks:{任务单号，客户名称，项目名称，任务单ID}
    ui->tableView->clear();
    sql=QString("select taskNum, clientName, inspectedProject ,id from test_task_info where creator='%1' and taskStatus>=%2;").arg(user()->name()).arg(TaskSheetUI::TESTING);
    if(user()->position()&(CUser::LabManager|CUser::LabSupervisor))
        sql=QString("select taskNum, clientName, inspectedProject ,id from test_task_info where taskStatus>=%1;").arg(TaskSheetUI::TESTING);
    doSqlQuery(sql,[this,&tasks](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询任务单时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        tasks=msg.result().toList();
        tasks.removeFirst();
        sqlFinished();

    });
    waitForSql();
    if(!tasks.count()){
        return;
    }
    for(auto task:tasks){
        sql="SELECT DISTINCT task_methods.reportNum, testType , status from task_methods left join test_type on task_methods.testTypeID=test_type.id left join report_status on task_methods.reportNum=report_status.reportNum where taskSheetID=?  ";
        doSqlQuery(sql,[this, task](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查询报告时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            QStringList types;
            QString nowReport;
            for(int i=1;i<r.count();i++){
                QList<QVariant>row=r.at(i).toList();
                QString report=row.at(0).toString();
//                if(report.isEmpty()) report=task.toList().at(0).toString();
                if(report.isEmpty()) break;
                if(report!=nowReport){
                    if(!nowReport.isEmpty()){
                        ui->tableView->append({task.toList().at(0).toString(),nowReport,task.toList().at(1).toString(),task.toList().at(2).toString(),
                                               types.join("/"),TaskSheetUI::getStatusName(r.at(i-1).toList().at(2).toInt())});
                    }
                    nowReport=report;
                    types.clear();
                }
                if(!types.contains(row.at(1).toString())){
                    types.append(row.at(1).toString());
                }

            }
            ui->tableView->append({task.toList().at(0).toString(),nowReport,task.toList().at(1).toString(),task.toList().at(2).toString(),
                                   types.join("/"),TaskSheetUI::getStatusName(r.last().toList().at(2).toInt())});
            sqlFinished();

        },0,{task.toList().at(3).toInt()});
        waitForSql();
    }

}

FlowWidget *ReportManagerUI::flowWidget(const QFlowInfo &flowInfo)
{
    FlowWidget *w = new FlowWidget;
    QVBoxLayout* lay=new QVBoxLayout(w);
    w->setLayout(lay);

    QString flowName=flowInfo.flowName();

    QString reportNum=flowInfo.flowAbs();
    QWidget* r=showFlowRecord(reportNum,"report_flows","reportNum");
    lay->addWidget(r);
    if(flowName=="报告初审"){
        connect(w,&FlowWidget::pushProcess,[this, reportNum](const QFlowInfo&flowInfo,bool passed){
            QString sql;
            bool error=false;
            //更新状态表
            sql="update report_status set status=? where reportNum=?";
            doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新状态表时出错：",msg.errorMsg());
                    sqlFinished();
                    error=true;
                    return;
                }
                sqlFinished();
            },0,{passed?TaskSheetUI::REPORT_REVIEW2:TaskSheetUI::REPORT_MODIFY,reportNum});
            waitForSql();
            if(error) return;
            if(passed){
                //提交至报告审核
                sql="select A.id from sys_employee_login as A left join users on A.name=users.name where position & ?;";
                QList<int>reviewerIDs;
                doSqlQuery(sql,[this, &reviewerIDs](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"查询审核人员时出错：",msg.errorMsg());
                        sqlFinished();
                        return;
                    }
                    QList<QVariant>r=msg.result().toList();
                    for(int i=1;i<r.count();i++){
                        reviewerIDs.append(r.at(i).toList().at(0).toInt());
                    }
                    sqlFinished();
                },0,{CUser::ReportReviewer|CUser::AuthorizedSignatory});
                waitForSql();
                if(!reviewerIDs.count()){
                    QMessageBox::information(nullptr,"error","未找到合适的审核人员.");
                    return;
                }
                QFlowInfo info("报告审核",this->tabName());
                info.setFlowAbs(reportNum);
                info.setCreator(flowInfo.creator());
                submitFlow(info,reviewerIDs,reportNum,1,"report_flows","reportNum","flowID");
            }
        });
        return w;
    }
    if(flowName=="报告审核"){
        connect(w,&FlowWidget::pushProcess,[this, reportNum](const QFlowInfo&flowInfo,bool passed){
            QString sql;
            bool error=false;
            //更新状态表
            sql="update report_status set status=? where reportNum=?";
            doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新状态表时出错：",msg.errorMsg());
                    sqlFinished();
                    error=true;
                    return;
                }
                sqlFinished();
            },0,{passed?TaskSheetUI::REPORT_REVIEW3:TaskSheetUI::REPORT_MODIFY,reportNum});
            waitForSql();
            if(error) return;
            if(passed){
                //提交至报告签发
                sql="select A.id from sys_employee_login as A left join users on A.name=users.name where position & ?;";
                QList<int>reviewerIDs;
                doSqlQuery(sql,[this, &reviewerIDs](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"查询签发人员时出错：",msg.errorMsg());
                        sqlFinished();
                        return;
                    }
                    QList<QVariant>r=msg.result().toList();
                    for(int i=1;i<r.count();i++){
                        reviewerIDs.append(r.at(i).toList().at(0).toInt());
                    }
                    sqlFinished();
                },0,{CUser::AuthorizedSignatory});
                waitForSql();
                if(!reviewerIDs.count()){
                    QMessageBox::information(nullptr,"error","未找到合适的签发人员.");
                    return;
                }
                QFlowInfo info("报告签发",this->tabName());
                info.setFlowAbs(reportNum);
                info.setCreator(flowInfo.creator());
                submitFlow(info,reviewerIDs,reportNum,1,"report_flows","reportNum","flowID");
            }
        });
        return w;
    }
    if(flowName=="报告签发"){
        connect(w,&FlowWidget::pushProcess,[this, reportNum](const QFlowInfo&flowInfo,bool passed){
            QString sql;
            bool error=false;
            //更新状态表
            sql="update report_status set status=? where reportNum=?";
            doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新状态表时出错：",msg.errorMsg());
                    sqlFinished();
                    error=true;
                    return;
                }
                sqlFinished();
            },0,{passed?TaskSheetUI::REPORT_ARCHIVING:TaskSheetUI::REPORT_MODIFY,reportNum});
            waitForSql();
            if(error) return;
        });
        return w;
    }
    return nullptr;
}

void ReportManagerUI::on_submitBtn_clicked()
{
    int row=ui->tableView->selectedRow();
    if(row<0) return;
    QString sql;
    //先判断任务单状态
//    if(ui->tableView->value(row,"当前状态").toString()!=TaskSheetUI::getStatusName(TaskSheetUI::REPORT_COMPILATION)&&
//        ui->tableView->value(row,"当前状态").toString()!=TaskSheetUI::getStatusName(TaskSheetUI::REPORT_MODIFY))
//        return;
    QString reportNum=ui->tableView->value(row,1).toString();
    if(reportNum.isEmpty()) return;
    bool error=false;
    int status;
    sql="select status from report_status where reportNum=?";
    doSqlQuery(sql,[this, &error, &status](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询报告状态时出错：",msg.errorMsg());
            sqlFinished();
            error=true;
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()!=2){
            QMessageBox::information(nullptr,"查询报告状态时出错：","没有相关报告编号");
            error=true;
            sqlFinished();
            return;
        }
        status=r.at(1).toList().at(0).toInt();
        sqlFinished();
    },0,{reportNum});
    waitForSql();
    if(error) return;
    if(status!=TaskSheetUI::REPORT_COMPILATION&&status!=TaskSheetUI::REPORT_MODIFY) return;
    //提交流程
    QFlowInfo flowinfo("报告初审",this->tabName());
    flowinfo.setFlowAbs(reportNum);
    flowinfo.setCreator(user()->name());
    sql="select B.name ,B.id from users left join sys_employee_login as B on users.name=B.name where position&?;";
    QList<int>ids;
    QStringList names;
    doSqlQuery(sql,[this, &ids, &names](const QSqlReturnMsg&msg){
        qDebug()<<"hehe";
        if(msg.error()){
            QMessageBox::information(nullptr,"查询操作人员时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        for(int i=0;i<r.count();i++){
            ids.append(r.at(i).toList().at(1).toInt());
            names.append(r.at(i).toList().at(0).toString());
        }
        sqlFinished();
    },0,{CUser::ReportWriter});
    waitForSql();
    if(!ids.count()) return;
    ids=itemsSelectDlg::getSelectedItemsID(names,ids,names);
    if(!ids.count()) return;
    if(!submitFlow(flowinfo,ids,reportNum,1,"report_flows","reportNum","flowID")) return;
    //更新状态
    sql="update report_status set status=? where reportNum=?;";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新流程时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{TaskSheetUI::REPORT_REVIEW1,reportNum});
    waitForSql();
    initCMD();
}


void ReportManagerUI::on_reportEditBtn_clicked()
{
    int row=ui->tableView->selectedRow();
    if(row<0) return;
    QString taskNum=ui->tableView->value(row,0).toString();
    QString reportNum=ui->tableView->value(row,1).toString();
    QString sql;
    bool error=false;
    //先看下是否已经编制
    if(!reportNum.isEmpty()){
        sql="select id from report_status where reportNum=?;";
        doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"检查报告编制情况时出错：",msg.errorMsg());
                error=true;
                sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            if(r.count()){
                error=true;
                QMessageBox::information(nullptr,"error","报告已编制");
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,{reportNum});
        waitForSql();
    }
    //更新报告编号
    sql="update task_methods set reportNum=? where taskSheetID=(select ID from test_task_info where taskNum=?) and reportNum is null;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新流程时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{taskNum,taskNum});
    waitForSql();
    if(error) return;
    if(reportNum.isEmpty()) reportNum=taskNum;//报告未拆分
    sql="insert into report_status (reportNum, taskNum, status) values(?,?,?);";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新报告状态时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{reportNum,taskNum,TaskSheetUI::REPORT_COMPILATION});
    waitForSql();
    if(error) return;
    initCMD();
}


void ReportManagerUI::on_refleshBtn_clicked()
{
    initCMD();
}


void ReportManagerUI::on_reviewRecordBtn_clicked()
{
    int row=ui->tableView->selectedRow();
    if(row<0) return;
    QString taskNum=ui->tableView->value(row,0).toString();
    QString reportNum=ui->tableView->value(row,1).toString();
    if(reportNum.isEmpty()) return;
    showFlowRecord(reportNum,"report_flows","reportNum","flowID");
}


void ReportManagerUI::on_splitBtn_clicked()
{
    int row=ui->tableView->selectedRow();
    if(row<0) return;
    QString taskNum=ui->tableView->value(row,0).toString();
    QString reportNum=ui->tableView->value(row,1).toString();
    QStringList bases={"按点位","按类型","按分包项目"};
    QString base=itemsSelectDlg::getSelectedItem(bases);
    QString sql;
    if(base=="按点位"){
        sql="select DISTINCT monitoringInfoID from task_methods where reportNum=?;";
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查询点位信息时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            for(int i=1;i<r.count();i++){
                auto row=r.at(i).toList();
            }
        },0,{reportNum});
        waitForSql();
    }
}


void ReportManagerUI::on_sortTypeBox_currentIndexChanged(int index)
{
    initCMD();
}

