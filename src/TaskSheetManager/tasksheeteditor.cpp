#include "tasksheeteditor.h"
#include "tasksheetui.h"
#include "ui_tasksheeteditor.h"
#include<QInputDialog>
#include<QMessageBox>
#include"clientmanagerdlg.h"
#include"implementingstandardselectdlg.h"
#include"methodselectdlg.h"
#include<QJsonArray>
#include<QThread>
#include"QExcel.h"
#include<QTimer>
#include<QShortcut>
#include<QCloseEvent>
TaskSheetEditor::TaskSheetEditor(TabWidgetBase *tabWiget, int openMode) :
    QMainWindow(tabWiget),
    SqlBaseClass(tabWiget),
    ui(new Ui::TaskSheetEditor),
    m_infoEditor(nullptr,this->tabWiget()),
    m_userOpereate(false),
    m_isSaving(false),
//    m_bSaved(false),
    m_bTasksheetModified(false),
    m_bTestInfoModified(false),
    m_status(0),
    m_taskSheetID(0),
    m_mode(openMode),
    m_copiedRow(-1),
    m_methodDlgInited(false)
{
    ui->setupUi(this);
    ui->widget->hide();
    setWindowTitle("任务单");

    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    m_MethodDlg=new MethodSelectDlg(this->tabWiget());
    connect(m_MethodDlg,&MethodSelectDlg::accepted,[this](){
        m_bMethodModified=true;
    });
    if(m_mode==ReviewMode){
        ui->submitBtn->hide();
    }
    else if(m_mode==ViewMode){
        ui->saveBtn->hide();
        ui->submitBtn->hide();
    }
    connect(ui->testInfoTableView,&MyTableView::dataChanged,[this](int row,int column,const QVariant& value){
        m_bTestInfoModified=true;
        switch(column){
        case 0:
        {
            m_testInfo[row]->sampleType=value.toString();
        }
        break;
        case 1:
        {
            if(ui->sampleSourceBox->currentIndex()){//送样，为样品名称
                 m_testInfo[row]->sampleName=value.toString();
            }
            m_testInfo[row]->samplingSites=value.toString();
        }
        break;
        case 3:
        {
             m_testInfo[row]->sampleCount=value.toInt();
        }
        break;
        case 4:
        {
              m_testInfo[row]->sampleDesc=value.toString();
        }
        break;
        case 5:
        {
             m_testInfo[row]->remark=value.toString();
        }
        break;
        }
    });
    ui->testInfoTableView->setHeader({"样品类型","检测点位","检测项目","检测频次","执行标准","备注"});
//    ui->testInfoTableView->setEditableColumn(0);
    ui->testInfoTableView->setEditableColumn(1);
    ui->testInfoTableView->setEditableColumn(5);
    ui->testInfoTableView->addContextAction("修改",[this](){
        if(!m_testInfo.count()) return;
        int row=ui->testInfoTableView->selectedRow();
        if(row<0) return;
        auto info=m_testInfo.at(row);
        info->delieveryTest=ui->sampleSourceBox->currentIndex();
        qDebug()<<"<info->samplingSites;"<<info<<info->samplingSites;
        m_infoEditor.load(info);
        m_infoEditor.show();
    });
    ui->testInfoTableView->addContextAction("删除",[this](){
        if(!m_testInfo.count()) return;
        int row=ui->testInfoTableView->selectedRow();
        if(row<0) return;
        int as=QMessageBox::question(nullptr,"","确认删除该行检测信息？");
        if(as==QMessageBox::Yes){
            m_testInfo.removeAt(row);
            m_bTestInfoModified=true;
            updateTestInfoView();
        }
    });
    ui->testInfoTableView->addContextAction("添加",[this](){
        TestInfo* info=new TestInfo;
        info->delieveryTest=ui->sampleSourceBox->currentIndex();
        m_infoEditor.load(info);
        testInfoEditor ie(info,this->tabWiget());
        ie.setWindowTitle(info->delieveryTest?"添加送样样品信息": "添加采样点位信息");
        int r=m_infoEditor.exec();
        if(r==QDialog::Accepted){
            m_testInfo.append(info);
//            ui->testInfoTableView->append(info->infoList());
//            m_bTestInfoModified=true;
            ui->sampleSourceBox->setDisabled(true);
            updateTestInfoView();
        }
        else{
            qDebug()<<"rejested";
            delete info;
        }
    });

//    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    //添加复制粘贴快捷键，用于监测信息的快速复制修改
    // 创建复制快捷键（Ctrl+C）
    QShortcut *copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, [this](){
        if(!m_testInfo.count()) return;
        int row=ui->testInfoTableView->selectedRow();
        if(row<0) return;
        m_copiedRow=row;
    });

    // 创建粘贴快捷键（Ctrl+V）
    QShortcut *pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, [this](){
        if(m_copiedRow<0) return;
        ui->testInfoTableView->append(ui->testInfoTableView->data().at(m_copiedRow));
        TestInfo* info=new TestInfo(m_testInfo.at(m_copiedRow));
       // info=m_testInfo.at(m_copiedRow);//如此添加的是指针，显然不行
        m_testInfo.append(info);
    });


    ui->ClientEditBtn->hide();
    ui->ClientEditCancelBtn->hide();
    ui->addContactBtn->hide();
}

TaskSheetEditor::~TaskSheetEditor()
{
    delete ui;
    foreach (auto x, m_testInfo) {
        if(x) delete x;
    }
}

void TaskSheetEditor::init()
{
//    connect(&m_infoEditor,&testInfoEditor::doSql,this->tabWiget(),&TabWidgetBase::doSqlQuery);

    m_infoEditor.init();
    connect(&m_infoEditor,&testInfoEditor::accepted,[this](){//修改时，更新。添加时，需要在添加菜单里处理数据后更新。
        m_bTestInfoModified=true;
        qDebug()<<"m_bTestInfoModified=true";
        updateTestInfoView();
    });

    doSql("select name from users where position=?",[this](const QSqlReturnMsg& msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            sqlFinished();
            return;
        }
        ui->salesRepresentativeBox->clear();

        for(auto s:msg.table()){

            qDebug()<<s;
            ui->salesRepresentativeBox->addItem(s.at(0).toString());
        }
        sqlFinished();
    },0,{CUser::salesRepresentative});
    waitForSql();
     doSql("select  clientName,id, address from client_info where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"查询客户信息时出错：",msg.result().toString());
            sqlFinished();
            return;
        }
        QList<QVariant> clients=msg.result().toList();
        for(int i=1;i<clients.count();i++){
            m_clients[clients.at(i).toList().at(0).toString()]={clients.at(i).toList().at(1).toInt(),clients.at(i).toList().at(2).toString()};
        }
        ui->clientBox->init(m_clients.keys());
        ui->inspectedComBox->init(m_clients.keys());
        ui->inspectedAddrEdit->setText("");
        ui->inspectedComBox->setCurrentIndex(-1);
        ui->inspectedContactsBox->setCurrentIndex(-1);
        ui->inspectedPhoneEidt->setText("");
        m_userOpereate=true;//代码的改变选择框完成
        sqlFinished();
    });
    waitForSql();
//    doSql("select  clientName,id, address from client_info where deleted=0;",[&](const QSqlReturnMsg&msg){
//        if(msg.error()){
//            QMessageBox::information(this,"error",msg.result().toString());
//        }
//        QList<QVariant> clients=msg.result().toList();
//        for(int i=1;i<clients.count();i++){
//            m_clients[clients.at(i).toList().at(0).toString()]={clients.at(i).toList().at(1).toInt(),clients.at(i).toList().at(2).toString()};
//        }
//        ui->clientBox->init(m_clients.keys());
//        ui->inspectedComBox->init(m_clients.keys());
//    });


}

