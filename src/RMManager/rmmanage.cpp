#include "rmmanage.h"
#include "ui_rmmanage.h"
#include<QJsonArray>
#include<QJsonDocument>
#include<QDialog>
#include<QGridLayout>
#include<QDebug>
#include<QLabel>
//#include<QSqlQueryModel>
#include"rmaddui.h"
#include<QMessageBox>
#include<QDate>
#include"../../depends/MessageHeader.h"
#include<QMenu>
void RMManageUI::init()
{
    //添加标物查询条件1（列名需要与数据库对应）
    ui->comboBox1->addItem("名称","name");
    ui->comboBox1->addItem("编号","rm_number");
     //添加标物查询条件1
    ui->comboBox2->addItem("溶剂","solvent");
    ui->comboBox2->addItem("备注","mark");


}

RMManageUI::RMManageUI( QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::RMManageUI),
    _isQuerying(false)
{
    ui->setupUi(this);
    ui->groupBox_query->hide();
    ui->tableView->setModel(&_model);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);//比较重要 只有这样设置 才能使用信号SIGNAL(customContextMenuRequested(QPoint))
    connect( ui->tableView, &QTableView::customContextMenuRequested, this,[&](const QPoint& pos){
        QModelIndex index =ui->tableView->indexAt(pos);
        QMenu menu;
        //添加菜单项，指定图标、名称、响应函数
        menu.addAction( QStringLiteral("领用"),this,[&](){
            OnApply(index);
        });
        //在鼠标位置显示
        menu.exec(QCursor::pos());
    });

    init();
}

RMManageUI::~RMManageUI()
{
    delete ui;
}

bool RMManageUI::initMod()
{
    //创建标准物质信息表
    QString sql="drop table if exists rm_info ; "
"create table rm_info("
"id int AUTO_INCREMENT primary key,rm_number varchar(32) not null, "
"name varchar(32) not null, cas_number char(12), "
"lot_number char(12), manufactor varchar(50), "
"spec varchar(32), conc double(12,6) not null, "
"uncertainty double(8,6), unit varchar(12), "
"solvent varchar(12), validity date,"
" store_pos varchar(16), add_date date not null,"
"amount tinyint not null, remarks varchar(100))";
    QSqlCmd cmd(sql,NORETURN,this->tabText());
    emit sendData(cmd.jsCmd());
}

//void RMManageUI::doAddRM(const QJsonObject &dbINfo)//这个函数没用了，暂时留着（被on_btAddRM_clicked()替代了）
//{
//    qDebug()<<dbINfo;
//    QJsonArray columns=dbINfo.value("data").toArray();
//    QDialog* w=new QDialog(this);
//    w->setWindowTitle("增加标准物质");
//    QGridLayout* layout=new QGridLayout();
//    int m=0,n=0;
//    for(int i=0;i<columns.size();i++){
//        QJsonObject colunm=columns.at(i).toObject();

//        layout->addWidget(new QLabel(colunm.value("Field").toString()+(colunm.value("Null").toString()=="NO"?"*":"")),m,n);
//        n++;
//        layout->addWidget(new QLineEdit,m,n);
//        n++;
//        if((i+1)%5==0){
//            m++,n=0;
//        }
//    }
//    m++;
//    layout->addWidget(new QPushButton("增加"),m,6);
//    layout->addWidget(new QPushButton("完成"),m,8);
//    connect((QPushButton*)layout->itemAtPosition(m,8)->widget(),&QPushButton::clicked,w,&QDialog::accept);
//    w->setLayout(layout);
//    w->exec();

//}

void RMManageUI::setData(const QJsonArray &data)
{
    _model.setModelData(data);
}

