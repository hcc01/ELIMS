#include "samplegroupingdlg.h"

#include "qcalendarwidget.h"
#include "qpainter.h"
#include "tasksheetui.h"
#include "ui_samplegroupingdlg.h"
#include"itemsselectdlg.h"
#include"exceloperator.h"
#include "QZXing.h"
#include<QDir>
#include"dbmater.h"
#include<QFileDialog>
#include<QScreen>
SampleGroupingDlg::SampleGroupingDlg(TabWidgetBase *parent) :
    QDialog(parent),
    SqlBaseClass(parent),
    ui(new Ui::SampleGroupingDlg),
    m_groupChanged(false)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"检测类型","检测点位","检测项目","检测频次"});
    ui->dateEdit->setCalendarPopup(true);
    ui->dateEdit->setDate(QDate::currentDate().addDays(1));
    QTextCharFormat format = ui->dateEdit->calendarWidget()->weekdayTextFormat(Qt::Saturday);
    format.setForeground(QBrush(QColor::fromRgb(150,0,0), Qt::SolidPattern));
    ui->dateEdit->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateEdit->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);
    ui->groupView->setHeader({"序号","测试项目"});

    ui->tableView->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
    ui->printGroup->hide();
    ui->groupView->addContextAction("合并分组",[this](){
//        QString type=ui->typeView->currentItem()->text();
        auto indexs=ui->groupView->selectedIndexes();
        if(!indexs.count()) return;        
        int testTypeID=m_groups.keys().at(ui->typeView->currentRow());//类型列表是按m_groups排序的
        QList<int>rows;//选择的行，可能不同行，可能先选择下面再选择上面，所以后面要排序
        for(int i=0;i<indexs.count();i++){
            if(indexs.at(i).column()!=0) continue;
            rows.append(indexs.at(i).row());
        }
        if(rows.count()<2) return;
        qSort(rows);
        QMap<int,QList<int>>groups=m_groups.value(testTypeID);//当前类型的分组列表
//        QStringList items;
        int first=rows.first();
        int firstkey=groups.keys().at(first);
        QList<int>removekeys;
        for(int row:rows){
            int key=groups.keys().at(row);
//            items.append(groups.value(key));
            if(row==first) continue;
            groups[firstkey].append(groups.value(key));//把后面的都拉到第1组。
            removekeys.append(key);
        }
        for(int k:removekeys){
             groups.remove(k);
        }
//        groups[first+1]=items;
        //其它行重新按顺序修改KEY
        QList<int>items;
        for(int i=rows.at(0)+1;i<groups.count();i++){
            int pre=groups.keys().at(i-1);//QMap有排序，rows之前也排过序，可以这么操作
            int now=groups.keys().at(i);
            items=groups.value(now);
            groups.remove(now);
            while(now-1>pre) now--;
            groups[now]=items;
        }
        qDebug()<<groups;
        m_groups[testTypeID]=groups;//合并完成，保存数据
        m_groupChanged=true;
        on_typeView_currentRowChanged(ui->typeView->currentRow());
    });
    ui->groupView->addContextAction("拆分分组",[this](){
        int row=ui->groupView->selectedRow();
        if(row<0) return;
        QStringList items;
        items=ui->groupView->value(row,1).toString().split("、");
        if(items.count()==1) return;
        int testTypeID=m_groups.keys().at(ui->typeView->currentRow());//类型列表是按m_groups排序的
        QMap<int,QList<int>>groups=m_groups.value(testTypeID);
        int groupOrder=groups.keys().at(row);
        QList<int>oldGroup=groups.value(groupOrder);
        QList<int> newGroup=itemsSelectDlg::getSelectedItemsID(items,oldGroup,items,"请选择要分离出去的项目：");
        for(int item:newGroup){
            oldGroup.removeOne(item);
        }
        groups[groupOrder]=oldGroup;
        groups[groups.lastKey()+1]=newGroup;
        m_groups[testTypeID]=groups;
        m_groupChanged=true;
        on_typeView_currentRowChanged(ui->typeView->currentRow());
//        on_typeView_itemClicked(ui->typeView->currentItem());
    });
    ui->groupView->addContextAction("现场监测",[this](){
        int row=ui->groupView->selectedRow();
        if(row<0) return;
        int testTypeID=m_groups.keys().at(ui->typeView->currentRow());//类型列表是按m_groups排序的
        QList<int> items;
        QMap<int,QList<int>>groups=m_groups.value(testTypeID);
        int key=ui->groupView->value(row,0).toInt();
        if(key==0) return;//已经是现场监测的项目了
        items=groups.value(key);//标识当前项目
        int x=0;
        if(!groups.contains(0)) {//注意，groups增加了一行
            groups[0]=items;
            x=1;
        }
        else groups[0].append(items);
        groups.remove(key);//改为现场监测的序号；注意groups减少了一行
        if(groups.count()>1)//如果还有其它的项目，将不连续的编码改为连续
        {
            //
            for(int i=row+x;i<groups.count();i++){
                groups[groups.keys().at(i)-1]=groups.value(groups.keys().at(i));
                if(i==groups.count()-1)
                    groups.remove(groups.lastKey());//这个不能放外面，因为可能不进入重排的情况。
            }
        }

        m_groups[testTypeID]=groups;
        m_groupChanged=true;
        on_typeView_currentRowChanged(ui->typeView->currentRow());
//        on_typeView_itemClicked(ui->typeView->currentItem());
    });

    ui->tableView->addContextAction("加入生成表",[this](){
        if(!ui->radioGenerate->isChecked()) return;
        int row=ui->tableView->selectedRow();
        if(row<0) return;
        QModelIndexList indexs=ui->tableView->selectedIndexes();
        for(auto index:indexs){
            ui->tableView->setBackgroundColor(index,SAMPLING_COLOR);
            if(index.column()==0){
                int sideID=ui->tableView->cellFlag(index.row(),0).toInt();
                if(!m_samplingSideID.contains(sideID)) m_samplingSideID.append(sideID);
            }
        }
        ui->checkBox->setChecked(false);
    });
}

