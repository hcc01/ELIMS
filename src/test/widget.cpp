#include "widget.h"
#include "ui_widget.h"
#include"../TcpServer/TcpServer.h"
#include"../TcpServer/TcpServer_global.h"
#include<QtSql/QSqlDriver>
#include<QtSql/QSqlDatabase>

#include<QTableView>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //TcpServer a;
   // TcpServer1 b;
   Test *t = new Test;
   t->n=5;
   char*p=(char*)t;
   int m=int(*p);
   qDebug()<<m;
   t->str2="hehehehe";
   qDebug()<<t;
   qDebug()<<"&t->n"<<(&t->n);
   qDebug()<<"&t->str"<<&t->str;
   qDebug()<<"&t->str2"<<&t->str2;
   qDebug()<<"&t->m"<<&t->m;
   qDebug()<<sizeof(Test);

}

Widget::~Widget()
{
    delete ui;
}

long long Widget::jiecheng(int n)
{
    long long r=n;
    while (n!=1) {
        r=r*--n;
    }
    return r;
}

