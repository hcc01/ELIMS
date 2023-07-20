#include "tabwigetbase.h"
#include<QDialog>
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
//    qDebug()<<m_fucMap.value(flag).target_type().name();;
//    if(m_fucMap.value(flag)){
//       m_fucMap.value(flag) (jsCmd);
//    }
//    else{
//       qDebug()<<"error: wrong sqlFlag."<<flag;
    //    }
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
    emit sendData(cmd.jsCmd());
    m_fucMap.insert(flag,f);//标识下处理结果返回的函数
    flag++;
}

SqlBaseClass::SqlBaseClass(TabWidgetBase *tab):
    m_tabWiget(tab)
{

}

void SqlBaseClass::doSql(const QString &sql, DealFuc f, int p, const QJsonArray &values)
{
    m_tabWiget->doSqlQuery(sql,f,p,values);
}

