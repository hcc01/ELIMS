#ifndef CUSER_H
#define CUSER_H

#include"../../depends/user.h"
#include<QString>
#include<QVector>
class CUser:public CELLUser
{
public:
    CUser();
    CUser(int ID, const QString& name);
    void OnDisconnect()override;
    QString name()const override{
        return _name;
    }
    int id()const override{
        return _id;
    }
private:
    QString _name;
    int _id;
    QString _group;
    QVector<CUser*> _friendsOnline;
};

#endif // CUSER_H
