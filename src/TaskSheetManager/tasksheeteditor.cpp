#include "tasksheeteditor.h"
#include "ui_tasksheeteditor.h"
#include<QInputDialog>
#include<QMessageBox>
#include"clientmanagerdlg.h"
#include"implementingstandardselectdlg.h"
TaskSheetEditor::TaskSheetEditor(QWidget *parent) :
    QMainWindow(parent),
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
        testInfoEditor ie(info);
        connect(&ie,&testInfoEditor::doSql,this,&TaskSheetEditor::doSql);
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

void TaskSheetEditor::on_inspectedComBox_currentIndexChanged(int index)
{
    QString clientName=ui->inspectedComBox->currentText();
    int id=m_clients.value(clientName).ID;
    ui->inspectedAddrEdit->setText(m_clients.value(clientName).clientAddr);
    QString sql=QString("select name, phoneNum from client_contacts where clientID=%1 and deleted = 0;").arg(id);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
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
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
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

