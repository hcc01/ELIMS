#include "samplegroupingdlg.h"
#include "ui_samplegroupingdlg.h"
#include"itemsselectdlg.h"
SampleGroupingDlg::SampleGroupingDlg(TabWidgetBase *parent) :
    QDialog(parent),
    SqlBaseClass(parent),
    ui(new Ui::SampleGroupingDlg)
{
    ui->setupUi(this);
    ui->groupView->setHeader({"序号","测试项目"});
    ui->printGroup->hide();
}

SampleGroupingDlg::~SampleGroupingDlg()
{
    delete ui;
}

void SampleGroupingDlg::init(QString taskNum)
{
    QString sql;
    m_taskNum=taskNum;
    setWindowTitle(QString("任务单号 - %1").arg(taskNum));
    sql="select DISTINCT F.testType,A.parameterID,B.parameterName,A.sampleOrder,A.subpackage, E.id,E.sampleGroup,A.testTypeID,A.taskSheetID from ("
          "select * from task_methods where taskSheetID=(select id from test_task_info where taskNum='SST240124010')) as A "
          "left join detection_parameters as B on A.parameterID=B.id  left join method_parameters as C on A.testMethodID= C.id "
          "left join test_methods as E on C. methodID=E.id "
          "left join test_type as F on A.testTypeID=F.id where testingMode=0;";//testingMode=0过滤掉现场测试的项目
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询样品信息时出错：",msg.errorMsg());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QHash <QString,int>groupNameNumMap;//方法维护表分组名称：分配的样品序号
        QHash<int,int>methodGroup;//方法ID：分配的样品序号
        QString nowType;
        int nowGroupNum=0;
        m_typeItemGroupMap.clear();
        m_typeItemsMap.clear();
        if(r.count()<2){
            QMessageBox::information(nullptr,"error","没有相关样品信息");
            sqlFinished();
            return;
        }
        m_taskSheetID=r.at(1).toList().at(8).toInt();//保存下任务单ID
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            QString type=row.at(0).toString();
            if(type!=nowType){
                nowType=type;
                nowGroupNum=0;
                m_typeIdMap[type]=row.at(7).toInt();//保存下类型ID
            }
            QString parameter=row.at(2).toString();
            m_typeItemsMap[type].append(parameter);//检测项目
            m_itemIdMap[parameter]=row.at(1).toInt();

            int samplerOrder=row.at(3).toInt();//样品组号，-1为未编号 ，0为现场测试项目
            bool subpackage=row.at(4).toBool();
            int methodID=row.at(5).toInt();
            QString sampleGroup=row.at(6).toString();
            if(samplerOrder==-1){//未分组，根据方法维护表自动分组
                if(sampleGroup.isEmpty()||subpackage){//维护表未分组，根据方法，同一方法分成一组。
                    if(!methodGroup.contains(methodID)){
                        nowGroupNum++;//新组，添加序号
                        methodGroup[methodID]=nowGroupNum;
                        m_typeItemGroupMap[type][nowGroupNum].append(parameter);
                    }
                    else{
                        m_typeItemGroupMap[type][methodGroup.value(methodID)].append(parameter);
                    }
                }
                else{//方法有分组
                    if(!groupNameNumMap.contains(sampleGroup)){
                        nowGroupNum++;//新组，添加序号
                        groupNameNumMap[sampleGroup]=nowGroupNum;
                        m_typeItemGroupMap[type][nowGroupNum].append(parameter);
                    }
                    else{
                          m_typeItemGroupMap[type][groupNameNumMap.value(sampleGroup)].append(parameter);
                    }
                }
            }
            else{//已经分好组了
                m_typeItemGroupMap[type][samplerOrder].append(parameter);
            }
        }
        ui->typeView->addItems(m_typeItemGroupMap.keys());
        sqlFinished();
    },0,{taskNum});
    waitForSql();
}




void SampleGroupingDlg::on_typeView_itemClicked(QListWidgetItem *item)
{
    if(!item) return;
    QMap<int,QStringList>group=m_typeItemGroupMap.value(item->text());
    ui->groupView->clear();
    for(auto it=group.begin();it!=group.end();++it){
        ui->groupView->append({it.key(),it.value().join("、")});
    }
}