void TaskSheetEditor::doSave()
{
//保存任务单，需要保存采样检测任务表test_task_info、点位监测信息表site_monitoring_info、检测方法评审表task_methods、任务单状态表task_status
    if(m_status!=TaskSheetUI::CREATE&&m_status!=TaskSheetUI::MODIFY&&m_mode!=NewMode){//保存仅限于创建和修改状态(后续有其它需求再处理）
        QMessageBox::information(nullptr,"error","当前状态无法修改。");
        return;
    }
    QString sql;
    QJsonArray values;
//    if(!this->tabWiget()->connectDB(CMD_START_Transaction)){//使用事务操作（移动到保存点击时开启）
//        return;
//    }
    if(m_mode==EditMode){//修改模式，更新数据库;目前修改模式不改委托方式

        if(m_bTasksheetModified){
            bool ok=false;
            int salerID;
            doSql("select id from users where name=?;",[&salerID, this, &ok](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"查询业务人员时出错：",msg.errorMsg());
                    sqlFinished();
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                if(r.count()==1){
                    QMessageBox::information(nullptr,"error","业务人员不存在，请确认。");
                    sqlFinished();
                    return;
                }
                salerID=r.at(1).toList().at(0).toInt();
                ok=true;
                sqlFinished();
            },0,{ui->salesRepresentativeBox->currentText()});
            waitForSql();
            if(!ok) {
//                tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                return;
            }
            //进行基本信息保存
            sql="update test_task_info set contractNum=?,clientID=?, clientName=?,clientAddr=?,clientContact=?,clientPhone=?,"
                  "inspectedEentityName=?,inspectedEentityContact=?,inspectedEentityPhone=?,inspectedProject=?,projectAddr=?,"
                  "reportPurpos=?,reportPeriod=?,sampleDisposal=?,reprotCopies=?,methodSource=?,subpackage=?,otherRequirements=? ,inspectedEentityAddr=?,salesRepresentative=?  "
                  "where taskNum=?;";
            values={ui->contractEdit->text(),m_clients.value(ui->clientBox->currentText()).ID,ui->clientBox->currentText(),ui->clientAddrEdit->text(),ui->clientContactsBox->currentText(),ui->clientContactsPhoneEdit->text(),
                      ui->inspectedComBox->currentText(),ui->inspectedContactsBox->currentText(),ui->inspectedPhoneEidt->text(),ui->projectNameEdit->text(),ui->projectAddrEdit->text(),
                      ui->reportPurposEdit->currentText(),ui->reportPeriodBox->value(),ui->sampleDisposalBox->currentText(),ui->reportCopiesBox->value(),ui->methodSourseBox->currentIndex(),ui->subpackageBox->currentIndex(),
                      ui->otherRequirementsEdit->text(),ui->inspectedAddrEdit->text(),salerID,m_taskNum};
            qDebug()<<"m_taskNum"<<m_taskNum;
//            tabWiget()->connectDB(CMD_START_Transaction);//开启事务
            doSql(sql,[this, &ok](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"保存任务单信息时出错：",msg.errorMsg());

                        sqlFinished();
                        return;
                    }
                    m_bTasksheetModified=false;
                    ok=true;
                    sqlFinished();
                },0,values);
            waitForSql("正在保存任务单信息...");
            if(!ok){
//                tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                return;
            }
//            tabWiget()->releaseDB(CMD_COMMIT_Transaction);
        }
        if(m_bTestInfoModified){
            //保存检测信息
//            if(!ui->sampleSourceBox->currentIndex()){//采样任务单的保存
            bool error=false;
            tabWiget()->connectDB(CMD_START_Transaction);//开启事务
            sql="delete from task_parameters where taskSheetID= ?;delete from site_monitoring_info where taskSheetID=?;";//删除点位监测信息表和检测项目表
            values={m_taskSheetID,m_taskSheetID};//保存时用到m_taskSheetID，要确保任务单ID得到正确获取

            foreach (auto info, m_testInfo) {//保存监测信息
                bool siteMatched=false;
                QStringList sites=info->samplingSites.split("、");//检查点位数量与名称的匹配性，如果匹配，识别点位名称，否则不识别，全部保存
                qDebug()<<"点位匹配:"<<info<<sites<<info->samplingSiteCount;
                if(sites.count()==info->samplingSiteCount){
                    siteMatched=true;
                }
                for (int i = 0; i < info->samplingSiteCount;i++) { // 将合并的点位拆开保存(当为送样单时，samplingSiteCount=1，也会进入保存）
                    sql += "INSERT INTO site_monitoring_info (taskSheetID, testTypeID, "
                          "samplingSiteName, samplingFrequency, samplingPeriod, "
                          "limitValueID, remark, sampleType,sampleName,sampleCount,sampleDesc) "
                          "VALUES (?, ?,?,?,?,?,?,?,?,?,?);SET @site_id = "
                          "LAST_INSERT_ID();";
                    QString smaplingSite;
                    if(siteMatched) {
                        smaplingSite=sites.at(i);
                    }
                    else smaplingSite=info->samplingSites;
                    values.append(m_taskSheetID);
                    values.append(info->testTypeID);
                    values.append(smaplingSite);
                    values.append(info->samplingFrequency);
                    values.append(info->samplingPeriod);
                    values.append(info->limitStandardID ? info->limitStandardID : QJsonValue::Null);
                    values.append(info->remark);
                    values.append(info->sampleType);
                    //补充了送样信息的保存
                    values.append(info->sampleName);
                    values.append(info->sampleCount);
                    values.append(info->sampleDesc);

                    sql += "INSERT INTO task_parameters ( monitoringInfoID, taskSheetID, "
                           " parameterID, parameterName,testTypeID) "
                           "VALUES ";
                    // 开始保存点位监测项目

                    for (int i = 0; i < info->monitoringParameters.count(); i++) {
                        sql += "(@site_id,?, ?, ?,?)";
                        if (i == info->monitoringParameters.count() - 1) {
                            sql += ";";
                        } else
                            sql += ",";
                        values.append(m_taskSheetID);
                        values.append(info->parametersIDs.at(i));
                        values.append(info->monitoringParameters.at(i));
                        values.append(info->testTypeID);
                    }                    
                }
            }
            doSql(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                    error=true;
                    QMessageBox::information(nullptr,"保存检测信息时出错：",msg.errorMsg());
                }
                sqlFinished();
            },0,values);
            waitForSql("正在保存检测信息。");
            if(error) return;
            tabWiget()->releaseDB(CMD_COMMIT_Transaction);
        }
        if(m_bMethodModified){
            m_MethodDlg->saveMethod(m_taskSheetID);
            m_bMethodModified=false;
        }
        m_bTestInfoModified=false;
        QMessageBox::information(nullptr,"","保存完成。");
        return;//修改操作完成，返回，下面是新建操作。
    }
    //保存新任务单
    else if(m_mode==NewMode){
        bool ok=false;
        int salerID;
        doSql("select id from users where name=?;",[&salerID, this, &ok](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查询业务人员时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            if(r.count()==1){
                QMessageBox::information(nullptr,"error","业务人员不存在，请确认。");
                sqlFinished();
                return;
            }
            salerID=r.at(1).toList().at(0).toInt();
            ok=true;
            sqlFinished();
        },0,{ui->salesRepresentativeBox->currentText()});
        waitForSql();
        if(!ok){
//            tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
            return;
        }
        tabWiget()->connectDB(CMD_START_Transaction);
        sql="INSERT INTO test_task_info (taskNum,contractNum,clientID,clientName,clientAddr,clientContact,clientPhone,"
              "inspectedEentityName,inspectedEentityContact,inspectedEentityPhone,inspectedProject,projectAddr,"
              "reportPurpos,reportPeriod,sampleDisposal,reprotCopies,methodSource,subpackage,otherRequirements, creator, createDate,sampleSource,inspectedEentityAddr,salesRepresentative) "
              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, NOW(),?,?,?);set @taskSheetID=LAST_INSERT_ID();";
        values={m_taskNum,ui->contractEdit->text(),m_clients.value(ui->clientBox->currentText()).ID,ui->clientBox->currentText(),ui->clientAddrEdit->text(),ui->clientContactsBox->currentText(),ui->clientContactsPhoneEdit->text(),
                  ui->inspectedComBox->currentText(),ui->inspectedContactsBox->currentText(),ui->inspectedPhoneEidt->text(),ui->projectNameEdit->text(),ui->projectAddrEdit->text(),
                  ui->reportPurposEdit->currentText(),ui->reportPeriodBox->value(),ui->sampleDisposalBox->currentText(),ui->reportCopiesBox->value(),ui->methodSourseBox->currentIndex(),ui->subpackageBox->currentIndex(),
                  ui->otherRequirementsEdit->text(),user()->name(),ui->sampleSourceBox->currentIndex(),ui->inspectedAddrEdit->text(),salerID};

        //开始保存点位监测信息

        //逐条保存监测信息
        foreach(auto info,m_testInfo){
            bool siteMatched=false;
            QStringList sites=info->samplingSites.split("、");//检查点位数量与名称的匹配性，如果匹配，识别点位名称，否则不识别，全部保存
            if(sites.count()==info->samplingSiteCount){
                siteMatched=true;
            }

            for(int i=0;i<info->samplingSiteCount;i++){//将合并的点位拆开保存，按点位保存检测信息
                sql+="INSERT INTO site_monitoring_info (taskSheetID, testTypeID, samplingSiteName, samplingFrequency, samplingPeriod, limitValueID, remark, sampleType,sampleName,sampleCount,sampleDesc) "
                       "VALUES (@taskSheetID, ?,?,?,?,?,?,?,?,?,?);SET @site_id = LAST_INSERT_ID();";
                QString smaplingSite;
                if(siteMatched) smaplingSite=sites.at(i);
                else smaplingSite=info->samplingSites;
                values.append(info->testTypeID);
                values.append(smaplingSite);
                values.append(info->samplingFrequency);
                values.append(info->samplingPeriod);
                values.append(info->limitStandardID?info->limitStandardID:QJsonValue::Null);
                values.append(info->remark);
                values.append(info->sampleType);
                values.append(info->sampleName);
                values.append(info->sampleCount);
                values.append(info->sampleDesc);

                sql+="INSERT INTO task_parameters ( monitoringInfoID, taskSheetID, parameterID, parameterName, testTypeID) "
                       "VALUES ";

                //开始保存点位监测项目
                for(int i=0;i<info->monitoringParameters.count();i++){
                    sql+="(@site_id,@taskSheetID, ?, ?,?)";
                    if(i==info->monitoringParameters.count()-1){
                        sql+=";";
                    }
                    else sql+=",";
                    QJsonArray a={info->parametersIDs.at(i),info->monitoringParameters.at(i),info->testTypeID};
                    foreach(auto x,a) values.append(x);
                }
            }
        }
        sql+="select @taskSheetID";
        doSql(sql,[this, &ok](const QSqlReturnMsg&msg){
            if(msg.error()){
                tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                QMessageBox::information(nullptr,"保存任务单时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            ok=true;
            m_taskSheetID=msg.result().toList().at(1).toList().at(0).toInt();
            sqlFinished();
        },0,values);
        waitForSql();
        if(!ok) return;
        if(m_bMethodModified){
            saveMethod();
        }
        tabWiget()->releaseDB(CMD_COMMIT_Transaction);
        m_bTestInfoModified=false;
        //保存完成后，改为修改模式，以便用户继续修改。
        m_mode=EditMode;
        m_bTestInfoModified=false;
        QMessageBox::information(nullptr,"","保存完成。");//保存方法在监测信息更新完后才能进行

    }
}

