
#include "reportmanagerui.h"
#include "qtextedit.h"
#include "tasksheetui.h"
#include "ui_reportmanagerui.h"
#include"itemsselectdlg.h"
#include<QDialogButtonBox>
ReportManagerUI::ReportManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::ReportManagerUI),
    manual(false)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"任务单号","报告编号","委托单位","项目名称","检测类型","当前状态","交接时间"});

    ui->sortTypeBox->clear();
    ui->sortTypeBox->addItem("未完成");
    ui->sortTypeBox->addItem("待编制");
    QList<int> status={0,0};
    for(int i=TaskSheetUI::REPORT_COMPILATION;i<=TaskSheetUI::FINISHED;i++){
        ui->sortTypeBox->addItem(TaskSheetUI::getStatusName(i));
        status.append(i);
    }
    manual=true;

    connect(ui->sortTypeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, status](int index) {
        if (!manual) return;
        if(index==0){//未完成
            initCMD();
            return;
        }
        int s=status.at(index);
        QString sql;
        if(index==1){//待编制
            sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status ,max(E.rt) "
                          "from site_monitoring_info as C  "
                          "left join test_task_info as B on C.taskSheetID=B.id "
                          "left join ("
                          "select max(X.receiveTime) as rt,X.monitoringInfoID from sampling_info as X  group by X.monitoringInfoID  "
                          ") as E on E.monitoringInfoID=C.id  "
                          "left join task_parameters as A on A.monitoringInfoID=C.id "
                          "left join report_status as D on A.reportNum=D.reportNum "
                          "where B.taskStatus>=%1 and D.status is null and B.deleted=0").arg(TaskSheetUI::TESTING);
        }
        else sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status ,max(E.rt) "
                      "from site_monitoring_info as C  "
                      "left join test_task_info as B on C.taskSheetID=B.id "
                      "left join ("
                      "select max(X.receiveTime) as rt,X.monitoringInfoID from sampling_info as X  group by X.monitoringInfoID  "
                      ") as E on E.monitoringInfoID=C.id  "
                      "left join task_parameters as A on A.monitoringInfoID=C.id "
                      "left join report_status as D on A.reportNum=D.reportNum "
                      "where ( D.status=%2) and  B.deleted=0").arg(s);
        if(!(user()->position()&(CUser::LabManager|CUser::LabSupervisor))){
            sql+=QString(" and B.creator='%1'").arg(user()->name());
        }
        sql+=" group by B.taskNum, A.reportNum, B.clientName, B.inspectedProject,D.status,E.rt order by max(E.rt) DESC";
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
    });
}

ReportManagerUI::~ReportManagerUI()
{
    delete ui;
}

void ReportManagerUI::initCMD()
{
    QString sql;

    sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status from task_parameters as A "
                  "left join test_task_info as B on A.taskSheetID=B.id "
                  "left join site_monitoring_info as C on A.monitoringInfoID=C.id "
                  "left join report_status as D on A.reportNum=D.reportNum "
                  //                      "left join (select "//后面要选择样品交接时间并排序
                  "where B.taskStatus>=%1 and (D.status is null or D.status<=%2) and B.deleted=0").arg(TaskSheetUI::TESTING).arg(TaskSheetUI::REPORT_REVIEW3);
//    sql="select B.taskNum, E.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status ,E.rt"
//          "from site_monitoring_info as C  "
//          "left join test_task_info as B on C.taskSheetID=B.id "
//          "left join ("
//          "select max(X.receiveTime) as rt,Z.reportNum, Z.taskSheetID from sampling_info as X "
//          "left join sampling_info as Y on X.sampleNum=Y.sampleNumber "
//          "left join task_parameters as Z on Y.sampleOrder=Z.sampleGroup "
//          "Group by Z.reportNum,Z.taskSheetID "
//          ") as E on E.taskSheetID=B.id "
//          "left join report_status as D on E.reportNum=D.reportNum "
//          "where B.taskStatus>=%1 and (D.status is null or D.status<=%2) and B.deleted=0";
    //目前样品交接没有记录样品编号，先按以下查询显示交接时间
    //采样状态即可进入报告显示，因为可能只有监测，不存在样品流转的情况；也可能分多次采样，可以对部分项目先拆分出报告。
    sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status ,max(E.rt) "
          "from site_monitoring_info as C  "
          "left join test_task_info as B on C.taskSheetID=B.id "
          "left join ("
          "select max(X.receiveTime) as rt,X.monitoringInfoID from sampling_info as X  group by X.monitoringInfoID  "
          ") as E on E.monitoringInfoID=C.id  "
          "left join task_parameters as A on A.monitoringInfoID=C.id "
          "left join report_status as D on A.reportNum=D.reportNum "
                  "where B.taskStatus>=%1 and (D.status is null or D.status<=%2) and B.deleted=0").arg(TaskSheetUI::SAMPLING).arg(TaskSheetUI::REPORT_REVIEW3);
    if(!(user()->position()&(CUser::LabManager|CUser::LabSupervisor))){
        sql+=QString(" and B.creator='%1'").arg(user()->name());
    }
    sql+=" group by B.taskNum, A.reportNum, B.clientName, B.inspectedProject,D.status  order by max(E.rt) DESC";
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
        sql="SELECT DISTINCT task_parameters.reportNum, testType , status from task_parameters left join test_type on task_parameters.testTypeID=test_type.id left join report_status on task_parameters.reportNum=report_status.reportNum where taskSheetID=?  ";
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

