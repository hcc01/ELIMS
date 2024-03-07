#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

#include <cstdint>
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
    CMD_NOTICE,//字符串
    CMD_INFORMATION,
    CMD_ERROR,
    CMD_JSON_CMD,//JSON信息
    CMD_RM_GET_RM_TITLE,
    CMD_INIT,
    CMD_START_QUERY,
    CMD_END_QUERY,
    CMD_START_Transaction,
    CMD_COMMIT_Transaction,
    CMD_ROLLBACK_Transaction,
};

enum LoginResult{
    LOGIN_SUCCESSED,
    LOGIN_FAIL,
    DB_ERROR
};

enum JSON_CDM{
    JC_DO_SQL,
    JC_WORKFLOW,
    JC_NOTICE,
};
enum SQL_TYPE{
    SQL_ADD_RM,
    SQL_QUERY_RM,
    SQL_QUERY_EMPLOYEE,
    SQL_ADD_EMPLOYEE,
    SQL_NO_RETURN,
    SQL_GET_TABLES,
};
enum WF_TYPE{
    WF_INIT,
    WF_CREATE_PROCESS,
    WF_CREATE_NODE,
    WF_NEW_PROCESS,
};

enum NOTICE_TYPE{
    NT_WORKFLOW,
};

struct netmsg_DataHeader
{
	netmsg_DataHeader()
	{
		dataLength = sizeof(netmsg_DataHeader);
		cmd = CMD_ERROR;
	}
//	unsigned short dataLength;
    uint32_t dataLength;
	unsigned short cmd;
};

//DataPackage
struct netmsg_Login : public netmsg_DataHeader
{
	netmsg_Login()
	{
		dataLength = sizeof(netmsg_Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
	char data[32];
};

struct netmsg_LoginR : public netmsg_DataHeader
{
	netmsg_LoginR()
	{
		dataLength = sizeof(netmsg_LoginR);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
    int position;
    char name[32];

};

struct netmsg_Logout : public netmsg_DataHeader
{
	netmsg_Logout()
	{
		dataLength = sizeof(netmsg_Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct netmsg_LogoutR : public netmsg_DataHeader
{
	netmsg_LogoutR()
	{
		dataLength = sizeof(netmsg_LogoutR);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct netmsg_NewUserJoin : public netmsg_DataHeader
{
	netmsg_NewUserJoin()
	{
		dataLength = sizeof(netmsg_NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

struct netmsg_c2s_Heart : public netmsg_DataHeader
{
	netmsg_c2s_Heart()
	{
		dataLength = sizeof(netmsg_c2s_Heart);
		cmd = CMD_C2S_HEART;
	}
};

struct netmsg_s2c_Heart : public netmsg_DataHeader
{
	netmsg_s2c_Heart()
	{
		dataLength = sizeof(netmsg_s2c_Heart);
		cmd = CMD_S2C_HEART;
	}
};

struct netMsg_Init: public netmsg_DataHeader{
    netMsg_Init(){
        dataLength = sizeof(netMsg_Init);
        cmd = CMD_INIT;
    }
};

#endif // !_MessageHeader_hpp_