bool TaskSheetEditor::saveMethod()//
{
    m_bMethodModified=false;
    return m_MethodDlg->saveMethod(this->m_taskSheetID);
}

void TaskSheetEditor::load(const QString &taskNum, bool newMode)
{
    if(taskNum.isEmpty()){
        QMessageBox::information(nullptr,"载入任务信息时出错：","空的任务单号。");
        return;
    }

    if(!newMode) {
        m_taskNum=taskNum;
        setWindowTitle("任务单："+m_taskNum);
    }
    ui->sampleSourceBox->setDisabled(true);
    qDebug()<<"开始载入任务信息……";

    m_userOpereate=false;

    QString sql;
    sql=QString("SELECT contractNum, users.name, clientName, clientAddr, clientContact, clientPhone, inspectedEentityName, inspectedEentityContact, "
          "inspectedEentityPhone, inspectedProject, projectAddr, reportPurpos, reportPeriod, sampleDisposal, reprotCopies, methodSource, "
                  "subpackage, otherRequirements, A.id,taskStatus ,creator, sampleSource,inspectedEentityAddr from (select * from test_task_info where taskNum=?) as A left join users on A.salesRepresentative=users.id;");

    doSql(sql,[this, newMode](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"载入任务信息时错误",msg.result().toString());
            emit sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList().at(1).toList();
        ui->sampleSourceBox->setCurrentIndex(r.at(21).toInt());
        ui->contractEdit->setText(r.at(0).toString());
        ui->salesRepresentativeBox->setCurrentText(r.at(1).toString());
        ui->clientBox->setCurrentText(r.at(2).toString());
//        ui->clientBox->setCurrentIndex(ui->clientBox->findText(r.at(2).toString()));
        ui->clientAddrEdit->setText(r.at(3).toString());
        ui->clientContactsBox->setCurrentText(r.at(4).toString());

        ui->clientContactsPhoneEdit->setText(r.at(5).toString());
//        ui->inspectedComBox->setCurrentIndex(ui->clientBox->findText(r.at(6).toString()));
        ui->inspectedComBox->setCurrentText(r.at(6).toString());
//        ui->inspectedAddrEdit->setText(m_clients.value(ui->inspectedComBox->currentText()).clientAddr);
        ui->inspectedAddrEdit->setText(r.at(22).toString());
        ui->inspectedContactsBox->setCurrentText(r.at(7).toString());
        ui->inspectedPhoneEidt->setText(r.at(8).toString());
        ui->projectNameEdit->setText(r.at(9).toString());
        ui->projectAddrEdit->setText(r.at(10).toString());
        ui->reportPurposEdit->setCurrentText(r.at(11).toString());
        ui->reportPeriodBox->setValue(r.at(12).toInt());
        ui->sampleDisposalBox->setCurrentText(r.at(13).toString());
        ui->reportCopiesBox->setValue(r.at(14).toInt());
        ui->methodSourseBox->setCurrentText(r.at(15).toString());
        ui->subpackageBox->setCurrentIndex(r.at(16).toInt());
        ui->otherRequirementsEdit->setText(r.at(17).toString());

        m_taskSheetID=r.at(18).toInt();
        if(!newMode){
            m_status=r.at(19).toInt();
            m_createor=r.at(20).toString();
        }
        sqlFinished();
    },0,{taskNum});
    waitForSql("载入任务信息...");

    sql="SELECT A.testTypeID, A.sampleType, A.samplingSiteName, A.samplingFrequency, A.samplingPeriod, A.limitValueID, A.remark, A.id, "
          "B.standardName,B.standardNum, B.tableName, B.classNum, C.testFieldID, A.sampleName,A.sampleCount,A.sampleDesc, "
          "GROUP_CONCAT(D.parameterID  ORDER BY D.id ASC SEPARATOR ','), GROUP_CONCAT(D.parameterName ORDER BY D.id ASC SEPARATOR '/' )  "
          "from task_parameters as D "
          "left join site_monitoring_info as A on  D.monitoringInfoID=A.id "
          "left join implementing_standards as B on A.limitValueID = B.id "
          "left join test_type as C on A.testTypeID= C.id "
