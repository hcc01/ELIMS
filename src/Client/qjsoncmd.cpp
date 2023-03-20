#include "qjsoncmd.h"
QJsonCmd::QJsonCmd()
{

}

QSqlCmd::QSqlCmd(const QString &sql, int flag, const QString &tytle, int queryPage)
{
    _cmd["cmd"]=JC_DO_SQL;
    _cmd["sql"]=sql;
    _cmd["flag"]=flag;
    _cmd["tytle"]=tytle;
    _cmd["queryPage"]=queryPage;
}

QSqlCmd::QSqlCmd(const QJsonObject &json):
    _cmd(json)
{

}

QSqlCmd::QSqlCmd(const QSqlCmd &sqlCmd)
{
    _cmd=sqlCmd._cmd;
}

QString QSqlCmd::tytle() const
{
    return _cmd.value("tytle").toString();
}

QString QSqlCmd::sql() const
{
    return _cmd.value("sql").toString();
}

int QSqlCmd::flag() const
{
    return _cmd.value("flag").toInt();
}

int QSqlCmd::queryPage() const
{
    return _cmd.value("queryPage").toInt();;
}

QJsonObject QSqlCmd::jsCmd() const
{
    return _cmd;
}

QSqlReturnMsg::QSqlReturnMsg(const QVariant &result, int flag, const QString &tytle, bool error, int currentPage, int totalPage)
{
    _cmd["cmd"]=JC_DO_SQL;
    _cmd["result"]=result.toJsonValue();
    _cmd["flag"]=flag;
    _cmd["tytle"]=tytle;
    _cmd["currentPage"]=currentPage;
    _cmd["totalPage"]=totalPage;
    _cmd["error"]=error;
}

QSqlReturnMsg::QSqlReturnMsg(const QJsonObject &json)
{
    _cmd=json;
}

QVariant QSqlReturnMsg::result() const
{
    return _cmd.value("result").toVariant();
}

QString QSqlReturnMsg::tytle() const
{
    return _cmd.value("tytle").toString();
}

int QSqlReturnMsg::flag() const
{
    return _cmd.value("flag").toInt();
}

bool QSqlReturnMsg::error() const
{
    return  _cmd.value("error").toBool();
}

int QSqlReturnMsg::currentPage() const
{
    return _cmd.value("currentPage").toInt();
}

int QSqlReturnMsg::totalPage() const
{
    return _cmd.value("totalPage").toInt();
}

QJsonObject QSqlReturnMsg::jsCmd() const
{
    return _cmd;
}

NewWorkFlowCMD::NewWorkFlowCMD(const QString &processName, const QJsonObject &content)
{
    _cmd["cmd"]=JC_WORKFLOW;
    _cmd["type"]=WF_NEW_PROCESS;
    _cmd["content"]=content;//流程内容，与具体流程相关
    _cmd["processName"]=processName;//
}
