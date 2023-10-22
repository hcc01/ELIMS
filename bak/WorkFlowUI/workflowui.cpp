#include "workflowui.h"

WorkFlowUI::WorkFlowUI(QWidget* parent):
    TabWidgetBase(parent)
{
}

bool WorkFlowUI::initMod()
{

    QString sql;
    //操作记录表
    sql="drop table if exists wf_record;"
"create table wf_record"
"("
"   node_id              int,"
"   instance_id          int,"
"   operator_time        datetime,"
"   operator_id          char(32),"
"   record_id            int not null auto_increment,"
"   content              varchar(255),"
"   action               boolean,"//NULL 时为待操作节点
"   primary key (record_id)"
");"
"alter table wf_record add constraint FK_Reference_10 foreign key (node_id)"
"      references wf_process_node (node_id) on delete restrict on update restrict;"
"alter table wf_record add constraint FK_Reference_12 foreign key (instance_id)"
"      references wf_instance (instance_id) on delete restrict on update restrict;";

    //流程记录表
    sql+="drop table if exists wf_instance;"
"create table wf_instance"
"("
"   instance_id          int not null auto_increment,"
"   process_id           int,"
"   creator_id           int,"
"   craete_time          datetime,"
"   content              varchar(255),"
"   primary key (instance_id)"
");"
"alter table wf_instance add constraint FK_Reference_9 foreign key (process_id)"
"      references wf_process_info (process_id) on delete restrict on update restrict;";

    //流程节点表
    sql+="    drop table if exists wf_process_node;"
   " create table wf_process_node"
    "("
      " node_id              int not null,"//不使用自增加，id关联节点序号，判断前后关系
      " process_id           int,"
       "node_name            varchar(32) not null,"
      " possion_id          int,"
      " accept_node         int,"
      " refuse_node         int,"
      " primary key (node_id,process_id)"
   " );"
    "alter table wf_process_node add constraint FK_Reference_6 foreign key (process_id)"
         " references wf_process_info (process_id) on delete restrict on update restrict;";

    //流程信息表
    sql+="drop table if exists wf_process_info;"
"create table wf_process_info"
"("
  " process_id           int not null auto_increment,"
  " process_name         varchar(30) not null,"
  " process_desc         varchar(100),"
"operate_tab          varchar(100),"//注明操作的窗口，用以处理此流程相关事项
 "  primary key (process_id)"
");";

}

void WorkFlowUI::createProceesInfo(const QString &processName, const QString &processDesc, const QString &operateTab)
{
//    QString sql=QString("insert into wf_process_info(process_name, process_desc, process_desc) values(%1, %2, %3);").arg(processName).arg(processDesc).arg(operateTab);
//    QSqlCmd cmd(sql,0,"mainwindow");
    //让服务端的流程管理器进行数据库操作，这里只传递内容就好

}

void WorkFlowUI::createProcessNode(const QString &processName, const QString&nodeName,const QString&acceptToNode,const QString&refuseToNode,const QString&posisionName )
{

}

void WorkFlowUI::newProcess(const QString &processName, const QJsonObject content)
{

}

void WorkFlowUI::doProcess(int recordID, bool action, const QString remarks)
{

}
