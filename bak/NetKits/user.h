#ifndef USER_H
#define USER_H
//用户类，用于登录后的用户信息管理。
#include"CELLClient.h"
#include"NetKits_global.h"
class NETKITS_EXPORT CELLUser
{
public:
    CELLUser();
    virtual ~CELLUser();
    virtual void OnDisconnect()=0;//断开连接事件,用以清理USER资源。
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
