#include "cdatabasemanage.h"
#include"../../depends/CELLLog.h"
#include<QDebug>
#include<QSqlRecord>
#include<QSqlField>
#include<QJsonArray>
#include<QSqlError>
#include<QDate>
void CDatabaseManage::connectDb()
{
    _db=QSqlDatabase::addDatabase("QMYSQL","elims");
    _db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

    _db.setHostName("127.0.0.1");
    _db.setUserName("root");
    _db.setPassword("spring01");
    if(!_db.open()){
        CELLLog::Info("CDatabaseManage error on open database:%s",_db.lastError().text().toUtf8().data());
        qDebug()<<_db.lastError().text();
        return;
    }
    _db.exec("CREATE DATABASE IF NOT EXISTS elims");
   _db.setDatabaseName("elims");
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
    if(!query.exec("create table if not exists flow_operate_records (id int not null auto_increment, flowID int, operatorID int,operateStatus int default 0, operateComments varchar(255), operateTime DateTime, primary key (id), FOREIGN KEY (flowID) REFERENCES flow_records (id), FOREIGN KEY (operatorID) REFERENCES sys_employee_login (id)  );")){
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

QSqlReturnMsg CDatabaseManage::doQuery(const QSqlCmd &sqlCmd)
{
    QSqlQuery query(_db);
    QString sql=sqlCmd.sql();
    if(sqlCmd.useBindMod()){//增加了使用格式化的查询模式，适用于大量操作，开启事务模式。
        _db.transaction();
        query.prepare(sql);
        QJsonArray values=sqlCmd.getBindValues();
        qDebug()<<"bindValues:"<<values;
        for (int i=0;i<values.count();i++) {
            query.bindValue(i,values.at(i).toVariant());
        }
        if(!query.exec()){
            _lastError=query.lastError().text();
            _db.rollback();
            return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
        }
//        if(query.numRowsAffected() == 0) {
//            _lastError="0条成功。";
//            _db.rollback();
//            return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
//        }

        _db.commit();
    }
    else if(!query.exec(sql)){
        _lastError=query.lastError().text();
        return QSqlReturnMsg(_lastError,sqlCmd.flag(),sqlCmd.tytle(),true);
    }
    QJsonArray table;
    QSqlRecord record=query.record();
    QJsonArray row;
    int columns=query.record().count();
    int rows=query.size();
    int page=sqlCmd.queryPage();
    //处理分页显示：
    int pages=(rows-1)/ITEMS_PER_PAGE+1;
    int start=(page-1)*ITEMS_PER_PAGE+1;
    int end=start+ITEMS_PER_PAGE-1;
    qDebug()<<query.lastQuery().toStdString();
    qDebug()<<query.numRowsAffected();
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

        if(!page){//不分页显示
            start=1;
            pages=1;
            end=rows;
        }
        if(pages>=1){
            if(page>pages) page=pages-1;
        }
        query.seek(start-2);

        // 逐行获取查询结果，并将数据添加到 JSON 数组中
        while (query.next() && query.at() < end) {
            QJsonArray row;
            for (int i = 0; i < columns; i++) {
                row.append(query.value(i).toString());
            }
            table.append(row);
        }
    }

    return QSqlReturnMsg(table,sqlCmd.flag(),sqlCmd.tytle(),false,page,pages);
}

CDatabaseManage::CDatabaseManage():
    _dbOpen(false)
{
    connectDb();
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