FlowWidget* ReportManagerUI::flowWidget(const QFlowInfo &flowInfo)
{
    FlowWidget *w = new FlowWidget;
    QVBoxLayout* lay=new QVBoxLayout(w);
    w->setLayout(lay);

    QString flowName=flowInfo.flowName();
    QString reportNum=flowInfo.flowAbs();
    QWidget* r=flowRecordView(reportNum,"report_flows","reportNum");
    lay->addWidget(r);
    if(flowName=="报告初审"){
        connect(w,&FlowWidget::pushProcess,[this, reportNum](const QFlowInfo&flowInfo,bool passed){
            QString sql;
            bool error=false;
            //更新状态表
            qDebug()<<"报告初审";
            sql="update report_status set status=? where reportNum=?";
            qDebug()<<sql;
            doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新状态表时出错：",msg.errorMsg());
                    sqlFinished();
                    error=true;
                    return;
                }
                sqlFinished();
            },0,{passed?TaskSheetUI::REPORT_REVIEW2:TaskSheetUI::REPORT_MODIFY,reportNum});
            waitForSql("正在更新状态表");
            if(error) return;
            qDebug()<<"更新状态表完成";
            if(passed){
                //提交至报告审核
                qDebug()<<"查询审核人员";
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
                qDebug()<<"查询审核人员完成";
                if(!reviewerIDs.count()){
                    QMessageBox::information(nullptr,"error","未找到合适的审核人员.");
                    return;
                }
                QFlowInfo info("报告审核",this->tabName());
                info.setNode(TaskSheetUI::REPORT_REVIEW2);
                info.setFlowAbs(reportNum);
                info.setCreator(flowInfo.creator());
                qDebug()<<"开始提交审核";
                submitFlow(info,reviewerIDs,reportNum,1,"report_flows","reportNum","flowID");
                qDebug()<<"开始提交审核完成";
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
                reviewerIDs.removeOne(user()->id());//审核完就不进行签发
                if(!reviewerIDs.count()){
                    QMessageBox::information(nullptr,"error","未找到合适的签发人员.");
                    return;
                }
                QFlowInfo info("报告签发",this->tabName());
                info.setNode(TaskSheetUI::REPORT_REVIEW3);
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
            waitForSql("正在更新状态表");
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
    //增加提交时添加备注功能：
    QString comments;
    QString oldFlow;
    QList<int>operateIDs;
    if(status==TaskSheetUI::REPORT_MODIFY){//报告修改，提交至上一审批人
        QList<QVariant>r;
        sql="select A.operateComments,A.operatorID,C.flowInfo "
              "from flow_operate_records as A "
              "left join report_flows as B on B.flowID=A.flowID "
              "left join flow_records as C on A.flowID=C.id "
              "where B.reportNum=? order by A.id desc limit 1";
        doSqlQuery(sql,[this, &error, &r](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查询报告状态时出错：",msg.errorMsg());
                error=true;
                sqlFinished();
                return;
            }
            r=msg.result().toList();
            if(r.count()!=2){
                QMessageBox::information(nullptr,"查询报告状态时出错：","流程信息错误。");
                error=true;
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,{reportNum});
        waitForSql("正在查询当前流程信息");
        if(error) return;
        QList<QVariant>row=r.at(1).toList();
        comments=row.first().toString();

        operateIDs={row.at(1).toInt()};
        oldFlow=row.at(2).toString();
//        if(!submitFlow(flowInfo,{operatorID},reportNum,1,"report_flows","reportNum","flowID")) return;
//        //更新状态
//        sql="update report_status set status=? where reportNum=?;";
//        sql+="update flow_operate_records set revisionNotes=? where flowID=? and operatorID=?;";//更新修改说明
//        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
//            if(msg.error()){
//                QMessageBox::information(nullptr,"更新流程时出错：",msg.errorMsg());
//                sqlFinished();
//                return;
//            }
//            sqlFinished();
//        },0,{flowInfo.node(),reportNum,comments,flowID,operatorID});
//        waitForSql();
//        initCMD();
//        return;
    }
    //
        QDialog dlg;
        QTextEdit *edit=new QTextEdit(&dlg);
        QVBoxLayout *lay=new QVBoxLayout(&dlg);
        QLabel* label=new QLabel("请填写审批备注：",&dlg);
        QHBoxLayout* hlay=new QHBoxLayout(&dlg);
        QPushButton* okBtn=new QPushButton("提交",&dlg);
        QPushButton* cancelBtn=new QPushButton("取消",&dlg);
        hlay->addWidget(okBtn);
        hlay->addWidget(cancelBtn);
        lay->addWidget(label);
        lay->addWidget(edit);
        lay->addLayout(hlay);
        dlg.setLayout(lay);
        edit->setText(comments);

        bool cancel=false;
        connect(okBtn, &QPushButton::clicked, &dlg,&QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, &dlg,&QDialog::reject);
        connect(&dlg,&QDialog::accepted,[&comments, edit](){
            comments=edit->toPlainText();
        });
        connect(&dlg,&QDialog::rejected,[ &cancel](){
            cancel=true;
        });
        dlg.exec();
        if(cancel) return;
    //提交流程

    QFlowInfo flowinfo("报告初审",this->tabName());
    if(!oldFlow.isEmpty()) flowinfo=QFlowInfo(oldFlow);
    else{
        flowinfo.setNode(TaskSheetUI::REPORT_REVIEW1);
        flowinfo.setFlowAbs(reportNum);
        flowinfo.setCreator(user()->name());

        sql="select B.name ,B.id from users left join sys_employee_login as B on users.name=B.name where position&?;";

        QStringList names;
        doSqlQuery(sql,[this, &operateIDs, &names](const QSqlReturnMsg&msg){
            qDebug()<<"hehe";
            if(msg.error()){
                QMessageBox::information(nullptr,"查询操作人员时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            for(int i=0;i<r.count();i++){
                operateIDs.append(r.at(i).toList().at(1).toInt());
                names.append(r.at(i).toList().at(0).toString());
            }
            sqlFinished();
        },0,{CUser::ReportWriter});
        waitForSql();
        if(!operateIDs.count()) return;
        operateIDs=itemsSelectDlg::getSelectedItemsID(names,operateIDs,names);
        if(!operateIDs.count()) return;
    }
    if(!submitFlow(flowinfo,operateIDs,reportNum,1,"report_flows","reportNum","flowID",comments)) return;
    //更新状态
    sql="update report_status set status=? where reportNum=?;";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新流程时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{flowinfo.node(),reportNum});
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
    sql="update task_parameters set reportNum=? where taskSheetID=(select ID from test_task_info where taskNum=?) and reportNum is null;";
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
    showFlowInfo(reportNum,"report_flows","reportNum","flowID");
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
        sql="select DISTINCT monitoringInfoID from task_parameters where reportNum=?;";
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

void ReportManagerUI::on_searchEdit_returnPressed()
{
    QStringList findType={"taskNum","clientName","inspectedEentityName","inspectedProject"};
    QString what=ui->searchEdit->text();
    if(what.isEmpty()) return;

    QString sql;
    sql=QString("select B.taskNum, A.reportNum, B.clientName, B.inspectedProject,GROUP_CONCAT(DISTINCT  C.sampleType SEPARATOR '/'),D.status ,E.rt "
                  "from site_monitoring_info as C  "
                  "left join test_task_info as B on C.taskSheetID=B.id "
                  "left join ("
                  "select max(X.receiveTime) as rt,X.monitoringInfoID from sampling_info as X  group by X.monitoringInfoID  "
                  ") as E on E.monitoringInfoID=C.id  "
                  "left join task_parameters as A on A.monitoringInfoID=C.id "
                  "left join report_status as D on A.reportNum=D.reportNum "
                  "where B.taskStatus>=%1 and (D.status is null or D.status<=%2) and B.deleted=0 and B.%3 like '%%4%'")
              .arg(TaskSheetUI::TESTING).arg(TaskSheetUI::REPORT_REVIEW3).arg(findType.at(ui->searchTypeBox->currentIndex())).arg(what);
    if(!(user()->position()&(CUser::LabManager|CUser::LabSupervisor))){
        sql+=QString(" and B.creator='%1'").arg(user()->name());
    }
    sql+=" group by B.taskNum, A.reportNum, B.clientName, B.inspectedProject,D.status,E.rt order by E.rt DESC";
    if(user()->position()&(CUser::LabManager|CUser::LabSupervisor))
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
}


void ReportManagerUI::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()){
        return;
    }
    QDialog dlg;
    dlg.showMaximized();
    QString taskNum=ui->tableView->value(index.row(),0).toString();
    TaskSheetEditor* sheet=new TaskSheetEditor(this,TaskSheetEditor::ViewMode);
    sheet->load(taskNum);
    sheet->show();
    sheet->setParent(&dlg);
    QVBoxLayout *lay=new QVBoxLayout(&dlg);
    lay->addWidget(sheet);
    lay->setMargin(0);
    dlg.exec();
}

