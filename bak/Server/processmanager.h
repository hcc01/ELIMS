#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H
//流程管理，用来处理流程逻辑（结合客户端的流程操作）
//服务端的流程管理内容：
//1. 处理流程数据库
//2. 通知流程操作
#include <QObject>
#include"../Client/qjsoncmd.h"
class ProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit ProcessManager(QObject *parent = nullptr);
    void onProcessCMD();//处理流程相关指令
    void dealProcess(int instanceID,int nodeID,bool pass,const QString& remark);//推进流程
    void newProcess();//新建流程
    void noticeToDo();//通知待办流程
    void onJsonCmd(const QJsonCmd& cmd);
signals:

};

#endif // PROCESSMANAGER_H