SampleGroupingDlg::~SampleGroupingDlg()
{
    delete ui;
}

void SampleGroupingDlg::init(int taskSheetID, const QStringList &samplers)
{
    m_taskSheetID=taskSheetID;
    m_samplers=samplers;
    //获取分组结果
    QString sql;
    sql="select A.testTypeID, A.sampleGroup, GROUP_CONCAT(DISTINCT A.parameterID SEPARATOR ',') "
          "from task_parameters as A "
          "left join site_monitoring_info as B on A.monitoringInfoID=B.id "
          "where A.taskSheetID=? "
          "group by A.testTypeID, A.sampleGroup;";
    bool error=false;
    doSql(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询样品分组结果时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        auto r=msg.result().toList();
        m_groups.clear();
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            for(auto id:row.at(2).toString().split(","))
                m_groups[row.first().toInt()][row.at(1).toInt()].append(id.toInt());
        }
        sqlFinished();
    },0,{taskSheetID});
    waitForSql();
    if(error) return;
    //获取点位的样品分组。
    sql="select A.monitoringInfoID, A.sampleGroup, GROUP_CONCAT(DISTINCT A.parameterID SEPARATOR ',') "
          "from task_parameters as A "
          "left join site_monitoring_info as B on A.monitoringInfoID=B.id "
          "where A.taskSheetID=? "
          "group by A.monitoringInfoID, A.sampleGroup;";
    doSql(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询样品分组结果时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        auto r=msg.result().toList();
        m_siteGroups.clear();
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            for(auto id:row.at(2).toString().split(","))
                m_siteGroups[row.first().toInt()][row.at(1).toInt()].append(id.toInt());
        }
        sqlFinished();
    },0,{taskSheetID});
    waitForSql();
    if(error) return;

    showGroup();
    /*
    QString sql;
    m_taskNum=taskNum;
    m_samplers=samplers;
    setWindowTitle(QString("任务单号 - %1").arg(taskNum));
    sql="select F.testType,A.parameterID,B.parameterName,A.sampleOrder,A.subpackage, E.id,E.sampleGroup,A.testTypeID,A.taskSheetID ,A.clientID, A.inspectedEentityID ,"
          "GROUP_CONCAT(A.monitoringInfoID SEPARATOR ','), A.testTypeID, E.seriesConnection "
          "from (select task_methods.* , X.clientID,X.inspectedEentityID from task_methods left join test_task_info as X on task_methods.taskSheetID=X.id where taskNum=?) as A "
          "left join detection_parameters as B on A.parameterID=B.id  left join method_parameters as C on A.testMethodID= C.id "
          "left join test_methods as E on C. methodID=E.id "
          "left join test_type as F on A.testTypeID=F.id "
          "group by F.testType,A.parameterID,B.parameterName,A.sampleOrder,A.subpackage, E.id,E.sampleGroup,A.testTypeID,A.taskSheetID ,A.clientID, A.inspectedEentityID , A.testTypeID, E.seriesConnection;";
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询样品信息时出错：",msg.errorMsg());
            sqlFinished();
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
        int id=r.at(1).toList().at(10).toInt();
        if(!id) id=r.at(1).toList().at(9).toInt();
        m_clientID=QString("%1").arg(id,3,10,QChar('0'));//保存下客户编号，用于样品编号
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
//            m_siteIDTypeID[row.at(11).toInt()]=row.at(12).toInt();//保存下点位的类型
            QStringList siteIDs=row.at(11).toString().split(",");
            for(auto id:siteIDs) m_siteIDTypeID[id.toInt()]=row.at(12).toInt();//保存下点位的类型
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
                samplerOrder=nowGroupNum;
            }
            else{//已经分好组了
                m_typeItemGroupMap[type][samplerOrder].append(parameter);
            }
            m_seriesConnection[samplerOrder]=row.at(13).toInt();//保存样品号的串联情况
        }
        ui->typeView->addItems(m_typeItemGroupMap.keys());

        sqlFinished();
    },0,{taskNum});
    waitForSql();
*/
}

void SampleGroupingDlg::showGroup() const
{
    QList<int>TypeList=m_groups.keys();
    QSqlQuery query(DB.database());
    ui->typeView->clear();
    for(int typeID:TypeList){
        query.prepare("select testType from test_type where id=?");
        query.addBindValue(typeID);
        if(!query.exec()){
            QMessageBox::information(nullptr,"查询类型时出错：",query.lastError().text());
        }
        if(!query.next()){
            QMessageBox::information(nullptr,"查询类型时出错：",QString("数据库错误：没有相关检测类型：id=%1。").arg(typeID));
        }
        QString type=query.value(0).toString();
        ui->typeView->addItem(type);
    }
    ui->typeView->setCurrentRow(0);
}

