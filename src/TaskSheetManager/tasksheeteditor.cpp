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
TaskSheetEditor::TaskSheetEditor(TabWidgetBase *tabWiget, int openMode) :
    QMainWindow(tabWiget),
    SqlBaseClass(tabWiget),
    ui(new Ui::TaskSheetEditor),
    m_infoEditor(nullptr,0),
    m_userOpereate(false),
    m_isSaving(false),
//    m_bSaved(false),
    m_bTasksheetModified(false),
    m_bTestInfoModified(false),
    m_status(0),
    m_mode(openMode)
{
    ui->setupUi(this);
    ui->widget->hide();
    setWindowTitle("任务单");

    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    m_MethodDlg=new MethodSelectDlg(this->tabWiget());
    if(m_mode==ReviewMode){
        ui->submitBtn->hide();
    }
    else if(m_mode==ViewMode){
        ui->saveBtn->hide();
        ui->submitBtn->hide();
    }

    ui->testInfoTableView->setHeader({"样品类型","检测点位","检测项目","检测频次","执行标准","备注"});
    ui->testInfoTableView->addContextAction("修改",[this](){
        if(!m_testInfo.count()) return;
        int row=ui->testInfoTableView->selectedRow();
        if(row<0) return;
        auto info=m_testInfo.at(row);
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
        m_infoEditor.load(info);
        testInfoEditor ie(info);
        int r=m_infoEditor.exec();
        if(r==QDialog::Accepted){
            m_testInfo.append(info);
//            ui->testInfoTableView->append(info->infoList());
//            m_bTestInfoModified=true;
            updateTestInfoView();
        }
        else{
            qDebug()<<"rejested";
            delete info;
        }
    });

    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Custom);
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
    connect(&m_infoEditor,&testInfoEditor::doSql,this->tabWiget(),&TabWidgetBase::doSqlQuery);
    m_infoEditor.init();
    connect(&m_infoEditor,&testInfoEditor::accepted,[this](){//修改时，更新。添加时，需要在添加菜单里处理数据后更新。
        m_bTestInfoModified=true;
        m_bMethodModified=false;//检测项目更改时，为了提醒用户重新做方法选择，这个强制把方法更改设为否。
        qDebug()<<"m_bTestInfoModified=true";
        updateTestInfoView();
    });
    connect(m_MethodDlg,&MethodSelectDlg::accepted,[this](){//修改时，更新。添加时，需要在添加菜单里处理数据后更新。
        m_bMethodModified=true;
        qDebug()<<"m_bMethodModified";
    });
     doSql("select  clientName,id, address from client_info where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
        }
        QList<QVariant> clients=msg.result().toList();
        for(int i=1;i<clients.count();i++){
            m_clients[clients.at(i).toList().at(0).toString()]={clients.at(i).toList().at(1).toInt(),clients.at(i).toList().at(2).toString()};
        }
        ui->clientBox->init(m_clients.keys());
        ui->inspectedComBox->init(m_clients.keys());
        m_userOpereate=true;//代码的改变选择框完成
    });
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
    if(m_status!=TaskSheetUI::CREATE&&m_status!=TaskSheetUI::MODIFY){//保存仅限于创建和修改状态(后续有其它需求再处理）
        QMessageBox::information(nullptr,"error","当前状态无法修改。");
        return;
    }
    QString sql;
    QJsonArray values;
    if(m_mode==EditMode){//修改模式，更新数据库
        if(m_bTasksheetModified){
            qDebug()<<"m_bTasksheetModified,saving sheetinfo";
            //进行基本信息保存
            sql="update test_task_info set contractNum=?,clientID=?, clientName=?,clientAddr=?,clientContact=?,clientPhone=?,"
                  "inspectedEentityID=?,inspectedEentityName=?,inspectedEentityContact=?,inspectedEentityPhone=?,inspectedProject=?,projectAddr=?,"
                  "reportPurpos=?,reportPeriod=?,sampleDisposal=?,reprotCopies=?,methodSource=?,subpackage=?,otherRequirements=? "
                  "where taskNum=?;";
            values={ui->contractEdit->text(),m_clients.value(ui->clientBox->currentText()).ID,ui->clientBox->currentText(),ui->clientAddrEdit->text(),ui->clientContactsBox->currentText(),ui->clientContactsPhoneEdit->text(),
                      m_inspectedEentityID,ui->inspectedComBox->currentText(),ui->inspectedContactsBox->currentText(),ui->inspectedPhoneEidt->text(),ui->projectNameEdit->text(),ui->projectAddrEdit->text(),
                      ui->reportPurposEdit->currentText(),ui->reportPeriodBox->value(),ui->sampleDisposalBox->currentText(),ui->reportCopiesBox->value(),ui->methodSourseBox->currentIndex(),ui->subpackageBox->currentIndex(),
                      ui->otherRequirementsEdit->text(),m_taskNum};
        }
        if(m_bTestInfoModified){
            qDebug()<<"m_bTestInfoModified, saving testInfo";
            //保存检测信息
            sql+="delete from task_methods where taskSheetID= ?;delete from site_monitoring_info where taskSheetID=?;";//删除点位监测信息表和检测方法表
            values.append(m_taskSheetID);values.append(m_taskSheetID);//保存时用到m_taskSheetID，要确保任务单ID得到正确获取
            foreach (auto info, m_testInfo) {//保存监测信息
                for (int i = 0; i < info->samplingSiteCount;i++) { // 将合并的点位拆开保存
                    sql += "INSERT INTO site_monitoring_info (taskSheetID, testTypeID, "
                           "samplingSiteName, samplingFrequency, samplingPeriod, "
                           "limitValueID, remark, sampleType) "
                           "VALUES (?, ?,?,?,?,?,?,?);SET @site_id = "
                           "LAST_INSERT_ID();";
                    QString smaplingSite;
                    if (info->samplingSiteCount == info->samplingSites.count())
                        smaplingSite = info->samplingSites.at(i);
                    values.append(m_taskSheetID);
                    values.append(info->testTypeID);
                    values.append(smaplingSite);
                    values.append(info->samplingFrequency);
                    values.append(info->samplingPeriod);
                    values.append(info->limitStandardID ? info->limitStandardID : QJsonValue::Null);
                    values.append(info->remark);
                    values.append(info->sampleType);

                    sql += "INSERT INTO task_methods ( monitoringInfoID, taskSheetID, "
                           "testTypeID, parameterID, parameterName) "
                           "VALUES ";
                    // 开始保存点位监测项目

                    for (int i = 0; i < info->monitoringParameters.count(); i++) {
                        sql += "(@site_id,?, ?, ?, ?)";
                        if (i == info->monitoringParameters.count() - 1) {
                            sql += ";";
                        } else
                            sql += ",";
                        values.append(m_taskSheetID);
                        values.append(info->testTypeID);
                        values.append(info->parametersIDs.at(i));
                        values.append(info->monitoringParameters.at(i));
                    }
                }
            }
//            m_bMethodModified=true;//同时修改和保存方法表，保存后将方法修改标为非。
            if(!m_bMethodModified){
                int a=QMessageBox::question(nullptr,"","检测到监测信息有变更，但检测方法未做更改，是否继续保存？");
                if(a==QMessageBox::No) return;
            }
        }
        if(m_bMethodModified){
            //更新方法
            saveMethod();
        }
        if(sql.isEmpty()) {

            return;
        }
        m_isSaving=true;//准备发送数据，在完成前，防止重复操作。
        doSql(sql,[this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"保存出错：",msg.result().toString());
                    return;
                }

//                m_bSaved=true;
                m_isSaving=false;
                m_bMethodModified=false;
                m_bTasksheetModified=false;
                m_bTestInfoModified=false;
                //            m_taskID=msg.result().toList().at(1).toList().at(0).toInt();
                QMessageBox::information(nullptr,"","保存完成。");

            },0,values);
        return;//修改操作完成，返回，下面是新建操作。
    }
    //保存新任务单
    else if(m_mode==NewMode){
        sql="INSERT INTO test_task_info (taskNum,contractNum,clientID,clientName,clientAddr,clientContact,clientPhone,"
              "inspectedEentityID,inspectedEentityName,inspectedEentityContact,inspectedEentityPhone,inspectedProject,projectAddr,"
              "reportPurpos,reportPeriod,sampleDisposal,reprotCopies,methodSource,subpackage,otherRequirements, creator, createDate) "
              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, NOW());SET @task_id = LAST_INSERT_ID();";
        values={m_taskNum,ui->contractEdit->text(),m_clients.value(ui->clientBox->currentText()).ID,ui->clientBox->currentText(),ui->clientAddrEdit->text(),ui->clientContactsBox->currentText(),ui->clientContactsPhoneEdit->text(),
                  m_inspectedEentityID,ui->inspectedComBox->currentText(),ui->inspectedContactsBox->currentText(),ui->inspectedPhoneEidt->text(),ui->projectNameEdit->text(),ui->projectAddrEdit->text(),
                  ui->reportPurposEdit->currentText(),ui->reportPeriodBox->value(),ui->sampleDisposalBox->currentText(),ui->reportCopiesBox->value(),ui->methodSourseBox->currentIndex(),ui->subpackageBox->currentIndex(),
                  ui->otherRequirementsEdit->text(),user()->name()};
        //开始保存点位监测信息
        foreach(auto info,m_testInfo){
            for(int i=0;i<info->samplingSiteCount;i++){//将合并的点位拆开保存
                sql+="INSERT INTO site_monitoring_info (taskSheetID, testTypeID, samplingSiteName, samplingFrequency, samplingPeriod, limitValueID, remark, sampleType) "
                       "VALUES (@task_id , ?,?,?,?,?,?,?);SET @site_id = LAST_INSERT_ID();";
                QString smaplingSite;
                if(info->samplingSiteCount==info->samplingSites.count()) smaplingSite=info->samplingSites.at(i);
                QJsonArray a={info->testTypeID,smaplingSite,info->samplingFrequency,info->samplingPeriod,info->limitStandardID?info->limitStandardID:QJsonValue::Null,info->remark,info->sampleType};

                foreach(auto x,a) values.append(x);
                sql+="INSERT INTO task_methods ( monitoringInfoID, taskSheetID, testTypeID, parameterID, parameterName) "
                       "VALUES ";

                //开始保存点位监测项目
                for(int i=0;i<info->monitoringParameters.count();i++){
                    sql+="(@site_id,@task_id, ?, ?, ?)";
                    if(i==info->monitoringParameters.count()-1){
                        sql+=";";
                    }
                    else sql+=",";
                    QJsonArray a={info->testTypeID,info->parametersIDs.at(i),info->monitoringParameters.at(i)};
                    foreach(auto x,a) values.append(x);
                }
            }

        }
        m_isSaving=true;//准备发送数据，在完成前，防止重复操作。
        doSql(sql,[this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"error",msg.result().toString());
                    return;
                }
                //保存完成后，改为修改模式，以便用户继续修改。
                m_mode=EditMode;
                if(saveMethod()) QMessageBox::information(nullptr,"","保存完成。");//保存方法在监测信息更新完后才能进行
                //新任务单保存后，记录任务单ID
                doSql(QString("select id from test_task_info where taskNum='%1'").arg(m_taskNum),[this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(this,"查询任务单ID时出错",msg.result().toString());
                        return;
                    }
                    if(msg.result().toList().count()!=2){
                        QMessageBox::information(this,"error","查询任务单ID时出错，数量错误。");
                        return;
                    }
                     m_taskSheetID=msg.result().toList().at(1).toList().at(0).toInt();
                });

            },0,values);
    }
}

