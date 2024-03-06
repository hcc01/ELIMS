#include "cserer.h"
#include"QSqlError"
#include<QDebug>
#include"cdatabasemanage.h"
#include<QDateTime>
CServer::CServer()
{
}

void CServer::OnNetJoin(CELLClient *pClient)
{
    EasyTcpServer::OnNetJoin(pClient);
}

void CServer::OnNetLeave(CELLClient *pClient)
{
    EasyTcpServer::OnNetLeave(pClient);
    if(pClient->isLogined()) _lgManager.removeUser((CUser*)pClient->getUser());
}

void CServer::OnNetError(CELLClient *pClient)
{
    EasyTcpServer::OnNetLeave(pClient);
    //断线时应当给一定时间让用户进行重新连接，以免用户数据丢失（用在游戏时）
}

void CServer::OnNetMsg(CELLServer *pServer, CELLClient *pClient, netmsg_DataHeader *header)
{
    EasyTcpServer::OnNetMsg(pServer,pClient,header);
    auto cmd=header->cmd;
    if(cmd!=CMD_LOGIN&&!pClient->isLogined()) return;
    switch(cmd){
        case CMD_LOGIN:
        {
            netmsg_Login* msgLg=(netmsg_Login*)header;
            CUser* user=_lgManager.doLogin(pClient,msgLg->userName,msgLg->PassWord);
          //  if(user) init(user);//这边一弄，DOLOGIN里面的发送登录结果的消息客户端就收不到，不知道 为什么。
        }
        break;
        case CMD_C2S_HEART:
        {

            pClient->resetDTHeart();
            netmsg_s2c_Heart ret;
            pClient->SendData(&ret);
        }
        break;
    case CMD_INIT:
    {
        init(pClient);
    }
        break;
    case CMD_JSON_CMD:
    {
        QJsonObject js=CELLReadStream(header).getJsonData();
        qDebug()<<pClient<<js;
        onJsonCMD(pClient,js);
    }
        break;
    default:
        CELLLog::Info("recv <ip=%s> undefine msgType,dataLen：%d\n", pClient->IP(), header->dataLength);
    }
}

void CServer::onJsonCMD(CELLClient *pClient, QJsonObject &json)
{
    int cmd=json["cmd"].toInt();
    switch (cmd) {
        case JC_DO_SQL:
    {
        //这里需要增加判断指令操作权限
//        qDebug()<<"SQL_JSON"<<json;
        QSqlCmd sqlCmd(json);
        QSqlReturnMsg slqR=CDatabaseManage::Instance().doQuery(sqlCmd);
        pClient->SendData(slqR.jsCmd());
    }
        break;
    case JC_WORKFLOW:
    {
//        _wfManager.onJsonCmd(json, pClient->getUser()->name())

/*        NewWorkFlowCMD wfCmd(json);

           // wfCmd.setOperator(pClient->getUser()->id());
            createWorkFlow(wfCmd,pClient)*/;


    }
        break;
//    case JC_QUERY_RM:
//    {
//         bool r=CDatabaseManage::Instance().queryRM(json);
//         if(r) json["result"]=true;
//         else{
//             json["result"]=false;
//             json["reason"]=CDatabaseManage::Instance().lastError();
//         }
//         pClient->SendData(json);
//    }
//        break;
    default:
        qDebug()<<"收到未知的JSON数据。"<<json;
        break;
    }
}

void CServer::init( CELLClient *pClient)
{
    //通知待办流程：
    QSqlQuery query(CDatabaseManage::Instance().database());
    if(!query.exec(QString("select id from wf_record where operator_id='%1' and operate_time is null;").arg(pClient->getUser()->id()))){
        qDebug()<<"查询待办流程记录时错误："<<query.lastError().text();
        return ;
    }
    while(query.next()){
        noticeToDo(query.value(0).toInt(), pClient);
    }
}

