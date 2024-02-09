#include "tabwigetbase.h"
#include "mytableview.h"
#include "qeventloop.h"
#include "qthread.h"
#include <QLabel>
#include<QDialog>
#include<QTimer>
TabWidgetBase::TabWidgetBase(QWidget *parent) : QWidget(parent),flag(0)
{
    m_view=new MyTableView;

    m_view->setHeader({"流程名称","审批结果","审批意见","审批人","审批时间"});
    m_view->resize(800,400);
}

TabWidgetBase::~TabWidgetBase()
{
    if(m_view) delete m_view;
}

void TabWidgetBase::onSqlReturn(const QSqlReturnMsg &jsCmd)
{    
    int flag=jsCmd.flag();
    qDebug()<<QString("收到sql处理结果，处理函数ID：%1").arg(flag);
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
    qDebug()<<"TabWidgetBase::dealProcess";
}

bool TabWidgetBase::pushProcess(QFlowInfo flowInfo, bool passed,const QString& comments)
{
    int flowID=flowInfo.flowID();
    qDebug()<<flowInfo.object();

    int node=flowInfo.node();
    int backNode=flowInfo.backNode();
    int nextNode=flowInfo.nextNode();
    //考虑到有多人审核的情况，先检查下是否已经被其它人处理
    //目前先不处理需要多人共同审批的情况
    QString sql;
    sql="select * from flow_records where id=? and status=0;";//确认这条流程还未完成审批
    bool ok=false;
    QJsonArray values;
    values.append(flowID);
    doSqlQuery(sql,[this,&ok](const QSqlReturnMsg&msg){
        if(msg.error()){
            return notifySqlError("查询流程审核记录出错",msg.result().toString());
        }
        ok=msg.result().toList().count()==2;
        qDebug()<<msg.result();
//        emit sqlFinished();
        sqlEnd();
    },0,values);
    waitForSql();
    if(!ok){
        QMessageBox::information(nullptr,"","流程已经被其它人处理完成。");
//            removeTodo(row);
        return false;
    }
//    if(passed){
//        QString sql="update flow_records set operatorCountPassed=operatorCountPassed+1 where id=? and status=0;";//通过人数+1；0为待审核状态，如果没有，说明已经审核完成
        sql="update flow_records set status=? where id=? ;";//状态1为通过，2为驳回（目前不处理需要多人共同审批的流程）

    values={passed?1:2,flowID};
        ok=false;
        doSqlQuery(sql,[this,&ok](const QSqlReturnMsg&msg){
            if(msg.error()){
                return notifySqlError("更新流程审核记录出错",msg.result().toString());
            }
            ok=msg.numRowsAffected();
            qDebug()<<msg.result();
            sqlEnd();
        },0,values);
        waitForSql();
        if(!ok) {
            QMessageBox::information(nullptr,"","更新流程审核记录出错，0条修改成功");
            return false;
        }

      //更新操作记录
        sql="update flow_operate_records set operateStatus=? ,operateComments=?, operateTime=NOW() where operatorID=(select id from sys_employee_login where name=?) and flowID=?;";
        values={passed?1:2,comments,user()->name(),flowID};

            doSqlQuery(sql, [this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新操作记录出错：",msg.result().toString());
                    sqlFinished();
                    return;
                }
                sqlFinished();
            },0,values);
        waitForSql();
            //流程处理完成
//            emit dealFLow(flowInfo,AGREE);//当前节点通过，发出信号，由各自模块处理下一步流程
            //通知发起人审批结果

            //对于多人审批的，检查流程是否审批完成，如果完成，则取消其它人的审批。(直接删除数据）
            sql="delete from flow_operate_records  where flowID=? and operateStatus=0 and (select status from flow_records where id=?)!=0; ";
            values={flowID,flowID};
            doSqlQuery(sql, [this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"取消其它审批人时出错：",msg.result().toString());
                    sqlFinished();
                    return;
                }

                sqlFinished();
            },0,values);

        waitForSql();

        //更新待办
//        removeTodo(row);
        //若其它人的审批被取消，更新其它人的待办（这个不操作了，麻烦）
        emit processOk(passed);//通知主窗口更新待办
        return true;


}

