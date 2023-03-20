//#include "widget.h"

#include <QApplication>
#include<QDateTime>
#include<QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Widget w;
//    w.show();
    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss" );
    qDebug()<<time;
    return a.exec();
}
