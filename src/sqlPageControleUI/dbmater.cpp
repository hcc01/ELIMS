#include "dbmater.h"
#include "qdatetime.h"
#include "qmessagebox.h"
#include<QSqlDatabase>
#include<QDebug>

int DBMater::getParameterID(int testTypeID, const QString &parameterName)
{
    QSqlQuery query(m_db);
    QString sql=QString("SELECT A.id FROM detection_parameters AS A "
                          "LEFT JOIN detection_parameter_alias AS B ON A.id = B.parameterID "
                          "WHERE A.testFieldID =(select testFieldID from test_type where id=%2) "
                          "and (A.parameterName = '%1' OR B.alias = '%1')").arg(parameterName).arg(testTypeID);

    if(!query.exec(sql)){
        QMessageBox::information(nullptr,"查询项目ID时出错：",query.lastError().text());
        return 0;
    }
    if(!query.next()){
        QMessageBox::information(nullptr,"查询项目ID时出错：","未知的项目："+parameterName);
                                                                return 0;
    }
    return query.value(0).toInt();

}

QString DBMater::getTypeName(int testTypeID)
{
    QSqlQuery query(m_db);
    QString sql=QString("SELECT testType FROM test_type where id=%1").arg(testTypeID);

    if(!query.exec(sql)){
        QMessageBox::information(nullptr,"查询类型时出错：",query.lastError().text());
        return 0;
    }
    if(!query.next()){
        QMessageBox::information(nullptr,"查询类型时出错：","未知的类型ID：");
                                                                return 0;
    }
    return query.value(0).toString();
}

void DBMater::doLog(const QString &log)
{
    QString logTime=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QSqlQuery query(m_logDb);
    if(!query.exec(QString("insert into logData values(%1,%2);").arg(logTime,log))){
        qDebug()<<query.lastError().text();
    }
    qDebug()<<"DBMater::doLog"<<log;
}

DBMater::DBMater(QObject *parent)
    : QObject{parent}
{
    m_db=QSqlDatabase::addDatabase("QSQLITE","systermDb");
    m_db.setDatabaseName("BaseData");
    if(!m_db.open()){
        QMessageBox::information(nullptr,"无法打开数据库systermDb",m_db.lastError().text());
        return;
    }
    m_logDb=QSqlDatabase::addDatabase("QSQLITE","logDb");
    QString logDb=QString("%1.log").arg(QDate::currentDate().toString("yyyyMMdd"));
    m_logDb.setDatabaseName(logDb);
    if(!m_logDb.open()){
        QMessageBox::information(nullptr,"无法打开数据库logDb",m_db.lastError().text());
                                                            return;
    }
    QSqlQuery q(m_logDb);
    if(!q.exec(QString("create table if not exists logData( logTime text, msg text)"))){
        qDebug()<<"无法创建日志表格."<<q.lastError().text();
    }
    qDebug()<<"database opened.";

}
