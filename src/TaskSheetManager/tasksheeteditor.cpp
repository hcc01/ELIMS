#include "tasksheeteditor.h"
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
TaskSheetEditor::TaskSheetEditor(TabWidgetBase *tabWiget, QWidget *parent) :
    QMainWindow(parent),
    SqlBaseClass(tabWiget),
    ui(new Ui::TaskSheetEditor),
    m_isSaving(false),
    m_bSaved(false),
    m_bTasksheetModified(false),
    m_bTestInfoModified(false),
    editable(true)
{
    ui->setupUi(this);
    ui->widget->hide();
    setWindowTitle("任务单");
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
     doSql("select  clientName,id, address from client_info where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
        }
        QVector<QVariant> clients=msg.result().toList();
        for(int i=1;i<clients.count();i++){
            m_clients[clients.at(i).toList().at(0).toString()]={clients.at(i).toList().at(1).toInt(),clients.at(i).toList().at(2).toString()};
        }
        ui->clientBox->init(m_clients.keys());
        ui->inspectedComBox->init(m_clients.keys());
    });
    doSql("select  clientName,id, address from client_info where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
        }
        QVector<QVariant> clients=msg.result().toList();
        for(int i=1;i<clients.count();i++){
            m_clients[clients.at(i).toList().at(0).toString()]={clients.at(i).toList().at(1).toInt(),clients.at(i).toList().at(2).toString()};
        }
        ui->clientBox->init(m_clients.keys());
        ui->inspectedComBox->init(m_clients.keys());
    });


    ui->testInfoTableView->setHeader({"样品类型","检测点位","检测项目","检测频次","执行标准","备注"});
    ui->testInfoTableView->addContextAction("修改",[this](){
        if(!m_testInfo.count()) return;
        int row=ui->testInfoTableView->selectedRow();
        auto info=m_testInfo.at(row);
    });
    ui->testInfoTableView->addContextAction("添加",[this](){
        TestInfo* info=new TestInfo;
        testInfoEditor ie(info,m_inspectedEentityID);
        connect(&ie,&testInfoEditor::doSql,tabWiget(),&TabWidgetBase::doSqlQuery);
        ie.init();
        int r=ie.exec();
        if(r==QDialog::Accepted){
            m_testInfo.append(info);
            ui->testInfoTableView->append(info->infoList());
        }
        else{
            qDebug()<<"rejested";
            delete info;
        }
        qDebug()<<m_testInfo;
    });

    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->testInfoTableView->setColumnWidth(1,100);
    ui->testInfoTableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    ui->testInfoTableView->setColumnWidth(5,100);


}

void TaskSheetEditor::doSave()
{
//保存任务单，需要保存采样检测任务表test_task_info、点位监测信息表site_monitoring_info、检测方法评审表task_methods、任务单状态表task_status
    QString sql;
    QJsonArray values;
    if(m_bSaved){//更新
        sql="update test_task_info set contractNum=?,clientID=?, clientName=?,clientAddr=?,clientContact=?,clientPhone=?,"
        "inspectedEentityID=?,inspectedEentityName=?,inspectedEentityContact=?,inspectedEentityPhone=?,inspectedProject=?,projectAddr=?,"
        "reportPurpos=?,reportPeriod=?,sampleDisposal=?,reprotCopies=?,methodSource=?,subpackage=?,otherRequirements=? "
              "where taskNum=?;";
        values={ui->contractEdit->text(),m_clients.value(ui->clientBox->currentText()).ID,ui->clientBox->currentText(),ui->clientAddrEdit->text(),ui->clientContactsBox->currentText(),ui->clientContactsPhoneEdit->text(),
                  m_inspectedEentityID,ui->inspectedComBox->currentText(),ui->inspectedContactsBox->currentText(),ui->inspectedPhoneEidt->text(),ui->projectNameEdit->text(),ui->projectAddrEdit->text(),
                  ui->reportPurposEdit->currentText(),ui->reportPeriodBox->value(),ui->sampleDisposalBox->currentText(),ui->reportCopiesBox->value(),ui->methodSourseBox->currentIndex(),ui->subpackageBox->currentIndex(),
                  ui->otherRequirementsEdit->text(),m_taskNum};
        //开始保存点位监测信息
        //这个修改有点复杂：先得确认流程状态，对于已经出具报告的，无法直接修改；
        sql+="SET @task_id = (select id from test_task_info where taskNum=?);delete from task_methods where taskSheetID= @task_id;delete from site_monitoring_info where taskSheetID= @task_id;";
        values.append(m_taskNum);
        qDebug() << m_taskNum ;
        foreach (auto info, m_testInfo) {
          for (int i = 0; i < info->samplingSiteCount;
               i++) { // 将合并的点位拆开保存
            sql += "INSERT INTO site_monitoring_info (taskSheetID, testTypeID, "
                   "samplingSiteName, samplingFrequency, samplingPeriod, "
                   "limitValueID, remark, sampleType) "
                   "VALUES (@task_id , ?,?,?,?,?,?,?);SET @site_id = "
                   "LAST_INSERT_ID();";
            QString smaplingSite;
            if (info->samplingSiteCount == info->samplingSites.count())
              smaplingSite = info->samplingSites.at(i);
            QJsonArray a = {info->testTypeID,
                            smaplingSite,
                            info->samplingFrequency,
                            info->samplingPeriod,
                            info->limitStandardID ? info->limitStandardID
                                                  : QJsonValue::Null,
                            info->remark,
                            info->sampleType};

            foreach (auto x, a)
              values.append(x);
            sql += "INSERT INTO task_methods ( monitoringInfoID, taskSheetID, "
                   "testTypeID, parameterID, parameterName) "
                   "VALUES ";

            // 开始保存点位监测项目
            for (int i = 0; i < info->monitoringParameters.count(); i++) {
              sql += "(@site_id,@task_id, ?, ?, ?)";
              if (i == info->monitoringParameters.count() - 1) {
                sql += ";";
              } else
                sql += ",";
              QJsonArray a = {info->testTypeID, info->parametersIDs.at(i),
                              info->monitoringParameters.at(i)};
              foreach (auto x, a)
                values.append(x);
            }
          }
        }
        doSql(sql,[this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"error",msg.result().toString());
                    return;
                }

                m_bSaved=true;
                m_isSaving=false;
                //            m_taskID=msg.result().toList().at(1).toList().at(0).toInt();

            },0,values);
        return;
    }
    //保存到任务信息表
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
    //保存任务状态，确认之前没有保存过
    if(!m_bSaved) {
        sql+="INSERT INTO task_status (taskSheetID, taskStatus, operator, operateTime) values(@task_id, ?, ?, NOW());";
        values.append(0);
        values.append(user()->name());
    }
