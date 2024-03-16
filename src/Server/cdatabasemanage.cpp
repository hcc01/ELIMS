#include "cdatabasemanage.h"
#include"../../depends/CELLLog.h"
#include "qsqltablemodel.h"
#include<QDebug>
#include<QSqlRecord>
#include<QSqlField>
#include<QJsonArray>
#include<QSqlError>
#include<QDate>
#include<QDateTime>
#include<QThread>
#include<QSettings>
#include<QRegularExpression>
void CDatabaseManage::connectDb(QString dataBase)
{
    m_dataBaseName=dataBase;

    m_logDb=QSqlDatabase::addDatabase("QSQLITE");

    QString logDb=QString("%1.log").arg(QDate::currentDate().toString("yyyyMMdd"));
    m_logDb.setDatabaseName(logDb);
    m_logDb.open();
    QSqlQuery q(m_logDb);
    if(!q.exec(QString("create table if not exists logData(logType INTEGER, logTime text, mod text, user INTEGER, msg text)"))){
        qDebug()<<"无法创建日志表格."<<q.lastError().text();
    }


    _db=QSqlDatabase::addDatabase("QMYSQL",dataBase);
    _db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

    _db.setHostName("127.0.0.1");
    _db.setUserName("root");
    _db.setPassword("spring01");
    if(!_db.open()){
        CELLLog::Info("CDatabaseManage error on open database:%s",_db.lastError().text().toUtf8().data());
        qDebug()<<_db.lastError().text();
        return;
    }
    _db.exec(QString("CREATE DATABASE IF NOT EXISTS %1").arg(dataBase));
   _db.setDatabaseName(dataBase);
   _db.open();
    qDebug()<<"db opened";
    _dbOpen=true;
    QSqlQuery query(_db);

    if(!query.exec("create table if not exists sys_employee_login (id int not null auto_increment, name  char(32) not null unique, password  char(32),last_login_time datetime ,last_login_ip char(16), primary key (id)  );")){
        qDebug()<<"query error:"<<query.lastError().text();
    }
    if(!query.exec("create table if not exists flow_records (id int not null auto_increment, creator  varchar(32) not null,createTime DATETIME ,tabName  varchar(32),flowInfo JSON ,status int default 0, operatorCountNeeded int default 1, operatorCountPassed int default 0, primary key (id)  );")){
        qDebug()<<"query error:"<<query.lastError().text();
    }
    if(!query.exec("create table if not exists flow_operate_records (id int not null auto_increment, flowID int, operatorID int,operateStatus int default 0, operateComments varchar(255), operateTime DateTime, revisionNotes varchar(255), primary key (id), FOREIGN KEY (flowID) REFERENCES flow_records (id), FOREIGN KEY (operatorID) REFERENCES sys_employee_login (id)  );")){
        qDebug()<<"query error:"<<query.lastError().text();
    }
    if(!query.exec("create table if not exists all_flows (id int not null auto_increment, flowID int, identityColumn varchar(32) unique, primary key (id), FOREIGN KEY (flowID) REFERENCES flow_records (id)  );")){
        qDebug()<<"query error:"<<query.lastError().text();
    }
    if(!query.exec("select id from sys_employee_login where name='admin';")){
        qDebug()<<"query error:"<<query.lastError().text();
    }
    if(!query.next()){
        if(!query.exec("insert into sys_employee_login(name, password) values( 'admin', 'c09e4c730a31cd5ed3548434f075ef8e');")){
            qDebug()<<"query error:"<<query.lastError().text();
        }
    }


}

bool CDatabaseManage::startTransaction(int userID)
{
    if(m_userDbConnections.contains(userID)){
        doLog(QString(" CDatabaseManage::startTransaction：用户重复开启事务，用户ID=%1").arg(userID),DEBUG_MSG,userID);
        return false;
    }
    DatabaseConnection* connection=DB_POOL->getConnection(userID);
    if(!connection){
        doLog(QString(" CDatabaseManage::startTransaction失败：无法获取连接，用户ID=%1").arg(userID),ERROR_MSG,userID);
        return false;
    }
    if(!DB_POOL->beginTransaction(connection)){
        doLog(QString(" CDatabaseManage::startTransaction失败：无法开启事务，用户ID=%1").arg(userID),ERROR_MSG,userID);
        return false;
    }

    return true;
}