QStringList SampleGroupingDlg::parameterNames(QList<int> parameterIDs) const
{
    QStringList items;
    QSqlQuery query(DB.database());
        for(int id:parameterIDs){
//            query.clear();
//            query.prepare("select parameterName from detection_parameters where id=?");
//            query.addBindValue(id);

            if(!query.exec(QString("select parameterName from detection_parameters where id=%1").arg(id))){
                qDebug()<<"query.lastQuery()"<<query.lastQuery();
                QMessageBox::information(nullptr,"查询项目名称时出错：",query.lastError().text());
                break;
            }
            if(!query.next()){
                QMessageBox::information(nullptr,"查询项目名称时出错：",QString("数据库出错，没有相关项目：id=%1").arg(id));
                break;
            }
            items.append(query.value(0).toString());
        }
        return items;

}

QString SampleGroupingDlg::dateNum(const QDate &date)
{
    return date.toString("yyMMdd");
}

QString SampleGroupingDlg::clientNum(const QDate &date,int taskSheetID)
{
    return "";
}

void SampleGroupingDlg::initNum(const QDate&dateStart,int days)
{
    //先把客户编码编上
    m_clientNum.clear();
    tabWiget()->connectDB(CMD_START_Transaction);
    QList<QVariant>r;
    bool error=false;
        QString sql;
        int order=0;
        //按所给采样日期的采样任务单顺序编号
        //先确认下这个日期中已经安排的任务单
        sql="select taskSheetID,number from sampling_task_order "
              "where samplingDate=?";
        doSql(sql,[this, &r, &error](const QSqlReturnMsg&msg){
            if(msg.error()){
                tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                QMessageBox::information(nullptr,"查询采样单序号时出错：",msg.errorMsg());
                error=true;
                sqlFinished();
                return;
            }
            r=msg.result().toList();

            sqlFinished();
        },0,{dateStart.toString("yyyy-MM-dd"),dateStart.toString("yyyy-MM-dd")});
        waitForSql();
        if(error) return;
        QHash<int,int>nums;
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            nums[row.at(0).toInt()]=row.at(1).toInt();//记录各个任务单的编号
        }
        if(nums.contains(m_taskSheetID)){
            order=nums.value(m_taskSheetID);//如果任务单已经编号，使用该编号（这种情况出现在前天有采样的情况）
        }
        else{
            QList<int> usedOrders=nums.values();
            int i=1;
            while (usedOrders.contains(i)) {//从1开始，看看哪个编号没有使用，就用哪个号
                i++;
            }
            order=i;
        }
        //更新数据库
        sql="";
        QJsonArray values;
        for(int i=0;i<days;i++){
            sql+="insert IGNORE  into sampling_task_order(samplingDate, taskSheetID,number) values(?,?,?) ;";
            values.append(dateStart.addDays(i).toString("yyyy-MM-dd"));
            values.append(m_taskSheetID);
            values.append(order);
        }
        //确认客户编码
        if(order<=26){
            m_clientNum.append(QChar('A'+order-1));
        }
        else{
            order-=26;
            m_clientNum.append(QChar('a'+order-1));
        }
        doSql(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                    QMessageBox::information(nullptr,"更新采样单序号时出错：",msg.errorMsg());
                    error=true;
                    sqlFinished();
                    return;
                }

                sqlFinished();
            },0,values);

        if(error) return;
        tabWiget()->releaseDB(CMD_COMMIT_Transaction);
    //把点位序号初始化
    testTypeNum("0",0,true);
}

QString SampleGroupingDlg::testTypeNum(int testTypeID, int start)
{
    QString testType;
    QSqlQuery query(DB.database());/*
    query.prepare("select tyestType from test_type where id=?;");
    query.addBindValue(testTypeID);*/
    if(!query.exec(QString(("select testType from test_type where id=%1;")).arg(testTypeID))){
        QMessageBox::information(nullptr,"error",query.lastError().text());
        return "";
    }
    qDebug()<<QString(("select testType from test_type where id=%1;")).arg(testTypeID);
    if(!query.next()){
        QMessageBox::information(nullptr,"error",QString("未找到检测类型,id:%1。").arg(testTypeID));
        return "";
    }
    testType=query.value(0).toString();
    return testTypeNum(testType,start);
}