//          "right join task_parameters as D on D.monitoringInfoID=A.id "
          "where A.taskSheetID=? "
          "group by A.testTypeID, A.sampleType, A.samplingSiteName, A.samplingFrequency, A.samplingPeriod, A.limitValueID, A.remark, A.id,"
          " B.standardName,B.standardNum, B.tableName, B.classNum, C.testFieldID, A.sampleName,A.sampleCount,A.sampleDesc;";

    doSql(sql,[this](const QSqlReturnMsg&msg){
        qDebug()<<"处理检测信息";
        if(msg.error()){
            QMessageBox::information(this,"载入检测信息时错误",msg.result().toString());
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        foreach (auto x, m_testInfo) {
            if(x) delete x;
        }
        m_testInfo.clear();
        QList<QVariant>row;
        QStringList ids;
        bool deliveryTest=ui->sampleSourceBox->currentIndex();
        for(int i=1;i<r.count();i++){
            row=r.at(i).toList();
//            int monitorID=row.at(7).toInt();
            ids=row.at(16).toString().split(",");

            QList<int>parameterIDs;
            for(auto id:ids){
                parameterIDs.append(id.toInt());
            }
            QStringList monitoringParameters=row.at(17).toString().split("/");

            if(deliveryTest||!m_testInfo.count()||(m_testInfo.last()->monitoringParameters!=monitoringParameters
                                ||m_testInfo.last()->sampleType!=row.at(1).toString()
                                ||m_testInfo.last()->samplingFrequency!=row.at(3).toInt()
                                ||m_testInfo.last()->samplingPeriod!=row.at(4).toInt()
                                ||m_testInfo.last()->limitStandardID!=row.at(5).toInt()
                                ||m_testInfo.last()->remark!=row.at(6).toString())){
                TestInfo* info=new TestInfo;
                info->delieveryTest=ui->sampleSourceBox->currentIndex();
                info->testTypeID=row.at(0).toInt();
                info->testFieldID=row.at(12).toInt();
                info->sampleType=row.at(1).toString();
//                    if(!row.at(2).toString().isEmpty()) info->samplingSites.append(row.at(2).toString());
                info->samplingSites=row.at(2).toString();
                info->samplingFrequency=row.at(3).toInt();
                info->samplingPeriod=row.at(4).toInt();
                info->limitStandardID=row.at(5).toInt();
                info->remark=row.at(6).toString();
                info->monitoringParameters=monitoringParameters;
                info->parametersIDs=parameterIDs;
                info->sampleName=row.at(13).toString();
                info->sampleCount=row.at(14).toInt();
                info->sampleDesc=row.at(15).toString();
                info->limitStandard=QString("%1(%2)%3%4").arg(row.at(8).toString()).arg(row.at(9).toString()).arg(row.at(10).toString()).arg(row.at(11).toString());
                m_testInfo.append(info);

            }
            else{//合并监测指标相同的点位
                m_testInfo.last()->samplingSiteCount++;
                if(row.at(2).toString()!=m_testInfo.last()->samplingSites) m_testInfo.last()->samplingSites+="、"+row.at(2).toString();
            }
            if(i==r.count()-1){
                ui->testInfoTableView->clear();
                for(auto info:m_testInfo)
                    ui->testInfoTableView->append(info->infoList());

            }

        }
        sqlFinished();

    },0,{m_taskSheetID});
    waitForSql("载入检测信息...");

/*
    m_userOpereate=true;
    QList<QVariant>r;
    sql=QString("SELECT A.testTypeID, A.sampleType, A.samplingSiteName, A.samplingFrequency, A.samplingPeriod, A.limitValueID, A.remark, A.id, standardName,standardNum, tableName, classNum, testFieldID, A.sampleName,A.sampleCount,A.sampleDesc from"
                  "(SELECT testTypeID, sampleType, samplingSiteName, samplingFrequency, samplingPeriod, limitValueID, remark, id ,sampleName,sampleCount,sampleDesc from site_monitoring_info "
                  "where taskSheetID=?) as A "
                  "left join implementing_standards on A.limitValueID = implementing_standards.id left join test_type on A.testTypeID= test_type.id;");
    qDebug()<<"m_taskSheetID"<<m_taskSheetID;
    doSql(sql,[this,&r](const QSqlReturnMsg&msg){
        qDebug()<<"处理检测信息";
        if(msg.error()){
            QMessageBox::information(this,"载入检测信息时错误",msg.result().toString());
            sqlFinished();
            return;
        }
        r=msg.result().toList();
        sqlFinished();

    },0,{m_taskSheetID});
    waitForSql("载入检测信息...");
    qDebug()<<"处理检测信息...";
    qDebug()<<r;

    /*

    /*
    m_testInfo.clear();
    for(int i=1;i<r.count();i++){
        QList<QVariant>row=r.at(i).toList();
        int monitorID=row.at(7).toInt();

        QString sql="SELECT parameterID, parameterName "
                      "from task_parameters  "
                      "where monitoringInfoID=? ;";
        doSql(sql,[this, r,i, row](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"载入项目信息时错误",msg.result().toString());
                sqlFinished();
                return;
            }
            QList<QVariant>parameters=msg.result().toList();
            QList<int>parameterIDs;
            QStringList monitoringParameters;
            for(int i=1;i<parameters.count();i++){
                parameterIDs.append(parameters.at(i).toList().at(0).toInt());
                monitoringParameters.append(parameters.at(i).toList().at(1).toStringList());
            }
            if(ui->sampleSourceBox->currentIndex()||!m_testInfo.count()||(m_testInfo.last()->monitoringParameters!=monitoringParameters
                                ||m_testInfo.last()->sampleType!=row.at(1).toString()
                                ||m_testInfo.last()->samplingFrequency!=row.at(3).toInt()
                                ||m_testInfo.last()->samplingPeriod!=row.at(4).toInt()
                                ||m_testInfo.last()->limitStandardID!=row.at(5).toInt()
                                ||m_testInfo.last()->remark!=row.at(6).toString())){
                TestInfo* info=new TestInfo;
                info->delieveryTest=ui->sampleSourceBox->currentIndex();
                info->testTypeID=row.at(0).toInt();
                info->testFieldID=row.at(12).toInt();
                info->sampleType=row.at(1).toString();
//                    if(!row.at(2).toString().isEmpty()) info->samplingSites.append(row.at(2).toString());
                info->samplingSites=row.at(2).toString();
                info->samplingFrequency=row.at(3).toInt();
                info->samplingPeriod=row.at(4).toInt();
                info->limitStandardID=row.at(5).toInt();
                info->remark=row.at(6).toString();
                info->monitoringParameters=monitoringParameters;
                info->parametersIDs=parameterIDs;
                info->sampleName=row.at(13).toString();
                info->sampleCount=row.at(14).toInt();
                info->sampleDesc=row.at(15).toString();
                info->limitStandard=QString("%1(%2)%3%4").arg(row.at(8).toString()).arg(row.at(9).toString()).arg(row.at(10).toString()).arg(row.at(11).toString());
                m_testInfo.append(info);

            }
            else{//合并监测指标相同的点位
                m_testInfo.last()->samplingSiteCount++;
                if(row.at(2).toString()!=m_testInfo.last()->samplingSites) m_testInfo.last()->samplingSites+="、"+row.at(2).toString();
            }
            if(i==r.count()-1){
                ui->testInfoTableView->clear();
                for(auto info:m_testInfo)
                    ui->testInfoTableView->append(info->infoList());

            }

            sqlFinished();
        },0,{monitorID});

        waitForSql();
    }*/
    //加载方法

//    sql=QString("SELECT A.testTypeID, sampleType, parameterName, testMethodName, CMA, subpackage, subpackageDesc,parameterID from "
//                  "(select * from task_methods where taskSheetID=%1 ) as A left join site_monitoring_info on A.monitoringInfoID= site_monitoring_info.id "
//                  "group by A.testTypeID, sampleType, parameterName, testMethodName,  CMA, subpackage, subpackageDesc,parameterID;").arg(m_taskSheetID);
//    doSql(sql,[this](const QSqlReturnMsg&msg){
//        if(msg.error()){
//            QMessageBox::information(nullptr,"载入方法信息时错误",msg.result().toString());
//            sqlFinished();
//            return;
//        }
//        QList<QVariant>methods=msg.result().toList();
//        QList<QList<QVariant>> methodTable;
////        m_mthds.clear();
//        QHash<int,QHash<int,int>>parametersRows;//[类型，【参数ID，行号】】
//        for(int i=1;i<methods.count();i++){
//            QList row=methods.at(i).toList();
//            int typeID=row.at(0).toInt();
//            int parameterID=row.at(7).toInt();
//            QString sampleType=row.at(1).toString();
//            //把相同检测类型的项目合并在一起
//            if(parametersRows.value(typeID).contains(parameterID)){
//                int r=parametersRows.value(typeID).value(parameterID);
//                QString type=methodTable.value(r).at(0).toString();
//                if(!type.contains(sampleType)){
//                    methodTable[r][0]=type.append("/").append(sampleType);
//                }

//            }
//            else{
//                methodTable.append({row.at(1),row.at(2),row.at(3),row.at(4).toInt()?"是":"否",row.at(5).toInt()?"是":"否",row.at(6)});
//                parametersRows[typeID][parameterID]=methodTable.count()-1;
//                MethodMore* mm=new MethodMore;
//                mm->testMethodName=row.at(3).toString();//这里用于方法选择时载入已选的方法，不用保存ID
//                mm->subpackage=row.at(5).toInt();
//                mm->subpackageDesc=row.at(6).toString();
//                m_MethodDlg->addMethod(typeID,parameterID,mm);//已选的方法，用于方法加载时，自动选用已经确认的方法。
//            }
//        }
//        m_MethodDlg->showMethods(methodTable);
//        sqlFinished();
//    });
//    waitForSql("载入方法信息...");
//    //    m_bSaved=true;
}

