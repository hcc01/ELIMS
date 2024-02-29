#include "tasksheetui.h"
#include "ui_tasksheetui.h"
#include"tasksheeteditor.h"
#include<QMessageBox>
#include<qexcel.h>
#include"contractreviewdlg.h"
#include<QInputDialog>
TaskSheetUI::TaskSheetUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::TaskSheetUI),
    m_sheet(nullptr)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"任务单号","录单员","业务员" ,"委托单位","受检单位","项目名称","当前状态","委托类型"});
    ui->tableView->addContextAction("编辑",[this](){

        on_tasksheetEditBtn_clicked();
    });
    ui->tableView->addContextAction("查看",[this](){
        on_tasksheetViewBtn_clicked();

//        connect(sheet,&TaskSheetEditor::submitReview,this,&TaskSheetUI::submitReview);
    });
//    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
        //保存审核记录record

    });
    connect(&m_contractReviewDlg,&ContractReviewDlg::getTaskInfo,[this](){
        viewTaskSheet(m_contractReviewDlg.flowInfo().value("taskNum").toString());
    });
    ui->pageCtrl->setTabWidget(this);
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
          "inspectedEentityAddr VARCHAR(255),"
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
          "deleted TINYINT NOT NULL DEFAULT 0,"//是否删除
          "delReason varchar(255),"//删除原因
          "noteMsg varchar(255),"//备忘信息
          "sampleSource TINYINT NOT NULL DEFAULT 0,"//样品来源（采样/送样）
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
          //送样的相关信息
          "sampleName varchar(255), "//样品名称
          "sampleDesc varchar(255), "//样品状态
          "sampleCount int not null default 1, "//样品数量

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


    //方法评审表(报告编号也在这个表中保存，方便按类型、资质、分包情况进行分包）
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
           "sampleGroup int DEFAULT -1, "           //样品组
           "subpackage TINYINT NOT NULL DEFAULT 0, "          //是否分包
           "subpackageDesc VARCHAR(255),"//分包说明
            "CMA TINYINT NOT NULL DEFAULT 0,"//是否在资质范围内
           "reportNum varchar(32), "
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
          "FOREIGN KEY (taskSheetID) REFERENCES test_task_info (id) "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"contract_review_info error",msg.result().toString());
            return;
        }
    });
    //样品采集信息表：
    sql="CREATE TABLE IF NOT EXISTS sampling_info("
          "id int AUTO_INCREMENT primary key, "//
          "monitoringInfoID int not null,"//监测信息ID
          "samplingSiteName varchar(64),"
          "samplingRound  int default 1, "
          "samplingPeriod  int default 1, "
          "sampleOrder int, "
          "sampleNumber varchar(32) unique ,"
          "samplingParameters json,"
          "samplers varchar(12),"
          "siteOrder int, "
          "UNIQUE (monitoringInfoID, samplingRound,samplingPeriod,sampleOrder),  "
          "FOREIGN KEY (monitoringInfoID) REFERENCES site_monitoring_info (id) "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"CREATE TABLE IF NOT EXISTS sampling_info",msg.result().toString());
            return;
        }
    });

        //样品流转表：
        sql="CREATE TABLE IF NOT EXISTS sample_circulate("
          "id int AUTO_INCREMENT primary key, "//
          "taskNum varchar(32) not null,"//任务单号
          "sampleNum varchar(32) unique,"//样品编号
          "deleiver varchar(16) ,"
          "receiver varchar(16), "
          "receiveTime DateTime "
//          "FOREIGN KEY (taskNum) REFERENCES test_task_info (taskNum) "
          ");";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"CREATE TABLE IF NOT EXISTS sampling_info",msg.result().toString());
            return;
        }
    });
    //报告审批状态表：
    sql="CREATE TABLE IF NOT EXISTS report_status("
          "id int AUTO_INCREMENT primary key, "//
          "status int,"//
          "reportNum varchar(32) unique,"
          "taskNum varchar(32),"
          "ramarks varchar(255) "
          //          "FOREIGN KEY (reportNum) REFERENCES test_task_info (id), "
