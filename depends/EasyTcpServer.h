#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"CELL.h"
#include"CELLClient.h"
#include"CELLServer.h"
#include"INetEvent.h"
#include"CELLNetWork.hpp"

#include<thread>
#include<mutex>
#include<atomic>
class EasyTcpServer : public INetEvent
{
private:
	//
	CELLThread _thread;
	//消息处理对象，内部会创建线程
	std::vector<CELLServer*> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	//
	SOCKET _sock;
    std::list<CELLClient*> _clients;
protected:
	//SOCKET recv计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
        EasyTcpServer();
        virtual ~EasyTcpServer();
	//初始化Socket
        SOCKET InitSocket();

	//绑定IP和端口号
        int Bind(const char* ip, unsigned short port);

	//监听端口号
        int Listen(int n);

	//接受客户端连接
        SOCKET Accept();
	
        void addClientToCELLServer(CELLClient* pClient);
        void removeClient(CELLClient* pClient);
        void Start(int nCELLServer=1);
	//关闭Socket
        void Close();

	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
        virtual void OnNetJoin(CELLClient* pClient);
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
        virtual void OnNetLeave(CELLClient* pClient);
        virtual void OnNetError(CELLClient* pClient);
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
        virtual void OnNetMsg(CELLServer* pServer, CELLClient* pClient, netmsg_DataHeader* header);

        virtual void OnNetRecv(CELLClient* pClient);

        std::list<CELLClient*> Clients()const;
private:
	//处理网络消息
        void OnRun(CELLThread* pThread);

	//计算并输出每秒收到的网络消息
        void time4msg();
};

#endif // !_EasyTcpServer_hpp_
