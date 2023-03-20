#ifndef USER_H
#define USER_H
//用户类，用于登录后的用户信息管理。
#include"CELLClient.h"
class CELLUser
{
public:
    CELLUser();
    virtual ~CELLUser();
    virtual void OnDisconnect()=0;//断开连接事件,用以清理USER资源。
    virtual int id()const{
        return 0;
    }
    virtual QString name()const{
        return "";
    }
    void linktoClient(CELLClient* client){
        _clientSocket=client;
        client->setUser(this);
    }
    CELLClient* client()const{
        return _clientSocket;
    }
private:
    CELLClient* _clientSocket;
};

#endif // USER_H