bool CDatabaseManage::commitTransaction(int userID)
{

    DatabaseConnection* connection=DB_POOL->getConnection(userID);
    if(!connection){
        doLog(QString(" CDatabaseManage::commitTransaction失败：无法获取连接，用户ID=%1").arg(userID),ERROR_MSG,userID);
        return false;
    }
    if(!DB_POOL->commitTransaction(connection)){
        doLog(QString(" CDatabaseManage::commitTransaction失败：无法提交事务，用户ID=%1").arg(userID),ERROR_MSG,userID);
        return false;
    }
    if(!m_userDbConnections.contains(userID)){
        doLog(QString(" CDatabaseManage::startTransaction：m_userDbConnections异常，没有用户连接数据，用户ID=%1").arg(userID),DEBUG_MSG,userID);
        //        return false;
    }

    DB_POOL->releaseConnection(connection);//释放连接
    return true;
}

bool CDatabaseManage::rollbackTransaction(int userID)
{
    DatabaseConnection* connection=DB_POOL->getConnection(userID);
    if(!connection){
        doLog(QString(" CDatabaseManage::rollbackTransaction失败：无法获取连接，用户ID=%1").arg(userID),ERROR_MSG,userID);
        return false;
    }
    if(!DB_POOL->commitTransaction(connection)){
        doLog(QString(" CDatabaseManage::rollbackTransaction失败：无法回滚事务，用户ID=%1").arg(userID),ERROR_MSG,userID);
        return false;
    }
    if(!m_userDbConnections.contains(userID)){
        doLog(QString(" CDatabaseManage::startTransaction：m_userDbConnections异常，没有用户连接数据，用户ID=%1").arg(userID),DEBUG_MSG,userID);
        //        return false;
    }
    DB_POOL->releaseConnection(connection);//释放连接
    return true;
}

void CDatabaseManage::doLog(const QString logMsg, int logType, int user, const QString &mod,  QString logTime)
{
    QSqlQuery query(m_logDb);
    if(logTime.isEmpty()) logTime=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    query.prepare("insert into logData values(?,?,?,?,?)");
    query.addBindValue(logType);
    query.addBindValue(logTime);
    query.addBindValue(mod);
    query.addBindValue(user);
    query.addBindValue(logMsg);
    if(!query.exec()){
        qDebug()<<query.lastError().text();
    }

}

void CDatabaseManage::showLog(QString logDb,QSqlTableModel* model)
{
    m_logDb.setDatabaseName(logDb);
    QSqlQuery query(m_logDb);
    model->setQuery("select * from logData;",m_logDb);
    model->select();
}

QJsonObject CDatabaseManage::getRMHead()
{
    QSqlQuery query(_db);
    QJsonObject js;
    if(!query.exec("show columns from rm_info")){
        qDebug()<<"query error:"<<query.lastError().text();
        return js;
    }
    js["cmd"]=CMD_RM_GET_RM_TITLE;
    QJsonArray data;
    while(query.next()){
        QJsonObject columnInfo;
        columnInfo["Field"]=query.value(0).toString();
        columnInfo["Type"]=query.value(1).toString();
        columnInfo["Null"]=query.value(2).toString();
        data.append(columnInfo);
    }
    js["data"]=data;
    return js;
}

bool CDatabaseManage::doSql(const QString &sql)
{
    QSqlQuery query(_db);
    if(!query.exec(sql)){
        _lastError=query.lastError().text();
        return false;
    }
    return  true;
}

bool CDatabaseManage::addRM(const QJsonObject &rmData)
{
    QSqlQuery query(_db);
    QStringList keys=rmData.keys();
    int n=keys.size();
    QString sql1="insert into rm_info(";
    QString sql2="values(";
    for(int i=0;i<n;i++){
        sql1+=keys.at(i)+",";
        sql2+="?, ";

    }
    sql1+="add_date) "+sql2+"?)";
    query.prepare(sql1);
    for(int i=0;i<n;i++){
        query.addBindValue(rmData.value(keys.at(i)).toVariant());
    }
    query.addBindValue(QDate::currentDate());
    qDebug()<<sql1;
    if(!query.exec()){
        _lastError=query.lastError().text();
        return false;
    }
    return true;
}

