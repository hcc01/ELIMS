#include "employeemanageui.h"
#include "ui_employeemanageui.h"
#include"QDebug"
#include"employeeeditor.h"
EmployeeManageUI::EmployeeManageUI( QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::EmployeeManageUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"姓名","电话","学历","职称","岗位"});
    ui->tableView->addContextAction("添加",[this](){
        employeeEditor ed(m_positions,this);
        ed.exec();
    });
    ui->tableView->addContextAction("修改",[this](){
        employeeEditor ed(m_positions,this);
        ed.exec();
    });
}

EmployeeManageUI::~EmployeeManageUI()
{
    qDebug()<<"~EmployeeManageUI()";
    delete ui;
}

void EmployeeManageUI::initCMD()
{
    QString sql;
    sql="SELECT name, phone, EducationDegree, Title, position, state from users;";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"初始化查询失败",msg.result().toString());
            return;
        }
        QList<QVariant> r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            int p=row.at(4).toInt();
            QString position;
            bool first=true;
            for(int x:m_positions.keys()){

                if(p&x){
                    if(first) first=false;
                    else position+=" 兼\n";
                    position+=m_positions.value(x);
                }
            }
            ui->tableView->append({row.at(0),row.at(1),row.at(2),row.at(3),position});
        }
    },1);
}

void EmployeeManageUI::initMod()
{
    QString sql;
    //
    sql="CREATE TABLE IF NOT EXISTS users("
          "id int AUTO_INCREMENT primary key, "
          "name varchar(32) unique NOT NULL, "
          "phone varchar(20),"
          "position int not null,"
          "EducationDegree varchar(20),"
          "Title varchar(20),"
          "state TINYINT NOT NULL DEFAULT 1); ";
    doSqlQuery(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
    });

}
