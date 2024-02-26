#ifndef CDATABASEMANAGE_H
#define CDATABASEMANAGE_H

#include <QObject>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<QJsonObject>
#include"../Client/qjsoncmd.h"
#include"../../depends/CELLLog.h"
#include <QThreadPool>
#include <QMutex>
#include <QMutexLocker>
#include<QQueue>
#include<QWaitCondition>
#define ITEMS_PER_PAGE 10
#define DB CDatabaseManage::Instance()


//使用连接池处理数据库


class DatabaseConnection : public QObject
{
    Q_OBJECT

public:
    DatabaseConnection(int id,QObject *parent = nullptr)
        : QObject(parent)
    {
        // 创建数据库连接
        m_id=QString("DB%1").arg(id);
        m_database=QSqlDatabase::addDatabase("QMYSQL",m_id);
        m_database.setConnectOptions("MYSQL_OPT_RECONNECT=1");

        m_database.setHostName("127.0.0.1");
        m_database.setUserName("root");
        m_database.setPassword("spring01");
        if(!m_database.open()){
            CELLLog::Info("CDatabaseManage error on open database:%s",m_database.lastError().text().toUtf8().data());
            qDebug()<<m_database.lastError().text();
            return;
        }
        m_database.setDatabaseName("elims");
        qDebug()<<QString("%1 opened!").arg(m_database.connectionName())<<this->m_database;
    }

    QSqlDatabase dataBase(){qDebug()<<"m_database"<<m_database;return m_database;}
    QString dbName()const{return m_id;}
    // 执行数据库查询操作
    bool executeQuery(QSqlQuery& q,const QString &queryStr)
    {
        QSqlQuery query(m_database);
        qDebug()<<queryStr;
        if (query.exec(queryStr)) {
//            q = std::move(query);
            qDebug()<<"query ok";
            return true;
        } else {
//            q = std::move(query);
            qDebug()<<"query.lastError().text();"<<query.lastError().text();
            return false;
        }
    }

    // 开始事务
    bool beginTransaction()
    {
        QMutexLocker locker(&m_mutex);
        return m_database.transaction();
    }

    // 提交事务
    bool commitTransaction()
    {
        QMutexLocker locker(&m_mutex);
        return m_database.commit();
    }

    // 回滚事务
    bool rollbackTransaction()
    {
        QMutexLocker locker(&m_mutex);
        return m_database.rollback();
    }
  virtual ~DatabaseConnection() = default;
private:
    QSqlDatabase m_database;
    QMutex m_mutex;
    QString m_id;
};

class ConnectionPool
{
public:
    ConnectionPool(int maxConnections=5)
        : m_maxConnections(maxConnections)
    {
        // 创建连接池中的数据库连接
        for (int i = 0; i < maxConnections; ++i) {
            DatabaseConnection *connection = new DatabaseConnection(i);
            m_connections.append(connection);
        }
    }

    // 获取可用的数据库连接
    DatabaseConnection *getConnection(int userID)
    {
        QMutexLocker locker(&m_mutex);
        for(auto it=m_usedConnections.begin();it!=m_usedConnections.end();++it){
            if(it.value()==userID) return it.key();
        }
        // 查找一个可用的连接
        for (DatabaseConnection *connection : m_connections) {
            if (!m_usedConnections.contains(connection)) {
                m_usedConnections[connection]=userID;
                return connection;
            }
        }

        // 如果没有可用的连接，则创建新连接
        if (m_connections.size() < m_maxConnections) {
            DatabaseConnection *connection = new DatabaseConnection(m_connections.size()+1);
            m_connections.append(connection);
            m_usedConnections[connection]=userID;
            return connection;
        }

        // 所有连接都在使用中
        return nullptr;
    }

    // 释放已使用的数据库连接
    void releaseConnection(DatabaseConnection *connection)
    {
        QMutexLocker locker(&m_mutex);
        m_usedConnections.remove(connection);
    }

    // 开始事务
    bool beginTransaction(DatabaseConnection *connection)
    {
        if (connection) {
            return connection->beginTransaction();
        }
        return false;
    }

    // 提交事务
    bool commitTransaction(DatabaseConnection *connection)
    {
        if (connection) {
            return connection->commitTransaction();
        }
        return false;
    }

    // 回滚事务
    bool rollbackTransaction(DatabaseConnection *connection)
    {
        if (connection) {
            return connection->rollbackTransaction();
        }
        return false;
    }

private:
    int m_maxConnections;
    QList<DatabaseConnection*> m_connections;
    QHash<DatabaseConnection*,int> m_usedConnections;
    QMutex m_mutex;
};
//连接池END



class CDatabaseManage
{
public:
    static CDatabaseManage& Instance(){
        static CDatabaseManage dm;
        return dm;
    }
    QSqlDatabase& database(){
        return _db;
    }
    void connectDb();
    bool isOpen(){
        return _dbOpen;
    }
    QString lastError()const{
        return _lastError;
    }

 //处理标准物质表格的接口
    bool addRM(const QJsonObject& rmData);//已作废
    bool queryRM(QJsonObject &jsCMD);//已作废
    QSqlReturnMsg doQuery(const QSqlCmd &sqlCm,int userID);
    bool startQuery(int userID, bool Transaction=false);
    void endQuery(int userID, int TransactionState=0);
    QJsonObject getRMHead();
    bool doSql(const QString& qsl);


private:
    CDatabaseManage();
    bool doTranslate(QString &word, bool toCN=true);
private:
    QSqlDatabase _db;
    bool _dbOpen;
    QString _lastError;
    ConnectionPool m_connectionPool;
    QHash<int, DatabaseConnection*>m_userDb;
};

#endif // CDATABASEMANAGE_H
