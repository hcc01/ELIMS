﻿#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include"MessageHeader.h"
#include"CELLStream.h"

#include<QJsonObject>
#include<QJsonDocument>
//消息数据字节流
class CELLReadStream :public CELLStream
{
public:
        CELLReadStream(netmsg_DataHeader* header);

        CELLReadStream(char* pData, int nSize, bool bDelete = false);

	uint16_t getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
	}
    QString getNoticeStr(){//直接用于字符串类信息的读取
        int n=ReadInt32();
        qDebug()<<"len:"<<n;
        getNetCmd();
        if(n<1024){
            char str[1024]={};
            ReadArray(str,n);
            return QString(str);
        }
        if(n<2048){
            char str[2048]={};
            ReadArray(str,n);
            return QString(str);
        }
        if(n<8192){
            char str[8192]={};
            ReadArray(str,n);
            return QString(str);
        }
        if(n<102400){
            char str[102400]={};
            ReadArray(str,n);
            return QString(str);
        }
        if(n<512400){
            char str[512400]={};
            ReadArray(str,n);
            return QString(str);
        }
        qDebug()<<"warning: too long msg,length="<<n;
        return QString();
    }
    QJsonObject getJsonData(){//直接用于JSON类信息的读取
        QJsonDocument jd=QJsonDocument::fromJson(getNoticeStr().toUtf8());
       // qDebug()<<jd;
        return jd.object();
    }
};

//消息数据字节流
class CELLWriteStream :public CELLStream
{
public:
        CELLWriteStream(char* pData, int nSize, bool bDelete = false);

        CELLWriteStream(int nSize = 512400);

	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
	}

	bool WriteString(const char* str, int len)
	{
		return WriteArray(str, len);
	}

	bool WriteString(const char* str)
	{
		return WriteArray(str, strlen(str));
	}

	bool WriteString(std::string& str)
	{
		return WriteArray(str.c_str(), str.length());
	}

        void finsh();
};


#endif // !_CELL_MSG_STREAM_HPP_
