#include "tasksheetui.h"
#include "ui_tasksheetui.h"
#include"tasksheeteditor.h"
#include<QMessageBox>
#include<qexcel.h>
#include"contractreviewdlg.h"
TaskSheetUI::TaskSheetUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TaskSheetUI),
    m_sheet(nullptr)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"任务单号","录单员","业务员" ,"委托单位","受检单位","项目名称","当前状态"});
    ui->tableView->addContextAction("编辑",[this](){

        QString taskNum;
        int row=ui->tableView->selectedRow();
        if(row<0) return;
        int status=ui->tableView->cellFlag(row,6).toInt();
        if(user()->name()!=ui->tableView->value(row,1).toString()) return;//非本人不可编辑
        qDebug()<<status;
        if(status!=CREATE&&status!=MODIFY) {
            QMessageBox::information(nullptr,"","已提交的任务单不能编辑。");
            return;//提交审核后，任务单锁定不能编辑
        }
        taskNum=ui->tableView->value(row,0).toString();
        TaskSheetEditor* sheet=sheetEditorDlg(TaskSheetEditor::EditMode);
        sheet->load(taskNum);
        sheet->show();        
    });
    ui->tableView->addContextAction("查看",[this](){
        QString taskNum;
        int row=ui->tableView->selectedRow();
        if(row<0) return;

        taskNum=ui->tableView->value(row,0).toString();
        viewTaskSheet(taskNum);
//        connect(sheet,&TaskSheetEditor::submitReview,this,&TaskSheetUI::submitReview);
    });
    //合同评审的处理信号
    connect(&m_contractReviewDlg,&ContractReviewDlg::reviewResult,[this](const QString&record,const QString&comments,bool passed){
        QFlowInfo flowInfo=m_contractReviewDlg.flowInfo();
        int flowID=flowInfo.flowID();
        int taskSheetID=flowInfo.value("taskSheetID").toInt();
        int node=flowInfo.node();
        int backNode=flowInfo.backNode();
        int nextNode=flowInfo.nextNode();
        QString taskNum=flowInfo.value("taskNum").toString();
        if(!taskSheetID){
            QMessageBox::information(nullptr,"","处理流程时错误：无法获取任务单ID。");
            return;
        }
        //考虑到有多人审核的情况，先检查下是否已经被其它人处理

        QString sql;
        QJsonArray values;
        if(pushProcess(flowInfo,passed,comments)){
            sql="update test_task_info set taskStatus=? where id=?";
            if(passed){//同意，进入采样排单
                //进入新流程（后面处理）
                //更改任务单状态
                values={nextNode,taskSheetID};
            }
            else{//拒绝，退回任务单修改
                //更改任务单状态为“待修改”
                values={backNode,taskSheetID};
            }
            doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        return notifySqlError("更改任务单状态时出错：",msg.errorMsg());
                    }
                },0,values);
        }else{
            QMessageBox::information(nullptr,"","无法推进流程。");
            return;
        }

    });
    connect(&m_contractReviewDlg,&ContractReviewDlg::getTaskInfo,[this](){
        viewTaskSheet(m_contractReviewDlg.flowInfo().value("taskNum").toString());
    });
}

TaskSheetUI::~TaskSheetUI()
{
    delete ui;
}

void TaskSheetUI::submitProcess(int node)
{
    switch(node){
    case REVIEW:
    {

    }
    break;
    }
}

void TaskSheetUI::dealProcess(const QFlowInfo &flowInfo,int operateFlag)
{
    qDebug()<<"TaskSheetUI::dealProcess正在处理流程审批:"<<flowInfo.object();
    int flowID=flowInfo.flowID();
    int taskSheetID=flowInfo.value("taskSheetID").toInt();
    int node=flowInfo.node();
    int backNode=flowInfo.backNode();
    int nextNode=flowInfo.nextNode();
    QString taskNum=flowInfo.value("taskNum").toString();
    if(!taskSheetID){
        QMessageBox::information(nullptr,"","处理流程时错误：无法获取任务单ID。");
        return;
    }
    //进行合同评审
    m_contractReviewDlg.setFlowInfo(flowInfo);
    m_contractReviewDlg.show();
    return;
    //流程管理改变，以下先放着
    QString sql;
    switch(operateFlag){
    case VIEWINFO:
    {
        switch (node) {
        case REVIEW:
            viewTaskSheet(taskNum);
            break;
        default:
            break;
        }
    }
    break;
    case AGREE://流程同意，更新任务单状态
    {
        sql="update test_task_info set taskStatus=? where id=?";
        doSqlQuery(sql,[](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"更新流程节点时出错：",msg.errorMsg());
                return;
            }
        },0,{nextNode,taskSheetID});
    }
    break;
    case REJECT:
    {
        //流程拒绝，将流程更新到退回节点
        sql="update test_task_info set taskStatus=? where id=?";
        doSqlQuery(sql,[](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"更新流程节点时出错：",msg.errorMsg());
                return;
            }
        },0,{backNode,taskSheetID});
        switch (node) {
        case REVIEW:
            //流程拒绝，退到修改。

            break;
        default:
            break;
        }

    }
    break;
    }
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
           "taskStatus  int, "          //任务状态
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
          "flowID int, "
