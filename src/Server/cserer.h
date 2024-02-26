#ifndef CSERER_H
#define CSERER_H
#include"../../depends/EasyTcpServer.h"
#include"cloginmanager.h"
#include"cuser.h"
#include<QMap>
#include<QJsonObject>
#include"../Client/qjsoncmd.h"
#include"processmanager.h"
class CServer:public EasyTcpServer
{
public:
    CServer();
    void OnNetJoin(CELLClient* pClient) override;
    void OnNetLeave(CELLClient* pClient) override;
    void OnNetError(CELLClient* pClient) override;//处理掉线问题
    void OnNetMsg(CELLServer* pServer, CELLClient* pClient, netmsg_DataHeader* header) override;
private:
    void onJsonCMD(CELLClient *pClient, QJsonObject &json);
    void init(CELLClient *pClient);//初始化用户信息，传回一些初始化的信息，如用户的信息、用户的离线消息、待办事项……
    void createWorkFlow(const NewWorkFlowCMD &jsCMD,CELLClient *pClient);
    void noticeToDo(int recordID, CELLClient *pClient);
    void sendMsgTo(CELLClient *pClient, const char *msg, int msgType);
private:
    CLoginManager _lgManager;
    ProcessManager _wfManager;
};

#endif // CSERER_H
