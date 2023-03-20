#include "dbmanagerui.h"
#include "ui_dbmanagerui.h"
#include<QMessageBox>
#include<QDebug>
DBManagerUI::DBManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::DBManagerUI)
{
    ui->setupUi(this);

    ui->tableView->setModel(&_model);
    connect(ui->pageCtrWidet,&SqlPageControleUI::pageChanged,this, &DBManagerUI::sendData);
}

DBManagerUI::~DBManagerUI()
{
    qDebug()<<"~DBManagerUI()";
    delete ui;
}

void DBManagerUI::onSqlReturn(const QSqlReturnMsg &jsCmd)//sqlreturn一个table(QJSONARRY)，其中0为表头，数据从1开始。
{

    bool r=jsCmd.error();
    if(r){
        QMessageBox::warning(this,"处理数据库时出错",jsCmd.result().toString());
        return;
    }
    int cmdType=jsCmd.flag();
    qDebug()<<"DBManagerUI::onSqlReturn json="<<jsCmd.jsCmd();
    switch (cmdType) {
        case GET_TABLE:
    {
        QJsonArray a=jsCmd.result().toJsonArray();
        qDebug()<<"BManagerUI::onSqlReturn: result="<<a;
        for(int i=1;i<a.size();i++){
            ui->comboBox->addItem(a[i].toArray().at(0).toString());
        }
    }
        break;
    case CHANGE_TABLE:
    case DO_SQL:
    {
        _model.setModelData(jsCmd.result().toJsonArray());
        int pages=jsCmd.totalPage();
        ui->pageCtrWidet->setTotalPage(pages);
        if(pages){
            int page=jsCmd.currentPage();
            ui->pageCtrWidet->setCurrentPage(page);
//            _sqlCmd.setPage(page);
//            ui->label->setText(QString("%1 / %2").arg(page+1).arg(pages));
//            ui->groupBox->show();
        }
        else {
           // ui->groupBox->hide();
        }
    }
        break;
    }

}

bool DBManagerUI::initMod()
{
    return true;
}

void DBManagerUI::on_comboBox_currentTextChanged(const QString &arg1)
{
    QSqlCmd jsCmd(QString("select * from %1").arg(arg1),CHANGE_TABLE,DBManagerUI::tabText(),1);
    //_sqlCmd=jsCmd;//保存下当前的查询命令
    ui->pageCtrWidet->setCmd(jsCmd);
    sendData(jsCmd.jsCmd());
    qDebug()<<"on_comboBox_currentTextChanged:"<<arg1;
}

void DBManagerUI::initCMD()
{
    QSqlCmd jsCmd("use elims",SQL_NO_RETURN,DBManagerUI::tabText());
    sendData(jsCmd.jsCmd());
    jsCmd.setSql("show tables");
    jsCmd.setFlag(GET_TABLE);
    ui->pageCtrWidet->setCmd(jsCmd);
    qDebug()<<"DBManagerUI::initCMD cmd:"<<jsCmd.jsCmd();
    sendData(jsCmd.jsCmd());
}

void DBManagerUI::on_lineEdit_returnPressed()
{
    QSqlCmd jsCmd(ui->lineEdit->text(),DO_SQL,DBManagerUI::tabText(),1);
    ui->pageCtrWidet->setCmd(jsCmd);
    qDebug()<<"DBManagerUI::on_lineEdit_returnPressed cmd:"<<jsCmd.jsCmd();
    sendData(jsCmd.jsCmd());
}

//void DBManagerUI::on_bt_PageUP_clicked()
//{
//    if(!_totalPage) return;
//    if(_sqlCmd.queryPage()<=1) return;
//    _sqlCmd.setPage(_sqlCmd.queryPage()-1);
//    sendData(_sqlCmd.jsCmd());
//}

//void DBManagerUI::on_bt_PageDown_clicked()
//{
//    if(!_totalPage) return;
//    if(_sqlCmd.queryPage()+1==_totalPage) return;
//    _sqlCmd.setPage(_sqlCmd.queryPage()+1);
//    qDebug()<<_sqlCmd.jsCmd();
//    sendData(_sqlCmd.jsCmd());
//}

void DBManagerUI::on_comboBox_editTextChanged(const QString &arg1)
{

//    QSqlCmd jsCmd(QString("select * from %1").arg(arg1),CHANGE_TABLE,DBManagerUI::tabText());
//    //_sqlCmd=jsCmd;//保存下当前的查询命令
//    ui->pageCtrWidet->setCmd(jsCmd);
//    sendData(jsCmd.jsCmd());
//    qDebug()<<arg1;
}
