#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include"MessageHeader.h"
#include"CELLStream.h"

#include<QJsonObject>
#include<QJsonDocument>
#include "NetKits_global.h"
//消息数据字节流
class NETKITS_EXPORT CELLReadStream :public CELLStream
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
        int n=ReadInt16();
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
        return QString();
    }
    QJsonObject getJsonData(){//直接用于JSON类信息的读取
        QJsonDocument jd=QJsonDocument::fromJson(getNoticeStr().toUtf8());
        return jd.object();
    }
};

//消息数据字节流
class NETKITS_EXPORT CELLWriteStream :public CELLStream
{
public:
        CELLWriteStream(char* pData, int nSize, bool bDelete = false);

        CELLWriteStream(int nSize = 1024);

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
