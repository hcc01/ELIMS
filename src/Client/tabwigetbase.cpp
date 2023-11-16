#include "tabwigetbase.h"
#include "qeventloop.h"
#include <QLabel>
#include<QDialog>
#include<QTimer>
TabWidgetBase::TabWidgetBase(QWidget *parent) : QWidget(parent)
{
    qDebug()<<m_tabName;
    connect(this,&TabWidgetBase::sqlFinished,&m_dlg,&QDialog::accept);
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

void TabWidgetBase::dealProcess(const QFlowInfo &flowInfo,int operateFlag)
{
}

void TabWidgetBase::initMod()
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

int TabWidgetBase::submitFlow(const QFlowInfo &flowInfo, QList<int> operatorIDs, int operatorCount)
{
    int ret=0;
    QString sql;
    sql="insert into flow_records(creator, flowInfo, operatorCountNeeded, createTime) values(?,?,?,now());set @flowID=LAST_INSERT_ID();";
    QJsonArray values;

    values={user()->name(),flowInfo.flowInfo(),operatorCount};

    for(int id:operatorIDs){
        sql+="insert into flow_operate_records(flowID,operatorID) values(@flowID,?);";
        values.append(id);
    }
    QEventLoop loop;
    connect(this,&TabWidgetBase::sqlFinished,&loop,&QEventLoop::quit);
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"新建流程出错：",msg.result().toString());
            emit sqlFinished();
            return;
        }
        emit sqlFinished();
    },0,values);
    loop.exec();

    sql="select @flowID;";

    doSqlQuery(sql,[this, &ret](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"获取流程ID出错：",msg.result().toString());
            emit sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()!=2){
            QMessageBox::information(nullptr,"获取流程ID出错：","无法获取ID。");
            emit sqlFinished();
            return;
        }
        ret=r.at(1).toList().first().toInt();
        emit sqlFinished();
    });
    loop.exec();
    //通知操作人员待办消息

    return ret;//传回所建流程ID，让发送者进行下一步处理
}

void TabWidgetBase::waitForSql(const QString &msg)
{
    m_dlg.setMsg(msg);
    m_dlg.exec();
}


SqlBaseClass::SqlBaseClass(TabWidgetBase *tab):
    m_tabWiget(tab)
{

}

void SqlBaseClass::doSql(const QString &sql, DealFuc f, int p, const QJsonArray &values)
{
    m_tabWiget->doSqlQuery(sql,f,p,values);
}

void SqlBaseClass::sqlFinished()
{
    m_tabWiget->sqlFinished();
}

void SqlBaseClass::waitForSql(const QString &msg)
{
    m_tabWiget->waitForSql(msg);
}


WaitDlg::WaitDlg(QWidget *parent): QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(200,20);
    m_msg=new QLabel("数据处理中……",this);
}