void TaskSheetEditor::setOpenMode(int mode)
{
    m_mode=mode;
    switch(mode){
    case NewMode:
    case EditMode:
        ui->saveBtn->show();
        ui->submitBtn->show();
        break;
//    case ReviewMode://目前还没开放其它模式下修改保存
//        ui->saveBtn->show();
//        ui->submitBtn->hide();
//        break;
    default:
        ui->saveBtn->hide();
        ui->submitBtn->hide();
    }
}

void TaskSheetEditor::reset()
{
//    m_bSaved=false;
//    ui->testInfoTableView->clear();
//    m_mthds.clear();
    m_status=0;
    m_testInfo.clear();
    ui->testInfoTableView->clear();
    m_isSaving=false;
    m_bMethodModified=false;
    m_bTasksheetModified=false;
    m_bTestInfoModified=false;
    ui->inspectedAddrEdit->setText("");
    ui->inspectedComBox->setCurrentText("");
    ui->inspectedContactsBox->setCurrentText("");
    ui->inspectedPhoneEidt->setText("");
    ui->projectNameEdit->setText("");
    ui->projectAddrEdit->setText("");
    ui->otherRequirementsEdit->setText("");
    ui->clientBox->setCurrentText("");
    ui->clientContactsBox->setCurrentText("");
    ui->clientAddrEdit->setText("");
    ui->clientContactsPhoneEdit->setText("");
    m_MethodDlg->reset();
}

void TaskSheetEditor::updateTestInfoView()//在视图中更新检测信息
{
    ui->testInfoTableView->clear();
    for(auto info:m_testInfo){
        ui->testInfoTableView->append(info->infoList());
    }
}

void TaskSheetEditor::setEntrustType(bool deliveryTest)
{
    if(deliveryTest){
        qDebug()<<"送样检测";
         ui->testInfoTableView->setHeader({"样品类型","样品名称","检测项目","样品数量","样品状态","备注"});
//        ui->testInfoTableView->setEditableColumn(0);
         ui->testInfoTableView->setEdiableColumns({1,3,4,5});
        return;
    }
    qDebug()<<"采样检测";
    ui->testInfoTableView->setHeader({"样品类型","检测点位","检测项目","检测频次","执行标准","备注"});
//    ui->testInfoTableView->setEditableColumn(0);
    ui->testInfoTableView->setEdiableColumns({1,5});
}

void TaskSheetEditor::closeEvent(QCloseEvent *event)
{
    if(m_mode==ViewMode) return;
    qDebug()<<m_bTasksheetModified<<m_bTestInfoModified<<m_bMethodModified;
    if(m_bTasksheetModified||m_bTestInfoModified||m_bMethodModified){
        if (QMessageBox::question(this, "", tr("任务单有变更未作保存，你确定要退出应用程序吗？"))!=QMessageBox::Yes) {
            // 如果用户选择“否”，则不关闭窗口
            event->ignore();
            return;
        }
    }

}


void TaskSheetEditor::on_inspectedComBox_currentIndexChanged(int index)
{
    if(!m_userOpereate) return;
    QString clientName=ui->inspectedComBox->currentText();
    if(clientName.isEmpty()) return;
    m_inspectedEentityID=m_clients.value(clientName).ID;
    ui->inspectedAddrEdit->setText(m_clients.value(clientName).clientAddr);
    QString sql=QString("select name, phoneNum from client_contacts where clientID=%1 and deleted = 0;").arg(m_inspectedEentityID);
    doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        ui->inspectedContactsBox->clear();        QList<QVariant> contacts=msg.result().toList();
        m_contacts.clear();
        for(int i=1;i<contacts.count();i++){
            m_contacts[contacts.at(i).toList().at(0).toString()]=contacts.at(i).toList().at(1).toString();
        }
        ui->inspectedContactsBox->clear();
        ui->inspectedContactsBox->addItems(m_contacts.keys());
    });
}


void TaskSheetEditor::on_clientBox_currentIndexChanged(int index)
{

    if(!m_userOpereate) return;
    QString clientName=ui->clientBox->currentText();
    if(clientName.isEmpty()) return;
    if(!m_clients.contains(clientName)){//增加委托单位
        if(QMessageBox::question(nullptr,"","该委托单不存在，是否新增委托单位？","确认","取消")) return;
        QString clientAddr=QInputDialog::getText(nullptr,"请输入单位地址:","");
        if(clientAddr.isEmpty()) return;
        QString contact=QInputDialog::getText(nullptr,"请输入联系人:","");
        if(contact.isEmpty()) return;
        QString phone=QInputDialog::getText(nullptr,"请输入联系电话:","");
        if(phone.isEmpty()) return;
        QString sql="insert into client_info(clientName,address) values(?,?);set @clientID=LAST_INSERT_ID();";
        sql+="insert into client_contacts(clientID, name, phoneNum) values(@clientID,?,?);";
        doSql(sql,[this,clientName,clientAddr,contact,phone](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"插入客户数据时错误：",msg.result().toString());
                return;
            }
            doSql("select @clientID;",[this,clientName,clientAddr,contact,phone](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"插入客户数据时错误：",msg.result().toString());
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                int clientID=r.at(1).toList().at(0).toInt();
                m_clients[clientName]={clientID,clientAddr};
                ui->clientContactsBox->clear();
                ui->clientContactsBox->addItem(contact);
                m_contacts[contact]=phone;
                ui->inspectedComBox->addItem(clientName);
                ui->clientAddrEdit->setText(clientAddr);
                ui->clientContactsBox->setCurrentText(contact);
                ui->clientContactsPhoneEdit->setText(phone);
            });

        },0,{clientName,clientAddr,contact,phone});
        return;

    }
    int id=m_clients.value(clientName).ID;
    ui->clientAddrEdit->setText(m_clients.value(clientName).clientAddr);
    QString sql=QString("select name, phoneNum from client_contacts where clientID=%1 and deleted = 0;").arg(id);
    doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QList<QVariant> contacts=msg.result().toList();
        m_contacts.clear();
        for(int i=1;i<contacts.count();i++){
            m_contacts[contacts.at(i).toList().at(0).toString()]=contacts.at(i).toList().at(1).toString();
        }
        ui->clientContactsBox->clear();
        ui->clientContactsBox->addItems(m_contacts.keys());
    });
}