bool CDatabaseManage::queryRM( QJsonObject &jsCMD)
{
//    QJsonObject condition=jsCMD.value("data").toObject();
//    QString sql="select * from rm_info ";
//    QStringList keys=condition.keys();
//    int n=keys.size();
//    if(n){
//        sql+="where ";
//        for(int i=0;i<n;i++){
//            sql+= doTranslate( keys.at(i),false)+condition.value(keys.at(i)).toString();
//            if(i<n-1) sql+=" and ";
//        }
//    }
//    QSqlQuery query(_db);
//    if(!query.exec(sql)){
//        _lastError=query.lastError().text();
//        return false;
//    }
//    QJsonArray table;
//    int columns=query.record().count();
//    while(query.next()){
//        QJsonArray row;
//        for(int i=0;i<columns;i++) row.append(query.value(i).toString());
//        table.append(row);

//    }
//    jsCMD["data"]=table;
    return true;
}

QSqlReturnMsg CDatabaseManage::doQuery(const QSqlCmd &sqlCmd,int userID)
{
    QSqlDatabase db;
    DatabaseConnection* connection=getUserConnection(userID);

    QString sql=sqlCmd.sql();
    sql=sql.trimmed();
    if(connection){
        db=connection->dataBase();
        if(!db.isOpen()){
            DB.doLog("用户数据库异常，sql: "+sql,ERROR_MSG,userID);
        }
        db.exec(QString("use %1").arg(m_dataBaseName));
    }
    else db=_db;
    QSqlQuery query(db);
    DB.doLog(QString("用户正在使用%1连接数据库:%2").arg(db.connectionName()).arg(QJsonDocument(sqlCmd.jsCmd()).toJson(QJsonDocument::Compact)),DEBUG_MSG,userID);
    int page=sqlCmd.queryPage();
    int totalPage=1;
    if(sqlCmd.useBindMod()){//增加了使用格式化的查询模式
//        _db.transaction();
        QStringList sqls=sql.split(";");

        static QRegularExpression re("\\?");
        int nowPos=0;
        QJsonArray values=sqlCmd.getBindValues();

        qDebug()<<"bindValues:"<<values;
        for(auto s:sqls){
            if(s.isEmpty()) continue;
            query.clear();
            QRegularExpressionMatchIterator i = re.globalMatch(s);
            int parameterCount = 0;
            while (i.hasNext()) {
                i.next();
                ++parameterCount;
            }
            if(nowPos+parameterCount>values.count()){
                return QSqlReturnMsg("参数数量不匹配",sqlCmd.flag(),sqlCmd.tytle(),true);
            }
            query.prepare(s);
            qDebug()<<s;
            for(int i=nowPos;i<nowPos+parameterCount;i++){
                query.bindValue(i-nowPos,values.at(i).toVariant());
                qDebug()<<values.at(i).toVariant();
            }

            if(!query.exec()){
                _lastError=query.lastError().text();
                //            _db.rollback();
                return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
            }
            nowPos+=parameterCount;

        }

    }
    else if(sqlCmd.queryPage()){//分页查询
        sql.replace(";"," ");//分页查询不能有分号，不能多个同时查询。
        //先获取总页数：
        QString str=QString("SELECT COUNT(*) FROM ( %1 ) as subquery;").arg(sql);
        if(!query.exec(str)){
            _lastError=query.lastError().text();
            return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
        }
        if(!query.next()){
            return QSqlReturnMsg("无效的查询：没有查询结果。",sqlCmd.flag(),sqlCmd.tytle(),true);
        }
        totalPage=(query.value(0).toInt()-1)/ITEMS_PER_PAGE+1;
        if(!totalPage){
            return QSqlReturnMsg("无效的分页查询：总页数为0。",sqlCmd.flag(),sqlCmd.tytle(),true);
        }
        //处理分页查询
//        str=QString("WITH PaginatedResults AS ( SELECT *, ROW_NUMBER() OVER (ORDER BY B.id) AS rn FROM ( %1 ) AS subquery)"
//                      "SELECT * FROM PaginatedResults WHERE rn BETWEEN %2 AND %3").arg(sql).arg((page-1)*ITEMS_PER_PAGE+1).arg(page*ITEMS_PER_PAGE);
        str=sql+QString(" LIMIT %1 OFFSET %2").arg(ITEMS_PER_PAGE).arg((page-1)*ITEMS_PER_PAGE);
        if(!query.exec(str)){
            _lastError=query.lastError().text();
            return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
        }
    }
    else{
        QStringList sqls=sql.split(";");
        for(auto s:sqls){
            if(s.isEmpty()) continue;
            query.clear();
            if(!query.exec(s)){
                _lastError=query.lastError().text();
                return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
            }
        }

    }
    QJsonArray table;
    QSqlRecord record=query.record();
    QJsonArray row;
    int columns=query.record().count();
//    int rows=query.size();
//    int page=sqlCmd.queryPage();
//    //处理分页显示：
//    int pages=(rows-1)/ITEMS_PER_PAGE+1;
//    int start=(page-1)*ITEMS_PER_PAGE+1;
//    int end=start+ITEMS_PER_PAGE-1;
    if(query.size()==-1)//非查询，返回影响的行数
    {
        table={query.numRowsAffected()};
    }
    else{
        for(int i=0;i<record.count();i++){
            QString name=record.fieldName(i);
            //        doTranslate(name);
            row.append(name);//获取列名

        }
        table.append(row);

//        if(!page){//不分页显示
//            start=1;
//            pages=1;
//            end=rows;
//        }
//        if(pages>=1){
//            if(page>pages) page=pages-1;
//        }
//        query.seek(start-2);

        // 逐行获取查询结果，并将数据添加到 JSON 数组中
        while (query.next() /*&& query.at() < end*/) {
            QJsonArray row;
            for (int i = 0; i < columns; i++) {
                row.append(query.value(i).toString());
            }
            table.append(row);
        }
    }

    return QSqlReturnMsg(table,sqlCmd.flag(),sqlCmd.tytle(),false,page,totalPage);
}

