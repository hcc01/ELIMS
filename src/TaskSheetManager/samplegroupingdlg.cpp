#include "samplegroupingdlg.h"
#include "qcalendarwidget.h"
#include "ui_samplegroupingdlg.h"
#include"itemsselectdlg.h"
#include"QExcel.h"
#include "QZXing.h"
#include<QDir>
SampleGroupingDlg::SampleGroupingDlg(TabWidgetBase *parent) :
    QDialog(parent),
    SqlBaseClass(parent),
    ui(new Ui::SampleGroupingDlg)
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
        QString type=ui->typeView->currentItem()->text();
        auto indexs=ui->groupView->selectedIndexes();
        if(!indexs.count()) return;
        QList<int>rows;
        for(int i=0;i<indexs.count();i++){
            if(indexs.at(i).column()!=0) continue;
            rows.append(indexs.at(i).row());
        }
        if(rows.count()<2) return;
        qSort(rows);
        QMap<int,QStringList>groups=m_typeItemGroupMap.value(type);
        QStringList items;
        int first=rows.first();
        for(int row:rows){
            int key=row+1;//row从0开始计；key从1开始计
            items.append(groups.value(key));
            if(row!=first) groups.remove(key);
        }
        for(int row:rows){
            int key=row+1;
            if(row!=first) groups.remove(key);
        }
        groups[first+1]=items;
        //其它行重新排序
        for(int i=rows.at(0)+1;i<groups.count();i++){
            int pre=groups.keys().at(i-1);//QMap有排序，rows之前也排过序，可以这么操作
            int now=groups.keys().at(i);
            items=groups.value(now);
            groups.remove(now);
            while(now-1>pre) now--;
            groups[now]=items;
        }
        m_typeItemGroupMap[type]=groups;
        on_typeView_itemClicked(ui->typeView->currentItem());
    });
    ui->groupView->addContextAction("拆分分组",[this](){
        int row=ui->groupView->selectedRow();
        if(row<0) return;
        QString type=ui->typeView->currentItem()->text();
        QStringList items;
        QMap<int,QStringList>groups=m_typeItemGroupMap.value(type);
        items=groups.value(ui->groupView->value(row,0).toInt());
        QStringList splitItems=itemsSelectDlg::getSelectedItems(items,"请选择要分离出去的项目：");
        for(const QString&item:splitItems){
            items.removeAll(item);
        }
        groups[ui->groupView->value(row,0).toInt()]=items;
        groups[groups.lastKey()+1]=splitItems;
        m_typeItemGroupMap[type]=groups;
        on_typeView_itemClicked(ui->typeView->currentItem());
    });
    ui->groupView->addContextAction("现场监测",[this](){
        int row=ui->groupView->selectedRow();
        if(row<0) return;
        QString type=ui->typeView->currentItem()->text();
        QStringList items;
        QMap<int,QStringList>groups=m_typeItemGroupMap.value(type);
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

            }
            groups.remove(groups.lastKey());
        }

        m_typeItemGroupMap[type]=groups;
        on_typeView_itemClicked(ui->typeView->currentItem());
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

void SampleGroupingDlg::init(QString taskNum, const QStringList &samplers)
{
    QString sql;
    m_taskNum=taskNum;
    m_samplers=samplers;
    setWindowTitle(QString("任务单号 - %1").arg(taskNum));
    sql="select F.testType,A.parameterID,B.parameterName,A.sampleOrder,A.subpackage, E.id,E.sampleGroup,A.testTypeID,A.taskSheetID ,A.clientID, A.inspectedEentityID ,GROUP_CONCAT(A.monitoringInfoID SEPARATOR ','), A.testTypeID, E.seriesConnection "
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
}




void SampleGroupingDlg::on_typeView_itemClicked(QListWidgetItem *item)//显示每个类型的分组情况
{
    if(!item) return;
    QMap<int,QStringList>group=m_typeItemGroupMap.value(item->text());
    ui->groupView->clear();
    for(auto it=group.begin();it!=group.end();++it){
        ui->groupView->append({it.key(),it.value().join("、")});
    }
}


