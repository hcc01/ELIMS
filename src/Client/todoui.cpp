#include "todoui.h"
#include "qpushbutton.h"
#include "qsqlquery.h"
#include "qtextedit.h"
#include "ui_todoui.h"
#include<QInputDialog>
#include<QMessageBox>
#include<mainwindow.h>
#include"dbmater.h"
ToDoUI::ToDoUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::ToDoUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"流程名称","流程摘要","发起人","发起日期"});
    ui->tableView->addContextAction("刷新",[this](){
        initCMD();
    });

}

ToDoUI::~ToDoUI()
{
    delete ui;
}

void ToDoUI::initCMD()
{

    QString sql=QString("select creator, createTime, flowInfo, flowID from ("
                          "select * from flow_operate_records where operateStatus=0 and operatorID=(select id from sys_employee_login where name='%1'))"
                          " as A left join flow_records as B on A.flowID=B.id where status=0;").arg(user()->name());
    ui->pageCtrl->startSql(this,sql,1,{},[this](const QSqlReturnMsg&msg){
//        if(msg.error()){
//            QMessageBox::information(nullptr,"查询待办事项时出错：",msg.result().toString());
//            return;
//        }
        ui->tableView->clear();
        m_flowIDs.clear();
        m_flowInfos.clear();
        QList<QVariant>r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            QList<QVariant>l=r.at(i).toList();
            QFlowInfo flowInfo(l.at(2).toString());
            ui->tableView->append({flowInfo.flowName(),flowInfo.flowAbs(),l.at(0).toString(),l.at(1).toDateTime().toString("MM月dd日hh时mm分")});
            flowInfo.setFlowID(l.at(3).toInt());
            m_flowInfos.append(flowInfo);//保存流程信息
            m_flowIDs.append(l.at(3).toInt());//保存流程ID
        }
        });
}

void ToDoUI::removeTodo(int row)
{
    ui->tableView->deleteRow(row);
    m_flowInfos.removeAt(row);
    m_flowIDs.removeAt(row);
}

void ToDoUI::loadUser(CUser *user, MainWindow *main)
{
    m_main=main;
    QString sql="select phone, B.id from sys_employee_login as B left join  users on users.name=B.name where B.name=?";
    doSqlQuery(sql,[user](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"载入用户信息时出错：",msg.errorMsg());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        qDebug()<<r;
        if(r.count()<2) return;
        user->setPhone(r.at(1).toList().at(0).toString());
        user->setID(r.at(1).toList().at(1).toInt());
    },0,{user->name()});
}

void ToDoUI::updateTypes()
{
    QString sql;
    QSqlQuery query(DB.database());
    if(!query.exec("create table if not exists test_field (id INTEGER PRIMARY KEY,testField text)")){
        QMessageBox::information(nullptr,"无法创建检测领域表",query.lastError().text());
        return;
    }    if(!query.exec("create table if not exists test_type (id INTEGER PRIMARY KEY,testFieldID INTEGER, testType text)")){
        QMessageBox::information(nullptr,"无法创建检测类型表",query.lastError().text());
        return;
    }
    if(!query.exec("delete from test_type")){
        QMessageBox::information(nullptr,"删除检测类型时出错：",query.lastError().text());
        return;
    }
    if(!query.exec("delete from test_field")){
        QMessageBox::information(nullptr,"删除检测领域时出错：",query.lastError().text());
        return;
    }

    bool error=false;
    sql="select id, testField from test_field;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询检测领域表时出错",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QSqlQuery query(DB.database());
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            query.prepare("insert into test_field values(?,?);");
            query.addBindValue(row.at(0));
            query.addBindValue(row.at(1));
            if(!query.exec()){
                QMessageBox::information(nullptr,"更新检测领域时出错：",query.lastError().text());
                error=true;
                break;
            }
        }
        sqlFinished();
    });
    waitForSql();
    if(error) return;
    sql="select id, testFieldID, testType from test_type;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询检测领域表时出错",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QSqlQuery query(DB.database());
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            query.prepare("insert into test_type values(?,?,?);");
            query.addBindValue(row.at(0));
            query.addBindValue(row.at(1));
            query.addBindValue(row.at(2));
            if(!query.exec()){
                QMessageBox::information(nullptr,"更新检测领域时出错：",query.lastError().text());
                error=true;
                break;
            }
        }
        sqlFinished();
    });
    waitForSql();
    if(error) return;
    QMessageBox::information(nullptr,"","更新成功。");
}