bool TaskSheetEditor::saveMethod()
{
    QString sql;
    QJsonArray values;

    sql="update task_methods set testMethodID=?, testMethodName=?, subpackage=?, subpackageDesc=? where taskSheetID=? and testTypeID=? and parameterID=?;";
    for(auto info:m_testInfo){
        int testTypeID=info->testTypeID;
        for(auto parameterID:info->parametersIDs){

            bool error=false;
            MethodMorePtr mm=m_MethodDlg->getMethod(testTypeID,parameterID);//按检测类型和参数确认方法
            if(mm.isNull()) {
                QMessageBox::information(nullptr,"保存方法时出错：",QString("no mehtod find,参数：%1(%2)，类型:%3(%4)").arg(info->monitoringParameters.at(info->parametersIDs.indexOf(parameterID))).arg(parameterID).arg(info->sampleType).arg(testTypeID));
                return true;
            }
            if(mm->testMethodName.isEmpty()) continue;//没方法的项目，跳过
            values={mm->methodID,mm->testMethodName,mm->subpackage,mm->subpackageDesc,m_taskSheetID, testTypeID,parameterID};

            qDebug()<<"正在保存方法";
            doSql(sql,[this,&error,mm](const QSqlReturnMsg&msg){
                    qDebug()<<QString("数据处理中%1(%2)：").arg(mm->testMethodName).arg(mm->methodID),msg.result().toString();
                if(msg.error()){
                    QMessageBox::information(nullptr,QString("更新方法%1(%2)时错误：").arg(mm->testMethodName).arg(mm->methodID),msg.result().toString());
                    error=true;
                    sqlFinished();
                    return ;
                }
                sqlFinished();
                qDebug()<<"保存完成";
            },0,values);
            waitForSql("正在保存方法……");

            if(error) {
                return false;
            }
        }
    }
    return true;
}