void SampleGroupingDlg::on_printBtn_clicked()//打印标签
{
    on_radioGenerate_clicked();

}


void SampleGroupingDlg::on_saveBtn_clicked()//保存分组
{
    QString sql;
    for(auto it=m_typeItemGroupMap.begin();it!=m_typeItemGroupMap.end();++it){
        QString type=it.key();
        QMap<int,QStringList>itemMap=it.value();
        for(auto it=itemMap.begin();it!=itemMap.end();++it){
            for(auto item:it.value()){
                sql="update task_methods set sampleOrder=? where testTypeID= ? and parameterID=? and taskSheetID=?;";
                doSql(sql,[this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"保存样品号时出错：",msg.errorMsg());
                        sqlFinished();
                        return;
                    }
                    sqlFinished();
                },0,{it.key(),m_typeIdMap.value(type),m_itemIdMap.value(item),m_taskSheetID});
                waitForSql();
            }
        }
    }
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
        int nowType=0;
        int typeOrder=0;
        QString sampleNum;//实际样品编号
        QString showedNum;//用于记录的编号，不识别样品项目序号
        QString dateNum;
        QString typeNum;
        QString orderNum;
        QString SeriesNum;
        QString roundNum;
        QString sql;
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
        if(ui->checkBox->isChecked()){//全部点位
            siteIDs=m_allSides.keys();
        }
        else siteIDs=m_samplingSideID;
        for(int siteID:siteIDs){
            typeID=m_siteIDTypeID.value(siteID);
            typeNum=QString("%1").arg(QChar('A'-1+typeID));//类型编号
            if(m_siteUsedOrderMap.contains(siteID)){//点位的序号已经存在（也就是之前已经采过一些周期），使用之前的点位序号
                typeOrder=m_siteUsedOrderMap.value(siteID);
            }
            else{//需要给新的点位号
                if(typeID!=nowType){
                    typeOrder=m_typeUsedOrder.value(typeID)+1;
                    nowType=typeID;
                }

                else typeOrder++;//当前类型的点位序号
            }
            QString siteNum=QString("%1").arg(typeOrder,2,10,QChar('0'));//类型编号
            QString type;
            for(auto it=m_typeIdMap.begin();it!=m_typeIdMap.end();++it){
                if(it.value()==typeID){
                    type=it.key();
                    break;
                }
            }
            if(type.isEmpty()){
                QMessageBox::information(nullptr,"error","查找点位类型失败。");
                return;
            }

            int day=1;
            int f=m_frequencyMap.value(siteID);
            day=m_periodMap[siteID].first;
    //        if(ui->periodBox->currentIndex()!=0){
    //            day=ui->periodBox->currentIndex();
    //            if(day>m_periodMap[siteID].first) continue;//超出当前点位的最大周期，无效。
    //        }
            int leftDay=m_periodMap[siteID].second;
            int nowPeriod;
            int start=0;;

            for(int i=1;i<=day;i++){
                if(i<=day-leftDay) continue;//已完成的，跳过
                start++;//采第几天
                nowPeriod=i;//当前采的第几个周期（总体的第几天）
                if(ui->periodBox->currentIndex()){//不采全部周期
                    dateNum=ui->dateEdit->date().toString("yyMMdd");//时间编号;只选择其中一天采样的，采样日期就是当天
                    if(start>1) break;//本次不采全部周期（只采1天）的，就结束了。
                }
                else dateNum=ui->dateEdit->date().addDays(start-1).toString("yyMMdd");//时间编号；多个周期连续采样的，时间要推移



                QList<int>groupOrders=m_typeItemGroupMap.value(type).keys();
                for(int samplerOrder:groupOrders){
                    if(samplerOrder==0) continue;//现场监测，没有样品编号
                    QString orderNum=QString("%1").arg(samplerOrder,2,10,QChar('0'));//样品编号
                    int Series=m_seriesConnection.value(samplerOrder) ;
                    roundNum="";
                    SeriesNum="";
                    if(Series){
                        for(int i=0;i<Series;i++){
                            SeriesNum.append(QString("%1").arg(QChar('a'+Series)));//串联，样品保存编号直接加上abc，在显示编号是分开打印。
                        }
                    }
                    for(int i=0;i<f;i++){
                        if(f>1)  roundNum=QString("-%1").arg(i+1);//频次编号；样品编号到此完成
                        //输出标签
                        //保存的编号和显示的编号不一样，保存的编号没有串联信息，有样品序号，显示的编号有串联信息，没有样品序号。
                        sampleNum=QString("%1%2%3%4%5%6").arg(dateNum).arg(m_clientID).arg(typeNum).arg(siteNum).arg(orderNum).arg(roundNum);
                        showedNum=QString("%1%2%3%4%5%6").arg(dateNum).arg(m_clientID).arg(typeNum).arg(siteNum).arg(SeriesNum).arg(roundNum);


                        sql="insert into sampling_info(monitoringInfoID, samplingSiteName, samplingRound, samplingPeriod, sampleNumber, samplers,siteOrder,sampleOrder) "
                              "values(?,?,?,?,?,?,?,?) ;";
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
                        },0,{siteID,m_allSides.value(siteID).first,i+1,nowPeriod,sampleNum,ui->samplerEdit->text(),typeOrder,samplerOrder});
                        waitForSql();
                        if(error){
                            return;
                        }
                    }

                }

    //            if(ui->periodBox->currentIndex()!=0) break;//只选其中一个周期的（这个在上面使用start>1来判断处理
            }
        }
    }
    else{//打印标签
        auto indexs=ui->tableView->selectedIndexes();
        if(!indexs.count()){
            QMessageBox::information(nullptr,"","请选择需要打印的标签");
            return;
        }
        EXCEL.setScreenUpdating(false);
        EXCEL.setEnableEvents(false);
        Range* usedRange;
        Range* range;
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

        WorkBook* book=EXCEL.OpenBook(".\\采样标签.xlsx",QVariant(),true);
        if(!book){
            QMessageBox::information(nullptr,"无法打开样品标签文件:",EXCEL.LastError());
            return;
        }
        WorkSheet* sheet=book->sheet(1);
        if(!sheet){
            QMessageBox::information(nullptr,"无法打开样品标签文件:",EXCEL.LastError());
            return;
        }
         usedRange=sheet->usedRange();
        range=usedRange->find("[开始]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位标签开始位置:",EXCEL.LastError());
            return;
        }
        range->clear();
        startRow=range->row();
        startColumn=range->column();
        qDebug()<<"startRow"<<startRow;
        qDebug()<<"startColumn"<<startColumn;
        range=usedRange->find("[结束]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位标签结束位置:",EXCEL.LastError());
            return;
        }
        range->clear();

        int endRow=range->row();
        int endColumn=range->column();
        int leftStart=startColumn;
        int labelWidth=endColumn-startColumn+1;
        int labelHeight=endRow-startRow+1;
        delete range;

        range=usedRange->find("[二维码]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位标签二维码位置:",EXCEL.LastError());
            return;
        }
        range->setValue("");
        int codeRow=range->row()-startRow;
        int codeColumn=range->column()-startColumn;
        delete range;

        range=usedRange->find("[样品类型]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位检测类型位置:",EXCEL.LastError());
            return;
        }
        int typeRow=range->row()-startRow;
        int typeColumn=range->column()-startColumn;
        delete range;

        range=usedRange->find("[点位名称]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位点位名称位置:",EXCEL.LastError());
            return;
        }
        int siteRow=range->row()-startRow;
        int siteColumn=range->column()-startColumn;
        delete range;

        range=usedRange->find("[采样日期]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位采样日期位置:",EXCEL.LastError());
            return;
        }
        int dateRow=range->row()-startRow;
        int dateColumn=range->column()-startColumn;
        delete range;

        range=usedRange->find("[样品编号]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位样品编号位置:",EXCEL.LastError());
            return;
        }
        int numRow=range->row()-startRow;
        int numColumn=range->column()-startColumn;
        delete range;

        range=usedRange->find("[检测项目]");
        if(!range){
            QMessageBox::information(nullptr,"无法定位检测项目位置:",EXCEL.LastError());
            return;
        }
        int itemRow=range->row()-startRow;
        int itemColumn=range->column()-startColumn;
        delete range;

        Range *nowRange=nullptr;
        Range* nextRange=nullptr;


        int labelsPerRow=4;
        int nowLabelPosInRow=1;

        for(auto index:indexs){
            if(index.column()!=0) continue;
            int row=index.row();
            QString sampleType=ui->tableView->value(row,0).toString();
            QString siteName=ui->tableView->value(row,1).toString();
            QString testItems=ui->tableView->value(row,2).toString();
            QString sampleNum=ui->tableView->value(row,3).toString();
            QString num1,num2;
            num1=sampleNum.left(12);
            num2=sampleNum.mid(14);
            QStringList series;
            QStringList showSampleNums;
            if(num2.count()){
                while(num2.at(0)!=QChar('-')){//处理串联
                    series.append(num2.at(0));
                    num2=num2.mid(1);
                }
            }
            QString sampleDate=QString("20%1-%2-%3").arg(sampleNum.left(2)).arg(sampleNum.mid(2,2)).arg(sampleNum.mid(4,2));
            if(series.size()){
                for(auto s:series){
                    showSampleNums.append(QString("%1%2%3").arg(num1).arg(s).arg(num2));
                }
            }
            else showSampleNums.append(QString("%1%2").arg(num1).arg(num2));
            for(auto num:showSampleNums){
                if(toLabel){
                    QString path="./qrcode.png";
                    QZXing::encodeData(sampleNum,QZXing::EncoderFormat_QR_CODE,QSize(100, 100)).save(path);
                    path=QDir::current().absoluteFilePath(path);
                    sheet->insertPic(path,startRow+codeRow,startColumn+codeColumn);//二维码
                    sheet->setValue(sampleType,startRow+typeRow,startColumn+typeColumn);//样品类型
                    sheet->setValue(sampleDate,startRow+dateRow,startColumn+dateColumn);//采样日期
                    sheet->setValue(siteName,startRow+siteRow,startColumn+siteColumn);//点位名称
                    sheet->setValue(testItems,startRow+itemRow,startColumn+itemColumn);//检测项目
                    sheet->setValue(num,startRow+numRow,startColumn+numColumn);//样品编号
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
                        nextRange=sheet->selectRange(startRow,startColumn,endRow,endColumn);
                        range=nextRange->entireRow();
                        range->copy();
                        range=sheet->selectRow(startRow);
                        range->insert();
                        delete range;
                        delete nextRange;
                        nextRange=nullptr;
                    }
                }
                else{
                    sheet=book->sheet(2);
                    if(!sheet){
                        QMessageBox::information(nullptr,"表格错误","无法定位标签数据表格。");
                        return;
                    }
                    sheet->setValue(sampleType,startRow,1);
                    sheet->setValue(sampleDate,startRow,4);
                    sheet->setValue(siteName,startRow,2);
                    sheet->setValue(testItems,startRow,5);
                    sheet->setValue(num,startRow,3);
                    startRow++;
                }
            }

        }
        range=sheet->selectRange(startRow+labelHeight,startColumn,endRow+labelHeight,endColumn);
        range=range->entireRow();
        range->Delete();
        EXCEL.setScreenUpdating(true);
        EXCEL.setEnableEvents(true);
        QMessageBox::information(nullptr,"","操作完成。");
        delete book;
    }

