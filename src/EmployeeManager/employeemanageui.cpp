#include "employeemanageui.h"
#include "ui_employeemanageui.h"
#include"QDebug"
#include"employeeeditor.h"
#include"../Client/cuser.h"

EmployeeManageUI::EmployeeManageUI( QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::EmployeeManageUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"姓名","电话","学历","职称","岗位","状态"});
    ui->tableView->addContextAction("添加",[this](){
        employeeEditor ed(this);
        connect(&ed,&QDialog::accepted,[this](){
            initCMD();
        });
        ed.exec();
    });
    ui->tableView->addContextAction("修改",[this](){
        int row=ui->tableView->selectedRow();
        if(row<0) return;
        employeeEditor ed(this,false);
        ed.load(ui->tableView->value(row,0).toString(),ui->tableView->value(row,1).toString(),ui->tableView->value(row,2).toString(),
                ui->tableView->value(row,3).toString(),ui->tableView->value(row,4).toString(),ui->tableView->cellFlag(row,0).toInt());
        connect(&ed,&QDialog::accepted,[this](){
            initCMD();
        });
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
    auto m_positions=CUser::allPositions();
    sql="SELECT name, phone, EducationDegree, Title, position, state from users;";
    ui->pageCtrl->startSql(this,sql,1,{},[this, m_positions](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"初始化查询失败",msg.result().toString());
            return;
        }
        QList<QVariant> r=msg.result().toList();
        ui->tableView->clear();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            if(row.at(0).toString()=="admin") continue;
            int p=row.at(4).toInt();
            QString position;
            bool first=true;
            for(int x:m_positions.keys()){

                if(p&x){
                    if(first) first=false;
                    else position+="\n";
                    position+=m_positions.value(x);
                }
            }
            ui->tableView->append({row.at(0),row.at(1),row.at(2),row.at(3),position,row.last().toBool()?"在职":"离职"});
            ui->tableView->setCellFlag(i-1,0,p);
        }
    });
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