void TabWidgetBase::showFlowInfo(const QSqlReturnMsg &flowIDsQueryMsg)
{
        if(flowIDsQueryMsg.error()){
            QMessageBox::information(nullptr,"查询流程信息时出错：",flowIDsQueryMsg.errorMsg());
            return;
        }
        QList<QVariant>r=flowIDsQueryMsg.result().toList();
        if(r.count()<2){
            QMessageBox::information(nullptr,"error：","没有可查询的操作流程。");
            return;
        }
        QString sql;

        bool ok=false;
        m_view->clear();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            int flowID=row.at(0).toInt();
            if(!flowID){
                QMessageBox::information(nullptr,"error:","错误的流程ID信息。");
                qDebug()<<row;
                break;
            }
            sql="SELECT JSON_EXTRACT(flowInfo, '$.flowName'), CASE WHEN A.operateStatus = 1 THEN '通过' WHEN A.operateStatus = 2 THEN '驳回' ELSE '审批中' END AS '审批结果', A.operateComments, sys_employee_login.name, DATE_FORMAT(A.operateTime,'%Y/%m/%d %H:%i') FROM "
                  "(select * from (select flowID,operateStatus,operateComments, operatorID, operateTime from flow_operate_records where flowID=?) as T left join flow_records on T.flowID= flow_records.id) as A left join sys_employee_login on A.operatorID=sys_employee_login.id; ";

            doSqlQuery(sql,[this, &ok](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"获取审批信息时出错:",msg.errorMsg());
                    sqlFinished();
                    return;
                }
                QList<QVariant>r=msg.result().toList();
//                if(r.count()<2){
//                    QMessageBox::information(nullptr,"error:","没有相关流程信息。");
//                    sqlFinished();
//                    return;
//                }
                for(int i=1;i<r.count();i++){
                    auto row=r.at(i);
                    m_view->append(row.toList());
                }
                ok=true;
                sqlFinished();
            },1,{flowID});
            waitForSql();
        }
        m_view->show();
}

void TabWidgetBase::initMod()
{

}


void TabWidgetBase::doSqlQuery(const QString &sql, DealFuc f, int page,const QJsonArray&bindValues)
{
//    static int flag=0;
        qDebug()<<"tab:"<<this;
    QSqlCmd cmd(sql,flag,page);
    if(bindValues.count()){
        cmd.bindValue(bindValues);
    }
    QString sqlinfo=sql;
    int p=sqlinfo.indexOf("?");
    int i=0;
    while(p>0){
        sqlinfo.replace(p,1,bindValues.at(i).toVariant().toString());
        p=sqlinfo.indexOf("?");
        i++;
    }
    qDebug()<<QString("发送sql请求：sql=%1,处理函数ID：%2").arg(sqlinfo).arg(flag);

    m_fucMap.insert(flag,f);//标识下处理结果返回的函数
    flag++;

    emit sendData(cmd.jsCmd()); // 发射发送数据信号



}
//流程操作：提交到流程
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
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"新建流程出错：",msg.result().toString());
//            emit sqlFinished();
            sqlEnd();
            return;
        }
//        emit sqlFinished();
        sqlEnd();
    },0,values);
    waitForSql();

    sql="select @flowID;";

    doSqlQuery(sql,[this, &ret](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"获取流程ID出错：",msg.result().toString());
//            emit sqlFinished();
            sqlEnd();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()!=2){
            QMessageBox::information(nullptr,"获取流程ID出错：","无法获取ID。");
//            emit sqlFinished();
            sqlEnd();
            return;
        }
        ret=r.at(1).toList().first().toInt();
//        emit sqlFinished();
        sqlEnd();
    });
    waitForSql();
    //通知操作人员待办消息

    return ret;//传回所建流程ID，让发送者进行下一步处理
}

void TabWidgetBase::waitForSql(const QString &msg)
{
    m_dlg.setMsg(msg);
//    m_dlg.exec();
    m_dlg.wait();

}

void TabWidgetBase::sqlEnd()
{
//    m_dlg.accept();
    m_dlg.end();
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
//    m_tabWiget->sqlFinished();
    m_tabWiget->sqlEnd();
}

void SqlBaseClass::waitForSql(const QString &msg)
{
    m_tabWiget->waitForSql(msg);
}


WaitDlg::WaitDlg(QWidget *parent): QDialog(parent),m_execFlag(0)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(200,20);
    m_msg=new QLabel("数据处理中……",this);
}
