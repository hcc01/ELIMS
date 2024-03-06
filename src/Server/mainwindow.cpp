#include "mainwindow.h"
#include "qdir.h"
#include "qsettings.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include"cuser.h"
#include<QTimer>
#include"cdatabasemanage.h"
#include<QSqlQuery>
#include"staticdatamanager.h"
#include<QTableView>
#include<QSqlTableModel>
#include<QMessageBox>
#include"../Client/itemsselectdlg.h"
#include<QHeaderView>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    DB_POOL;
    ui->setupUi(this);
    QTimer *timer = new QTimer(this);
    timer->start(600000);
    connect(timer, &QTimer::timeout, this, [&]() {
        QSqlQuery query(DB.database());
        if (query.exec("SELECT 1")) {
            // 查询语句执行成功
            qDebug()<<"保持连接中……";
        } else {
            // 查询语句执行失败，输出错误信息
            qWarning() << "MainWindow: Failed to keep connection. Error:" << query.lastError().text();
        }
    });
    m_logView=new QTableView;
    m_model = new QSqlTableModel(m_logView);
    m_logView->setModel(m_model);
    m_logView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QSettings set("settings.ini",QSettings::IniFormat);
    QString tytle;
    if(set.value("company").isValid()){
        tytle=set.value("company").toString();
    }
    else{
        tytle="";
        set.setValue("company","未定义公司");
    }
    setWindowTitle(tytle);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(m_logView)delete m_logView;
}

void MainWindow::on_btStart_clicked()
{
    QSettings set("settings.ini",QSettings::IniFormat);
    int port;
    qDebug()<<QDir::currentPath();
    if(set.value("server/port").isValid()){
        port=set.value("server/port").toInt();
    }
    else{
        port=5555;
        set.setValue("server/port",5555);
    }
    if(_server.Bind(nullptr,port)==SOCKET_ERROR||_fileServer.Bind(nullptr,port+1)==SOCKET_ERROR){
        qDebug()<<"绑定端口失败。";
        return;
    }
    if(_server.Listen(8)==SOCKET_ERROR||_fileServer.Listen(8)==SOCKET_ERROR){
        qDebug()<<"监听端口失败。";
        return;
    }
    _server.Start();
    _fileServer.Start();
    ui->btStart->setEnabled(false);
}


void MainWindow::on_btCheckClients_clicked()
{
    std::list<CELLClient*> clients=_server.Clients();
    ui->listWidget->clear();
    if(!clients.size()){
        ui->listWidget->addItem("目前没有任何用户连接。");
        return;
    }
    ui->listWidget->addItem(QString("当前用户数量：%1").arg(clients.size()));
    for(auto &c:clients){
        ui->listWidget->addItem(c->IP());
        if(c->isLogined()) ui->listWidget->addItem(((CUser*)c->getUser())->name());
        else ui->listWidget->addItem("待登录");
        ui->listWidget->update();
    }


}

void MainWindow::on_staticDataBtn_clicked()
{
    StaticDataManager *w=new StaticDataManager(this);
    w->show();
}


void MainWindow::on_viewLogBtn_clicked()
{

    QDir dir("./");
    QStringList fileList;
    QFileInfoList entries = dir.entryInfoList(QDir::Files);
    QString suffix="log";
    for (const QFileInfo &file : entries) {
        if (file.suffix().toLower() == suffix.toLower()) {
            fileList.append(file.fileName());
        }
    }
    QString file=itemsSelectDlg::getSelectedItem(fileList);
    DB.showLog(file,m_model);
    m_logView->show();
}


void MainWindow::on_lineEdit_returnPressed()
{
    QString sql=ui->lineEdit->text();
    if(!DB.doSql(sql)){
        QMessageBox::information(nullptr,"error",DB.lastError());
    }
}