void TaskSheetEditor::load(const QString &taskNum)
{
    m_taskNum=taskNum;
    m_userOpereate=false;
    setWindowTitle("任务单："+m_taskNum);
    QString sql;
    sql=QString("SELECT contractNum, salesRepresentative, clientName, clientAddr, clientContact, clientPhone, inspectedEentityName, inspectedEentityContact, "
          "inspectedEentityPhone, inspectedProject, projectAddr, reportPurpos, reportPeriod, sampleDisposal, reprotCopies, methodSource, "
                  "subpackage, otherRequirements, id,taskStatus ,creator from test_task_info where taskNum='%1'").arg(taskNum);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"载入任务信息时错误",msg.result().toString());
            emit sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList().at(1).toList();
        ui->contractEdit->setText(r.at(0).toString());
        ui->salesRepresentativeBox->setCurrentText(r.at(1).toString());
        ui->clientBox->setCurrentText(r.at(2).toString());
//        ui->clientBox->setCurrentIndex(ui->clientBox->findText(r.at(2).toString()));
        ui->clientAddrEdit->setText(r.at(3).toString());
        ui->clientContactsBox->setCurrentText(r.at(4).toString());

        ui->clientContactsPhoneEdit->setText(r.at(5).toString());
        ui->inspectedComBox->setCurrentIndex(ui->clientBox->findText(r.at(6).toString()));
        ui->inspectedAddrEdit->setText(m_clients.value(ui->inspectedComBox->currentText()).clientAddr);
        qDebug()<<m_clients.keys();
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
        m_status=r.at(19).toInt();
        m_createor=r.at(20).toString();
        m_userOpereate=true;
        sqlFinished();
    });
    waitForSql();
    //处理检测信息
    sql=QString("SELECT A.testTypeID, A.sampleType, A.samplingSiteName, A.samplingFrequency, A.samplingPeriod, A.limitValueID, A.remark, A.id, standardName,standardNum, tableName, classNum, testFieldID from"
                  "(SELECT testTypeID, sampleType, samplingSiteName, samplingFrequency, samplingPeriod, limitValueID, remark, id from site_monitoring_info "
                  "where taskSheetID=%1) as A "
                  "left join implementing_standards on A.limitValueID = implementing_standards.id left join test_type on A.testTypeID= test_type.id;").arg(m_taskSheetID);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"载入检测信息时错误",msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        m_testInfo.clear();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            int monitorID=row.at(7).toInt();

            QString sql=QString("SELECT parameterID, parameterName, testMethodID, testMethodName from task_methods where monitoringInfoID='%1' ;").arg(monitorID);
            doSql(sql,[this, r,i, row](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"载入项目信息时错误",msg.result().toString());
                    return;
                }
                QList<QVariant>parameters=msg.result().toList();
                QList<int>parameterIDs;
                QStringList monitoringParameters;
                for(int i=1;i<parameters.count();i++){
                    parameterIDs.append(parameters.at(i).toList().at(0).toInt());
                    monitoringParameters.append(parameters.at(i).toList().at(1).toStringList());
                }
                if(!m_testInfo.count()||(m_testInfo.last()->monitoringParameters!=monitoringParameters
                                    ||m_testInfo.last()->sampleType!=row.at(1).toString()
                                    ||m_testInfo.last()->samplingFrequency!=row.at(3).toInt()
                                    ||m_testInfo.last()->samplingPeriod!=row.at(4).toInt()
                                    ||m_testInfo.last()->limitStandardID!=row.at(5).toInt()
                                    ||m_testInfo.last()->remark!=row.at(6).toString())){
                    TestInfo* info=new TestInfo;
                    info->testTypeID=row.at(0).toInt();
                    info->testFieldID=row.at(12).toInt();
                    info->sampleType=row.at(1).toString();
                    if(!row.at(2).toString().isEmpty()) info->samplingSites.append(row.at(2).toString());
                    info->samplingFrequency=row.at(3).toInt();
                    info->samplingPeriod=row.at(4).toInt();
                    info->limitStandardID=row.at(5).toInt();
                    info->remark=row.at(6).toString();
                    info->monitoringParameters=monitoringParameters;
                    info->parametersIDs=parameterIDs;
                    info->limitStandard=QString("%1(%2)%3%4").arg(row.at(8).toString()).arg(row.at(9).toString()).arg(row.at(10).toString()).arg(row.at(11).toString());
                    m_testInfo.append(info);

                }
                else{//合并监测指标相同的点位
                    m_testInfo.last()->samplingSiteCount++;
                    if(!row.at(2).toString().isEmpty()) m_testInfo.last()->samplingSites.append(row.at(2).toString());
                }
                if(i==r.count()-1){
                    ui->testInfoTableView->clear();
                    for(auto info:m_testInfo)
                        ui->testInfoTableView->append(info->infoList());
                }
            });
        }

    });
    //加载方法

    sql=QString("SELECT A.testTypeID, sampleType, parameterName, testMethodName, CMA, subpackage, subpackageDesc,parameterID from "
                  "(select * from task_methods where taskSheetID=%1 ) as A left join site_monitoring_info on A.monitoringInfoID= site_monitoring_info.id "
                  "group by A.testTypeID, sampleType, parameterName, testMethodName,  CMA, subpackage, subpackageDesc,parameterID;").arg(m_taskSheetID);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"载入方法信息时错误",msg.result().toString());
            return;
        }
        QList<QVariant>methods=msg.result().toList();
        QList<QList<QVariant>> methodTable;