CDatabaseManage::CDatabaseManage(QObject *parent):
    QObject(parent),
    _dbOpen(false)
{
    QSettings set("./settings.ini",QSettings::IniFormat);

    QString Database;
    if(!set.value("Database").isValid()){
        set.setValue("Database","elims");
        Database="elims";
        qDebug()<<"set.is not exist";
    }
    else{
        Database=set.value("Database","elims").toString();
        qDebug()<<"set is exist";
    }

    connectDb(Database);
}

bool CDatabaseManage::doTranslate( QString &word, bool toCN)
{
    QSqlQuery query(_db);
    QString sql;
    if(toCN){
        sql=QString("select cn from dictionary where en = '%1'").arg(word);
    }
    else sql=QString("select en from dictionary where cn = '%1'").arg(word);
    if(!query.exec(sql)){
        qDebug()<<"翻译错误："<<query.lastError().text();
        return false;
    }
    if (query.next())
        word=query.value(0).toString();
    return true;
}

DatabaseConnection::DatabaseConnection(int id, QObject *parent)

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
//            CELLLog::Info("CDatabaseManage error on open database:%s",m_database.lastError().text().toUtf8().data());
        qDebug()<<m_database.lastError().text();
        return;
    }
    m_database.setDatabaseName("elims");
    qDebug()<<QString("%1 opened!").arg(m_database.connectionName())<<this->m_database;
    m_timer=new QTimer;
    connect(m_timer,&QTimer::timeout,this,[this](){
        qDebug()<<"连接超时，rollBack:"+m_database.connectionName();
        m_database.rollback();
        emit timeOut();
    });
    connect(this,&DatabaseConnection::startCheck,this,[this](){m_timer->start(30000);});
    connect(this,&DatabaseConnection::stopCheck,this,[this](){m_timer->stop();});
}
bool DatabaseConnection::beginTransaction()
{
    QMutexLocker locker(&m_mutex);
    if(!m_database.transaction()){
        DB.doLog("开启事务失败。",CDatabaseManage::ERROR_MSG,DB_POOL->connectionUser(this));
        return false;
    }

    emit startCheck();
    qDebug()<<m_database.connectionName()+"启用超时检查 。";
    return true;
}

bool DatabaseConnection::commitTransaction()
{
    QMutexLocker locker(&m_mutex);
    DB.doLog("收到提交信号，开始提交事务。");
    if(!m_database.commit()){
        DB.doLog("提交事务失败。",CDatabaseManage::ERROR_MSG,DB_POOL->connectionUser(this));
        return false;
    }

    emit stopCheck();
//    DB_POOL->releaseConnection(this);
    return true;
}
bool DatabaseConnection::rollbackTransaction()
{
    QMutexLocker locker(&m_mutex);
    if(!m_database.rollback()){
        DB.doLog("回滚事务失败。",CDatabaseManage::ERROR_MSG,DB_POOL->connectionUser(this));
        return false;
    }    
    emit stopCheck();
//    DB_POOL->releaseConnection(this);
    return true;
}

