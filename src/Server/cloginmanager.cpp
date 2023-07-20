#include "cloginmanager.h"
#include"cdatabasemanage.h"
#include<QVariant>
CLoginManager::CLoginManager(QObject *parent) : QObject(parent)
{

}

CUser* CLoginManager::doLogin(CELLClient *pClient, const QString &name, const QString &passwd)
{
    netmsg_LoginR lr;
    CDatabaseManage &dbManager=CDatabaseManage::Instance();
    if(!dbManager.isOpen()) {
       CELLLog::Info("doLogin error: db not open.");
       lr.result=DB_ERROR;
       pClient->SendData(&lr);
       return nullptr;
    }
    QSqlQuery query(dbManager.database());
    QString sql(QString("select * from sys_employee_login where name='%1' and password='%2'").arg(name).arg(passwd));
    if(!query.exec(sql)){
        CELLLog::Info("Login error: %s",query.lastError().text());
        lr.result=DB_ERROR;
        pClient->SendData(&lr);
        return nullptr;
    }
    //QSqlRecord rcd=query.record();
    if(query.next()) {                              //密码验证成功
       lr.result=LOGIN_SUCCESSED;
        const char *name = query.value("name").toByteArray();
        size_t name_len = strlen(name);
        char *name_copy = new char[name_len + 1];  // 为复制的字符数组分配内存
        strcpy(name_copy, name);  // 复制字符数组
        memcpy(lr.name, name_copy, name_len);  // 将复制的字符数组复制到 lr.name 所指向的内存区域中
        lr.name[name_len] = '\0';  // 确保字符串以 null 结尾
        delete[] name_copy;  // 释放为复制的字符数组分配的内存
       CUser* user=new CUser(query.value("id").toInt(),query.value("name").toString());
       if(_userMap.contains(user->id())) {
           CUser* oldUser=_userMap.value(user->id());
           CELLClient* oldClient=oldUser->client();
           if(oldClient&&oldClient!=pClient){//其它地方登录，踢出原来连接
               QJsonObject jsonCMD;
               jsonCMD["cmd"]=CMD_LOGOUT;
               jsonCMD["data"]=QString("有人从%1处登录了该账号，如果不是本人操作，请及时更改密码。").arg(pClient->IP());
               oldClient->SendData(jsonCMD);
               oldClient->setUser(nullptr);
               oldUser->linktoClient(pClient);
           }
           else oldUser->linktoClient(pClient);//重新连接。网络断线时未清理USER（见onNectError），所以直接连接。
       }
       else{
           user->linktoClient(pClient);
           addUser(user);
       }
        pClient->SendData(&lr);             //传回登录成功的消息
        return user;//将用户信息返回给调用者操作。
    }
    lr.result=LOGIN_FAIL;
    pClient->SendData(&lr);
    return nullptr;
}

void CLoginManager::addUser(CUser* user)
{
    if(!user) return;
    _users.append(user);
    _userMap[user->id()]=user;
}

void CLoginManager::removeUser(CUser *user)
{
    if(!user) return;
    _users.removeOne(user);
    _userMap.remove(user->id());
}

CUser *CLoginManager::findUser(int id)
{
    if(!_userMap.contains(id)) return nullptr;
    return _userMap.value(id);
}