//    QString sql;


//    sql="insert into sampling_info(monitoringInfoID, samplingSigeName, samplingRound, samplingPeriod, sampleNumber, samplers) ";


////    this->tabWiget()->connectDB(CMD_START_Transaction);



//    this->tabWiget()->releaseDB(CMD_COMMIT_Transaction);



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
    sql="select B.sampleType, B.samplingSiteName,GROUP_CONCAT(A.parameterName SEPARATOR '、'), CONCAT(B.samplingFrequency,'次*', B.samplingPeriod,'天(剩',B.samplingPeriod-COALESCE(C.samplingPeriod,0),'天)') as c ,B.id ,C.siteOrder "
          "from task_methods as A left join site_monitoring_info as B on A.monitoringInfoID=B.id   "
          "left join (select monitoringInfoID , siteOrder, max(samplingPeriod) as samplingPeriod from sampling_info group by monitoringInfoID,siteOrder) as C on C.monitoringInfoID=B.id "
          "where A.taskSheetID=? and sampleOrder!=0 and (B.samplingPeriod-COALESCE(C.samplingPeriod,0))!=0 "
          "group by B.sampleType, B.samplingSiteName ,c,B.id ,C.siteOrder ;";
    ui->pageCtrl->startSql(this->tabWiget(),sql,1,{m_taskSheetID},[this](const QSqlReturnMsg&msg){

        QList<QVariant>r=msg.result().toList();

        ui->tableView->clear();
        ui->tableView->removeBackgroundColor();
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            int siteID=row.at(4).toInt();
            int typeID=m_siteIDTypeID.value(siteID);
            int siteOrder=row.at(5).toInt();
            if(siteOrder) m_siteUsedOrderMap[siteID]=siteOrder;
            if(siteOrder>m_typeUsedOrder.value(typeID)) m_typeUsedOrder[typeID]=siteOrder;//记录下类型点位序号被使用的情况
            if(!m_allSides.contains(siteID)) m_allSides[siteID]={row.at(1).toString(),row.first().toString()};
            row.removeLast();
            ui->tableView->append(row);
            ui->tableView->setCellFlag(i-1,0,siteID);
            if(m_samplingSideID.contains(siteID)) ui->tableView->setBackgroundColor(i-1,SAMPLING_COLOR);
            QString day=row.at(3).toString().split("*").last();
            QString leftDay=row.at(3).toString().split("剩").last();
            QString p=row.at(3).toString().split("*").first();
            int d=day.left(day.indexOf(QRegExp("[^0-9]"))).toInt();
            int left=leftDay.left(day.indexOf(QRegExp("[^0-9]"))).toInt();
            m_periodMap[siteID]={d,left};
            while(d>=ui->periodBox->count()){
                ui->periodBox->addItem(QString::number(ui->periodBox->count()));
            }
            m_frequencyMap[siteID]=p.left(p.indexOf(QRegExp("[^0-9]"))).toInt();
        }
    });
}


