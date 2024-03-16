#include "dbmater.h"
#include "qdatetime.h"
#include "qmessagebox.h"
#include<QSqlDatabase>
#include<QDebug>
DBMater::DBMater(QObject *parent)
    : QObject{parent}
{
    m_db=QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("BaseData");
    if(!m_db.open()){
        QMessageBox::information(nullptr,"无法打开数据库",m_db.lastError().text());
        return;
    }
    qDebug()<<"database opened.";
}
