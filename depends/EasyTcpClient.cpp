#include"EasyTcpClient.h"

EasyTcpClient::EasyTcpClient()
{
    _isConnect = false;
}

EasyTcpClient::~EasyTcpClient()
{
    Close();
}

void EasyTcpClient::InitSocket(int sendSize, int recvSize)
{
    CELLNetWork::Init();

    if (_pClient)
    {
        CELLLog::Info("warning, initSocket close old socket<%d>...\n", (int)_pClient->sockfd());
        Close();
    }
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sock)
    {
        CELLLog::Info("error, create socket failed...\n");
    }
    else {
        //CELLLog::Info("create socket<%d> success...\n", (int)sock);
        _pClient = new CELLClient(sock, nullptr,sendSize, recvSize);
    }
}

int EasyTcpClient::Connect(const char *ip, unsigned short port)
{
    if (!_pClient)
    {
        InitSocket();
    }
    // 2 连接服务器 connect
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(port);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
    _sin.sin_addr.s_addr = inet_addr(ip);
#endif
    //CELLLog::Info("<socket=%d> connecting <%s:%d>...\n", (int)_pClient->sockfd(), ip, port);
    int ret = ::connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        //CELLLog::Info("<socket=%d> connect <%s:%d> failed...\n", (int)_pClient->sockfd(), ip, port);
    }
    else {
        _isConnect = true;
        //CELLLog::Info("<socket=%d> connect <%s:%d> success...\n", (int)_pClient->sockfd(), ip, port);
    }
    if(_isConnect){
        _thread.Start(nullptr,
            [this](CELLThread* pThread) {
                OnRun();
            });
        _heartBeatTime.update();
    }
    return ret;
}

void EasyTcpClient::Close()
{
    _thread.Exit();
    if (_pClient)
    {
        delete _pClient;
        _pClient = nullptr;
    }
    _isConnect = false;
}

void EasyTcpClient::OnRun()
{
    while (isRun())
    {
        SOCKET _sock = _pClient->sockfd();

        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);

        fd_set fdWrite;
        FD_ZERO(&fdWrite);

        timeval t = { 0,1 };
        int ret = 0;
        if (_pClient->needWrite())
        {
            FD_SET(_sock, &fdWrite);
            ret = select(_sock + 1, &fdRead, &fdWrite, nullptr, &t);
        }else {
            ret = select(_sock + 1, &fdRead, nullptr, nullptr, &t);
        }

        if (ret < 0)
        {
            CELLLog::Info("error,<socket=%d>OnRun.select exit\n", (int)_sock);
            connectError("OnRun.select exit");
            Close();            
            break;
        }

        if (FD_ISSET(_sock, &fdRead))
        {
            if (-1 == RecvData(_sock))
            {
                CELLLog::Info("error,<socket=%d>OnRun.select RecvData exit\n", (int)_sock);
                connectError("OnRun.select RecvData exit");
                Close();
                break;
            }
        }

        if (FD_ISSET(_sock, &fdWrite))
        {
            if (-1 == _pClient->SendDataReal())
            {
                CELLLog::Info("error,<socket=%d>OnRun.select SendDataReal exit\n", (int)_sock);
                connectError("OnRun.select SendDataReal exit");
                Close();
                break;;
            }
        }
        if(_heartBeatTime.getElapsedSecond()>25){//向服务器发送心跳包
            _heartBeatTime.update();
            netmsg_c2s_Heart heart;
            _pClient->SendData(&heart);
        }
    }
}

int EasyTcpClient::RecvData(SOCKET cSock)
{
    if (isRun())
    {
        //接收客户端数据
        int nLen = _pClient->RecvData();
        if (nLen > 0)
        {
            //循环 判断是否有消息需要处理
            while (_pClient->hasMsg())
            {
                //处理网络消息
                OnNetMsg(_pClient->front_msg());
                //移除消息队列（缓冲区）最前的一条数据
                _pClient->pop_front_msg();
            }
        }
        return nLen;
    }
    return 0;
}

int EasyTcpClient::SendData(netmsg_DataHeader *header)
{
    if(isRun())
        return _pClient->SendData(header);
    return 0;
}
int EasyTcpClient::SendData(const QJsonObject& json)
{
    if(isRun())
        return _pClient->SendData(json);
    return 0;
}


int EasyTcpClient::SendData(const char *pData, int len)
{
    if (isRun())
        return _pClient->SendData(pData, len);
    return 0;
}
