#include"CELLClient.h"
#include"user.h"
CELLClient::CELLClient(SOCKET sockfd, const char* IP, int sendSize, int recvSize):
    _IP(IP),
    _sendBuff(sendSize),
    _recvBuff(recvSize),
    _user(nullptr)
{
    static int n = 1;
    id = n++;
    _sockfd = sockfd;

    resetDTHeart();
    resetDTSend();
}

CELLClient::~CELLClient()
{
    CELLLog::Info("s=%d CELLClient%d.~CELLClient\n", serverId, id);
    if (INVALID_SOCKET != _sockfd)
    {
#ifdef _WIN32
            closesocket(_sockfd);
#else
            close(_sockfd);
#endif
            _sockfd = INVALID_SOCKET;
    }
    if(_user) _user->OnDisconnect();//断开连接，通知服务器保存与清理用户信息
}