void TaskSheetEditor::on_clientContactsBox_currentIndexChanged(int index)
{
//    ui->clientContactsPhoneEdit->setText(m_contacts.value(ui->clientContactsBox->currentText()));
    if(!m_userOpereate) return;
    QString contact=ui->clientContactsBox->currentText();
    if(contact.isEmpty()) return;
//    if(!m_contacts.contains(contact)){
//        if(QMessageBox::question(nullptr,"","该联系人不存在，是否新增联系人？","确认","取消")) return;
//        QString phone=QInputDialog::getText(nullptr,"请输入联系电话:","");
//        if(phone.isEmpty()) return;
//        int id=m_clients.value(ui->clientBox->currentText()).ID;
//        QString sql="insert into client_contacts(clientID, name, phoneNum) values(?,?,?);";
//        doSql(sql,[this,contact,phone](const QSqlReturnMsg&msg){
//            if(msg.error()){
//                QMessageBox::information(nullptr,"插入客户数据时错误：",msg.result().toString());
//                return;
//            }

//                ui->clientContactsBox->addItem(contact);
//                m_contacts[contact]=phone;

//        },0,{id,contact,phone});
//        return;
//    }
    ui->clientContactsPhoneEdit->setText(m_contacts.value(contact));
}


void TaskSheetEditor::on_clientContactsBox_currentTextChanged(const QString &contact)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_clientContactsBox_currentTextChanged"<<contact;
}

void TaskSheetEditor::on_inspectedContactsBox_currentIndexChanged(int index)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    ui->inspectedPhoneEidt->setText(m_contacts.value(ui->inspectedContactsBox->currentText()));
}

//以下是一些无用的函数，功能已转移，但代码为自动生成的，删除会出错。
//void TaskSheetEditor::on_testFiledBox_currentIndexChanged(int index)
//{

//}


//void TaskSheetEditor::on_testFiledBox_currentTextChanged(const QString &arg1)
//{

//}


//void TaskSheetEditor::on_testTypeBox_currentTextChanged(const QString &arg1)
//{

//}
//以上截止

void TaskSheetEditor::on_testItemAddBtn_clicked()
{

}


void TaskSheetEditor::on_testInofOkBtn_clicked()
{

}


void TaskSheetEditor::on_methodSelectBtn_clicked()
{
    //进入方法选择界面，如果之前方法已经选择了，应当显示下之前的方法。
    //给方法界面传输检测信息（如果没有改动，应当不用设置。）
//    if(!m_taskSheetID){
//        QMessageBox::information(nullptr,"error","你需要先保存任务单。");
//        return;
//    }
    if(m_bTestInfoModified||!m_methodDlgInited){
        m_MethodDlg->setTestInfo(m_testInfo);
        m_MethodDlg->showMethod(m_taskSheetID);
        m_methodDlgInited=true;
    }
    m_MethodDlg->show();
    m_MethodDlg->raise();
}


void TaskSheetEditor::on_checkBox_stateChanged(int arg1)
{
    if(arg1){
        ui->inspectedComBox->setEnabled(false);
        ui->inspectedComBox->setCurrentIndex(ui->clientBox->currentIndex());
        ui->inspectedContactsBox->setEnabled(false);
        ui->inspectedContactsBox->setCurrentIndex(ui->clientContactsBox->currentIndex());
    }
    else{
        ui->inspectedComBox->setEnabled(true);
        ui->inspectedContactsBox->setEnabled(true);
    }
}


void TaskSheetEditor::on_saveBtn_clicked()
{

//    if(m_isSaving){
//        QMessageBox::information(this,"error","数据正在处理中，请稍候。");
//        return;
//    }
    if(m_status!=TaskSheetUI::CREATE&&m_status!=TaskSheetUI::MODIFY&&m_mode!=NewMode){//限制在创建阶段才能保存
        QMessageBox::information(this,"error","任务单已提交，无法保存。");
        return;
    }

    if(!m_testInfo.count()){
        QMessageBox::information(this,"error","请添加检测信息。");
        m_isSaving=false;
        return;
    }
    if(!m_clients.contains(ui->clientBox->currentText())){
        QMessageBox::information(this,"error","检测到新的委托单位，请先点击添加客户。");
        m_isSaving=false;
        return;
    }


    if(m_mode==NewMode){
        //新单，根据编码规则，确认任务单号：
        QString date=QDate::currentDate().toString("yyMMdd");
        QString sql;
        if(!this->tabWiget()->connectDB(CMD_START_Transaction)){//使用事务操作
            QMessageBox::information(nullptr,"error","无法开启事务");
            return;
        }
        sql=QString("INSERT INTO tasknumber (taskdate, taskcount) VALUES (?, 1) ON DUPLICATE KEY UPDATE taskcount = taskcount + 1;");
        doSql(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"更新任务单号时出错：",msg.result().toString());
                sqlFinished();
                tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                return;
            }
            sqlFinished();
        },0,{date});
        waitForSql("正在更新任务单号");
        sql="SELECT taskcount FROM tasknumber WHERE taskdate =?";
        doSql(sql,[date,this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"设置任务单号时出错：",msg.result().toString());
                sqlFinished();
                tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                return;
            }
            QList<QVariant>r=msg.result().toList();
            int count=(r.at(1).toList().at(0).toInt());
            m_taskNum=QString("SST%1%2").arg(date).arg(count,3,10,QChar('0'));
            sqlFinished();
        },0,{date});
        waitForSql("正在设置任务单号");
        tabWiget()->releaseDB(CMD_COMMIT_Transaction);//提交事务
    }
    for(auto info:m_testInfo){
        qDebug()<<"info:"<<info<<info->samplingSites;
    }
    doSave();

}