void ToDoUI::updateParameters()
{
    QString sql;
    QSqlQuery query(DB.database());
    if(!query.exec("create table if not exists detection_parameters (id INTEGER PRIMARY KEY,testFieldID INTEGER, parameterName text, additive INTEGER)")){
        QMessageBox::information(nullptr,"无法创建检测参数表",query.lastError().text());
        return;
    }
    if(!query.exec("create table if not exists detection_parameter_alias (id INTEGER PRIMARY KEY,parameterID INTEGER, alias text)")){
        QMessageBox::information(nullptr,"无法创建参数别名表",query.lastError().text());
        return;
    }
    if(!query.exec("create table if not exists detection_subparameters (id INTEGER PRIMARY KEY,parameterID INTEGER, subName text, subparameterID INTEGER)")){
        QMessageBox::information(nullptr,"无法创建子参数表",query.lastError().text());
        return;
    }
    if(!query.exec("delete from detection_parameters")){
        QMessageBox::information(nullptr,"删除检测参数时出错：",query.lastError().text());
        return;
    }
    if(!query.exec("delete from detection_parameter_alias")){
        QMessageBox::information(nullptr,"删除参数别名时出错：",query.lastError().text());
        return;
    }
    if(!query.exec("delete from detection_subparameters")){
        QMessageBox::information(nullptr,"删除子参数时出错：",query.lastError().text());
        return;
    }

    bool error=false;
    sql="select id, testFieldID,parameterName, additive from detection_parameters;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询检测参数表时出错",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QSqlQuery query(DB.database());
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            query.prepare("insert into detection_parameters values(?,?,?,?);");
            query.addBindValue(row.at(0));
            query.addBindValue(row.at(1));
            query.addBindValue(row.at(2));
            query.addBindValue(row.at(3));
            if(!query.exec()){
                QMessageBox::information(nullptr,"更新检测领域时出错：",query.lastError().text());
                error=true;
                break;
            }
        }
        sqlFinished();
    });
    waitForSql();
    if(error) return;
    sql="select id ,parameterID , alias  from detection_parameter_alias;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询参数别名时出错",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QSqlQuery query(DB.database());
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            query.prepare("insert into detection_parameter_alias values(?,?,?);");
            query.addBindValue(row.at(0));
            query.addBindValue(row.at(1));
            query.addBindValue(row.at(2));
            if(!query.exec()){
                QMessageBox::information(nullptr,"更新参数别名时出错：",query.lastError().text());
                error=true;
                break;
            }
        }
        sqlFinished();
    });
    waitForSql();
    if(error) return;
    sql="select id ,parameterID , subName , subparameterID from detection_subparameters;";
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询子参数时出错",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QSqlQuery query(DB.database());
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            query.prepare("insert into detection_subparameters values(?,?,?,?);");
            query.addBindValue(row.at(0));
            query.addBindValue(row.at(1));
            query.addBindValue(row.at(2));
            query.addBindValue(row.at(3));
            if(!query.exec()){
                QMessageBox::information(nullptr,"更新子参数时出错：",query.lastError().text());
                error=true;
                break;
            }
        }
        sqlFinished();
    });
    waitForSql();
    if(error) return;
    QMessageBox::information(nullptr,"","更新成功。");
}

bool ToDoUI::pushProcess(QFlowInfo flowInfo, bool passed, const QString &comments)
{
    int flowID=flowInfo.flowID();
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
        removeTodo(ui->tableView->selectedRow());
        return false;
    }
