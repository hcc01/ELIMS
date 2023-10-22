#include "clientmanagerdlg.h"
#include "ui_clientmanagerdlg.h"
#include<QMessageBox>
#include<QCompleter>
ClientManagerDlg::ClientManagerDlg(bool editMod, QMap<QString, ClientInfo> *clients, QWidget *parent) :
    QDialog(parent),
    m_editMod(editMod),
    ui(new Ui::ClientManagerDlg),
    m_clients(clients)
{
    ui->setupUi(this);
    if(!editMod){
        ui->clientAddrEdit->setEnabled(false);
//        ui->clientNameBox->setEditable(false);
        ui->contactsGroup->setVisible(false);
        foreach (QWidget *widget, ui->contactsGroup->findChildren<QWidget*>()) {
            widget->setVisible(false);
        }
    }

    ui->contactsView->setHeader({"联系人","职务","联系电话","电子邮箱"});
    ui->clientNameBox->addItems(m_clients->keys());
    QCompleter *completer = new QCompleter(m_clients->keys(), ui->clientNameBox);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    ui->clientNameBox->setCompleter(completer);
}

ClientManagerDlg::~ClientManagerDlg()
{
    delete ui;
}


void ClientManagerDlg::on_OKButton_clicked()
{
    QString clientName=ui->clientNameBox->currentText();
    if(clientName.isEmpty()){
        QMessageBox::information(this,"error","请输入客户名称。");
        return;
    }
    QString clientAddr=ui->clientAddrEdit->text();
    if(clientAddr.isEmpty()){
        QMessageBox::information(this,"error","请输入客户地址。");
        return;
    }
    QList<QList<QVariant>> contactsList=ui->contactsView->data();
    if(!contactsList.count()){
        QMessageBox::information(this,"error","请添加至少1个联系人信息。");
        return;
    }
    if(m_editMod){
        emit doSql("select clientName from client_info where deleted=0",[&](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
            }
            ui->clientNameBox->clear();
            ui->clientNameBox->addItems( qvariant_cast<QStringList>(msg.result().toList().at(0).toList()));
        });
        QString sql;
        sql+="START TRANSACTION;";

        sql+=QString("INSERT INTO client_info (clientName, address) VALUES ('%1', '%2');").arg(clientName).arg(clientAddr);

        sql+="SET @clientID = LAST_INSERT_ID();";
        for(auto contact:contactsList){
            sql+=QString("INSERT INTO client_contacts (clientID, name, post, phoneNum, eMail) VALUES (@clientID, '%1', '%2', '%3', '%4');")
                       .arg(contact.at(0).toString()).arg(contact.at(1).toString()).arg(contact.at(2).toString()).arg(contact.at(3).toString());
        }

        sql+="COMMIT;";

        emit doSql(sql,[&](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                return;
            }
            else accept();
        });
    }
    else{


    }
}


void ClientManagerDlg::on_addContactsBtn_clicked()
{
    QString name=ui->contactsEdit->text();
    if(name.isEmpty()){
        QMessageBox::information(this,"error","请输入联系人姓名。");
        return;
    }
    QString post=ui->postEdit->text();
    QString phoneNum=ui->phoneEdit->text();
    if(phoneNum.isEmpty()){
        QMessageBox::information(this,"error","请输入联系电话。");
        return;
    }
    QString email=ui->eMailEdit->text();
    ui->contactsView->append({name,post,phoneNum,email});
}


void ClientManagerDlg::on_clientNameBox_currentIndexChanged(int index)
{
    QString arg1=ui->clientNameBox->currentText();
    if(!m_editMod){
        int id=m_clients->value(arg1).ID;
        ui->clientAddrEdit->setText(m_clients->value(arg1).clientAddr);
        QString sql=QString("select name, post, phoneNum, eMail from client_contacts where clientID=%1 and deleted = 0;").arg(id);
        emit doSql(sql,[&](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                return;
            }
            ui->contactsView->clear();
            QList<QVariant> contacts=msg.result().toList();
            for(int i=1;i<contacts.count();i++){
                ui->contactsView->append(contacts.at(i).toList());
            }
        });
    }
}