void TaskSheetEditor::on_exportBtn_clicked()
{
    QString sql;
    statusBar->showMessage("正在打开EXCEL……");
    bool deliveryTest=ui->sampleSourceBox->currentIndex();
    QString creator,phone;
    sql="select creator, phone from (select creator from test_task_info where taskNum=?)  as A left join users on A.creator=users.name;";
    doSql(sql,[this, &creator, &phone](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"获取录单员信息时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()!=2){
            QMessageBox::information(nullptr,"error","未获取到录单员信息。");
            sqlFinished();
            return;
        }
        creator=r.at(1).toList().at(0).toString();
        phone=r.at(1).toList().at(1).toString();
        sqlFinished();
    },1,{m_taskNum});
    waitForSql();
    qApp->processEvents();
    QAxObject*book;
    if(deliveryTest) book=EXCEL.Open(".\\送样任务单.xlsm",QVariant(),true);
    else book=EXCEL.Open(".\\采样任务单.xlsm",QVariant(),true);
    EXCEL.setScreenUpdating(false);
    if(!book){
        QMessageBox::information(nullptr,"错误","无法打开采样任务单。");
        return;
    }
    statusBar->showMessage("正在写入数据……");
    qApp->processEvents();
    QAxObject *sheet=EXCEL.ActiveSheet(book);
    if(deliveryTest){//处理送样单
        EXCEL.writeRange(ui->contractEdit->text(),"C3",sheet);//合同编号
        EXCEL.writeRange(ui->salesRepresentativeBox->currentText(),"C5",sheet);//业务员
        EXCEL.writeRange(m_taskNum,"C4",sheet);//任务单号
        EXCEL.writeRange(user()->name(),"F5",sheet);//录单员
        EXCEL.writeRange(user()->phone(),"J5",sheet);//客服电话
        EXCEL.writeRange(ui->clientBox->currentText(),"C7",sheet);//委托单位
        EXCEL.writeRange(ui->clientAddrEdit->text(),"H7",sheet);//委托单位地址
        EXCEL.writeRange(ui->clientContactsBox->currentText(),"C8",sheet);//委托单位联系人
        EXCEL.writeRange(ui->clientContactsPhoneEdit->text(),"H8",sheet);//联系电话
//        EXCEL.writeRange(ui->inspectedComBox->currentText(),"C9",sheet);//受检单位
//        EXCEL.writeRange(ui->inspectedAddrEdit->text(),"H9",sheet);//受检单位地址
//        EXCEL.writeRange(ui->inspectedContactsBox->currentText(),"C10",sheet);//联系人
//        EXCEL.writeRange(ui->inspectedPhoneEidt->text(),"J10",sheet);//联系电话
        EXCEL.writeRange(ui->projectNameEdit->text(),"C9",sheet);//项目名称
        EXCEL.writeRange(ui->projectAddrEdit->text(),"H9",sheet);//项目地址
        auto testTable=ui->testInfoTableView->data();
        QAxObject* range;
        int startRow=22;
        for(int i=0;i<testTable.count();i++){
            range=EXCEL.selectRow(startRow,sheet);
            range->dynamicCall("copy()");
            range->clear();
            range=EXCEL.selectRow(startRow+1,sheet);
            range->dynamicCall("insert()");
            range->clear();
            auto info=testTable.at(i);
            EXCEL.writeRange(i+1,QString("A%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(0),QString("B%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(1),QString("D%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(2),QString("E%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(3),QString("K%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(4),QString("I%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(5),QString("L%1").arg(startRow),sheet);
            startRow++;
        }
        range=EXCEL.selectRow(startRow,sheet);
        range->dynamicCall("delete()");

        //处理方法表
        statusBar->showMessage("正在填充方法……");
        qApp->processEvents();
        startRow+=4;
        auto m_mthds=m_MethodDlg->methodTable();
        for(int i=0;i<m_mthds.count();i++){
            range=EXCEL.selectRow(startRow,sheet);
            range->dynamicCall("copy()");
            range->clear();
            range=EXCEL.selectRow(startRow+1,sheet);
            range->dynamicCall("insert()");
            range->clear();
            auto info=m_mthds.at(i);
            EXCEL.writeRange(i+1,QString("A%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(0),QString("B%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(1),QString("D%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(2),QString("E%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(4),QString("K%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(5),QString("N%1").arg(startRow),sheet);
            startRow++;
        }
        range=EXCEL.selectRow(startRow,sheet);
        range->dynamicCall("delete()");
    }
    else{//处理采样单
        EXCEL.writeRange(ui->contractEdit->text(),"C3",sheet);//合同编号
        EXCEL.writeRange(ui->salesRepresentativeBox->currentText(),"C4",sheet);//业务员
        EXCEL.writeRange(m_taskNum,"J3",sheet);//任务单号
        EXCEL.writeRange(creator,"F4",sheet);//录单员
        EXCEL.writeRange(phone,"L4",sheet);//客服电话
        EXCEL.writeRange(ui->clientBox->currentText(),"C7",sheet);//委托单位
        EXCEL.writeRange(ui->clientAddrEdit->text(),"J7",sheet);//委托单位地址
        EXCEL.writeRange(ui->clientContactsBox->currentText(),"C8",sheet);//委托单位联系人
        EXCEL.writeRange(ui->clientContactsPhoneEdit->text(),"J8",sheet);//联系电话
        EXCEL.writeRange(ui->inspectedComBox->currentText(),"C9",sheet);//受检单位
        EXCEL.writeRange(ui->inspectedAddrEdit->text(),"J9",sheet);//受检单位地址
        EXCEL.writeRange(ui->inspectedContactsBox->currentText(),"C10",sheet);//联系人
        EXCEL.writeRange(ui->inspectedPhoneEidt->text(),"J10",sheet);//联系电话
        EXCEL.writeRange(ui->projectNameEdit->text(),"C11",sheet);//项目名称
        EXCEL.writeRange(ui->projectAddrEdit->text(),"J11",sheet);//项目地址
        auto testTable=ui->testInfoTableView->data();
        QAxObject* range;
        int startRow=22;
        for(int i=0;i<testTable.count();i++){
            range=EXCEL.selectRow(startRow,sheet);
            range->dynamicCall("copy()");
            range->clear();
            range=EXCEL.selectRow(startRow+1,sheet);
            range->dynamicCall("insert()");
            range->clear();
            auto info=testTable.at(i);
            EXCEL.writeRange(i+1,QString("A%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(0),QString("B%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(1),QString("D%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(2),QString("E%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(3),QString("K%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(5),QString("N%1").arg(startRow),sheet);
            startRow++;
        }
        range=EXCEL.selectRow(startRow,sheet);
        range->dynamicCall("delete()");

        //处理方法表
        statusBar->showMessage("正在填充方法……");
        qApp->processEvents();
        startRow+=8;
        auto m_mthds=m_MethodDlg->methodTable();
        for(int i=0;i<m_mthds.count();i++){
            range=EXCEL.selectRow(startRow,sheet);
            range->dynamicCall("copy()");
            range->clear();
            range=EXCEL.selectRow(startRow+1,sheet);
            range->dynamicCall("insert()");
            range->clear();
            auto info=m_mthds.at(i);
            EXCEL.writeRange(i+1,QString("A%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(0),QString("B%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(1),QString("D%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(2),QString("E%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(4),QString("K%1").arg(startRow),sheet);
            EXCEL.writeRange(info.at(5),QString("N%1").arg(startRow),sheet);
            startRow++;
        }
        range=EXCEL.selectRow(startRow,sheet);
        range->dynamicCall("delete()");
    }


    EXCEL.setScreenUpdating(true);
    statusBar->showMessage("");
    QMessageBox::information(nullptr,"","导出完成。");
}



void TaskSheetEditor::on_submitBtn_clicked()
{
    if(this->tabWiget()->tabName().isEmpty()){
         QMessageBox::information(nullptr,"error","模块名称错误，空的字符串。");
        return;
    }
    if(m_taskNum.isEmpty()){
        QMessageBox::information(nullptr,"error","请先保存任务单。");
        return;
    }
    if(!m_taskSheetID){
        QMessageBox::information(nullptr,"error","请先保存任务单。");
        return;
    }
    if(m_status!=TaskSheetUI::CREATE&&m_status!=TaskSheetUI::MODIFY){
        QMessageBox::information(nullptr,"error","审核流程已经提交过了。");
        return;
    }
    emit submitReview(m_taskSheetID,m_taskNum,ui->sampleSourceBox->currentIndex());
    this->close();
}


void TaskSheetEditor::on_salesRepresentativeBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_salesRepresentativeBox_currentTextChanged"<<arg1;
}


void TaskSheetEditor::on_reportPurposEdit_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_reportPurposEdit_currentTextChanged"<<arg1;
}


void TaskSheetEditor::on_clientBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_clientBox_currentTextChanged"<<arg1;
}

void TaskSheetEditor::on_inspectedComBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_inspectedComBox_currentTextChanged"<<arg1;
}

void TaskSheetEditor::on_inspectedContactsBox_currentTextChanged(const QString &arg1)
{
     qDebug()<<"on_inspectedContactsBox_currentTextChanged:m_userOpereate"<<m_userOpereate;
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
     qDebug()<<"m_bTasksheetModified:on_inspectedContactsBox_currentTextChanged"<<arg1;
}

void TaskSheetEditor::on_reportPeriodBox_textChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_reportPeriodBox_textChanged"<<arg1;
}


void TaskSheetEditor::on_reportCopiesBox_textChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_reportCopiesBox_textChanged"<<arg1;
}


void TaskSheetEditor::on_sampleDisposalBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_sampleDisposalBox_currentTextChanged"<<arg1;
}


void TaskSheetEditor::on_methodSourseBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_methodSourseBox_currentTextChanged"<<arg1;
}


void TaskSheetEditor::on_subpackageBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
    qDebug()<<"m_bTasksheetModified:on_subpackageBox_currentTextChanged"<<arg1;
}

void TaskSheetEditor::on_sampleSourceBox_currentIndexChanged(int index)
{
    setEntrustType(index);
}


void TaskSheetEditor::on_addClientAction_triggered()
{
    ui->ClientEditBtn->setText("确认添加");
    ui->ClientEditBtn->show();
    ui->ClientEditCancelBtn->show();
}


void TaskSheetEditor::on_ClientEditBtn_clicked()
{
    if(ui->ClientEditBtn->text()=="确认添加"){
    QString clientName=ui->clientBox->currentText();
    if(clientName.isEmpty()){
        QMessageBox::information(nullptr,"","请输入委托单位");
        return;
    }
    if(m_clients.contains(clientName)){
        QMessageBox::information(nullptr,"","委托单位已经存在");
        return;
    }
    QString clientAddr=ui->clientAddrEdit->text();
    QString contact=ui->clientContactsBox->currentText();
    if(contact.isEmpty()){
        QMessageBox::information(nullptr,"","请输入联系人");
        return;
    }

    QString phone=ui->clientContactsPhoneEdit->text();

        QString sql="insert into client_info(clientName,address) values(?,?);set @clientID=LAST_INSERT_ID();";
        sql+="insert into client_contacts(clientID, name, phoneNum) values(@clientID,?,?);";
        doSql(sql,[this,clientName,clientAddr,contact,phone](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"插入客户数据时错误：",msg.result().toString());
                return;
            }
            doSql("select @clientID;",[this,clientName,clientAddr,contact,phone](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"插入客户数据时错误：",msg.result().toString());
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                int clientID=r.at(1).toList().at(0).toInt();
                m_clients[clientName]={clientID,clientAddr};
                ui->clientContactsBox->clear();
                ui->clientContactsBox->addItem(contact);
                m_contacts[contact]=phone;
                ui->inspectedComBox->addItem(clientName);
                ui->clientAddrEdit->setText(clientAddr);
                ui->clientContactsBox->setCurrentText(contact);
                ui->clientContactsPhoneEdit->setText(phone);
                QMessageBox::information(nullptr,"","添加成功");
                ui->ClientEditBtn->hide();
                ui->ClientEditCancelBtn->hide();
            });

        },0,{clientName,clientAddr,contact,phone});
    }
    else if(ui->ClientEditBtn->text()=="保存修改"){
        QString client=ui->clientBox->currentText();
        QString oldClient=ui->clientBox->itemText(ui->clientBox->currentIndex());
        int id=m_clients.value(oldClient).ID;
        QJsonArray values;
        bool first = false;
        QString sql="update client_info set ";
        if(client!=oldClient){
            sql+="clientName=? ";
            values.append(client);
            first=true;
        }
        QString address=ui->clientAddrEdit->text();
        if(address!=m_clients.value(oldClient).clientAddr){
            if(first) sql+=", ";
            sql+="address=?";
            values.append(address);

        }
        bool error=false;
        if(values.count()){

            sql+=" where id=?";
            values.append(id);
            doSql(sql,[this, &error, client, address, id](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"保存客户信息时出错：",msg.errorMsg());
                        error=true;
                        sqlFinished();
                        return;
                    }
                    ui->clientBox->setItemText(ui->clientBox->currentIndex(),client);
                    m_clients[client].clientAddr=address;
                    m_clients[client].ID=id;
                    sqlFinished();
                    qDebug()<<" sqlFinished();";
            },0,values);

            waitForSql("在正保存客户信息……");
        }
        if(error) return;

        QString contact=ui->clientContactsBox->currentText();
        QString oldContact=ui->clientContactsBox->itemText(ui->clientContactsBox->currentIndex());
        QString phone=ui->clientContactsPhoneEdit->text();
        QString oldPhone=m_contacts.value(contact);
        sql="update client_contacts set ";
        values={};
        first=false;
        if(contact!=oldContact){
            sql+="name=? ";
            values.append(contact);
            first=true;
        }
        if(phone!=oldPhone){
            if(first) sql+=", ";
            sql+="phoneNum=?";
            values.append(phone);
        }
        if(values.count()){
            sql+=" where name=?";
            values.append(oldContact);
            doSql(sql,[this, &error, contact, phone](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"保存联系人时出错：",msg.errorMsg());
                        error=true;
                        sqlFinished();
                        return;
                    }
                    ui->clientContactsBox->setItemText(ui->clientContactsBox->currentIndex(),contact);
                    m_contacts[contact]=phone;
                    sqlFinished();
                    qDebug()<<" sqlFinished();";
                },0,values);

            waitForSql("正在保存联系人信息……");
        }
        if(!error){
            QMessageBox::information(nullptr,"","修改成功");
        }


    }

}