void SampleGroupingDlg::on_printBtn_clicked()
{
    QString sql;
    ui->sortGroup->hide();
    ui->printGroup->show();
    if(m_sampling.count()) return;
    sql="select id, sampleType, samplingSiteName, samplingFrequency, samplingPeriod,testTypeID ,testFieldID from site_monitoring_info where taskSheetID=? left join test_type;";
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询监测信息时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        int n=1;
        QString nowType;
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            int siteID=row.at(0).toInt();
            QString sampleType=row.at(1).toString();
            if(sampleType!=nowType){
                nowType=sampleType;
                n=1;
            }
            QString siteName=row.at(2).toString();
            if(siteName.isEmpty()) {
                siteName=QString("未命名%1#(%2)").arg(n).arg(sampleType);
                n++;
            }
            int frequency=row.at(3).toInt();
            int period=row.at(4).toInt();

//            int m=2;
//            if(m_sampling[sampleType].contains(siteName)){
//                QString newName=siteName;
//                while(m_sampling[sampleType].contains(newName)){

//                    newName=siteName+QString("(%1#)").arg(m);
//                    m++;
//                }
//                siteName=newName;
//            }
            m_periodMap[siteID]=period;
            m_sampling[sampleType][siteID]=siteName;
            m_frequencyMap[siteID]=frequency;
            m_siteIDTypeMap[siteID]=row.at(5).toInt();
            m_siteAreaMap[siteID]=row.at(6).toInt();
        }
        sqlFinished();
    },0,{m_taskSheetID});
    waitForSql();
}


void SampleGroupingDlg::on_saveBtn_clicked()
{
    QString sql;
    for(auto it=m_typeItemGroupMap.begin();it!=m_typeItemGroupMap.end();++it){
        QString type=it.key();
        QMap<int,QStringList>itemMap=it.value();
        for(auto it=itemMap.begin();it!=itemMap.end();++it){
            for(auto item:it.value()){
                sql="update task_methods set sampleOrder=? where testTypeID= ? and parameterID=?;";
                doSql(sql,[this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"保存样品号时出错：",msg.errorMsg());
                        sqlFinished();
                        return;
                    }
                    sqlFinished();
                },0,{it.key(),m_typeIdMap.value(type),m_itemIdMap.value(item)});
                waitForSql();
            }
        }
    }
}


void SampleGroupingDlg::on_typeSelectBox_currentIndexChanged(int index)
{
    if(index){
        auto types=itemsSelectDlg::getSelectedItems(m_sampling.keys());
        if(!types.count()){
            ui->typeSelectBox->setCurrentIndex(0);
            return;
        }
        ui->typeBrowser->setText(types.join("\n"));
    }
}


void SampleGroupingDlg::on_siteSelectBox_currentIndexChanged(int index)
{
    if(index){
        QStringList types=ui->typeBrowser->toPlainText().split("\n");
        QStringList sites;
        m_selectSiteIDs.clear();
        if(!types.count()){
//            for(auto it=m_sampling.begin();it!=m_sampling.end();++it){
//                QString type=it.key();
//                auto siteMap=it.value();
//                int n=1;
//                for(auto it=siteMap.begin();it!=siteMap.end();++it){
//                    QString site=it.value();

//                    sites.append(site);
//                }
//            }
            return;
        }
        else{
            for(auto type:types){
                auto siteMap=m_sampling.value(type);
                sites.append(siteMap.values());
                m_selectSiteIDs.append(siteMap.keys());
            }
            m_selectSiteIDs=itemsSelectDlg::getSelectedItemsID(sites,m_selectSiteIDs,sites);
        }
        ui->siteBrowser->setText(sites.join("\n"));
    }
}