void CServer::createWorkFlow(const NewWorkFlowCMD &jsCMD, CELLClient *pClient)
{
    QString sql;
    QString processName=jsCMD.processName();
    QJsonObject contentOb=jsCMD.content();
    QString content=QString(QJsonDocument(contentOb).toJson(QJsonDocument::Compact));//将JSON内容转为STRING内容，保存于数据库

        QSqlQuery query(CDatabaseManage::Instance().database());
        if(!query.exec(QString("select process_id from wf_process_info where process_name='%1';").arg(processName))){
            qDebug()<<query.lastError().text();
            return ;
        }
        if(!query.next()){
            qDebug()<<"Database error: cannot find process : "<<processName;
            return ;
        }
        int processID=query.value(0).toInt();
        if(!query.exec("select instance_id from wf_instance")){
            qDebug()<<query.lastError().text();
            return ;
        }

        int instanceID;
        QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss" );
        sql=QString("insert into wf_instance(process_id,creator_id,create_time, content) values(%1,%2,now(),'%3')")
                .arg(processID).arg(pClient->getUser()->id()).arg(content);
        if(!query.exec(sql)){//新建流程
            qDebug()<<"无法创建流程："<<query.lastError().text();
            return ;
        }

        if(!query.exec("select LAST_INSERT_ID();")){//查询插入的流程id;
            qDebug()<<"查询插入的流程id时出错："<<query.lastError().text();
            return ;
        }
        query.next();
        instanceID=query.value(0).toInt();

        sql=QString("select node_id from wf_process_info where process_id = %1").arg(processID);
        if(!query.exec(sql)){//找到首节点ID，在流程信息表中的要定义
            qDebug()<<"查找流程首节点ID时错误："<<query.lastError().text();
            return ;
        }
        if(!query.next()){
            qDebug()<<"error, 未找到符合条件的首节点ID";
            return ;
        }
        int nodeID=query.value(0).toInt();
        int operatorID;
        sql=QString("select operator_id from wf_process_node where node_id=%1").arg(nodeID);
        if(!query.exec(sql)){
            qDebug()<<"查询节点操作人时错误："<<query.lastError().text();
            return ;
        }
        if(!query.next()){
            qDebug()<<"未找到符合要求的节点ID";
            return ;
        }
        operatorID=query.value(0).toInt();
        sql=QString("insert into wf_record(node_id,instance_id,operator_id) values(%1,%2,%3);").arg(nodeID).arg(instanceID).arg(operatorID);
        if(!query.exec(sql)){
            qDebug()<<"创建操作记录时错误："<<query.lastError().text();
            return ;
        }

        if(!query.exec("select LAST_INSERT_ID();")){//查询插入的操作记录id;
            qDebug()<<"查询插入的操作记录id时出错："<<query.lastError().text();
            return ;
        }
        query.next();
        int recordID=query.value(0).toInt();
        //表处理完毕，此处需要通知操作人员进行流程处理。（人员有在线时才通知）
        CUser* user=_lgManager.findUser(operatorID);
        if(user){
            noticeToDo(recordID,user->client());
        }

        //return true;
}

void CServer::noticeToDo(int recordID,CELLClient *pClient)
{
    QSqlQuery query(CDatabaseManage::Instance().database());
    if(!query.exec(QString("select instance_id, node_id from wf_record where id=%1;").arg(recordID))){
        qDebug()<<"查询操作记录时错误："<<query.lastError().text();
        return ;
    }
    if(!query.next()){
        qDebug()<<"操作记录记录为空！";
        return;
    }
    int instanceID=query.value(0).toInt();
    int nodeID=query.value(1).toInt();

    if(!query.exec(QString("select node_name from wf_process_node where node_id=%1;").arg(nodeID))){
        qDebug()<<"查询节点名称时错误："<<query.lastError().text();
        return ;
    }
    if(!query.next()){
        qDebug()<<"节点记录为空！";
        return ;
    }
    QString nodeName=query.value(0).toString();

    if(!query.exec(QString("select creator_id,create_time,process_id from wf_instance where instance_id=%1;").arg(instanceID))){
        qDebug()<<"查询流程实例时错误："<<query.lastError().text();
        return ;
    }
    if(!query.next()){
        qDebug()<<"流程实例为空！";
        return ;
    }
    int creatorID=query.value(0).toInt();
    QString createTime=query.value(1).toString();
    int processID=query.value(2).toInt();

    if(!query.exec(QString("select process_name,widget_name from wf_process_info where process_id=%1;").arg(processID))){
        qDebug()<<"查询流程名称时错误："<<query.lastError().text();
        return ;
    }
    if(!query.next()){
        qDebug()<<"流程为空！";
        return ;
    }
    QString processName=query.value(0).toString();
    QString tabText=query.value(1).toString();

    if(!query.exec(QString("select name from org_employee where id=%1;").arg(creatorID))){
        qDebug()<<"查询创建人名称时错误："<<query.lastError().text();
        return ;
    }
    if(!query.next()){
        qDebug()<<"创建人为空！";
        return ;
    }
    QString creatorName=query.value(0).toString();

    ProcessNoticeCMD notice(tabText,recordID,processName,creatorName,nodeName,createTime);
    if(SOCKET_ERROR==pClient->SendData(notice.data())){
        qDebug()<<"error on noticeTodo: 发送数据时错误.";
    }
   // qDebug()<<notice.data();
}
