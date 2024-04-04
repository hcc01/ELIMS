#include "samplecirculationui.h"
#include "qlineedit.h"
#include "tasksheetui.h"
#include "ui_samplecirculationui.h"
#include<QInputDialog>
#include<QDialogButtonBox>
#include"samplegroupingdlg.h"
#include"exceloperator.h"
#include<QFileDialog>
SampleCirculationUI::SampleCirculationUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::SampleCirculationUI)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"任务单号","委托单位","受检单位","项目名称"});
    ui->groupBox_2->hide();
    ui->groupBox_3->hide();
    ui->samplingView->setHeader({"任务单号","样品编号","测试项目","交接状态"});
    ui->deliveryView->setHeader({"任务单号","项目名称"});
    ui->deliveryView->addContextAction("接样",[this](){
        doDeliveryReceive();
    });
    ui->deliveryView->addContextAction("打印标签",[this](){

    });
}

SampleCirculationUI::~SampleCirculationUI()
{
    delete ui;
}

void SampleCirculationUI::initCMD()
{

//    QString sql;
//    QJsonArray values;
//    if(ui->samplingBtn->isChecked()){
//        sql="select taskNum,clientName, inspectedEentityName, inspectedProject,id from test_task_info where creator=? and deleted!=1 and taskStatus=?;";
//        values={user()->name(),TaskSheetUI::WAIT_SAMPLING};
//    }
//    if(ui->deliveryBtn->isChecked()){
//        sql="select taskNum,clientName, inspectedEentityName, inspectedProject,id from test_task_info where creator=? and deleted!=1 and taskStatus=?;";
//        values={user()->name(),TaskSheetUI::SAMPLE_CIRCULATION};
//    }
//    ui->pageCtrl->startSql(this,sql,1,values,[this](const QSqlReturnMsg&msg){
//        if(msg.error()){
//            QMessageBox::information(nullptr,"查询任务单信息出错：",msg.errorMsg());
//            return;
//        }
//        QList<QVariant>r=msg.result().toList();
//        ui->tableView->clear();
//        for(int i=1;i<r.count();i++){
//            QList<QVariant>row=r.at(i).toList();
//            int id=row.last().toInt();
//            row.removeLast();
//            ui->tableView->append(row);
//            ui->tableView->setCellFlag(i-1,0,id);
//        }
//    });
    QString sql="select name from users where position&?;";
    doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查询采样人员时出错：",msg.errorMsg());
            return;
        }
        auto r=msg.result().toList();
        ui->submitterBox->clear();
        for(int i=1;i<r.count();i++){
            ui->submitterBox->addItem(r.at(i).toList().at(0).toString());
        }
        ui->submitterBox->setCurrentIndex(-1);
    },0,{CUser::Sampler});
    on_samplingBtn_clicked();
}

