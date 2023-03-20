#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include"cuser.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
