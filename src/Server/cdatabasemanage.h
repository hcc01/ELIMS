#ifndef CDATABASEMANAGE_H
#define CDATABASEMANAGE_H

#include <QObject>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<QJsonObject>
#include"../Client/qjsoncmd.h"
#include "qtimer.h"
#include<QMutex>
#include<QSqlTableModel>
#define ITEMS_PER_PAGE 10
#define DB CDatabaseManage::Instance()
#define DB_POOL ConnectionPool::getInstance()

//使用连接池
class DatabaseConnection : public QObject
{
    Q_OBJECT
public:
    DatabaseConnection(int id,QObject *parent = nullptr);

    QSqlDatabase dataBase(){return m_database;}
    QString dbName()const{return m_id;}

    // 开始事务
    bool beginTransaction();
    // 提交事务
    bool commitTransaction();
    // 回滚事务
    bool rollbackTransaction();

    virtual ~DatabaseConnection() = default;
signals:
    void timeOut();
    void startCheck();
    void stopCheck();
private:
    QSqlDatabase m_database;
    QMutex m_mutex;
    QString m_id;
    QTimer* m_timer;
};
class ConnectionPool : public QObject
{
    Q_OBJECT

public:
    static ConnectionPool* getInstance()
    {
        static ConnectionPool instance(5, 30000);
        return &instance;
    }
    // 获取可用的数据库连接
    DatabaseConnection *getConnection(int userID);

    int connectionUser(DatabaseConnection *c)const{return m_usedConnections.value(c);}
    // 释放已使用的数据库连接
    void releaseConnection(DatabaseConnection *connection);

    // 开始事务
    bool beginTransaction(DatabaseConnection *connection)
    {
        if (connection) {
            // 更新连接的活跃时间
//            QTimer* timer = connection->findChild<QTimer*>();
//            if (timer) {
//                qDebug()<<"timer->start(m_timeout);"<<m_timeout;
//                timer->start(m_timeout);
//            }
            return connection->beginTransaction();
        }
        return false;
    }

    // 提交事务
    bool commitTransaction(DatabaseConnection *connection);


    // 回滚事务
    bool rollbackTransaction(DatabaseConnection *connection)
    {
        if (connection) {
            // 更新连接的活跃时间
//            QTimer* timer = connection->findChild<QTimer*>();
//            if (timer) {
//                timer->start(m_timeout);
//            }
            return connection->rollbackTransaction();
        }
        return false;
    }
private slots:
    // 检查连接超时情况
    void checkConnections();


private:
    ConnectionPool(int maxConnections = 5, int timeout = 30000);
    QMutex m_mutex;
    QList<DatabaseConnection*> m_connections;
    QHash<DatabaseConnection*, int> m_usedConnections;
    int m_maxConnections;
    int m_timeout;
    QTimer m_timer;
};


class CDatabaseManage:public QObject
{
    Q_OBJECT
public:
    static CDatabaseManage& Instance(){
        static CDatabaseManage dm;
        return dm;
    }
    QSqlDatabase& database(){
        return _db;
    }
    void connectDb(QString dataBase);
    bool isOpen(){
        return _dbOpen;
    }
    QString lastError()const{
        return _lastError;
    }
    bool startTransaction(int userID);
    bool commitTransaction(int userID);
    bool rollbackTransaction(int userID);
    void removeUserConnection(int userID){m_userDbConnections.remove(userID); }
    void addUserConnection(int userID,DatabaseConnection*connection){m_userDbConnections[userID]=connection;}
    DatabaseConnection* getUserConnection(int userID){return m_userDbConnections.value(userID);}
    //处理日志
    enum LogType{
        DEBUG_MSG,INFO_MSG,WARNING_MSG,ERROR_MSG ,
    };
    void doLog(const QString logMsg, int logType=DEBUG_MSG, int user=0, const QString&mod="", QString logTime="" );
    QSqlDatabase logDB(){return m_logDb;}
    void showLog(QString logDb,QSqlTableModel* model);
 //处理标准物质表格的接口
    bool addRM(const QJsonObject& rmData);
    bool queryRM(QJsonObject &jsCMD);
    QSqlReturnMsg doQuery(const QSqlCmd &sqlCm, int userID);
    QJsonObject getRMHead();
    bool doSql(const QString& qsl);


private:
    CDatabaseManage(QObject*parent=nullptr);
    bool doTranslate(QString &word, bool toCN=true);
private:
    QSqlDatabase _db;
    QSqlDatabase m_logDb;
    QString m_dataBaseName;
    bool _dbOpen;
    QString _lastError;
    QHash<int,DatabaseConnection*>m_userDbConnections;//保存用户的数据库连接，在开启事务后，使用此连接操作数据库
};

#endif // CDATABASEMANAGE_H