void SampleCirculationUI::doSamplingReceive()
{
    QDialog dlg;
    QLineEdit* edit=new QLineEdit(&dlg);
    QVBoxLayout *lay=new QVBoxLayout(&dlg);
    lay->addWidget(edit);
    dlg.setLayout(lay);
    connect(edit,&QLineEdit::returnPressed,[this, edit](){
        edit->selectAll();
        QString sql;

        bool error=false;
        QString taskNum;
        int taskSheetID;
        QString sampleNum=edit->text();
        if(sampleNum.isEmpty()) return;
        //显示相关任务单的样品交接情况
        if(!m_taskSheetID){
            sql="select B.taskSheetID, C.taskNum from sampling_info as A "
                  "left join site_monitoring_info as B on A.monitoringInfoID =B.id "
                  "left join test_task_info as C on B.taskSheetID=C.id "
                  "where sampleNumber=? ;";
            doSqlQuery(sql,[this, &error, &taskNum, edit, &taskSheetID](const QSqlReturnMsg&msg){
                if(msg.error()){
                    //                QMessageBox::information(nullptr,"查询样品时出错：",msg.errorMsg());
                    edit->setText("查询流转表时出错："+msg.errorMsg());edit->selectAll();
                    error=true;
                    sqlFinished();
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                if(r.count()!=2){
                    //                QMessageBox::information(nullptr,"查询样品时出错：","未找到此编号");
                    edit->setText("未找到此编号");edit->selectAll();
                        error=true;
                    sqlFinished();
                    return;
                }
                QList<QVariant>row=r.at(1).toList();
                taskNum=row.at(1).toString();
                taskSheetID=row.first().toInt();
                sqlFinished();
            },0,{sampleNum});
            waitForSql("正在查询样品");
            if(error) return;
            if(ui->samplingView->findInColumn(taskNum,0)<0){
                sql="select A.sampleNumber, GROUP_CONCAT( DISTINCT C.parameterName SEPARATOR ','), A.receiver from sampling_info as A "
                      "left join site_monitoring_info as B on B.id=A.monitoringInfoID "
                      "left join task_parameters as C on C.taskSheetID=B.taskSheetID and C.sampleGroup=A.sampleOrder "
                      "where C.taskSheetID=? "
                      "group by A.sampleNumber,A.receiver;";
                doSqlQuery(sql,[this, &error, taskNum, edit](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        //                    QMessageBox::information(nullptr,"查询流转表时出错：",msg.errorMsg());
                        edit->setText("查询流转表时出错："+msg.errorMsg());edit->selectAll();
                        error=true;
                        sqlFinished();
                        return;
                    }
                    QList<QVariant>r=msg.result().toList();
                    ui->samplingView->clear();
                    for(int i=1;i<r.count();i++){
                        QList<QVariant>row=r.at(i).toList();
                        ui->samplingView->append({taskNum,row.at(0).toString(),row.at(1).toString(),row.at(2).toString().isEmpty()?"":"已交接"});
                    }
                    sqlFinished();
                },0,{taskSheetID});
                waitForSql("正在获取样品表");
            }
            m_taskSheetID=taskSheetID;
        }
        qDebug()<<"m_taskSheetID"<<m_taskSheetID;
        if(error) return;
        int row=ui->samplingView->findInColumn(sampleNum,1);
        if(row<0){
            QMessageBox::information(nullptr,"error","显示出错，没有相关样品。");
            return;
        }
        if(ui->samplingView->value(row,3)=="已交接"){
            edit->setText("该样品已经交接过了！");edit->selectAll();
            return;
        }

        //保存交接记录
        sql="update sampling_info set deleiver=?, receiver=?, receiveTime=now() where sampleNumber=? ;";
        doSqlQuery(sql,[this, &error, edit](const QSqlReturnMsg&msg){
            if(msg.error()){
//                QMessageBox::information(nullptr,"保存交接记录时出错：",msg.errorMsg());

                edit->setText("更新流转表时出错："+msg.errorMsg());edit->selectAll();
                error=true;
                sqlFinished();
                return;
            }
            if(!msg.numRowsAffected()){
//                QMessageBox::information(nullptr,"保存交接记录时出错：","没样此样品编号或样品已经被交接");
                edit->setText("更新流转表时出错：无效的操作。");edit->selectAll();
                error=true;
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,{ui->submitterBox->currentText(),user()->name(),sampleNum});
        waitForSql("正在保存交接记录");
        if(error) return;



        ui->samplingView->setData(row,3,"交接完成");
    });

    connect(&dlg,&QDialog::rejected,[this](){
        //检查是否全部完成
        if(!ui->samplingView->rowCount()) return;
        if(ui->samplingView->findInColumn("",2)<0){
            int a=QMessageBox::question(nullptr,"","还有样品没有完成交接，确认退出？");
            if(a!=QMessageBox::Yes) doSamplingReceive();
            else{
                //没交接完成，需要确认原因（取消部分点位或频次、下次继续采样）
            }
        }
        else{
            //交接完成
            m_taskSheetID=0;
            //更新状态
            QString sql="update test_task_info set taskStatus=? where id=?;";
            doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"更新任务单状态时出错：",msg.errorMsg());
                    sqlFinished();
                    return;
                }
                sqlFinished();
            },0,{TaskSheetUI::TESTING,m_taskSheetID});
            waitForSql();
        }
    });
    dlg.exec();
}