//          " FOREIGN KEY (flowID) REFERENCES flow_records (id)"
          ");";

    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"CREATE TABLE IF NOT EXISTS report_status",msg.result().toString());
            return;
        }
    });
    //报告审批记录表：
    sql="CREATE TABLE IF NOT EXISTS report_flows("
          "id int AUTO_INCREMENT primary key, "//
//          "status int,"//
          "flowID int , "//
          "reportNum varchar(32) unique,"
//          "FOREIGN KEY (reportNum) REFERENCES test_task_info (id), "
          " FOREIGN KEY (flowID) REFERENCES flow_records (id)"
          ");";

    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"CREATE TABLE IF NOT EXISTS report_flows",msg.result().toString());
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

FlowWidget *TaskSheetUI::flowWidget(const QFlowInfo &flowInfo)
{
    if(flowInfo.flowName()=="任务单审核"){
        FlowWidget*w= new FlowWidget;
        QVBoxLayout* vlay=new QVBoxLayout(w);
        MyTableView* view=new MyTableView(w);
        QPushButton* infoBtn=new QPushButton("查看任务单",w);
        vlay->addWidget(infoBtn);
        vlay->addWidget(view);
        view->setHeader({"评审要素","评审内容","评审记录"});
        view->append({"1.检测方法评审","检测方法是否明确、适用、有效","是"});
        view->append({"2.检测项目评审","检测项目是否明确	","是"});
        view->append({"3.样品要求评审","样品类型、数量、采/抽样、运输、存储、制备、防护、处理处置、是否确定","是"});
        view->append({"4.安全环保评审	","样品存储、制备、检测过程中是否存在潜在的危险或危害		","否"});
        view->append({"5.结果传递评审","结果提交的方式是否明确","是"});
        view->append({"6.分包评审","检测过程中是否需要分包","否"});
        view->append({"7.人力资源评审	","检测人员是否具备相应检测技能，是否需要采/抽样、其他特殊资质要求","是"});
        view->append({"8.设备设施评审","是否具备开展检测和客户需求所需设备设施","是"});
        view->append({"9.资质能力评审","是否具备检验资质	","是"});
        view->append({"10.时间评审","检测周期是否满足客户要求","是"});
        view->setEditableColumn(2);
        ComboBoxDelegate* editor=new ComboBoxDelegate(view,{"否","是"});
        view->setItemDelegateForColumn(2,editor);
        view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        QString taskNum=flowInfo.value("taskNum").toString();
        //查看任务单信息
        connect(infoBtn,&QPushButton::clicked,[this, flowInfo, taskNum](){
            viewTaskSheet(taskNum);
        });
        //处理流程审批结果
        connect(w,&FlowWidget::pushProcess,[this](const QFlowInfo&flowInfo,bool passed){
            QString sql="update test_task_info set taskStatus=? where id=?";//更新任务单状态
            int flowID=flowInfo.flowID();
            int taskSheetID=flowInfo.value("taskSheetID").toInt();
            int node=flowInfo.node();
            int backNode=flowInfo.backNode();
            int nextNode=flowInfo.nextNode();
            QJsonArray values;
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
        });

        return w;
    }
    return nullptr;
}

void TaskSheetUI::initCMD()//初始化和刷新
{
    ui->tableView->clear();
    m_taskIDs.clear();
    QString sql=QString("SELECT taskNum, creator, users.name, clientName, inspectedEentityName, inspectedProject, taskStatus , sampleSource,A.id from (select * from test_task_info where creator='%1' and deleted!=1 ) as A left join users on A.salesRepresentative=users.id ORDER BY createDate DESC;").arg(user()->name());
    if(user()->position()&(CUser::LabManager|CUser::LabSupervisor)) sql="SELECT taskNum, creator, users.name, clientName, inspectedEentityName, inspectedProject, taskStatus ,sampleSource ,A.id from (select * from test_task_info where deleted!=1 ) as A left join users on A.salesRepresentative=users.id ORDER BY createDate DESC;";
    DealFuc f=[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"获取任务单信息失败",msg.result().toString());
                return;
            }
            QList<QVariant> r=msg.result().toList();
            ui->tableView->clear();
            for(int i=1;i<r.count();i++){
                QList<QVariant>row=r.at(i).toList();
                int status=row.at(6).toInt();
                row[6]=getStatusName(status);
                if(row.at(7).toBool()){
                    row[7]="送样检测";
                }
                else row[7]="采样检测";
                ui->tableView->append(row);
                ui->tableView->setCellFlag(i-1,6,status);
                m_taskIDs[row.at(0).toString()]=row.at(8).toInt();
            }
            qDebug()<<msg.jsCmd();
    };
    ui->pageCtrl->startSql(this,sql,1,{},f);

