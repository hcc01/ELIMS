#include "dbmanagerui.h"
#include "ui_dbmanagerui.h"
#include<QMessageBox>
#include<QDebug>
#include"qexcel.h"
DBManagerUI::DBManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::DBManagerUI),
    _sqlCmd(QJsonObject())
{
    ui->setupUi(this);

    ui->tableView->setModel(&_model);
    connect(ui->pageCtrWidet,&SqlPageControleUI::pageChanged,this, &DBManagerUI::onPageChanged);
    qDebug()<<ui->pageCtrWidet;

}

DBManagerUI::~DBManagerUI()
{
    qDebug()<<"~DBManagerUI()";
    delete ui;
}

void DBManagerUI::showEvent(QShowEvent *event)
{
    TabWidgetBase::showEvent(event);
//    initCMD();
}

//void DBManagerUI::onSqlReturn(const QSqlReturnMsg &jsCmd)//sqlreturn一个table(QJSONARRY)，其中0为表头，数据从1开始。
//{

//    bool r=jsCmd.error();
//    if(r){
//        QMessageBox::warning(this,"处理数据库时出错",jsCmd.result().toString());
//        return;
//    }
//    int cmdType=jsCmd.flag();
//    qDebug()<<"DBManagerUI::onSqlReturn json="<<jsCmd.jsCmd();
//    switch (cmdType) {
//        case GET_TABLE:
//    {
//        QJsonArray a=jsCmd.result().toJsonArray();
//        qDebug()<<"BManagerUI::onSqlReturn: result="<<a;
//        ui->comboBox->clear();
//        for(int i=1;i<a.size();i++){
//            ui->comboBox->addItem(a[i].toArray().at(0).toString());
//        }
//    }
//        break;
//    case CHANGE_TABLE:
//    case DO_SQL:
//    {
//        _model.setModelData(jsCmd.result().toJsonArray());
//        int pages=jsCmd.totalPage();
//        ui->pageCtrWidet->setTotalPage(pages);
//        if(pages){
//            int page=jsCmd.currentPage();
//            ui->pageCtrWidet->setCurrentPage(page);
////            _sqlCmd.setPage(page);
////            ui->label->setText(QString("%1 / %2").arg(page+1).arg(pages));
////            ui->groupBox->show();
//        }
//        else {
//           // ui->groupBox->hide();
//        }
//    }
//        break;
//    }

//}

void DBManagerUI::initMod()
{
    return;
}

void DBManagerUI::showTable(const QSqlReturnMsg &msg)
{
    _model.setModelData(msg.result().toJsonArray());
    int pages=msg.totalPage();
    ui->pageCtrWidet->setTotalPage(pages);
    if(pages){
        int page=msg.currentPage();
        ui->pageCtrWidet->setCurrentPage(page);
        //            _sqlCmd.setPage(page);
        //            ui->label->setText(QString("%1 / %2").arg(page+1).arg(pages));
        //            ui->groupBox->show();
    }
    else {
        // ui->groupBox->hide();
    }
}

void DBManagerUI::onPageChanged(const QString &sql, int p)
{
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            showTable(msg);
        },p);
}

void DBManagerUI::on_comboBox_currentTextChanged(const QString &arg1)
{
    doSqlQuery(QString("select * from %1").arg(arg1),[this](const QSqlReturnMsg&msg){
            showTable(msg);
    },1);
    ui->pageCtrWidet->setSql(QString("select * from %1").arg(arg1),1);
}

void DBManagerUI::initCMD()
{
    doSqlQuery("use elims");
    doSqlQuery("show tables",[this](const QSqlReturnMsg&msg){
        QJsonArray a=msg.result().toJsonArray();
                qDebug()<<"BManagerUI::onSqlReturn: result="<<a;
                ui->comboBox->clear();
                for(int i=1;i<a.size();i++){
                    ui->comboBox->addItem(a[i].toArray().at(0).toString());
                }
    });
    qDebug()<<"initCMD"<<ui->pageCtrWidet;
    ui->pageCtrWidet->setSql("show tables",1);

}

void DBManagerUI::on_lineEdit_returnPressed()
{
    doSqlQuery(ui->lineEdit->text(),[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"error",msg.result().toString());
                return;
            }
            else showTable(msg);
    },1);
    ui->pageCtrWidet->setSql(ui->lineEdit->text(),1);
}

void DBManagerUI::on_bt_PageUP_clicked()
{
    if(!_totalPage) return;
    if(_sqlCmd.queryPage()<=1) return;
    _sqlCmd.setPage(_sqlCmd.queryPage()-1);
    sendData(_sqlCmd.jsCmd());
}

void DBManagerUI::on_bt_PageDown_clicked()
{
    if(!_totalPage) return;
    if(_sqlCmd.queryPage()+1==_totalPage) return;
    _sqlCmd.setPage(_sqlCmd.queryPage()+1);
    qDebug()<<_sqlCmd.jsCmd();
    sendData(_sqlCmd.jsCmd());
}

void DBManagerUI::on_comboBox_editTextChanged(const QString &arg1)
{

    doSqlQuery(QString("select * from %1").arg(arg1),[this](const QSqlReturnMsg&msg){
            showTable(msg);
        },1);
    ui->pageCtrWidet->setSql(QString("select * from %1").arg(arg1),1);
}

void DBManagerUI::on_exportToExcelBtn_clicked()
{
    if(QMessageBox::question(nullptr,"","确认导出表格:"+ui->comboBox->currentText()+"？")!=QMessageBox::Yes){
        return;
    }

    QString sql=QString("select * from %1").arg(ui->comboBox->currentText());
    int p=1;
    QAxObject* book=EXCEL.newBook();

    while(1){
        QEventLoop loop;
        connect(this,&DBManagerUI::sqlFinished,&loop,&QEventLoop::quit);
        bool error=false;
        doSqlQuery(sql,[this,&error,book,&p](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查询数据错误：",msg.result().toString());
                error=true;
                emit sqlFinished();
                return;
            }
            QList<QVariant>r=msg.result().toList();
            QAxObject* sheet=EXCEL.ActiveSheet(book);
            int row=EXCEL.rowCount(sheet)+1;
            if(p==1){
                EXCEL.writeRow(row-1,sheet,r.at(0).toList());
            }
            for(int i=1;i<r.count();i++){
                EXCEL.writeRow(row,sheet,r.at(i).toList());
                row++;
            }
            if(p==msg.totalPage()){
                error=true;

            }
            p++;
            delete sheet;
            emit sqlFinished();
        },p);
        loop.exec();
        if(error) break;
    }
    EXCEL.setScreenUpdating(true);
    delete book;
}

