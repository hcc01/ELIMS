#include "tasksheeteditor.h"
#include "ui_tasksheeteditor.h"
#include<QInputDialog>
#include<QMessageBox>
#include"clientmanagerdlg.h"
#include"implementingstandardselectdlg.h"
#include"methodselectdlg.h"
#include<QJsonArray>
TaskSheetEditor::TaskSheetEditor(TabWidgetBase *tabWiget, QWidget *parent) :
    QMainWindow(parent),
    SqlBaseClass(tabWiget),
    m_bTasksheetModified(false),
    m_isSaving(false),
    m_bTestInfoModified(false),
    m_bSaved(false),
    ui(new Ui::TaskSheetEditor)
{
    ui->setupUi(this);
    ui->widget->hide();
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
    emit doSql("select  clientName,id, address from client_info where deleted=0;",[&](const QSqlReturnMsg&msg){
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

    QString sql;
    QJsonArray values;
    //保存到任务信息表
    sql="INSERT INTO test_task_info (taskNum,contractNum,clientID,clientName,clientAddr,clientContact,clientPhone,"
          "inspectedEentityID,inspectedEentityName,inspectedEentityContact,inspectedEentityPhone,inspectedProject,projectAddr,"
          "reportPurpos,reportPeriod,sampleDisposal,reprotCopies,methodSource,subpackage,otherRequirements) "
          "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);SET @task_id = LAST_INSERT_ID();";
    values={m_taskNum,ui->contractEdit->text(),m_clients.value(ui->clientBox->currentText()).ID,ui->clientBox->currentText(),ui->clientAddrEdit->text(),ui->clientContactsBox->currentText(),ui->clientContactsPhoneEdit->text(),
              m_inspectedEentityID,ui->inspectedComBox->currentText(),ui->inspectedContactsBox->currentText(),ui->inspectedPhoneEidt->text(),ui->projectNameEdit->text(),ui->projectAddrEdit->text(),
              ui->reportPurposEdit->currentText(),ui->reportPeriodBox->value(),ui->sampleDisposalBox->currentText(),ui->reportCopiesBox->value(),ui->methodSourseBox->currentIndex(),ui->subpackageBox->currentIndex(),
              ui->otherRequirementsEdit->text()};
    //开始保存点位监测信息
    foreach(auto info,m_testInfo){
        for(int i=0;i<info->samplingSiteCount;i++){
            sql+="INSERT INTO site_monitoring_info (taskSheetID, testTypeID, samplingSiteName, samplingFrequency, samplingPeriod, limitValueID, remark) "
                   "VALUES (@task_id , ?,?,?,?,?,?);SET @site_id = LAST_INSERT_ID();";
            QString smaplingSite;
            if(info->samplingSiteCount==info->samplingSites.count()) smaplingSite=info->samplingSites.at(i);
            QJsonArray a={info->testTypeID,smaplingSite,info->samplingFrequency,info->samplingPeriod,info->limitStandardID?info->limitStandardID:QJsonValue::Null,info->remark};
            qDebug()<<a;
            foreach(auto x,a) values.append(x);
            sql+="INSERT INTO task_methods ( monitoringInfoID, taskSheetID, testTypeID, parameterID, parameterName) "
                   "VALUES ";
            //开始保存点位监测项目
            for(int i=0;i<info->monitoringParameters.count();i++){
                sql+="(@site_id,@task_id , ?,?,?)";
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
    ui->clientContactsPhoneEdit->setText(m_contacts.value(ui->clientContactsBox->currentText()));
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


void TaskSheetEditor::on_testItemAddBtn_clicked()
{

}


void TaskSheetEditor::on_testInofOkBtn_clicked()
{

}


void TaskSheetEditor::on_methodSelectBtn_clicked()
{
    qDebug()<<user()->name();
    //保存到数据库
    if(!m_bSaved){
        QMessageBox::information(this,"error","请先保存。");
        return;

    }
    QString sql=QString("select id from test_task_info where taskNum='%1'").arg(m_taskNum);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            qDebug()<<"error.";
            return;
        }

        MethodSelectDlg *dlg=new MethodSelectDlg(msg.result().toList().at(1).toList().at(0).toInt(),this);
        connect(dlg,&MethodSelectDlg::doSql,tabWiget(),&TabWidgetBase::doSqlQuery);
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

            QMessageBox::information(this,"error","已经保存过了。");
            return;

    }
    if(m_isSaving){
        QMessageBox::information(this,"error","数据正在处理中，请稍候。");
        return;
    }
    m_isSaving=true;
    if(!m_testInfo.count()){
        QMessageBox::information(this,"error","请添加检测信息。");
        return;
    }
    //获取任务单号：
    QString date=QDate::currentDate().toString("yyMMdd");
    QString sql;
    sql=QString("select taskcount from tasknumber where taskdate='%1';").arg(date);
    doSql(sql,[date,this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()==1){
            QString sql=QString("INSERT INTO tasknumber VALUES('%1',1);").arg(date);
            doSql(sql,[date,this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"error",msg.result().toString());
                    return;
                }
                m_taskNum=QString("SST{%1}001").arg(date);
                doSave();
            });
        }
        else{
            int count=(r.at(1).toList().at(0).toInt());
            count++;
            m_taskNum=QString("SST%1%2").arg(date).arg(count,3,10,QChar('0'));
            QString sql=QString("update tasknumber set taskcount =%1 where taskdate='%2';").arg(count).arg(date);
            doSql(sql,[date,this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"error",msg.result().toString());
                    return;
                }
                doSave();
            });
        }
    });

}

