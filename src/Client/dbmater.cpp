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