ConnectionPool::ConnectionPool(int maxConnections, int timeout)
    : m_maxConnections(maxConnections), m_timeout(timeout)
{
    // 创建连接池中的数据库连接
    for (int i = 0; i < maxConnections; ++i) {
        DatabaseConnection *connection = new DatabaseConnection(i);
        m_connections.append(connection);
        connect(connection,&DatabaseConnection::timeOut,this,[this, connection](){
            DB.doLog(QString("连接池被强制回滚:%1").arg(connection->dataBase().connectionName()));
            releaseConnection(connection);
        });
    }

    // 启动定时器，定时检查连接超时情况
//    m_timer.setInterval(timeout);
//    connect(&m_timer, &QTimer::timeout, this, &ConnectionPool::checkConnections);
//    m_timer.start();
}

DatabaseConnection *ConnectionPool::getConnection(int userID)
{
    QMutexLocker locker(&m_mutex);
    for (auto it = m_usedConnections.begin(); it != m_usedConnections.end(); ++it) {
        if (it.value() == userID) return it.key();
    }
    // 查找一个可用的连接
    for (DatabaseConnection *connection : m_connections) {
        if (!m_usedConnections.contains(connection)) {
            // 设置连接的活跃时间定时器
//            QTimer* timer = new QTimer(connection);
//            timer->setSingleShot(true);
//            connect(timer, &QTimer::timeout, this, [this, connection]() {
//                qDebug()<<"timeout";
//                rollbackTransaction(connection);
//                releaseConnection(connection);
//            });
//            timer->start(m_timeout);

            m_usedConnections[connection] = userID;
            DB.addUserConnection(userID,connection);//保存用户的连接，用于用户操作。
            return connection;
        }
    }

    // 如果没有可用的连接，则创建新连接
    if (m_connections.size() < m_maxConnections) {
        DatabaseConnection *connection = new DatabaseConnection(m_connections.size() + 1);
        // 设置连接的活跃时间定时器
//        QTimer* timer = new QTimer(connection);
//        timer->setSingleShot(true);
//        connect(timer, &QTimer::timeout, this, [this, connection]() {
//            rollbackTransaction(connection);
//            releaseConnection(connection);
//        });
//        timer->start(m_timeout);

        m_connections.append(connection);
        m_usedConnections[connection] = userID;
        DB.addUserConnection(userID,connection);//保存用户的连接，用于用户操作。
        connect(connection,&DatabaseConnection::timeOut,this,[this, connection](){
            DB.doLog(QString("连接池被强制回滚:%1").arg(connection->dataBase().connectionName()));
            releaseConnection(connection);
        });
        return connection;
    }

    // 所有连接都在使用中，返回nullptr表示无可用连接
    return nullptr;
}

void ConnectionPool::releaseConnection(DatabaseConnection *connection)
{
    QMutexLocker locker(&m_mutex);
    int userID=m_usedConnections.value(connection);
    DB.removeUserConnection(userID);
    m_usedConnections.remove(connection);
    DB.doLog(QString("事务完成，释放连接%1").arg(connection->dataBase().connectionName()),CDatabaseManage::DEBUG_MSG,userID);
    // 移除连接的活跃时间定时器
//    QTimer* timer = connection->findChild<QTimer*>();
//    if (timer) {
//        timer->stop();
//        timer->deleteLater();
//    }
}
bool ConnectionPool::commitTransaction(DatabaseConnection *connection)
{
    if (connection) {
        // 更新连接的活跃时间
        //            QTimer* timer = connection->findChild<QTimer*>();
        //            if (timer) {
        //                timer->start(m_timeout);
        //            }
//        DB.doLog("发送提交事务信号");
        return connection->commitTransaction();
//        return true;
    }
    return false;
}

void ConnectionPool::checkConnections()
{
    QMutexLocker locker(&m_mutex);
    for (auto it = m_usedConnections.begin(); it != m_usedConnections.end(); ) {
        DatabaseConnection *connection = it.key();
        int userID = it.value();
        // 查找连接的活跃时间定时器
        QTimer* timer = connection->findChild<QTimer*>();
        if (timer && timer->isActive()) {
            ++it;
        } else {
            // 超时未提交事务，回滚并释放连接
            DB.doLog(QString("事务超时被回滚：用户ID：%1").arg(userID),DB.ERROR_MSG);
            DB.rollbackTransaction(userID);
//            rollbackTransaction(connection);
//            releaseConnection(connection);
            it = m_usedConnections.erase(it);

        }
    }

}
