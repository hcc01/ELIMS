#ifndef CLOGINMANAGER_H
#define CLOGINMANAGER_H

#include <QObject>
#include"../../depends/CELLClient.h"
#include"cuser.h"
#include<QMap>
class CLoginManager : public QObject
{
    Q_OBJECT
public:
    explicit CLoginManager(QObject *parent = nullptr);
    CUser* doLogin(CELLClient *pClient, const QString& id, const QString &passwd);//处理登录，成功后传回用户。
    QMap<int,CUser*>userMap()const{
        return _userMap;
    }
    QList<CUser*>userList()const{
        return _users;
    }
    void addUser(CUser *user);
    void removeUser(CUser *user);
    CUser *findUser(int id);
signals:
private:
    QMap<int,CUser*>_userMap;

    QList<CUser*> _users;
};

#endif // CLOGINMANAGER_H
