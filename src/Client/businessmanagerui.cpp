#include "businessmanagerui.h"
#include "ui_businessmanagerui.h"
#include"clienteditdlg.h"
#include"itemsselectdlg.h"
BusinessManagerUI::BusinessManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::BusinessManagerUI)
{
    ui->setupUi(this);
    ui->clientView->setHeader({"客户名称","地址","联系人","联系电话","业务负责人"});

}

BusinessManagerUI::~BusinessManagerUI()
{
    delete ui;
}

void BusinessManagerUI::initMod()
{
    QString sql;
    //客户信息表，同时也是受检单位表
    sql="CREATE TABLE IF NOT EXISTS client_info("
          "id int AUTO_INCREMENT primary key, "
          "clientName varchar(32) NOT NULL, "
          "address varchar(32) NOT NULL, "
          "remark VARCHAR(255),"
          "saleMan varchar(32), "
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

}

void BusinessManagerUI::initCMD()
{
    if(!(user()->position()&CUser::SalesSupervisor)){
        ui->EditSaleManBtn->hide();
    }
    QString sql;
    sql="select B.clientName, B.address, A.name, A.phoneNum, B.saleMan ,B.id from client_contacts as A left join  client_info as B on A.clientID=B.id where B.deleted=0 and A.deleted=0;";
    ui->clientPageCtrl->startSql(this,sql,1,{},[this](const QSqlReturnMsg&msg){
        QList<QVariant>r=msg.result().toList();
        ui->clientView->clear();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            int id=row.last().toInt();
            row.removeLast();
            ui->clientView->append(row);
            ui->clientView->setCellFlag(i-1,0,id);
        }
    });
}

void BusinessManagerUI::on_clientView_doubleClicked(const QModelIndex &index)
{
    on_clientEditBtn_clicked();
}


void BusinessManagerUI::on_addClientBtn_clicked()
{
    ClientEditDlg dlg(this);
    dlg.exec();
}


void BusinessManagerUI::on_refleshBtn_clicked()
{
    initCMD();
}


void BusinessManagerUI::on_EditSaleManBtn_clicked()
{
    int row=ui->clientView->selectedRow();
    if(row<0) return;
    QString sql;
    sql="select name from users where position&?";
    if(!m_sales.count()){
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查询业务人员时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            for(int i=1;i<r.count();i++){
                m_sales.append(r.at(i).toList().at(0).toString());
            }
            sqlFinished();
        },0,{CUser::salesRepresentative});
        waitForSql("正在加载业务人员");
    }
    if(!m_sales.count()){
         QMessageBox::information(nullptr,"error","未找到业务人员。");
        return;
    }
    bool error=false;
    QString saleMan=itemsSelectDlg::getSelectedItem(m_sales);
    if(saleMan.isEmpty()) return;
    sql="update client_info set saleMan=? where id=?;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询业务人员时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0, {saleMan,ui->clientView->cellFlag(row,0).toInt()});
    waitForSql("正在更新客户信息");
    if(!error){
        QMessageBox::information(nullptr,"","操作成功。");
    }
}


void BusinessManagerUI::on_clientEditBtn_clicked()
{
    int row=ui->clientView->selectedRow();
    if(row<0) return;
    ClientEditDlg dlg(this);
    dlg.load(ui->clientView->value(row,0).toString(),ui->clientView->value(row,1).toString(),ui->clientView->value(row,2).toString(),ui->clientView->value(row,3).toString(),ui->clientView->cellFlag(row,0).toInt());
    dlg.exec();

}

