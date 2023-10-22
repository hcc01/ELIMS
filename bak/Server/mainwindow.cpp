#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include"cuser.h"
#include<QTimer>
#include"cdatabasemanage.h"
#include<QSqlQuery>
#include"staticdatamanager.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btStart_clicked()
{
    if(_server.Bind(nullptr,5555)==SOCKET_ERROR||_fileServer.Bind(nullptr,5556)==SOCKET_ERROR){
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

