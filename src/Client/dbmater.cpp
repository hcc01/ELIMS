#include "dbmater.h"
#include "qdatetime.h"
#include "qmessagebox.h"
#include<QSqlDatabase>
#include<QDebug>
int DBMater::getParameterID(const QString &parameterName)
{
    QSqlQuery query;
    query.prepare("select id from detection_parameters as A "
                  "left join detection_parameter_alias as B on A.id=B.parameterID "
                  "where A.parameterName=? or B.alias =?;");
    query.addBindValue(parameterName);
    query.addBindValue(parameterName);
    if(!query.exec()){
        QMessageBox::information(nullptr,"查询项目ID时出错：",query.lastError().text());
        return 0;
    }
    if(!query.next()){
        QMessageBox::information(nullptr,"查询项目ID时出错：","未知的项目："+parameterName);
                                                                return 0;
    }
    return query.value(0).toInt();
}

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