//        m_mthds.clear();
        QHash<int,QHash<int,int>>parametersRows;//[类型，【参数ID，行号】】
        for(int i=1;i<methods.count();i++){
            QList row=methods.at(i).toList();
            int typeID=row.at(0).toInt();
            int parameterID=row.at(7).toInt();
            QString sampleType=row.at(1).toString();
            //把相同检测类型的项目合并在一起
            if(parametersRows.value(typeID).contains(parameterID)){
                int r=parametersRows.value(typeID).value(parameterID);
                QString type=methodTable.value(r).at(0).toString();
                if(!type.contains(sampleType)){
                    methodTable[r][0]=type.append("/").append(sampleType);
                }

            }
            else{
                methodTable.append({row.at(1),row.at(2),row.at(3),row.at(4).toInt()?"是":"否",row.at(5).toInt()?"是":"否",row.at(6)});
                parametersRows[typeID][parameterID]=methodTable.count()-1;
                MethodMore* mm=new MethodMore;
                mm->testMethodName=row.at(3).toString();//这里用于方法选择时载入已选的方法，不用保存ID
                mm->subpackage=row.at(5).toInt();
                mm->subpackageDesc=row.at(6).toString();
                m_MethodDlg->addMethod(typeID,parameterID,mm);//已选的方法，用于方法加载时，自动选用已经确认的方法。
            }
        }
        m_MethodDlg->showMethods(methodTable);
    });
    //    m_bSaved=true;
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