void TaskSheetEditor::on_ClientEditCancelBtn_clicked()
{
        ui->ClientEditBtn->hide();
        ui->ClientEditCancelBtn->hide();
        ui->addContactBtn->hide();
}


void TaskSheetEditor::on_ClientEditAction_triggered()
{
        ui->ClientEditBtn->setText("保存修改");
        ui->ClientEditBtn->show();
        ui->addContactBtn->show();
        ui->ClientEditCancelBtn->show();
}


void TaskSheetEditor::on_addContactBtn_clicked()
{
        QString contact=QInputDialog::getText(nullptr,"请输入联系人姓名:","");
        if(contact.isEmpty()) return;
        QString phone=QInputDialog::getText(nullptr,"请输入联系电话:","");
        if(phone.isEmpty()) return;
        int id=m_clients.value(ui->clientBox->currentText()).ID;
        if(!id){
            QMessageBox::information(nullptr,"error","未查到委托单位。");
            return;
        }
        QString sql="insert into client_contacts(clientID, name, phoneNum) values(?,?,?);";
        doSql(sql,[this,contact,phone](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"插入客户数据时错误：",msg.result().toString());
                return;
            }

                ui->clientContactsBox->addItem(contact);
                m_contacts[contact]=phone;

        },0,{id,contact,phone});
        return;
}


void TaskSheetEditor::on_otherRequirementsEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_projectAddrEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_projectNameEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_inspectedPhoneEidt_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_inspectedAddrEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_clientAddrEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_clientContactsPhoneEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}


void TaskSheetEditor::on_contractEdit_textChanged(const QString &arg1)
{
        if(!m_userOpereate) return;
        m_bTasksheetModified=true;
}