//          "operator VARCHAR(10), "//操作人员
//          "operateTime datetime, "//操作时间
//          "operateComment VARCHAR(255), "    //操作说明，
          "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id), "
          " FOREIGN KEY (flowID) REFERENCES flow_records (id)"
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
    //合同评审记录表：
    sql="CREATE TABLE IF NOT EXISTS contract_review_info("
          "id int AUTO_INCREMENT primary key, "//
          "taskSheetID int not null,"//任务单ID
          "reviewRecord  VARCHAR(255), "         //合同评审表记录，目前以“是、是、否、是……”按顺序对各要点的评审情况进行保存
          "reviewor  VARCHAR(16), " //评审员
          "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id), "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"contract_review_info error",msg.result().toString());
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

void TaskSheetUI::initCMD()//初始化和刷新
{
    ui->tableView->clear();
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
                int status=row.at(6).toInt();
                row[6]=StatusName.value(status);
                ui->tableView->append(row);                
                ui->tableView->setCellFlag(i-1,6,status);
            }
        },1);
}


void TaskSheetUI::on_newSheetBtn_clicked()
{
    TaskSheetEditor* sheet=sheetEditorDlg(TaskSheetEditor::NewMode);
    sheet->show();
}

void TaskSheetUI::submitReview(int sheetID,const QString&taskNum)
{

    QFlowInfo flowInfo("任务单审核",tabName());
    flowInfo.setValue("taskSheetID",sheetID);//标记
    flowInfo.setValue("taskNum",taskNum);//标记
    flowInfo.setNode(REVIEW);//
    flowInfo.setBackNode(MODIFY);
    flowInfo.setNextNode(SCHEDULING);
    QList<int>operateorIDs;//操作人员的ID列表
    //找到任务单审核人员，由技术负责人或质量负责人或实验室主管负责审核任务单
    doSqlQuery("select id from (select name from users where position & ?) as A left join sys_employee_login as B on A.name=B.name ;",[this,&operateorIDs](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询操作人员时出错：",msg.result().toString());
            emit sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            operateorIDs.append(r.at(i).toList().first().toInt());
        }
        emit sqlFinished();
    },0,{CUser::TechnicalManager | CUser::QualityManager | CUser::LabSupervisor});
    waitForSql();
    if(!operateorIDs.count()){
        QMessageBox::information(nullptr,"添加操作人员时出错：","未获取到有效的可操作人员。");
        return;
    }
    //任务单审核提交到流利管理
    int flowID=submitFlow(flowInfo,operateorIDs);//提交到流程登记
    if(!flowID){
        QMessageBox::information(nullptr,"创建流程出错：","未获取到有效的流程ID。");
        return;
    }
    //关联任务单的审核流程ID
    doSqlQuery("insert into task_status (taskSheetID, taskStatus, flowID) values(?,?,?) ",[this](const QSqlReturnMsg&msg){//更新任务单状态
        if(msg.error()){
            QMessageBox::information(nullptr,"插入流程时出错：",msg.result().toString());
            emit sqlFinished();
            return;
        }
        emit sqlFinished();
    },0,{sheetID,REVIEW,flowID});
    waitForSql();
    updateTaskStatus(sheetID,REVIEW);
    initCMD();
}
bool TaskSheetUI::updateTaskStatus(int taskID, int status)
{
    bool ret=false;
    doSqlQuery("UPDATE test_task_info set taskStatus=? where id=?",[this, &ret](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"updateTaskStatus error",msg.result().toString());
            emit sqlFinished();
            return;
        }
        ret=true;
        emit sqlFinished();
    },0,{status,taskID});
    waitForSql();
    return ret;
}

TaskSheetEditor *TaskSheetUI::sheetEditorDlg(int openMode)
{
//    if(!m_sheet){
//        m_sheet=new TaskSheetEditor(this,openMode);
//        m_sheet->init();
//        connect(m_sheet,&TaskSheetEditor::submitReview,this,&TaskSheetUI::submitReview);
//    }
//    else{
//        m_sheet->setOpenMode(openMode);
//    }
//    m_sheet->reset();
//    return m_sheet;
    if(m_sheet) delete m_sheet;
    m_sheet=new TaskSheetEditor(this,openMode);
    m_sheet->init();
    connect(m_sheet,&TaskSheetEditor::submitReview,this,&TaskSheetUI::submitReview);
    return m_sheet;
}

void TaskSheetUI::viewTaskSheet(const QString& taskSheetNum)
{
    TaskSheetEditor* sheet=sheetEditorDlg(TaskSheetEditor::ViewMode);
    if(!sheet){
        qDebug()<<"error: sheet is null, taskSheetNum:"<<taskSheetNum;
    }
    sheet->load(taskSheetNum);
    sheet->show();
}

void TaskSheetUI::editTaskSheet(const QString &taskSheetNum)
{
    TaskSheetEditor* sheet=sheetEditorDlg(TaskSheetEditor::EditMode);
    sheet->load(taskSheetNum);
    sheet->show();
}


void TaskSheetUI::on_refleshBtn_clicked()
{
    initCMD();
}

void TaskSheetUI::doContractReview(const QFlowInfo &flowInfo, const QString &record, const QString &comments, bool passed)
{

}