void TaskSheetEditor::on_inspectedComBox_currentIndexChanged(int index)
{
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
    if(!m_contacts.contains(contact)){
        if(QMessageBox::question(nullptr,"","该联系人不存在，是否新增联系人？","确认","取消")) return;
        QString phone=QInputDialog::getText(nullptr,"请输入联系电话:","");
        if(phone.isEmpty()) return;
        int id=m_clients.value(ui->clientBox->currentText()).ID;
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
    ui->clientContactsPhoneEdit->setText(m_contacts.value(contact));
}


void TaskSheetEditor::on_clientContactsBox_currentTextChanged(const QString &contact)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}

void TaskSheetEditor::on_inspectedContactsBox_currentIndexChanged(int index)
{
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
    m_MethodDlg->setTestInfo(m_testInfo);
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

    if(m_isSaving){
        QMessageBox::information(this,"error","数据正在处理中，请稍候。");
        return;
    }
    if(m_status!=TaskSheetUI::CREATE&&m_status!=TaskSheetUI::MODIFY){//限制在创建阶段才能保存
        QMessageBox::information(this,"error","任务单已提交，无法保存。");
        return;
    }

    if(!m_testInfo.count()){
        QMessageBox::information(this,"error","请添加检测信息。");
        m_isSaving=false;
        return;
    }
    if(m_mode==NewMode){
        //新单，根据编码规则，确认任务单号：
        QString date=QDate::currentDate().toString("yyMMdd");
        QString sql;
        sql=QString("select taskcount from tasknumber where taskdate='%1';").arg(date);
        doSql(sql,[date,this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                m_isSaving=false;
                return;
            }
            QList<QVariant>r=msg.result().toList();
            if(r.count()==1){
                QString sql=QString("INSERT INTO tasknumber VALUES('%1',1);").arg(date);
                doSql(sql,[date,this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(this,"error",msg.result().toString());
                        m_isSaving=false;
                        return;
                    }
                    m_taskNum=QString("SST%1%2").arg(date).arg("001");
                    setWindowTitle("任务单："+m_taskNum);
                    sqlFinished();
//                    doSave();在这里保存会卡在等待界面
                });
            }
            else{
                int count=(r.at(1).toList().at(0).toInt());
                count++;
                m_taskNum=QString("SST%1%2").arg(date).arg(count,3,10,QChar('0'));
                setWindowTitle("任务单："+m_taskNum);
                QString sql=QString("update tasknumber set taskcount =%1 where taskdate='%2';").arg(count).arg(date);//任务单编号自增
                doSql(sql,[date,this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(this,"error",msg.result().toString());
                        m_isSaving=false;
                        return;
                    }
//                    doSave();
                    sqlFinished();
                });
            }
        });
        waitForSql();
        doSave();
    }
    else{
        doSave();
    }
}


