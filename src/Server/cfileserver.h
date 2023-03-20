#ifndef CFILESERVER_H
#define CFILESERVER_H

#include"../../depends/EasyTcpServer.h"
class CFileServer:public EasyTcpServer
{
public:
    CFileServer();
    void OnNetJoin(CELLClient* pClient) override;
    void OnNetLeave(CELLClient* pClient) override;
    void OnNetError(CELLClient* pClient) override;//处理掉线问题
    void OnNetMsg(CELLServer* pServer, CELLClient* pClient, netmsg_DataHeader* header) override;
};

#endif // CFILESERVER_H
