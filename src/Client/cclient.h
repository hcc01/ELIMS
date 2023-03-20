#ifndef CCLIENT_H
#define CCLIENT_H

#include"../../depends/EasyTcpClient.h"
#include<QObject>
class CClient:public EasyTcpClient
{
    Q_OBJECT
public:
    CClient();
    void OnNetMsg(netmsg_DataHeader* header);
    void connectError(const char* error) ;
    int SendData(netmsg_DataHeader *header)
    {

        int r=EasyTcpClient::SendData(header);
        if(r!=SOCKET_ERROR) emit sendingData();
        return r;
    }
    int SendData(const QJsonObject& json)
    {
        int r=EasyTcpClient::SendData(json);
        if(r!=SOCKET_ERROR) emit sendingData();
        return r;
    }

signals:
    void netMsg(netmsg_DataHeader* header);
    void sendingData();//主要是控制命令量，发一个命令后等服务器回应后才能执行下一次指令。
    void onConnectError(const char* error);

};

#endif // CCLIENT_H