void SampleGroupingDlg::on_radioPrint_clicked()
{
    if(ui->samplerEdit->text().isEmpty()){
        QMessageBox::information(nullptr,"","请至少选择一个采样人员。");
        return;
    }
    QStringList samplers=ui->samplerEdit->text().split("、");
    if(samplers.count()>2){
        QMessageBox::information(nullptr,"","采样人员不要超出2个。");
        return;
    }
    QString s="%"+samplers.join("、")+"%";
    ui->tableView->clear();
    m_samplingSideID.clear();
    QString sql;
    sql=QString("select A.monitoringInfoID,A.samplingSiteName,A.sampleNumber,A.sampleOrder ,B.sampleType from sampling_info as A "
          "left join site_monitoring_info as B on A.monitoringInfoID=B.id "
                  "where B.taskSheetID=? and A.samplers like ? "
                  "order by B.id;");
    ui->pageCtrl->startSql(this->tabWiget(),sql,0,{m_taskSheetID,s},[this](const QSqlReturnMsg&msg){
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
            int typeID=m_siteIDTypeID.value(siteID);
            QString type;
            for(auto it=m_typeIdMap.begin();it!=m_typeIdMap.end();++it){
                if(it.value()==typeID){
                    type=it.key();
                    break;
                }
            }
            QString items=m_typeItemGroupMap.value(type).value(sampleOrder).join("、");
            ui->tableView->append({sampleType,siteName,items,row.at(2)});
            qDebug()<<row.at(2);
            ui->tableView->setCellFlag(i-1,0,siteID);
        }
    });
}