QString SampleGroupingDlg::testTypeNum(QString testType, int start, bool resetNum)
{

    static int num[25]={0};
    if(resetNum) {
        for (int& i : num) {
            i = 0;
        }
        return "";//初始化
    }
    if(testType=="有组织废气"){
        int &N=num[0];
        if(start) N=start-1;
        N++;
        return QString("A%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="无组织废气"){
        int &N=num[1];
        if(start) N=start-1;
        N++;
        return QString("B%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="环境空气时均值"){
        int &N=num[2];
        if(start)N=start-1;
        N++;
        return QString("C%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="环境空气日均值"){
        int &N=num[3];
        if(start) N=start-1;
        N++;
        return QString("D%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="室内空气"){
        int &N=num[4];
        if(start) N=start-1;
        N++;
        return QString("E%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="地表水"){
        int &N=num[5];
        if(start) N=start-1;
        N++;
        return QString("F%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="地下水"){
        int &N=num[6];
        if(start) N=start-1;
        N++;
        return QString("G%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="工业废水"||testType=="生活污水"){
        int &N=num[7];
        if(start) N=start-1;
        N++;
        return QString("H%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="生活饮用水"){
        int &N=num[8];
        if(start) N=start-1;
        N++;
        return QString("I%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="海水"){
        int &N=num[9];
        if(start)N=start-1;
        N++;
        return QString("J%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="锅炉用水和冷却水"){
        int &N=num[10];
        if(start) N=start-1;
        N++;
        return QString("K%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="土壤"){
        int &N=num[11];
        if(start) N=start-1;
        N++;
        return QString("L%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="沉积物"){
        int &N=num[12];
        if(start)N=start-1;
        N++;
        return QString("M%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="污泥"){
        int &N=num[13];
        if(start) N=start-1;
        N++;
        return QString("N%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="固体废物"||testType=="固体废物浸出液"){
        int &N=num[14];
        if(start) N=start-1;
        N++;
        return QString("O%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="海洋生态"){
        int &N=num[15];
        if(start) N=start-1;
        N++;
        return QString("P%1").arg(N,2,10,QChar('0'));
    }
    if(testType=="生物体"){
        int &N=num[16];
        if(start) N=start-1;
        N++;
        return QString("Q%1").arg(N,2,10,QChar('0'));
    }
    return "";
}




//void SampleGroupingDlg::on_typeView_itemClicked(QListWidgetItem *item)//显示每个类型的分组情况
//{
//    if(!item) return;
//    QMap<int,QStringList>group=m_typeItemGroupMap.value(item->text());
//    ui->groupView->clear();
//    for(auto it=group.begin();it!=group.end();++it){
//        ui->groupView->append({it.key(),it.value().join("、")});
//    }
//}


void SampleGroupingDlg::on_printBtn_clicked()//打印标签
{
    if(m_groupChanged){
        int a=QMessageBox::question(nullptr,"","样品分组有变更，是否保存？");
        if(a==QMessageBox::Yes){
            on_saveBtn_clicked();
        }
        else{
            return;
        }
    }
    on_radioGenerate_clicked();

}


void SampleGroupingDlg::on_saveBtn_clicked()//保存分组
{
    QString sql;
    QJsonArray values;
    for(auto it=m_groups.begin();it!=m_groups.end();++it){
        int typeID=it.key();
        QMap<int,QList<int>>groups=it.value();
        for(auto it=groups.begin();it!=groups.end();++it){
            for(int item:it.value()){
                sql+="update task_parameters set sampleGroup=? where testTypeID= ? and parameterID=? and taskSheetID=?;";
                values.append(it.key());
                values.append(typeID);
                values.append(item);
                values.append(m_taskSheetID);
            }
        }
    }
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"保存样品号时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,values);
    waitForSql();
    m_groupChanged=false;
    init(m_taskSheetID,m_samplers);
}


//void SampleGroupingDlg::on_typeSelectBox_currentIndexChanged(int index)
//{
//    if(index){
//        auto types=itemsSelectDlg::getSelectedItems(m_sampling.keys());
//        if(!types.count()){
//            ui->typeSelectBox->setCurrentIndex(0);
//            return;
//        }
//        ui->typeBrowser->setText(types.join("\n"));
//    }
//}


//void SampleGroupingDlg::on_siteSelectBox_currentIndexChanged(int index)
//{
//    if(index){
//        QStringList types=ui->typeBrowser->toPlainText().split("\n");
//        QStringList sites;
//        m_selectSiteIDs.clear();
//        if(!types.count()){
////            for(auto it=m_sampling.begin();it!=m_sampling.end();++it){
////                QString type=it.key();
////                auto siteMap=it.value();
////                int n=1;
////                for(auto it=siteMap.begin();it!=siteMap.end();++it){
////                    QString site=it.value();

////                    sites.append(site);
////                }
////            }
//            return;
//        }
//        else{
//            for(auto type:types){
//                auto siteMap=m_sampling.value(type);
//                sites.append(siteMap.values());
//                m_selectSiteIDs.append(siteMap.keys());
//            }
//            m_selectSiteIDs=itemsSelectDlg::getSelectedItemsID(sites,m_selectSiteIDs,sites);
//        }
//        ui->siteBrowser->setText(sites.join("\n"));
//    }
//}

void SampleGroupingDlg::on_printOkbtn_clicked()
{


    if(ui->radioGenerate->isChecked()){//生成标签
        int typeID;
        int typeOrder=0;
        QString sampleNum;//实际样品编号
        QString showedNum;//用于记录的编号，不识别样品项目序号
        QString dateNum;//日期编码
        QString typeNum;//类型编码
        QString orderNum;
        QString SeriesNum;//串联编码
        QString roundNum;//频次编码
        QString sql;
        QJsonArray values;
        QList<int>siteIDs;
        if(ui->samplerEdit->text().isEmpty()){
            QMessageBox::information(nullptr,"error","请选择采样人员。");
            return;
        }
        QStringList samplers=ui->samplerEdit->text().split("、");
        samplers.removeDuplicates();
        if(samplers.count()>2){
            QMessageBox::information(nullptr,"","最多选择2个采样人员。");
            return;
        }
        if(ui->checkBox->isChecked()){//全部点位
            int a= QMessageBox::question(nullptr,"",QString("你将负责全部点位的采样，确认？").arg(ui->dateEdit->date().toString()));
            if(a!=QMessageBox::Yes) return;
        }
        int a= QMessageBox::question(nullptr,"",QString("采样开始日期为%1，确认？").arg(ui->dateEdit->date().toString()));
        if(a!=QMessageBox::Yes) return;
        //确认要采哪些点位
        if(ui->checkBox->isChecked()){//全部点位
            siteIDs=m_allSides.keys();
        }
        else siteIDs=m_samplingSideID;
        if(siteIDs.isEmpty()){
            QMessageBox::information(nullptr,"","请选择本次采样的点位。");
            return;
        }
        int samplingDays=ui->periodBox->currentIndex();//持续几天
        if(!samplingDays) samplingDays=ui->periodBox->count()-1;

        initNum(ui->dateEdit->date(),samplingDays);//先把类型点位编号初始化
        if(!m_clientNum.count()) return;

        //开始对每个点位进行分析
        for(int siteID:siteIDs){
            typeID=m_siteIDTypeID.value(siteID);
//            typeNum=QString("%1").arg(QChar('A'-1+typeID));//类型编号
            if(m_siteUsedOrderMap.contains(siteID)){//点位的序号已经存在（也就是之前已经采过一些周期），使用之前的点位序号
                typeOrder=m_siteUsedOrderMap.value(siteID);//如果这个点位编码这前的点位未采，那些编号不会被使用，先这样吧。
            }
            //延续之前的点位编号
            else if(m_typeOrder.contains(typeID)){//类型已经编号号，继续往下编
                typeOrder=m_typeOrder.value(typeID)+1;
                m_typeOrder.remove(typeID);//后续使用自增
            }

//            else{//需要给新的点位号
//                if(typeID!=nowType){
//                    typeOrder=m_typeUsedOrder.value(typeID)+1;
//                    qDebug()<<"m_typeUsedOrder"<<m_typeUsedOrder<<"typeID"<<typeID;
//                    nowType=typeID;
//                }

//                else typeOrder++;//当前类型的点位序号
//            }
            typeNum=testTypeNum(typeID,typeOrder);//获取类型与点位编码
            if(typeNum.isEmpty()) return;
//            QString siteNum=QString("%1").arg(typeOrder,2,10,QChar('0'));//类型编号
//            QString type;
            int day=1;
            int f=m_frequencyMap.value(siteID);
            day=m_periodMap[siteID].first;
    //        if(ui->periodBox->currentIndex()!=0){
    //            day=ui->periodBox->currentIndex();
    //            if(day>m_periodMap[siteID].first) continue;//超出当前点位的最大周期，无效。
    //        }
            int leftDay=m_periodMap[siteID].second;
            int nowPeriod;
            int start=0;
            QString clientNum;

            clientNum=m_clientNum.at(0);//这个就只有一个元素，每天都用同一代码
            for(int i=1;i<=samplingDays;i++){
                if(i>leftDay) break;//当前点位没有这么多周期了
                start++;//采第几天
                nowPeriod=day-leftDay+i;//当前采的第几个周期（总体的第几天）
                dateNum=ui->dateEdit->date().addDays(start-1).toString("yyMMdd");//时间编号；多个周期连续采样的，时间要推移

                QList<int>groupOrders=m_siteGroups.value(siteID).keys();
                for(int samplerOrder:groupOrders){
                    if(samplerOrder==0) continue;//现场监测，没有样品编号
                    QString orderNum=QString("%1").arg(samplerOrder,2,10,QChar('0'));//样品编号
//                    int Series=m_seriesConnection.value(samplerOrder) ;
                    roundNum="";
                    SeriesNum="";
//                    if(Series){
//                        for(int i=0;i<Series;i++){
//                            SeriesNum.append(QString("%1").arg(QChar('a'+Series)));//串联，样品保存编号直接加上abc，在显示编号是分开打印。
//                        }
//                    }
                    for(int i=0;i<f;i++){
                        if(f>1)  roundNum=QString("-%1").arg(i+1);//频次编号；样品编号到此完成
                        //输出标签
                        //保存的编号和显示的编号不一样，保存的编号没有串联信息，有样品序号，显示的编号有串联信息，没有样品序号。
                        sampleNum=QString("%1%2%3%4%5%6").arg(clientNum).arg(dateNum).arg(typeNum).arg(orderNum).arg(SeriesNum).arg(roundNum);
//                        showedNum=QString("%1%2%3%4%5").arg(clientNum).arg(dateNum).arg(typeNum).arg(SeriesNum).arg(roundNum);


                        sql+="insert into sampling_info(monitoringInfoID, samplingRound, samplingPeriod, sampleNumber, samplers,siteOrder,sampleOrder) "
                              "values(?,?,?,?,?,?,?) ;";
                        values.append(siteID);
//                        values.append(m_allSides.value(siteID).first);
                        values.append(i+1);
                        values.append(nowPeriod);
                        values.append(sampleNum);
                        values.append(ui->samplerEdit->text());
                        values.append(typeNum.mid(1).toInt());
                        values.append(samplerOrder);

                    }

                }

    //            if(ui->periodBox->currentIndex()!=0) break;//只选其中一个周期的（这个在上面使用start>1来判断处理
            }
        }
        bool error=false;
        doSql(sql,[this, &error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新样品信息时出错：",msg.errorMsg());
                    this->tabWiget()->releaseDB(CMD_ROLLBACK_Transaction);
                    error=true;
                    sqlFinished();
                    return;
                }
                sqlFinished();
            },0,values);
        waitForSql();
        if(error){
            return;
        }
        //更新任务单状态为采样
        sql="update test_task_info set taskStatus=? where id=?;";
        doSql(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"更新任务单状态时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,{TaskSheetUI::SAMPLING,m_taskSheetID});
        waitForSql();

        on_radioGenerate_clicked();
    }
    else{//打印标签
        auto indexs=ui->tableView->selectedIndexes();
        if(!indexs.count()){
            QMessageBox::information(nullptr,"","请选择需要打印的标签");
            return;
        }
        ExcelOperator excel;

        int startRow;
        int startColumn;

        QMessageBox msgBox;
        msgBox.setWindowTitle("选择操作");
        msgBox.setText("您想要做什么？");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, "生成EXCEL标签");
        msgBox.setButtonText(QMessageBox::No, "导出标签数据");
        int ret = msgBox.exec();
        bool toLabel;
        if (ret == QMessageBox::Yes) {
            toLabel=true;
        }
        else if(ret == QMessageBox::Yes){
            toLabel=false;
        }
        else{
            return;
        }

        if(!excel.openExcel(".\\采样标签.xlsx"))
        {
            QMessageBox::information(nullptr,"无法打开样品标签文件:",excel.LastError());
            return;
        }
        if(! excel.document()->selectSheet("采样标签")){
            QMessageBox::information(nullptr,"表格错误:","缺少采样标签表");
                                     return;
        }
         if(! excel.document()->selectSheet("标签数据")){
            QMessageBox::information(nullptr,"表格错误:","缺少标签数据表");
            return;
        }

        excel.document()->selectSheet("采样标签");
        CellRange range=excel.find("[开始]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位标签开始位置");
            return;
        }
        excel.setValue(range,"");
        startRow=range.firstRow();
        startColumn=range.firstColumn();
        qDebug()<<"startRow"<<startRow;
        qDebug()<<"startColumn"<<startColumn;
        range=excel.find("[结束]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位标签结束位置");
            return;
        }
         excel.setValue(range,"");

        int endRow=range.firstRow();
         int endColumn=range.firstColumn();
        int leftStart=startColumn;
        int labelWidth=endColumn-startColumn+1;
        int labelHeight=endRow-startRow+1;

        range=excel.find("[二维码]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位标签二维码位置");
            return;
        }
        excel.setValue(range,"");
        int codeRow=range.firstRow()-startRow;
        int codeColumn=range.firstColumn()-startColumn;
        //获取二维码单元格的大小：
        Format format=excel.document()->cellAt(range.firstRow(),range.firstColumn())->format();
        QFont font = format.font();
        // 创建一个 QPainter 对象来绘制到内存中（不需要实际的窗口或控件）
        QPainter painter;
        // 开始绘制到一个QImage上，这只是一个临时步骤，用于获取字体度量
        QImage dummyImage(1, 1, QImage::Format_ARGB32);
        dummyImage.fill(Qt::transparent);
        painter.begin(&dummyImage);

        // 获取字体度量
        QFontMetrics metrics(font);
        int charPixelWidth = metrics.horizontalAdvance('W'); // 使用'W'字符作为参考宽度
        int charPixelHeight = metrics.ascent() + metrics.descent(); // 字符的总高度

        // 结束绘制
        painter.end();

        // 获取A1单元格的宽度和高度（以字符的 1/256 为单位）
        double cellWidthInChars = 0.0;
        double cellHeightInChars = 0.0;
        for(int row=range.firstRow();row<=range.lastRow();row++){
            cellHeightInChars+=excel.rowHeight(row);
        }
        for(int col=range.firstColumn();col<=range.lastColumn();col++){
            cellWidthInChars+=excel.columnWidth(col);
        }

        // 将字符宽度和高度转换为像素
        int pixelWidth = static_cast<int>(cellWidthInChars * charPixelWidth);
        int pixelHeight = static_cast<int>(cellHeightInChars * charPixelHeight);
        int l=qMin(pixelWidth,pixelHeight);
        // 创建 QSize 对象
        QSize cellSize(l, l);//这个用来给二维码定大小。
        qDebug()<<"cellSize"<<cellSize;

        range=excel.find("[样品类型]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位检测类型位置:");
            return;
        }
        int typeRow=range.firstRow()-startRow;
        int typeColumn=range.firstColumn()-startColumn;

        range=excel.find("[点位名称]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位点位名称位置");
            return;
        }
        int siteRow=range.firstRow()-startRow;
        int siteColumn=range.firstColumn()-startColumn;

        range=excel.find("[采样日期]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位采样日期位置");
            return;
        }
        int dateRow=range.firstRow()-startRow;
        int dateColumn=range.firstColumn()-startColumn;

        range=excel.find("[样品编号]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"表格错误:","无法定位样品编号位置");
            return;
        }
        int numRow=range.firstRow()-startRow;
        int numColumn=range.firstColumn()-startColumn;

        range=excel.find("[检测项目]");
        if(!range.isValid()){
            QMessageBox::information(nullptr,"无法定位检测项目位置:",EXCEL.LastError());
            return;
        }
        int itemRow=range.firstRow()-startRow;
        int itemColumn=range.firstColumn()-startColumn;
        int sheet2Row=2;

        //标签占用的行数
        int labelsPerRow=4;
        int nowLabelPosInRow=1;

        CellRange usedRange=excel.usedRange();
        for(auto index:indexs){
            if(index.column()!=0) continue;
            int row=index.row();
            QString sampleType=ui->tableView->value(row,0).toString();
            QString siteName=ui->tableView->value(row,1).toString();
            QString testItems=ui->tableView->value(row,2).toString();
            QString sampleNum=ui->tableView->value(row,3).toString();
            QString num1,num2;
            num1=sampleNum.left(10);//11,12是项目序号，目前不显示
            num2=sampleNum.mid(12);
            QStringList series;
            QStringList showSampleNums;

            int seriesCount=ui->tableView->cellFlag(row,1).toInt();
            for(int i=0;i<seriesCount+1;i++){
                series.append(QChar('a'+i));
            }
            QString sampleDate=QString("20%1-%2-%3").arg(sampleNum.mid(1,2)).arg(sampleNum.mid(3,2)).arg(sampleNum.mid(5,2));
            QDate date(sampleDate.split("-").first().toInt(),sampleDate.split("-").at(1).toInt(),sampleDate.split("-").last().toInt());
//            if(date<QDate::currentDate()){
//                int a=QMessageBox::question(nullptr,"","当前标签时间已过期，是否继续使用？");
//                if(a!=QMessageBox::Yes) return;
//            }
            if(seriesCount){
                for(auto s:series){
                    showSampleNums.append(QString("%1%2%3").arg(num1).arg(s).arg(num2));
                }
            }
            else showSampleNums.append(QString("%1%2").arg(num1).arg(num2));
            for(auto num:showSampleNums){
                if(toLabel){
                    excel.document()->selectSheet("采样标签");
                    excel.document()->insertImage(startRow+codeRow-1,startColumn+codeColumn-1,QZXing::encodeData(sampleNum,QZXing::EncoderFormat_QR_CODE,cellSize));//二维码
                    excel.setValue(sampleType,startRow+typeRow,startColumn+typeColumn);//样品类型
                    excel.setValue(sampleDate,startRow+dateRow,startColumn+dateColumn);//采样日期
                    excel.setValue(siteName,startRow+siteRow,startColumn+siteColumn);//点位名称
                    excel.setValue(testItems,startRow+itemRow,startColumn+itemColumn);//检测项目
                    excel.setValue(num,startRow+numRow,startColumn+numColumn);//样品编号
                    nowLabelPosInRow++;
                    qDebug()<<"nowLabelPosInRow"<<nowLabelPosInRow;
                    bool newLine=false;
                    if(nowLabelPosInRow>labelsPerRow){//换行
                        qDebug()<<"换行";
                        nowLabelPosInRow=1;
                        startColumn=leftStart;
                        endColumn=leftStart+labelWidth-1;
                        startRow+=labelHeight;
                        endRow+=labelHeight;
                        newLine=true;
                    }
                    else{
                        startColumn+=labelWidth;
                        endColumn+=labelWidth;
                    }
        //
                    if(newLine){
                        CellRange nextRange(startRow,startColumn,usedRange.lastRow(),usedRange.lastColumn());
                        excel.copyAll(startRow,startColumn,usedRange.lastRow(),usedRange.lastColumn(),startRow+labelHeight,startColumn);
                    }
                }
                excel.document()->selectSheet("标签数据");
                excel.setValue(sampleType,sheet2Row,1);
                excel.setValue(sampleDate,sheet2Row,4);
                excel.setValue(siteName,sheet2Row,2);
                excel.setValue(testItems,sheet2Row,5);
                excel.setValue(num,sheet2Row,3);
                excel.setValue(sampleNum,sheet2Row,6);
                sheet2Row++;

            }

        }
        excel.document()->selectSheet("采样标签");
        excel.setValue(CellRange(startRow+labelHeight,startColumn,usedRange.lastRow(),usedRange.lastColumn()),"");
        QString filename=QFileDialog::getSaveFileName(nullptr,"采样标签保存为","","EXCEL文件(*.xlsx)");
        if(filename.isEmpty()) return;
        excel.saveAs(filename);
        excel.close();
//        excel.show();
        QMessageBox::information(nullptr,"","操作完成。");
    }

}


void SampleGroupingDlg::on_cancelBtn_clicked()
{
    ui->printGroup->hide();
    ui->sortGroup->show();
}


void SampleGroupingDlg::on_addSamplerBtn_clicked()
{
    QStringList samplers=itemsSelectDlg::getSelectedItems(m_samplers);
    ui->samplerEdit->setText(samplers.join("、"));
}


void SampleGroupingDlg::on_importLabelBtn_clicked()
{

}


void SampleGroupingDlg::on_radioGenerate_clicked()
{
    ui->tableView->setHeader({"检测类型","检测点位","检测项目","检测频次"});
    m_samplingSideID.clear();
    QString sql;
    ui->sortGroup->hide();
    ui->printGroup->show();
    sql="select B.sampleType, B.samplingSiteName,GROUP_CONCAT(A.parameterName SEPARATOR '、'), CONCAT(B.samplingFrequency,'次*', B.samplingPeriod,'天(剩',B.samplingPeriod-COALESCE(C.samplingPeriod,0),'天)') as c ,B.id ,C.siteOrder,B.testTypeID "
          "from task_parameters as A "
          "left join site_monitoring_info as B on A.monitoringInfoID=B.id   "
          "left join (select monitoringInfoID , siteOrder, max(samplingPeriod) as samplingPeriod from sampling_info group by monitoringInfoID,siteOrder) as C on C.monitoringInfoID=B.id "
          "where A.taskSheetID=? and sampleGroup!=0 and (B.samplingPeriod-COALESCE(C.samplingPeriod,0))!=0 "//查找条件：任务单匹配，非现场监测项目，还有周期没有采样
          "group by B.sampleType, B.samplingSiteName ,c,B.id ,C.siteOrder ;";
    ui->pageCtrl->startSql(this->tabWiget(),sql,1,{m_taskSheetID},[this](const QSqlReturnMsg&msg){

        QList<QVariant>r=msg.result().toList();

        ui->tableView->clear();
        ui->tableView->removeBackgroundColor();
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            int siteID=row.at(4).toInt();//点位ID
            int typeID=row.at(6).toInt();//检测类型ID
            int siteOrder=row.at(5).toInt();//点位序号，如果之前有打印该点位标签的话。

            if(siteOrder) m_siteUsedOrderMap[siteID]=siteOrder;
            if(siteOrder>m_typeUsedOrder.value(typeID)) m_typeUsedOrder[typeID]=siteOrder;//记录下已经使用类型点位的最大序号，用于继续往后编号（目前没用）
            if(!m_allSides.contains(siteID)) m_allSides[siteID]={row.at(1).toString(),row.first().toString()};
            //显示
            row.removeLast();
            row.removeLast();
            ui->tableView->append(row);
            ui->tableView->setCellFlag(i-1,0,siteID);//标识下点位ID
            m_siteIDTypeID[siteID]=typeID;//保存下点位的类型ID，用于后续其它操作
            if(m_samplingSideID.contains(siteID)) ui->tableView->setBackgroundColor(i-1,SAMPLING_COLOR);//标识下被选中的点位
            QString day=row.at(3).toString().split("*").last();
            QString leftDay=row.at(3).toString().split("剩").last();
            QString p=row.at(3).toString().split("*").first();
            int d=day.left(day.indexOf(QRegExp("[^0-9]"))).toInt();
            int left=leftDay.left(day.indexOf(QRegExp("[^0-9]"))).toInt();
            m_periodMap[siteID]={d,left};
            while(left>=ui->periodBox->count()){
                ui->periodBox->addItem(QString::number(ui->periodBox->count()));
            }
            m_frequencyMap[siteID]=p.left(p.indexOf(QRegExp("[^0-9]"))).toInt();
        }
    });
    //要查询和记录每个类型已经使用的点位编码
    sql="select max(A.siteOrder) , B.testTypeID "
          "from sampling_info as A "
          "left join site_monitoring_info as B on A.monitoringInfoID=B.id "
          "where taskSheetID=? "
          "group by B.testTypeID;";
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询当前点位序号时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        QList<QVariant>r=msg.result().toList();
        if(r.count()==1) {
            sqlFinished();
            return;
        }
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            m_typeOrder[row.at(1).toInt()]=row.first().toInt();
        }
        sqlFinished();
    },0,{m_taskSheetID});
    waitForSql();

}

void SampleGroupingDlg::on_radioPrint_clicked()
{
//    if(ui->samplerEdit->text().isEmpty()){
//        QMessageBox::information(nullptr,"","请至少选择一个采样人员。");
//        return;
//    }
//    QStringList samplers=ui->samplerEdit->text().split("、");
//    if(samplers.count()>2){
//        QMessageBox::information(nullptr,"","采样人员不要超出2个。");
//        return;
//    }
//    QString s="%"+samplers.join("、")+"%";
    ui->tableView->setHeader({"检测类型","检测点位","检测项目","样品编号"});
    ui->tableView->clear();
    m_samplingSideID.clear();
    QString sql;
    sql=QString("select A.monitoringInfoID,B.samplingSiteName,A.sampleNumber,A.sampleOrder ,B.sampleType,B.testTypeID ,E.seriesConnection "
                  "from sampling_info as A "
          "left join site_monitoring_info as B on A.monitoringInfoID=B.id "
                  "left join (select min(parameterID) as p,taskSheetID, sampleGroup from task_parameters group by taskSheetID, sampleGroup  )  as C on B.taskSheetID=C.taskSheetID and A.sampleOrder=C.sampleGroup "

                  "left join type_methods as D on B.taskSheetID=D.taskSheetID and B.testTypeID=D.testTypeID and C.p=D.parameterID "
                  "left join test_methods as E on E.id=D.testMethodID "
                  "where B.taskSheetID=?  "
                  "order by B.id;");
    ui->pageCtrl->startSql(this->tabWiget(),sql,0,{m_taskSheetID},[this](const QSqlReturnMsg&msg){
        ui->tableView->clear();
        ui->tableView->removeBackgroundColor();
        QList<QVariant>r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            int sampleOrder=row.at(3).toInt();
            if(!sampleOrder) continue;
            int siteID=row.first().toInt();
            QString siteName=row.at(1).toString();
            QString sampleType=row.at(4).toString();
            int typeID=row.at(5).toInt();
            int series=row.at(6).toInt();
            QString items=parameterNames(m_siteGroups.value(siteID).value(sampleOrder)).join("、");
            ui->tableView->append({sampleType,siteName,items,row.at(2)});
            ui->tableView->setCellFlag(i-1,0,siteID);
            ui->tableView->setCellFlag(i-1,1,series);//保存在串联情况
            qDebug() << "series" << series;
        }
    });
}



void SampleGroupingDlg::on_typeView_currentRowChanged(int currentRow)
{
    qDebug()<<"on_typeView_currentRowChanged"<<currentRow;
    QMap<int,QList<int>>group=m_groups.value(m_groups.keys().at(currentRow));
    QSqlQuery query(DB.database());
    qDebug()<<DB.database();
    ui->groupView->clear();
    for(auto it=group.begin();it!=group.end();++it){
        int order=it.key();

        QStringList items=parameterNames(it.value());

        ui->groupView->append({order,items.join("、")});
    }
}

