#include "clienteditdlg.h"
#include "ui_clienteditdlg.h"

ClientEditDlg::ClientEditDlg(TabWidgetBase *parent) :
    QDialog(parent),
    ui(new Ui::ClientEditDlg),
    tab(parent),
    m_editMod(false)

{
    ui->setupUi(this);
}

ClientEditDlg::~ClientEditDlg()
{
    delete ui;
}

void ClientEditDlg::load(const QString &clientName, const QString &clientAddr, const QString &contact, const QString &phone, int id)
{
    ui->clientAddrEdit->setText(clientAddr);
    ui->clientNameEdit->setText(clientName);
    ui->contactEdit->setText(contact);
    ui->phoneEdit->setText(phone);
    m_editMod=true;
    m_clientID=id;
}

void ClientEditDlg::on_okBtn_clicked()
{
    bool error=false;
    QString clientName=ui->clientNameEdit->text();
    QString clientAddr=ui->clientAddrEdit->text();
    QString contact=ui->contactEdit->text();
    QString phone=ui->phoneEdit->text();
    QString saleMan=tab->user()->name();
    QString sql;
    if(m_editMod){
        sql="update client_info set clientName=?, address=? where id=?;update client_contacts set name=?, phoneNum=? where clientID=?;";
        tab->doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
            if(msg.error()){
                tab->releaseDB(CMD_ROLLBACK_Transaction);
                QMessageBox::information(nullptr,"更新客户信息时出错：",msg.errorMsg());
                error=true;
                tab->sqlFinished();
                return;
            }
            tab->sqlFinished();
        },0,{clientName,clientAddr,m_clientID,contact,phone,m_clientID});
        tab->waitForSql("正在更新客户信息");
        if(error) return;
        accept();
        return;
    }
    sql="insert into client_info(clientName, address,saleMan) values(?,?,?);";

    tab->connectDB(CMD_START_Transaction);
    tab->doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            tab->releaseDB(CMD_ROLLBACK_Transaction);
            QMessageBox::information(nullptr,"增加客户时出错：",msg.errorMsg());
            error=true;
            tab->sqlFinished();
            return;
        }
        tab->sqlFinished();
    },0,{clientName,clientAddr,saleMan});
    tab->waitForSql("正在添加客户信息");
    sql="insert into client_contacts(clientID, name, phoneNum) values(LAST_INSERT_ID(),?,?);";
    tab->doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            tab->releaseDB(CMD_ROLLBACK_Transaction);
            QMessageBox::information(nullptr,"增加联系人时出错：",msg.errorMsg());
            error=true;
            tab->sqlFinished();
            return;
        }
        tab->sqlFinished();
        tab->releaseDB(CMD_COMMIT_Transaction);
    },0,{contact,phone});
    tab->waitForSql("正在添加联系人信息");
    if(!error) accept();
}


void ClientEditDlg::on_cancelBtn_clicked()
{
    reject();
}