void TaskSheetEditor::on_exportBtn_clicked()
{
    QString sql;
    statusBar->showMessage("正在打开EXCEL……");
    qApp->processEvents();
    QAxObject*book;book=EXCEL.Open(".\\采样任务单.xlsm",QVariant(),true);
    EXCEL.setScreenUpdating(false);
    if(!book){
        QMessageBox::information(nullptr,"错误","无法打开采样任务单。");
        return;
    }
    statusBar->showMessage("正在写入数据……");
    qApp->processEvents();
    QAxObject *sheet=EXCEL.ActiveSheet(book);
    EXCEL.writeRange(ui->contractEdit->text(),"C3");
    EXCEL.writeRange(ui->salesRepresentativeBox->currentText(),"C4",sheet);
    EXCEL.writeRange(m_taskNum,"J3",sheet);
    EXCEL.writeRange(user()->name(),"F4",sheet);
    EXCEL.writeRange(ui->clientBox->currentText(),"C7",sheet);
    EXCEL.writeRange(ui->clientAddrEdit->text(),"J7",sheet);
    EXCEL.writeRange(ui->clientContactsBox->currentText(),"C8",sheet);
    EXCEL.writeRange(ui->clientContactsPhoneEdit->text(),"J8",sheet);
    EXCEL.writeRange(ui->inspectedComBox->currentText(),"C9",sheet);
    EXCEL.writeRange(ui->inspectedAddrEdit->text(),"J9",sheet);
    EXCEL.writeRange(ui->inspectedContactsBox->currentText(),"C10",sheet);
    EXCEL.writeRange(ui->inspectedPhoneEidt->text(),"J10",sheet);
    EXCEL.writeRange(ui->projectNameEdit->text(),"C11",sheet);
    EXCEL.writeRange(ui->projectAddrEdit->text(),"J11",sheet);
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
    if(m_status!=TaskSheetUI::CREATE&&m_status!=TaskSheetUI::MODIFY){
        QMessageBox::information(nullptr,"error","审核流程已经提交过了。");
        return;
    }
    emit submitReview(m_taskSheetID,m_taskNum);
    this->close();
}


void TaskSheetEditor::on_contractEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_salesRepresentativeBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_reportPurposEdit_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_clientBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_clientAddrEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_clientContactsPhoneEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_inspectedComBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_inspectedAddrEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_inspectedContactsBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_inspectedPhoneEidt_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_projectNameEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_projectAddrEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_reportPeriodBox_textChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_reportCopiesBox_textChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_sampleDisposalBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_methodSourseBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_subpackageBox_currentTextChanged(const QString &arg1)
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}


void TaskSheetEditor::on_otherRequirementsEdit_editingFinished()
{
    if(!m_userOpereate) return;
    m_bTasksheetModified=true;
}

