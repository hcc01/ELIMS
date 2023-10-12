#include "tabwigetbase.h"
#include<QDialog>
#include<QTimer>
TabWidgetBase::TabWidgetBase(QWidget *parent) : QWidget(parent)
{

}

void TabWidgetBase::onSqlReturn(const QSqlReturnMsg &jsCmd)
{

    int flag=jsCmd.flag();
    qDebug()<<jsCmd.jsCmd();
    qDebug()<<"flag"<<flag;
    auto func = m_fucMap.value(flag);
    m_fucMap.remove(flag);//使用完后直接删除，避免越来越多
    if (func) {
        func(jsCmd);
    }
    else {
        qDebug() << "error: empty function object for flag" << flag;
    }

}

void TabWidgetBase::dealProcess(const ProcessNoticeCMD &)
{

}


void TabWidgetBase::doSqlQuery(const QString &sql, DealFuc f, int page,const QJsonArray&bindValues)
{
    static int flag=0;
    QSqlCmd cmd(sql,flag,page);
    if(bindValues.count()){
        cmd.bindValue(bindValues);
    }
    m_fucMap.insert(flag,f);//标识下处理结果返回的函数
    flag++;

    emit sendData(cmd.jsCmd()); // 发射发送数据信号



}


SqlBaseClass::SqlBaseClass(TabWidgetBase *tab):
    m_tabWiget(tab)
{

}

void SqlBaseClass::doSql(const QString &sql, DealFuc f, int p, const QJsonArray &values)
{
    m_tabWiget->doSqlQuery(sql,f,p,values);
}

