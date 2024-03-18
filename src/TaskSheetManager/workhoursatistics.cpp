#include "workhoursatistics.h"
#include "qcalendarwidget.h"
#include "qtextformat.h"
#include "ui_workhoursatistics.h"
#include"QMessageBox"
WorkHourSatistics::WorkHourSatistics(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::WorkHourSatistics)
{
    ui->setupUi(this);
    ui->dateEdit->setCalendarPopup(true);
    ui->dateEdit2->setCalendarPopup(true);
    ui->dateEdit->setDate(QDate::currentDate().addDays(1));
    QTextCharFormat format = ui->dateEdit->calendarWidget()->weekdayTextFormat(Qt::Saturday);
    format.setForeground(QBrush(QColor::fromRgb(150,0,0), Qt::SolidPattern));
    ui->dateEdit->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateEdit->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);
    int day=QDate::currentDate().day();
    int month=QDate::currentDate().month();
    int year=QDate::currentDate().year();
    if(day<25) month--;
    day=25;
    ui->dateEdit->setDate(QDate(year,month,day));
    ui->dateEdit2->setDate(QDate(year,month+1,day-1));

    ui->tableView->setHeader({"任务单号","项目名称","样品类型","检测项目","样品数量","下单人员","采样人员","接样时间"});
}

WorkHourSatistics::~WorkHourSatistics()
{
    delete ui;
}

void WorkHourSatistics::on_OKbtn_clicked()
{
    QString sql;
    QTime time(0, 0, 0);
    QDate date1=ui->dateEdit->date();
    QDate date2=ui->dateEdit2->date();
    QString fromDate=QDateTime(date1,time).toString("yyyy-MM-dd hh:mm:ss");
    QString ToDate=QDateTime(date2,time).toString("yyyy-MM-dd hh:mm:ss");
    if(date1>date2){
        QMessageBox::information(nullptr,"error","前后时间错误");
        return;
    }
    if(date2>date1.addMonths(1)){
        QMessageBox::information(nullptr,"error","前后时间不能超过1个月");
        return;
    }
    sql="Select DISTINCT E.taskNum, E. clientName , E. inspectedProject , D. sampleType ,  GROUP_CONCAT(DISTINCT  C.parameterName SEPARATOR ','),  CONCAT(B.samplingFrequency,'*', B.samplingPeriod),F.samplerLeader, F.samplers, E.creator, E.receiveTime  "
          "from task_parameters as C left join site_monitoring_info as D on D.taskSheetID=C.taskSheetID "
          "Left join (select A.* ,B. receiveTime  from  (select  receiveTime , taskNum  from sample_circulate  where receiveTime>=? and receiveTime<?) as B   "
          "left join test_task_info as A on B.tasKnum=A.taskNum where a.deleted=0) as E on E.id= C.taskSheetID  "
          "left join samplingSchedul as F on F.taskNum=E.taskNum where E.taskNum is not null";
    sql="select C.taskNum, C.inspectedProject,  "
          "COALESCE(B.samplingSiteName,B.sampleName) as site, "
          "GROUP_CONCAT(DISTINCT A.parameterName SEPARATOR ',') as items,"
          "sum(COALESCE(B.samplingFrequency*B.samplingPeriod, B.sampleCount)) as count, "
          "C.creator, CONCAT(D.samplerLeader,'、', D.samplers) as sampler, "
          "E.receiveTime "
          "from task_parameters as A "
          "left join site_monitoring_info as B on B.id=A.monitoringInfoID "
          "left join test_task_info as C on A.taskSheetID=C.id "
          "left join samplingSchedul as D on D.taskNum=C.taskNum "
          "left join sample_circulate as E on E.taskNum=C.taskNum "
          "where  E.receiveTime>=? and E.receiveTime<? "
          "group by  C.taskNum, C.inspectedProject,site, C.creator,sampler,E.receiveTime "
          "order by E.receiveTime";
    sql="select C.taskNum, C.inspectedProject, G.sampleType, G.items,sum(G.count) ,"
          "C.creator, CONCAT(D.samplerLeader,'、', D.samplers) as sampler, E.receiveTime "
          "from (select GROUP_CONCAT(DISTINCT A.parameterName SEPARATOR ',') as items,"
          "B.id,B.sampleType,COALESCE(B.samplingFrequency*B.samplingPeriod, B.sampleCount) as count,B.taskSheetID "
          "from task_parameters as A "
          "left join site_monitoring_info as B on B.id=A.monitoringInfoID "
          "group by B.id,B.sampleType, count,B.taskSheetID ) as G "
          "left join test_task_info as C on G.taskSheetID=C.id "
          "left join samplingSchedul as D on D.taskNum=C.taskNum "
          "left join sample_circulate as E on E.taskNum=C.taskNum "
          "where  E.receiveTime>=? and E.receiveTime<? "
          "group by C.taskNum, C.inspectedProject, G.sampleType, G.items,C.creator,sampler, E.receiveTime "
          "order by E.receiveTime"
        ;
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询任务单时出错:",msg.errorMsg());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        ui->tableView->clear();
        QString nowType;
        QStringList items;
        QList<QVariant>row;
        for(int i=1;i<r.count();i++){
            row=r.at(i).toList();
            ui->tableView->append(row);
//            auto preRow=r.at(i-1).toList();
//            QString type=QString("%1%2").arg(row.at(0).toString()).arg(row.at(3).toString());
//            if(type!=nowType){
//                if(!nowType.isEmpty()){
//                    preRow[4]=items.join("、");
//                    ui->tableView->append(preRow);
//                }
//                items.clear();
//                items.append(row.at(4).toString());
//                nowType=type;
//            }
//            else{
//                if(!items.contains(row.at(4).toString())) items.append(row.at(4).toString());
//            }
        }
        row[4]=items.join("、");
        ui->tableView->append(row);
    },0,{fromDate,ToDate});

}