void RMManageUI::onSqlReturn(const QSqlReturnMsg &jsCmd)
{

    bool r=jsCmd.error();
    switch (jsCmd.flag()) {
        case NORETURN:
        break;
                case ADD_RM:{
                    if(!r) QMessageBox::information(this,"","添加标准物质信息成功!");
                    else QMessageBox::information(this,"添加失败",jsCmd.result().toString());
                }
                break;
                case QUERY_RM:
               {
                   if(r){
                        QMessageBox::information(this,"查询失败",jsCmd.result().toString());
                        return;
                    }

                  setData( jsCmd.result().toJsonArray());
                }
                    break;
    case GET_CONTENT:
    {
        if(r){
             QMessageBox::information(this,"查询失败",jsCmd.result().toString());
             return;
         }
        setData( jsCmd.result().toJsonArray());
        qDebug()<<jsCmd.result().toJsonArray();
        QJsonObject content=QJsonDocument::fromJson(jsCmd.result().toJsonArray().at(0).toArray().at(0).toString().toUtf8()).object();
        _rmCheck.append("标物名称："+content.value("rm").toString());
        _rmCheck.append("申请数量："+content.value("amount").toString());
        _rmCheck.show();
    }
        break;
    }

}

void RMManageUI::dealProcess(const ProcessNoticeCMD &cmd)
{
    _rmCheck.append("申请人："+cmd.createName());
    _rmCheck.append("申请日期："+cmd.createTime());
    //QString sql=QString("select content from wf_instance left join wf_action_record where id=%1").arg(cmd.recordID());
    QString sql="select * from wf_instance ";
    QSqlCmd sqlCMD(sql,GET_CONTENT,this->tabText());
    sendData(sqlCMD.jsCmd());
}

void RMManageUI::test()
{
    QMessageBox::warning(nullptr,"","test");
}

void RMManageUI::on_pushButton_clicked()
{

}

void RMManageUI::on_btQuery_clicked()
{
    if(_isQuerying){
        _isQuerying=false;
        ui->btQuery->setText("查询");
        ui->groupBox_query->hide();
    }
    else{
        _isQuerying=true;
        ui->btQuery->setText(("关闭查询"));
        ui->groupBox_query->show();
    }
}

void RMManageUI::on_btAddRM_clicked()
{
    RMAddUI w;
    connect(&w,&RMAddUI::doQuery,this,[&](const QString& sql){
        QJsonObject jsonCMD;
        jsonCMD["cmd"]=JC_DO_SQL;
        jsonCMD["sql_info"]=sql;
        jsonCMD["sql_type"]=SQL_ADD_RM;
        sendData(jsonCMD);
    });
    w.exec();
//    netmsg_DataHeader header;
//    header.cmd=CMD_RM_GET_RM_TITLE;
    //    _client->SendData(&header);
}

void RMManageUI::OnApply(const QModelIndex &index)
{
    //领用的标 物ID
    //领用的数量
    QJsonObject content;
    content["rm"]=_model.value(index.row(),0).toString();
    content["amount"]=1;
    content["fuc"]=&RMManageUI::test;
    NewWorkFlowCMD wfCMD("标物领用",content);
    sendData(wfCMD.data());
}

void RMManageUI::on_btDoQuery_clicked()
{
    QString s1=ui->lineEdit_Query1->text();
//    if(s1.isEmpty()){
//        QMessageBox::warning(this,"","查询条件1不能为空！");
//        return;
//    }
    QString s2=ui->lineEdit_Query2->text();
    QJsonObject condition;

    if(ui->checkBox_3->isChecked()){
        if(!s1.isEmpty()) condition[ui->comboBox1->currentData().toString()]=QString(" like '%%1%' ").arg(s1);
        if(!s2.isEmpty()) condition[ui->comboBox2->currentData().toString()]=QString(" like '%%1%' ").arg(s2);
    }
    else{
        if(!s1.isEmpty()) condition[ui->comboBox1->currentData().toString()]=QString(" = '%1' ").arg(s1);
        if(!s2.isEmpty()) condition[ui->comboBox2->currentData().toString()]=QString(" = '%1' ").arg(s2);
    }

    if(ui->checkBox_amount->isChecked()) condition["amount"]=" > 0 ";
    QString today=QDate::currentDate().toString("yyyy-MM-dd");
    if(ui->checkBox_validity->isChecked()) condition["validity"]=QString(" > '%1' ").arg(today);
    QString sql="select * from rm_info ";
    QStringList keys=condition.keys();
    int n=keys.size();
    if(n){
        sql+="where ";
        for(int i=0;i<n;i++){
            sql+=  keys.at(i)+condition.value(keys.at(i)).toString();
            if(i<n-1) sql+=" and ";
        }
    }
    QSqlCmd jsCmd(sql,QUERY_RM,RMManageUI::tabText());
    sendData(jsCmd.jsCmd());
}
