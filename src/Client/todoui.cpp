#include "todoui.h"
#include "ui_todoui.h"
#include<QInputDialog>
#include<QMessageBox>
ToDoUI::ToDoUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::ToDoUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"流程名称","发起人","发起日期"});
    ui->tableView->addContextAction("刷新",[this](){
        initCMD();
    });
    ui->tableView->addContextAction("操作",[this](){
       if(!ui->tableView->currentIndex().isValid()) return;

       emit dealFLow(m_flowInfos.at(ui->tableView->currentIndex().row()),VIEWINFO);//发出操作信号，让各自模块去处理流程
    });

//    ui->tableView->addContextAction("同意",[this](){
//        if(!ui->tableView->currentIndex().isValid()) return;
//        QString comment=QInputDialog::getText(nullptr,"请输入意见：","");
//        int row=ui->tableView->currentIndex().row();
//        QFlowInfo flowInfo=m_flowInfos.at(row);
////        flowInfo.setComment(comment);
//        int flowID=m_flowIDs.at(row);
//        flowInfo.setFlowID(flowID);
//        QString sql;
//        QJsonArray values;
//        //更新流程审批记录，需要判断是否流程已经被审批
//        sql="update flow_records set operatorCountPassed=operatorCountPassed+1 where id=? and status=0;";//通过人数+1；0为待审核状态，如果没有，说明已经审核完成
//        values.append(flowID);
//        bool ok=false;
//        doSqlQuery(sql,[this,&ok](const QSqlReturnMsg&msg){
//            if(msg.error()){
//                return notifySqlError("更新流程审核记录出错",msg.result().toString());
//            }
//            ok=msg.numRowsAffected();
//            qDebug()<<msg.result();
//            emit sqlFinished();
//        },0,values);
//        waitForSql();
//        if(!ok){
//            QMessageBox::information(nullptr,"","流程已经被其它人处理完成。");
//            removeTodo(row);
//            return;
//        }
//        //检查是否全部通过
//        ok=false;
//        sql="update flow_records set status=1 where id=? and operatorCountPassed=operatorCountNeeded;";
//        doSqlQuery(sql,[this,&ok](const QSqlReturnMsg&msg){
//            if(msg.error()){
//                return notifySqlError("更新流程审核记录出错",msg.result().toString());
//            }
//            ok=msg.numRowsAffected();
//            qDebug()<<msg.result();
//            emit sqlFinished();
//        },0,values);
//        waitForSql();
//        if(ok){
//            //流程处理完成
//            emit dealFLow(flowInfo,AGREE);//当前节点通过，发出信号，由各自模块处理下一步流程
//            //通知发起人审批结果

//            //对于多人审批的，检查流程是否审批完成，如果完成，则取消其它人的审批。(直接删除数据）
//            sql="delete from flow_operate_records  where flowID=? and operateStatus=0 and (select status from flow_records where id=?)=1; ";
//            values={flowID,flowID};
//            doSqlQuery(sql, [](const QSqlReturnMsg&msg){
//                if(msg.error()){
//                    QMessageBox::information(nullptr,"取消其它审批人时出错：",msg.result().toString());
//                    return;
//                }

//            },0,values);

//        }
//      //更新操作记录
//        sql="update flow_operate_records set operateStatus=1 ,operateComments=?, operateTime=NOW() where operateStatus=0 and operatorID=(select id from sys_employee_login where name=?);";
//        values={comment,user()->name()};


//        //更新待办
//        removeTodo(row);
//        //若其它人的审批被取消，更新其它人的待办（这个不操作了，麻烦）




//    });
//    ui->tableView->addContextAction("驳回",[this](){
//        if(!ui->tableView->currentIndex().isValid()) return;
//        emit dealFLow(m_flowInfos.at(ui->tableView->currentIndex().row()),REJECT);
//    });
}

ToDoUI::~ToDoUI()
{
    delete ui;
}

void ToDoUI::initCMD()
{
    ui->tableView->clear();
    m_flowIDs.clear();
    m_flowInfos.clear();
    QString sql=QString("select creator, createTime, flowInfo, flowID from ("
                          "select * from flow_operate_records where operateStatus=0 and operatorID=(select id from sys_employee_login where name='%1'))"
                          " as A left join flow_records as B on A.flowID=B.id where status=0;").arg(user()->name());
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询待办事项时出错：",msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            QList<QVariant>l=r.at(i).toList();
            QFlowInfo flowInfo(l.at(2).toString());
            ui->tableView->append({flowInfo.flowName(),l.at(0).toString(),l.at(1).toDateTime().toString("MM月dd日hh时mm分")});
            flowInfo.setFlowID(l.at(3).toInt());
            m_flowInfos.append(flowInfo);//保存流程信息
            m_flowIDs.append(l.at(3).toInt());//保存流程ID
        }
        },1);
}

void ToDoUI::removeTodo(int row)
{
    ui->tableView->deleteRow(row);
    m_flowInfos.removeAt(row);
    m_flowIDs.removeAt(row);
}

void ToDoUI::loadUser(CUser *user)
{
    QString sql="select phone from users where name=?";
    doSqlQuery(sql,[user](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"载入用户信息时出错：",msg.errorMsg());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        user->setPhone(r.at(1).toList().at(0).toString());
    },0,{user->name()});
}

void ToDoUI::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    emit dealFLow(m_flowInfos.at(ui->tableView->currentIndex().row()),VIEWINFO);//发出操作信号，让各自模块去处理流程
}