void SampleCirculationUI::initDeliveryTask()
{
    QString sql;
    sql="select taskNum, inspectedProject ,id from test_task_info where taskStatus=?;";
    DealFuc f=[this](const QSqlReturnMsg&msg){
        ui->deliveryView->clear();
        QList<QVariant>r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            int taskSheetID=row.last().toInt();
            row.removeLast();
            ui->deliveryView->append(row);
            ui->deliveryView->setCellFlag(i-1,0,taskSheetID);
        }
    };
    ui->deliveryPageCtrl->startSql(this,sql,1,{TaskSheetUI::SAMPLE_CIRCULATION},f);

}

void SampleCirculationUI::doDeliveryReceive()
{
    int row=ui->deliveryView->selectedRow();
    if(row<0) return;
    QString taskNum=ui->deliveryView->value(row,0).toString();
    int taskSheetID=ui->deliveryView->cellFlag(row,0).toInt();
    QString deliver=QInputDialog::getText(nullptr,"","请输入交接人：");
    if(deliver.isEmpty()){
        QMessageBox::information(nullptr,"","交样人不能为空。");
        return;
    }
    QDate date=QDate::currentDate();
    connectDB(CMD_START_Transaction);
    QList<QVariant>r;
    bool error=false;
    QString sql;
    int order=0;
    //按所给采样日期的采样任务单顺序编号
    //先确认下这个日期中已经安排的任务单
    sql="select taskSheetID,number from sampling_task_order "
          "where samplingDate=?";
    doSqlQuery(sql,[this, &r, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            releaseDB(CMD_ROLLBACK_Transaction);
            QMessageBox::information(nullptr,"查询采样单序号时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        r=msg.result().toList();

        sqlFinished();
    },0,{date.toString("yyyy-MM-dd")});
    waitForSql();
    if(error) return;
    QHash<int,int>nums;
    for(int i=1;i<r.count();i++){
        QList<QVariant>row=r.at(i).toList();
        nums[row.at(0).toInt()]=row.at(1).toInt();//记录各个任务单的编号
    }
    if(nums.contains(taskSheetID)){
        order=nums.value(taskSheetID);//如果任务单已经编号，使用该编号（这种情况出现在前天有采样的情况）
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
        sql="insert IGNORE  into sampling_task_order(samplingDate, taskSheetID,number) values(?,?,?) ;";
        values.append(date.toString("yyyy-MM-dd"));
        values.append(taskSheetID);
        values.append(order);

    //确认客户编码
        QString clientNum;
    if(order<=26){
        clientNum=(QChar('A'+order-1));
    }
    else{
        order-=26;
        clientNum=(QChar('a'+order-1));
    }
    doSqlQuery(sql,[this, &error](const QSqlReturnMsg&msg){
            if(msg.error()){
                releaseDB(CMD_ROLLBACK_Transaction);
                QMessageBox::information(nullptr,"更新采样单序号时出错：",msg.errorMsg());
                error=true;
                sqlFinished();
                return;
            }

            sqlFinished();
        },0,values);

    if(error) return;
    releaseDB(CMD_COMMIT_Transaction);
    //送样有时候会一天送一些
    QDialog dlg;
    dlg.setWindowTitle("请选择要交接的样品：");
    dlg.resize(800,600);
    QVBoxLayout*lay=new QVBoxLayout(&dlg);
    MyTableView* view=new MyTableView(&dlg);
    view->setHeader({"样品名称","样品描述","样品数量","样品编号","样品类型","检测项目"});
    view->setEdiableColumns({0,1,2});
    lay->addWidget(view);
    QDialogButtonBox* btn=new QDialogButtonBox(&dlg);
    lay->addWidget(btn);
    dlg.setLayout(lay);
//    sql="select A.sampleName,A.sampleDesc,A.sampleCount,A.testTypeID,A.id ,A.sampleType from site_monitoring_info as A "
//          "left join sampling_info as B on A.id=B.monitoringInfoID where taskSheetID=? and B.sampleNumber is null";//过滤已经交接的
    sql="select A.sampleName,A.sampleDesc,A.sampleCount,A.testTypeID, A.id, A.sampleType, GROUP_CONCAT( B.parameterName SEPARATOR '、') "
          "from site_monitoring_info as A "
          "right join task_parameters as B on B.monitoringInfoID =A.id where A.taskSheetID=? "
          "group by A.sampleName,A.sampleDesc,A.sampleCount,A.testTypeID,A.id, A.sampleType;";
    doSqlQuery(sql,[this, &error, view](const QSqlReturnMsg&msg){
        if(msg.error()){
            error   =true;
            QMessageBox::information(nullptr,"查询任务信息时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        view->clear()   ;
        QList<QVariant>r=msg.result().toList();

        int testType;
        int siteID;
        for(int i=1;i<r.count();i++){
            QList<QVariant>row=r.at(i).toList();
            view->append({row.first().toString(),row.at(1).toString(),row.at(2).toInt(),"",row.at(5).toString(),row.at(6).toString()});
            testType=row.at(3).toInt();
            siteID=row.at(4).toInt();
            view->setCellFlag(i-1,0,testType);
            view->setCellFlag(i-1,1,siteID);
        }
        sqlFinished();
    },0,{taskSheetID});
    waitForSql();

    if(error) return;
    int typeOrder=1;
    view->addContextAction("确认接样",[this,clientNum, date,&view, deliver,  taskSheetID, &typeOrder](){
        auto indexes=view->selectedIndexes();
        if(!indexes.count()) return;
        QString sql;
        QJsonArray values;
        QString sampleNum;
        SampleGroupingDlg::testTypeNum("",0,true);
        int row=-1;
        QString typeNum;
        for(auto index:indexes){
            if(index.column()!=0) continue;
            row++;
            typeNum=SampleGroupingDlg::testTypeNum(view->cellFlag(row,0).toInt(),typeOrder);
            typeOrder++;
            if(typeNum.isEmpty()) return;
            //看下是不是已经交接了
            if(!view->value(row,3).isValid()){
                QMessageBox::information(nullptr,"","该样品已经交接过了！");
                return;
            }
            sampleNum=QString("%1%2%3").arg(clientNum).arg(date.toString("yyMMdd")).arg(typeNum);
            //显示样品编号
            view->setData(row,3,sampleNum);
            //更新样品编号和流转记录
            sql+="insert into sampling_info (sampleNumber,deleiver,receiver,receiveTime,monitoringInfoID) values(?, ?, ?, now(),?);";
            values.append(sampleNum);
            values.append(deliver);
            values.append(user()->name());
            values.append(view->cellFlag(row,1).toInt());
            //更新样品状态信息
//            if(descChanged){
                qDebug()<<"样品描述有变更 ";
                sql+="update site_monitoring_info set sampleName=?, sampleDesc =?, sampleCount=?,remark=? where id=?;";
                values.append(view->value(row,0).toString());
                values.append(view->value(row,1).toString());
                values.append(view->value(row,2).toString());
                values.append(view->value(row,3).toString());
                values.append(view->cellFlag(row,1).toInt());

//            }
//                view->deleteRow(row);
        }
        doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"更新数据库时出错：",msg.errorMsg());
                sqlFinished();
                return;
            }
            sqlFinished();
        },0,values);
        waitForSql();
        //检查下是否全部交接完成
        if(view->findInColumn(QVariant(),3)<0){
            qDebug()<<"交接完成";
            //更新状态
            sql="update test_task_info set taskStatus=? where id=?;";
            doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(nullptr,"更新任务单状态时出错：",msg.errorMsg());
                        sqlFinished();
                        return;
                    }
                    sqlFinished();
            },0,{TaskSheetUI::TESTING,taskSheetID});
            waitForSql();
            ui->deliveryView->deleteRow(ui->deliveryView->selectedRow());
        }
        //生成标签
        ExcelOperator excel;
        if(!excel.openExcel(".\\送样标签.xlsx"))
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
        int startRow=range.firstRow();
        int startColumn=range.firstColumn();
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

        //标签占用的行数
        int labelsPerRow=4;
        int nowLabelPosInRow=1;
        CellRange usedRange=excel.usedRange();
        for(auto index:view->selectedIndexes()){
            if(index.column()!=0) continue;
            int row=index.row();
//            if(toLabel){
//                excel.document()->selectSheet("采样标签");
//                excel.document()->insertImage(startRow+codeRow-1,startColumn+codeColumn-1,QZXing::encodeData(sampleNum,QZXing::EncoderFormat_QR_CODE,cellSize));//二维码
            excel.setValue(view->value(row,4),startRow+typeRow,startColumn+typeColumn);//样品类型
            excel.setValue(QDate::currentDate().toString("yyyy-MM-dd"),startRow+dateRow,startColumn+dateColumn);//采样日期
            excel.setValue(view->value(row,0).toString(),startRow+siteRow,startColumn+siteColumn);//点位名称
            excel.setValue(view->value(row,5).toString(),startRow+itemRow,startColumn+itemColumn);//检测项目
            excel.setValue(view->value(row,3).toString(),startRow+numRow,startColumn+numColumn);//样品编号
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
//            }


        }
