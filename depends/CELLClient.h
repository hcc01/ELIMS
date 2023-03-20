#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include"CELL.h"
#include"CELLBuffer.h"
#include"CELLMsgStream.h"
#include<QString>
#include<QJsonObject>
#include<QJsonDocument>
//客户端心跳检测死亡计时时间
#define CLIENT_HREAT_DEAD_TIME 60000
//在间隔指定时间后
//把发送缓冲区内缓存的消息数据发送给客户端
#define CLIENT_SEND_BUFF_TIME 200
//客户端数据类型
class CELLUser;
class CELLClient
{
public:
	int id = -1;
    //所属serverid
	int serverId = -1;
public:
        CELLClient(SOCKET sockfd , const char* IP, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);
        ~CELLClient();

	SOCKET sockfd()
	{
		return _sockfd;
	}

    QString IP()const{
        return _IP;
    }

	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuff.data();
	}

	void pop_front_msg()
	{
		if(hasMsg())
			_recvBuff.pop(front_msg()->dataLength);
	}

	bool needWrite()
	{
		return _sendBuff.needWrite();
	}

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}

	//缓冲区的控制根据业务需求的差异而调整
	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		return SendData((const char*)header, header->dataLength);
	}
    int SendData(const char* notice){//直接发送字符串
        CELLWriteStream s;
        s.setNetCmd(CMD_NOTICE);
        s.WriteString(notice);
        s.finsh();
        return SendData(s.data(),s.length());
    }
    int SendData(const QJsonObject& jsOb){//直接发送CMD_JSON
        CELLWriteStream s;
        s.setNetCmd(CMD_JSON_CMD);
        QJsonDocument jd(jsOb);
        QByteArray ba=jd.toJson(QJsonDocument::Compact);
        char* info=ba.data();
        s.WriteString(info);
        s.finsh();
        return SendData(s.data(),s.length());
    }
	int SendData(const char* pData, int len)
	{
		if (_sendBuff.push(pData, len))
		{
			return len;
		}
		return SOCKET_ERROR;
	}

	void resetDTHeart()
	{
		_dtHeart = 0;
	}

	void resetDTSend()
	{
		_dtSend = 0;
	}

	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			CELLLog::Info("checkHeart dead:s=%d,time=%ld\n",_sockfd, _dtHeart);
			return true;
		}
		return false;
	}

	//定时发送消息检测
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//CELLLog::Info("checkSend:s=%d,time=%d\n", _sockfd, _dtSend);
			//立即将发送缓冲区的数据发送出去
			SendDataReal();
			//重置发送计时
			resetDTSend();
			return true;
		}
		return false;
	}
    CELLUser* getUser()const{
        return _user;
    }
    void setUser(CELLUser* user){
        _user=user;
    }
    bool isLogined()const{
        return _user!=nullptr;
    }
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
    QString _IP;
	//第二缓冲区 接收消息缓冲区
	CELLBuffer _recvBuff;
	//发送缓冲区
	CELLBuffer _sendBuff;
	//心跳死亡计时
	time_t _dtHeart;
	//上次发送消息数据的时间 
	time_t _dtSend;
	//发送缓冲区遇到写满情况计数
	int _sendBuffFullCount = 0;
    CELLUser* _user;//给连接一个用户类，用于管理登录后的用户。
};

#endif // !_CELLClient_HPP_
