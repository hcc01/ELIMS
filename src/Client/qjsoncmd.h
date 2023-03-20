#ifndef QJSONCMD_H
#define QJSONCMD_H
#include<QVariant>
#include<QJsonObject>
#include<QJsonArray>

#include"../../depends/MessageHeader.h"
class QJsonCmd
{
public:
    QJsonCmd();
    virtual ~QJsonCmd(){}
    virtual QJsonObject  jsCmd()const{return _jsCmd;}
private:
    QJsonObject _jsCmd;

};

class WorkFlowCmd{
    virtual ~WorkFlowCmd(){}
    int type(){ return _jsCmd["type"].toInt(); }
private:
    QJsonObject _jsCmd;
};

class NoticeCMD{//返回的消息
public:
    NoticeCMD(){
        _data["cmd"]=JC_NOTICE;
    }
    NoticeCMD(const QJsonObject& data){
        _data=data;
    }
    int type()const{
        return _data.value("type").toInt();
    }
    QJsonObject data()const{
        return _data;
    }

protected:
    QJsonObject _data;
};

class ProcessNoticeCMD:public NoticeCMD{//流程处理通知(待办流程)
public:
    ProcessNoticeCMD(){
        _data["type"]=NT_WORKFLOW;
    }
    ProcessNoticeCMD(const QJsonObject& data){
        _data=data;
    }
    ProcessNoticeCMD(QString tabText, int recordID, const QString& processName, QString creatorName, QString nodeName, QString createTime){
        _data["recordID"]=recordID;
        _data["processName"]=processName;
        _data["creatorName"]=creatorName;
        _data["nodeName"]=nodeName;
        _data["createTime"]=createTime;
        _data["type"]=NT_WORKFLOW;
        _data["tabText"]=tabText;
    }
    QString ProcessName()const{                         //流程名称，用于显示
        return _data.value("processName").toString();
    }

    QString createName()const{                          //流程发起人，用于显示
        return _data.value("creatorName").toString();
    }

    QString nodeName()const{                            //操作节点名称，用于显示
        return _data.value("nodeName").toString();
    }

    QString createTime()const{                          //创建时间，用于显示
        return _data.value("createTime").toString();
    }
    int recordID()const{                                  //节点标识，用于查询
        return _data.value("recordID").toInt();
    }
    QString tabText()const{                          //标记操作窗口
        return _data.value("tabText").toString();
    }

    void setProcess(QString name){
        _data["processName"]=name;
    }
    void setCreator(QString name){
        _data["creatorName"]=name;
    }
    void setOperatorID(int id){
        _data["operator_id"]=id;
    }
    void setNodeName(QString name){
        _data["nodeName"]=name;
    }
    void setrecordID(int id){
        _data["recordID"]=id;
    }
    void setCreateTime(QString time){
        _data["createTime"]=time;
    }

    bool isNothing(){
        return _data.value("recordID").toBool();
    }
};

class NewWorkFlowCMD{//新建流程
public:
    NewWorkFlowCMD(const QString& processName, const QJsonObject& content);

    NewWorkFlowCMD(const QJsonObject& cmd):_cmd(cmd){

    }
//    void setOperator(int id){
//        _cmd["opereator_id"]=id;
//    }
    QJsonObject content()const{ //流程的具体内容以JSON保存
        return _cmd.value("content").toObject();
    }

    QJsonObject data()const{
        return _cmd;
    }
    QString processName()const{
        return _cmd.value("processName").toString();
    }
//    QString creatorName(){
//         return _cmd.value("creatorName").toString();
//    }
//    int operatorID()const{
//        return _cmd.value("operator_id").toInt();
//    }
private:
    QJsonObject _cmd;
};


class QSqlCmd{//数据库查询的sql命令包
public:
    QSqlCmd(const QString &sql/*操作语句*/, int flag/*操作类型标识，模块自定义*/, const QString& tytle/*模块窗体标识*/,int queryPage=0);
    QSqlCmd(const QJsonObject& json);
    QSqlCmd(const QSqlCmd &sqlCmd);   
    void setSql(const QString& sql){ _cmd["sql"]=sql;}
    void setFlag(int flag){ _cmd["flag"]=flag;}
    void setPage(int page){_cmd["queryPage"]=page;}
    QString tytle()const;
    QString sql() const;
    int flag() const;
    int queryPage()const;
    QJsonObject jsCmd() const;
private:
    QJsonObject _cmd;
};

class QSqlReturnMsg{//数据库查询的返回信息包
public:
    QSqlReturnMsg(const QVariant &result/*操作语句*/, int flag/*操作类型标识，模块自定义*/, const QString& tytle/*模块窗体标识*/, bool error, int currentPage=0, int totalPage=1);
    QSqlReturnMsg(const QJsonObject& json);
    QVariant result()const;
    QString tytle() const;
    int flag() const;
    bool error() const;
    int currentPage()const;
    int totalPage()const;
    QJsonObject jsCmd() const;
private:
    QJsonObject _cmd;
};


#endif // QJSONCMD_H