//        excel.document()->selectSheet("采样标签");
        excel.setValue(CellRange(startRow+labelHeight,startColumn,usedRange.lastRow(),usedRange.lastColumn()),"");
        QString filename=QFileDialog::getSaveFileName(nullptr,"送样标签保存为","","EXCEL文件(*.xlsx)");
        if(filename.isEmpty()) return;
        excel.saveAs(filename);
        excel.close();

    });

    dlg.exec();

}

void SampleCirculationUI::on_sampleReceiveBtn_clicked()
{
    QString taskNum;
    int row=ui->tableView->selectedRow();
    if(row<0) return ;
    bool error=false;
    taskNum=ui->tableView->value(row,0).toString();
    //更新任务单状态
    doSqlQuery("UPDATE test_task_info set taskStatus=? where taskNum=?",[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"updateTaskStatus error",msg.result().toString());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{TaskSheetUI::TESTING,taskNum});
    waitForSql();
    if(error) return;
    //写入交接表
    doSqlQuery("insert into  sample_circulate (taskNum, receiveTime) values(?,now());",[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"写入交接表出错：",msg.result().toString());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,{taskNum});
    waitForSql();
    if(error) return;
    initCMD();
}


void SampleCirculationUI::on_refleshBtn_clicked()
{

}


void SampleCirculationUI::on_samplingBtn_clicked()
{
//    if(ui->samplingBtn->isChecked()) return;
//    ui->samplingBtn->setChecked(true);
//    initCMD();
    ui->groupBox_2->show();
    ui->groupBox->hide();
    ui->groupBox_3->hide();
    QString sql;


}


void SampleCirculationUI::on_deliveryBtn_clicked()
{
//    if(ui->deliveryBtn->isChecked()) return;
//    ui->deliveryBtn->setChecked(true);
//    initCMD();
    ui->groupBox_3->show();
    ui->groupBox->hide();
    ui->groupBox_2->hide();
    initDeliveryTask();
}


void SampleCirculationUI::on_samplingReceiveBtn_clicked()
{
    QString submitter=ui->submitterBox->currentText();
    if(submitter.isEmpty()) {
        QMessageBox::information(nullptr,"","请选择交样人。");
        return;
    }
    doSamplingReceive();

}


void SampleCirculationUI::on_nextBtn_clicked()
{
    ui->samplingView->clear();
}


void SampleCirculationUI::on_printLabelBtn_clicked()
{

}

