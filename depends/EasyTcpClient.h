#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#include"CELL.h"
#include"CELLNetWork.hpp"
#include"MessageHeader.h"
#include"CELLClient.h"
#include<QObject>
class EasyTcpClient:public QObject
{
public:
        EasyTcpClient();
	
        virtual ~EasyTcpClient();
	//初始化socket
        void InitSocket(int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);

	//连接服务器
        int Connect(const char* ip,unsigned short port);

	//关闭套节字closesocket
        void Close();

	//处理网络消息
        void OnRun();

	//是否工作中
	bool isRun()
	{
		return _pClient && _isConnect;
	}

	//接收数据 处理粘包 拆分包
        int RecvData(SOCKET cSock);

	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header) = 0;
        virtual void connectError(const char*error)=0;
	//发送数据
        int SendData(netmsg_DataHeader* header);
        int SendData(const QJsonObject& json);

        int SendData(const char* pData, int len);
protected:
	CELLClient* _pClient = nullptr;
	bool _isConnect = false;
private:
    CELLThread _thread;
    CELLTimestamp _heartBeatTime;
};

#endif