void SampleGroupingDlg::on_printOkbtn_clicked()
{
    QStringList sites=ui->siteBrowser->toPlainText().split("\n");
    QString sql;
    QString date;
    QString clientID;
    bool error=false;
    sql="select inspectedEentityID, clientID from test_task_info where id=?";
    doSql(sql,[this, &clientID, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查找客户ID时出错：",msg.errorMsg());
            sqlFinished();
            error=true;
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()<2){
            QMessageBox::information(nullptr,"error","没有相关任务单信息");
                                     sqlFinished();
            error=true;
                                     return;
        }
        int id=r.at(1).toList().at(0).toInt();
        if(!id) id=r.at(1).toList().at(1).toInt();
        clientID=QString("%1").arg(id, 3, 10, QChar('0'));
        sqlFinished();
    },0,{m_taskSheetID});
    waitForSql();
    if(error) return;
    QString sampleNum;
    QHash<int,int>typeCount;
    for(int i=0;i<sites.count();i++){
        int siteID=m_selectSiteIDs.at(i);
//        QString site=sites.at(i);
        int frequency=m_frequencyMap.value(siteID);
        int period=m_periodMap.value(siteID);
        int testTypeID=m_siteIDTypeMap.value(siteID);
        QString typerOrder=QChar('A'-1+testTypeID);
        typeCount[testTypeID]++;
        QString siteNum=QString("%1").arg(typeCount[testTypeID],2,10,QChar('0'));//点位序号
        sql="select parameterName, sampleOrder from task_methods where monitoringInfoID=?";
        QMap<int,QStringList>itemOrder;
        doSql(sql,[this, &itemOrder, &error](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"查找样品组时出错：",msg.errorMsg());
                sqlFinished();
                error=true;
                return;
            }
            QList<QVariant>r=msg.result().toList();
            for(int i=1;i<r.count();i++){
                QList<QVariant>row=r.at(i).toList();
                if(row.at(1).toInt()<1){
                    QMessageBox::information(nullptr,"error","暂未进行样品分组。");
                    error=true;
                    sqlFinished();
                    return;
                }
                itemOrder[row.at(1).toInt()].append(row.at(0).toString());
            }
            sqlFinished();
        },0,{siteID});
        waitForSql();
        if(error) return;
        for(auto it=itemOrder.begin();it!=itemOrder.end();++it){

            QString sampleOrder=QString("%1").arg(it.key(),2,10,QChar('0'));//样品序号
            QString testParameters=it.value().join("、");
            //检查下是否串联（只针对气体样品，串联的编号不保存，但需要打印，要怎么设计？）
            int seriesConnection=0;
            //先判断下是不是气体样品
            if(m_siteAreaMap[siteID]==1){
                sql="select seriesConnection from test_methods where id=(select methodID from method_parameters where id=(select testMethodID from task_methods where parameterName=? and monitoringInfoID=?));";

                doSql(sql,[this, &seriesConnection, &error](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"查找样品串联情况时出错：",msg.errorMsg());
                        sqlFinished();
                        error=true;
                        return;
                    }
                    QList<QVariant>r=msg.result().toList();
                    if(r.count()<2){
                        QMessageBox::information(nullptr,"查找样品串联情况时出错：","未查找到检测方法");
                        sqlFinished();
                        error=true;
                        return;
                    }
                    seriesConnection=r.at(1).toList().at(0).toInt();
                    sqlFinished();
                },0,{it.value().at(0),siteID});
                waitForSql();
                if(error) return;
            }

            for(int i=0;i<period;i++){
                date=ui->dateEdit->date().addDays(i).toString("yyMMdd");//采样日期
                QString round;
                for(int j=0;j<frequency;j++){
                    if(frequency>1) round=QString("-%1").arg(j+1);
                    sampleNum=date+clientID+typerOrder+siteNum+sampleOrder+round;
                    qDebug()<<"sampleNum"<<sampleNum;
                    //开始保存样品编号
                    //是否需要检查已经编好号了？
                    sql="insert into sampling_info (monitoringInfoID,samplingRound, samplingPeriod, sampleOrder, sampleNumber) values(?,?,?,?,?);";
                    doSql(sql,[this, &error](const QSqlReturnMsg&msg){
                        if(msg.error()){
                            error=true;
                            QMessageBox::information(nullptr,"保存样品编号时出错：",msg.errorMsg());
                            sqlFinished();
                            return;
                        }
                        sqlFinished();
                    },0,{siteID,j+1,i+1,sampleOrder,sampleNum});
                    waitForSql();
                    if(error) return;
                }

            }
        }


    }

}


void SampleGroupingDlg::on_cancelBtn_clicked()
{
    ui->printGroup->hide();
    ui->sortGroup->show();
}