//    sql+="SELECT @task_id;";
    doSql(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                return;
            }

            m_bSaved=true;
            m_isSaving=false;
//            m_taskID=msg.result().toList().at(1).toList().at(0).toInt();

        },0,values);
}

void TaskSheetEditor::load(const QString &taskNum)
{
    m_taskNum=taskNum;
    setWindowTitle("任务单："+m_taskNum);
    QString sql;
    sql=QString("SELECT contractNum, salesRepresentative, clientName, clientAddr, clientContact, clientPhone, inspectedEentityName, inspectedEentityContact, "
          "inspectedEentityPhone, inspectedProject, projectAddr, reportPurpos, reportPeriod, sampleDisposal, reprotCopies, methodSource, "
                  "subpackage, otherRequirements from test_task_info where taskNum='%1'").arg(taskNum);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"载入任务信息时错误",msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList().at(1).toList();
        ui->contractEdit->setText(r.at(0).toString());
        ui->salesRepresentativeBox->setCurrentText(r.at(1).toString());
//        ui->clientBox->setCurrentText(r.at(2).toString());
        ui->clientBox->setCurrentIndex(ui->clientBox->findText(r.at(2).toString()));
        ui->clientAddrEdit->setText(r.at(3).toString());
//        ui->clientContactsBox->setCurrentText(r.at(4).toString());
//        ui->clientContactsPhoneEdit->setText(r.at(5).toString());
        ui->inspectedComBox->setCurrentIndex(ui->clientBox->findText(r.at(6).toString()));
//        ui->inspectedContactsBox->setCurrentText(r.at(7).toString());
//        ui->inspectedPhoneEidt->setText(r.at(8).toString());
        ui->projectNameEdit->setText(r.at(9).toString());
        ui->projectAddrEdit->setText(r.at(10).toString());
        ui->reportPurposEdit->setCurrentText(r.at(11).toString());
        ui->reportPeriodBox->setValue(r.at(12).toInt());
        ui->sampleDisposalBox->setCurrentText(r.at(13).toString());
        ui->reportCopiesBox->setValue(r.at(14).toInt());
        ui->methodSourseBox->setCurrentText(r.at(15).toString());
        ui->subpackageBox->setCurrentIndex(r.at(16).toInt());
        ui->otherRequirementsEdit->setText(r.at(17).toString());
    });
    //处理检测信息
    sql=QString("SELECT A.testTypeID, A.sampleType, A.samplingSiteName, A.samplingFrequency, A.samplingPeriod, A.limitValueID, A.remark, A.id, standardName,standardNum, tableName, classNum from"
                  "(SELECT testTypeID, sampleType, samplingSiteName, samplingFrequency, samplingPeriod, limitValueID, remark, id from site_monitoring_info "
                  "where taskSheetID=(SELECT id from test_task_info where taskNum='%1')) as A "
                  "left join implementing_standards on A.limitValueID = implementing_standards.id ").arg(taskNum);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"载入检测信息时错误",msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
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
                if(1/*!m_testInfo.count()||(m_testInfo.last()->monitoringParameters!=monitoringParameters
                                    ||m_testInfo.last()->sampleType!=row.at(1).toString()
                                    ||m_testInfo.last()->samplingFrequency!=row.at(3).toInt()
                                    ||m_testInfo.last()->samplingPeriod!=row.at(4).toInt()
                                    ||m_testInfo.last()->limitStandardID!=row.at(5).toInt())*/){
                    TestInfo* info=new TestInfo;
                    info->testTypeID=row.at(0).toInt();
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
                    for(auto info:m_testInfo)
                        ui->testInfoTableView->append(info->infoList());
                }
            });
        }

    });
    //加载方法
    sql=QString("SELECT B.testType, A.parameterName, A.testMethodName, A.CMA, A.subpackage, A.subpackageDesc from "
                  "(select * from task_methods where taskSheetID=(select id from test_task_info where taskNum='%1')) as A  left join test_type as B on A.testTypeID=B.id "
                  "group by B.testType, A.parameterName, A.testMethodName,  A.CMA, A.subpackage, A.subpackageDesc;").arg(taskNum);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"载入方法信息时错误",msg.result().toString());
            return;
        }
        QList<QVariant>methods=msg.result().toList();
        for(int i=1;i<methods.count();i++){
            m_mthds.append(methods.at(i).toList());
        }
    });
    m_bSaved=true;
}