//    doSqlQuery(sql,f,1);
}


void TaskSheetUI::on_newSheetBtn_clicked()
{

    TaskSheetEditor* sheet=sheetEditorDlg(TaskSheetEditor::NewMode);
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row>=0) {
        if(QMessageBox::question(nullptr,"","是否以当前任务单为模板进行创建？")==QMessageBox::Yes){
            taskNum=ui->tableView->value(row,0).toString();
            sheet->load(taskNum,true);
        }
    }

    sheet->show();
}

void TaskSheetUI::submitReview(int sheetID,const QString&taskNum,bool DeliveryTest)
{
    QString sql;
    sql="select taskStatus from test_task_info where taskNum=?";
    bool error=false;
    int status;
    doSqlQuery(sql,[this, &error, &status](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询当前状态时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()!=2){
            QMessageBox::information(nullptr,"查询当前状态时出错：","没有相关任务单");
            error=true;
            sqlFinished();
            return;
        }
        status=r.at(1).toList().at(0).toInt();
        sqlFinished();
    },0,{taskNum});
    waitForSql();
    if(error){
        return;
    }
    if(status!=CREATE&&status!=MODIFY){
        QMessageBox::information(nullptr,"error","当前任务单无须提交");
        return;
    }
    QFlowInfo flowInfo("任务单审核",tabName());
    flowInfo.setValue("taskSheetID",sheetID);//标记
    flowInfo.setValue("taskNum",taskNum);//标记
    flowInfo.setFlowAbs(taskNum);
    flowInfo.setNode(REVIEW);//
    flowInfo.setBackNode(MODIFY);
    if(DeliveryTest) flowInfo.setNextNode(TESTING);//送样分析，直接到分析节点
    else flowInfo.setNextNode(SCHEDULING);//采样分析，下一步采样安排
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
    waitForSql("正在处理操作人员……");
    if(!operateorIDs.count()){
        QMessageBox::information(nullptr,"添加操作人员时出错：","未获取到有效的可操作人员。");
        return;
    }
    //任务单审核提交到流利管理
    int flowID=submitFlow(flowInfo,operateorIDs,QString::number(sheetID),1,"task_status","taskSheetID");//提交到流程登记
    if(!flowID){
        QMessageBox::information(nullptr,"创建流程出错：","未获取到有效的流程ID。");
        return;
    }
//    //关联任务单的审核流程ID
//    doSqlQuery("insert into task_status (taskSheetID, taskStatus, flowID) values(?,?,?) ",[this](const QSqlReturnMsg&msg){//更新任务单状态
//        if(msg.error()){
//            QMessageBox::information(nullptr,"插入流程时出错：",msg.result().toString());
//            emit sqlFinished();
//            return;
//        }
//        emit sqlFinished();
//    },0,{sheetID,REVIEW,flowID});
//    waitForSql("正在创建审批流程……");
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
    if(openMode!=TaskSheetEditor::ViewMode) m_sheet->init();
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


bool TaskSheetUI::on_tasksheetEditBtn_clicked()//编辑任务单
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return false;
    int status=ui->tableView->cellFlag(row,6).toInt();
    if(user()->name()!=ui->tableView->value(row,1).toString()) return false;//非本人不可编辑
    qDebug()<<status;
    if(status!=CREATE&&status!=MODIFY) {
        QMessageBox::information(nullptr,"","已提交的任务单不能编辑。");
        return false;//提交审核后，任务单锁定不能编辑
    }
    taskNum=ui->tableView->value(row,0).toString();
    TaskSheetEditor* sheet=sheetEditorDlg(TaskSheetEditor::EditMode);
    sheet->load(taskNum);
    sheet->show();
    return true;
}