//    if(passed){
//        QString sql="update flow_records set operatorCountPassed=operatorCountPassed+1 where id=? and status=0;";//通过人数+1；0为待审核状态，如果没有，说明已经审核完成

    connectDB(CMD_START_Transaction);
    sql="update flow_records set status=? where id=? and status=0;";//状态1为通过，2为驳回（目前不处理需要多人共同审批的流程）

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
        QMessageBox::information(nullptr,"","更新流程审核记录出错，流程已经被其它人审批");
        releaseDB(CMD_ROLLBACK_Transaction);
        return false;
    }

  //更新操作记录
    sql="update flow_operate_records set operateStatus=? ,operateComments=?, operateTime=NOW() where operatorID=(select id from sys_employee_login where name=?) and flowID=?;";
    values={passed?1:2,comments,user()->name(),flowID};

    doSqlQuery(sql, [this, &ok](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"更新操作记录出错：",msg.result().toString());
            ok=false;
            sqlFinished();
            return;
        }
        ok=true;
        sqlFinished();
    },0,values);
    waitForSql();
    if(!ok) {
        releaseDB(CMD_ROLLBACK_Transaction);
        return false;
    }
            //流程处理完成
//            emit dealFLow(flowInfo,AGREE);//当前节点通过，发出信号，由各自模块处理下一步流程
            //通知发起人审批结果

            //对于多人审批的，检查流程是否审批完成，如果完成，则取消其它人的审批。(直接删除数据）
    sql="delete from flow_operate_records  where flowID=? and operateStatus=0 and (select status from flow_records where id=?)!=0; ";
    values={flowID,flowID};
    doSqlQuery(sql, [this, &ok](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"取消其它审批人时出错：",msg.result().toString());
            ok=false;
            sqlFinished();
            return;
        }
        ok=true;
        sqlFinished();
    },0,values);
    waitForSql();
    if(!ok) {
        releaseDB(CMD_ROLLBACK_Transaction);
        return false;
    }
    releaseDB(CMD_COMMIT_Transaction);
        //更新待办
//        removeTodo(row);
        //若其它人的审批被取消，更新其它人的待办（这个不操作了，麻烦）
        removeTodo(ui->tableView->selectedRow());
        return true;
}

void ToDoUI::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
//    emit dealFLow(m_flowInfos.at(ui->tableView->currentIndex().row()),VIEWINFO);//发出操作信号，让各自模块去处理流程
    auto flowInfo=m_flowInfos.at(ui->tableView->currentIndex().row());
    qDebug()<<flowInfo.object();
    QDialog* dlg=new QDialog;
    dlg->resize(800,600);
    QVBoxLayout* vlay=new QVBoxLayout(dlg);
    TabWidgetBase*tab=m_main->getModule(flowInfo.tabName());
    if(!tab){
            QMessageBox::information(nullptr,"error：","无法打开模块。");
        return;
    }
    FlowWidget* w=tab->flowWidget(flowInfo);
    if(w) vlay->addWidget(w);
    QLabel* lab=new QLabel("审批意见：",dlg);
    vlay->addWidget(lab);
    QTextEdit* edit=new QTextEdit(dlg);
    vlay->addWidget(edit);
    dlg->setLayout(vlay);
    QHBoxLayout* hlay=new QHBoxLayout(dlg);
    QPushButton *agreeBtn=new QPushButton("同意",dlg);
    QPushButton *rejectBtn=new QPushButton("驳回",dlg);
    hlay->addWidget(agreeBtn);
    hlay->addWidget(rejectBtn);
    vlay->addLayout(hlay);
    connect(dlg,&QDialog::close,[dlg, w](){delete dlg;if(w) delete w;});
    connect(agreeBtn,&QPushButton::clicked,[this, edit, flowInfo, dlg, w](){
        if(edit->toPlainText().length()>254){
            QMessageBox::information(nullptr,"error","审批文本过长。");
            return;
        }
        if(pushProcess(flowInfo,true,edit->toPlainText())){
            if(w) w->pushProcess(flowInfo,true);
        }
        dlg->accept();
    });
    connect(rejectBtn,&QPushButton::clicked,[this, edit, flowInfo, dlg, w](){
        if(edit->toPlainText().length()>254){
            QMessageBox::information(nullptr,"error","审批文本过长。");
            return;
        }
        if(pushProcess(flowInfo,false,edit->toPlainText())){
           if(w) w->pushProcess(flowInfo,false);
        }
        dlg->accept();
    });
    dlg->show();

}

