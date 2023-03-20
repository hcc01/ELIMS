#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"CELL.h"
#include"INetEvent.h"
#include"CELLClient.h"
#include"CELLSemaphore.hpp"

#include<vector>
#include<map>

//网络消息接收处理服务类
class CELLServer
{
public:
        CELLServer(int id);

        ~CELLServer();

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
        void Close();

	//处理网络消息
        void OnRun(CELLThread* pThread);

        void CheckTime();
        void OnClientLeave(CELLClient* pClient);

        void WriteData(fd_set& fdWrite);

        void ReadData(fd_set& fdRead);

	//接收数据 处理粘包 拆分包
        int RecvData(CELLClient* pClient);

	//响应网络消息
	virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

        void addClient(CELLClient* pClient);
        void removeClient(CELLClient* pClient);//add by hcc 2021.4.18,主动移除客户端（踢出）
        void Start();

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//void addSendTask(CELLClient* pClient, netmsg_DataHeader* header)
	//{
	//	_taskServer.addTask([pClient, header]() {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}
private:
        void ClearClients();
private:
	//正式客户队列
	std::map<SOCKET, CELLClient*> _clients;
	//缓冲客户队列
	std::vector<CELLClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CELLTaskServer _taskServer;
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//
	SOCKET _maxSock;
	//旧的时间戳
	time_t _oldTime = CELLTime::getNowInMilliSec();
	//
	CELLThread _thread;
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change = true;
};

#endif // !_CELL_SERVER_HPP_