void TaskSheetUI::on_tasksheetViewBtn_clicked()
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return;

    taskNum=ui->tableView->value(row,0).toString();
    viewTaskSheet(taskNum);
}


void TaskSheetUI::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!on_tasksheetEditBtn_clicked()) on_tasksheetViewBtn_clicked();
}


void TaskSheetUI::on_deleteBtn_clicked()
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return ;
    int status=ui->tableView->cellFlag(row,6).toInt();
    if(user()->name()!=ui->tableView->value(row,1).toString()) return ;//非本人不可编辑
    qDebug()<<status;
    if(status!=CREATE&&status!=MODIFY) {
        QMessageBox::information(nullptr,"","已提交的任务单不能编辑。");
        return ;//提交审核后，任务单锁定不能编辑
    }
    taskNum=ui->tableView->value(row,0).toString();
    int a=QMessageBox::question(nullptr,"",QString("确认删除任务单%1").arg(taskNum));
    if(a!=QMessageBox::Yes) return;
    QString reason=QInputDialog::getText(nullptr,"","请输入删除原因");
    QString sql;
    sql="update test_task_info set deleted=1, delReason=? where taskNum=?";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"删除任务单时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{reason,taskNum});
    waitForSql();
    initCMD();
}


void TaskSheetUI::on_reviewCommentsBtn_clicked()
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return ;
    taskNum=ui->tableView->value(row,0).toString();
    int flowID;
    QString sql="select flowID from task_status where taskSheetID=(select id from test_task_info where taskNum=?); ";
    QJsonObject m;
    doSqlQuery(sql,[this, &m](const QSqlReturnMsg&msg){
        m=msg.jsCmd();
        sqlFinished();
    },0,{taskNum});
    waitForSql();
    showFlowInfo(QSqlReturnMsg(m));

}


void TaskSheetUI::on_submitBtn_clicked()
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return ;
    taskNum=ui->tableView->value(row,0).toString();
    submitReview(m_taskIDs.value(taskNum),taskNum,ui->tableView->value(row,"委托类型").toString()=="送样检测");
}


//void TaskSheetUI::on_printLabelBtn_clicked()
//{
//    QString taskNum;
//    int row=ui->tableView->selectedRow();
//    if(row<0) return ;
//    if(ui->tableView->value(row,"委托类型").toString()!="送样检测") return;
//    if(ui->tableView->value(row,"当前状态").toString()!=getStatusName(SAMPLE_CIRCULATION)) return;
//    taskNum=ui->tableView->value(row,0).toString();
//    QString sql;
//    //先看看是否已经完成交接编号
//    sql="select sampleNumber from sampling_info as A "
//          "left join site_monitoring_info as B on A.monitoringInfoID=B.id "
//          "left join test_task_info as B on A.taskSheetID=B.id where B.taskNum=?; ";

//    //如果未交接，进行交接和编号
//    //存在分天送样的情况，所以需要分开确认交接的样品

//    sql="select A,sampleType ,A.sampeName, GROUP_CONCAT(C.parameterName SEPARATOR '、')  from task_methods as C "
//          "left join site_monitoring_info as A on C.monitoringInfoID=A.id "
//          "left join test_task_info as B on A.taskSheetID=B.id where B.taskNum=?;";
//    QList<QVariant>r;
//    doSqlQuery(sql,[this, &r](const QSqlReturnMsg&msg){
//        if(msg.error()){
//            QMessageBox::information(nullptr,"查询样品信息时出错：",msg.errorMsg());
//            sqlFinished();
//            return;
//        }
//        r=msg.result().toList();
//        sqlFinished();
//    },0,{taskNum});
//    waitForSql();
//    if(r.count()<2) return;
//    for(int i=1;i<r.count();i++){
//        QList<QVariant>row=r.at(i).toList();

//    }
//}

