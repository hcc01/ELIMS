#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

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
    CMD_ERROR,
    CMD_JSON_CMD,//JSON信息
    CMD_RM_GET_RM_TITLE,
};

enum LoginResult{
    LOGIN_SUCCESSED,
    LOGIN_FAIL,
    DB_ERROR
};
/*
 * JC_DO_SQL 执行SQL语句的命令
 * 格式：
 * {
 *      "CMD":JC_DO_SQL  注明命令类型
 *      "sql_info": variant  发给服务器时为要执行的SLQ语句(QString)，服务器返回值为执行结果（二维数组）或错误信息
 *      "sql_type": 表的操作类型标记，用于标记具体的动作，如修改人员数据、增加人员数据等。
 *      "result": bool  服务器返回的操作结果
 * }
*/
enum JsonCDM{
    JC_DO_SQL,
};
enum SQLMark{
    SQL_ADD_RM,
    SQL_QUERY_RM,
    SQL_QUERY_EMPLOYEE,
    SQL_ADD_EMPLOYEE,
};

struct netmsg_DataHeader
{
	netmsg_DataHeader()
	{
		dataLength = sizeof(netmsg_DataHeader);
		cmd = CMD_ERROR;
	}
	unsigned short dataLength;
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
	char data[92];
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

#endif // !_MessageHeader_hpp_