void TaskSheetEditor::on_inspectedComBox_currentIndexChanged(int index)
{
    QString clientName=ui->inspectedComBox->currentText();
    m_inspectedEentityID=m_clients.value(clientName).ID;
    ui->inspectedAddrEdit->setText(m_clients.value(clientName).clientAddr);
    QString sql=QString("select name, phoneNum from client_contacts where clientID=%1 and deleted = 0;").arg(m_inspectedEentityID);
    doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        ui->inspectedContactsBox->clear();        QVector<QVariant> contacts=msg.result().toList();
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
        QVector<QVariant> contacts=msg.result().toList();
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
}


void TaskSheetEditor::on_clientContactsBox_currentTextChanged(const QString &contact)
{
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

void TaskSheetEditor::on_inspectedContactsBox_currentIndexChanged(int index)
{
    ui->inspectedPhoneEidt->setText(m_contacts.value(ui->inspectedContactsBox->currentText()));
}

//以下是一些无用的函数，功能已转移，但代码为自动生成的，删除会出错。
void TaskSheetEditor::on_testFiledBox_currentIndexChanged(int index)
{

}


void TaskSheetEditor::on_testFiledBox_currentTextChanged(const QString &arg1)
{

}


void TaskSheetEditor::on_testTypeBox_currentTextChanged(const QString &arg1)
{

}
//以上截止

void TaskSheetEditor::on_testItemAddBtn_clicked()
{

}


void TaskSheetEditor::on_testInofOkBtn_clicked()
{

}


void TaskSheetEditor::on_methodSelectBtn_clicked()
{

    //保存到数据库    
    static bool checkSaved=false;
    if(!m_bSaved){

//        QMessageBox::information(this,"error","请先保存。");
        if(!checkSaved) {
            on_saveBtn_clicked();
            checkSaved=true;
        }

//        return;


        QTimer::singleShot(100, this, [this]() {
            on_methodSelectBtn_clicked();
        });
        return;
    }
    checkSaved=false;
    QString sql=QString("select id from test_task_info where taskNum='%1'").arg(m_taskNum);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            qDebug()<<"error.";
            return;
        }

        MethodSelectDlg *dlg=new MethodSelectDlg(msg.result().toList().at(1).toList().at(0).toInt(),this);
        connect(dlg,&MethodSelectDlg::doSql,tabWiget(),&TabWidgetBase::doSqlQuery);
        connect(dlg,&MethodSelectDlg::methodSelected,[this](const QVector<QVector<QVariant>>&table){
            m_mthds=table;
            qDebug()<<table;
        });
        dlg->showMethods(m_mthds);
        dlg->show();
    });

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
    if(m_bSaved){

//            QMessageBox::information(this,"error","已经保存过了。");
        doSave();
            return;

    }
    if(m_isSaving){
        QMessageBox::information(this,"error","数据正在处理中，请稍候。");
        return;
    }
    m_isSaving=true;
    if(!m_testInfo.count()){
        QMessageBox::information(this,"error","请添加检测信息。");
        m_isSaving=false;
        return;
    }
    //获取任务单号：
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
                doSave();
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
                doSave();
            });
        }
    });

}


void TaskSheetEditor::on_exportBtn_clicked()
{
    QString sql;
    QAxObject*book;book=EXCEL.Open("./采样任务单.xlsm",QVariant(),true);
    EXCEL.setScreenUpdating(false);
    if(!book){
        QMessageBox::information(nullptr,"错误","无法打开采样任务单。");
        return;
    }
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
    startRow+=8;
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
    QMessageBox::information(nullptr,"","导出完成。");
}


