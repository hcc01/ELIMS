#include "testinfoeditor.h"
#include "ui_testinfoeditor.h"
#include<QMessageBox>
#include"implementingstandardselectdlg.h"
#include<QInputDialog>
#include<global.h>
testInfoEditor::testInfoEditor(TestInfo *info, TabWidgetBase *parent) :
    QDialog(parent),
    SqlBaseClass(parent),
    ui(new Ui::testInfoEditor),
    m_info(info),
    m_manualOperate(true)
{
    ui->setupUi(this);
    ui->testItemEdit->setPlaceholderText("多个检测参数以“、”隔开");
    if(info){
        if(info->delieveryTest){//送样检测
            ui->groupBox_PinCi->hide();
            ui->groupBox_smapleSite->hide();
        }
        else{
            ui->groupBox_SampleDesc->hide();
        }
    }
}

testInfoEditor::~testInfoEditor()
{
    delete ui;
}

void testInfoEditor::init()
{
    QList<QVariant> fieldNames;
    doSql("SELECT testField, id from test_field where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            sqlFinished();
            return;
        }
        fieldNames=msg.result().toList();

        sqlFinished();
    });
    waitForSql();
    ui->testFiledBox->clear();
    m_testFieldIDs.clear();
    for(int i=1;i<fieldNames.count();i++){
        m_testFieldIDs.append(fieldNames.at(i).toList().at(1).toInt());
        ui->testFiledBox->addItem(fieldNames.at(i).toList().at(0).toString());

    }
}

void testInfoEditor::load(TestInfo* info)
{
    m_info=info;
    if(info->delieveryTest){//送样检测
        ui->groupBox_PinCi->hide();
        ui->groupBox_smapleSite->hide();
        ui->groupBox_SampleDesc->show();
    }
    else{
        ui->groupBox_PinCi->show();
        ui->groupBox_smapleSite->show();
        ui->groupBox_SampleDesc->hide();
    }
    int n=m_testFieldIDs.indexOf(m_info->testFieldID);
    qDebug()<<m_testFieldIDs;
    qDebug()<<m_info->testFieldID;
    if(n>=0){
        ui->testFiledBox->setCurrentIndex(n);
        ui->testTypeBox->setCurrentIndex(m_testTypeIDs.indexOf(m_info->testTypeID));
        ui->sampleTypeBox->setCurrentText(m_info->sampleType);
        ui->samplingPosCountBox->setValue(m_info->samplingSiteCount);
        ui->samplingFrequencyBox->setValue(m_info->samplingFrequency);
        ui->samplingPeriodCountBox->setValue(m_info->samplingPeriod);
        ui->samplePosEdit->setText(m_info->samplingSites);
        ui->testItemEdit->setText(m_info->monitoringParameters.join("、"));
        ui->remarkEdit->setText(m_info->remark);
        ui->standardNameEidt->setText(m_info->limitStandard);
        ui->sampleNameEdit->setText(info->sampleName);
        ui->sampleDescEdit->setText(info->sampleDesc);
        ui->sampleCountSpin->setValue(info->sampleCount);
        m_limitStandardID=m_info->limitStandardID;
    }
}

void testInfoEditor::on_testInofOkBtn_clicked()
{
    if(!m_info) return;
    QString s=ui->testItemEdit->toPlainText();
    s.replace("\n","、");
    toStdParameterName(s);
    ui->testItemEdit->setText(s);
    QStringList items=s.split("、");
        if(!items.count()){
            return;
        }
        QList<int>duplicateItems;
        m_monitoringParameters.clear();
        m_monitoringParameterIDs.clear();
        for(int i=0;i<items.count();i++){
            auto item=items.at(i);
            if(item.isEmpty()) continue;
            m_monitoringParameters.append(item);//这里操作可以避免出现空的项目，导致与ID数量不一致
            QString sql;

//            sql=QString("select A.id from (select id, parameterName, testFieldID from detection_parameters where testFieldID=%2) as A "
//                                  "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID "
//                                  "where parameterName='%1' or alias='%1';").arg(item).arg(m_testFieldIDs.at(ui->testFiledBox->currentIndex()));
            sql=QString("select A.id from (select id, parameterName, testFieldID from detection_parameters where testFieldID=?) as A "
                          "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID "
                          "where parameterName=? or alias=?;");

            bool error=false;
            // 发送异步的数据库查询请求
            doSql(sql,[this,item,i,items,&error, &duplicateItems](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"查找输入项目时出错：",msg.result().toString());
                    error=true;
                    sqlFinished();
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                if(r.count()==1){
                    QMessageBox::information(this,"项目错误",item+"不能被识别。");
                    error=true;
                    sqlFinished();
                    return;
                }
                //检查是否重复项目：
                int id=r.at(1).toList().at(0).toInt();
                if(m_monitoringParameterIDs.contains(id)){
                    duplicateItems.append(i);
                }
                else m_monitoringParameterIDs.append(id);
                if(i==items.count()-1){//最后一个项目查询完成，如果没有出错，执行下面操作
//                    ui->testItemEdit->clear();
//                    ui->testItemEdit->append(items.join("、"));
                    qDebug()<<"m_monitoringParameterIDs"<<m_monitoringParameterIDs;

                }
                sqlFinished();
            },0,{m_testFieldIDs.at(ui->testFiledBox->currentIndex()),item,item});
            // 等待异步查询完成
            waitForSql();
            if(duplicateItems.count()){
                for(int n:duplicateItems){
                    m_monitoringParameters.removeOne(items.at(n));
                    QMessageBox::information(nullptr,"",QString("%1是重复检测项目， 已自动删除。").arg(items.at(n)));
                }
            }
            if(error) return;
        }
        qDebug()<<m_monitoringParameterIDs;
//    m_monitoringParameters=items;

    if(m_monitoringParameterIDs.isEmpty()){
        QMessageBox::information(this,"error","请选择检测项目！");
        return;
    }

//    if(m_samplingSites.count()&&ui->samplingPosCountBox->value()!=m_samplingSites.count()){
//        QMessageBox::information(this,"error","点位数量不对，请确认");
//        return;
//    }//这个先不处理
    TestInfo* info=m_info;
    QString sites=ui->samplePosEdit->toPlainText();
    if(sites.split("、").count()!=ui->samplingPosCountBox->value()){
        int a=QMessageBox::question(nullptr,"","检测到点位数量和点位名称不匹配，是否继续？");
        if(a!=QMessageBox::Yes) return;
    }
    info->sampleType=ui->sampleTypeBox->currentText();
    info->testFieldID=m_testFieldIDs.at(ui->testFiledBox->currentIndex());
    info->testTypeID=m_testTypeIDs.at(ui->testTypeBox->currentIndex());

    info->monitoringParameters=m_monitoringParameters;
    info->parametersIDs=m_monitoringParameterIDs;
    info->limitStandard=ui->standardNameEidt->text();
    info->limitStandardID=m_limitStandardID;
    info->remark=ui->remarkEdit->text();
    info->samplingFrequency=ui->samplingFrequencyBox->value();
    info->samplingSites=ui->samplePosEdit->toPlainText();
    info->samplingSiteCount=ui->samplingPosCountBox->value();
    info->samplingPeriod=ui->samplingPeriodCountBox->value();
    info->sampleName=ui->sampleNameEdit->text();
    info->sampleCount=ui->sampleCountSpin->value();
    info->sampleDesc=ui->sampleDescEdit->text();

//    m_testInfo.append(info);
//    ui->testInfoTableView->append(info->infoList());
    m_info=nullptr;
    accept();
}


void testInfoEditor::on_testFiledBox_currentIndexChanged(int index)
{
//    if(!m_testFieldIDs.count()) return;
//    if(!m_manualOperate) return;
    QString sql=QString("SELECT testType, id from test_type where testFieldID=%1;").arg(m_testFieldIDs.at(index));

    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            sqlFinished();
            return;
        }
        QList<QVariant> typeNames=msg.result().toList();
        ui->testTypeBox->clear();
        m_testTypeIDs.clear();
        for(int i=1;i<typeNames.count();i++){
            ui->testTypeBox->addItem(typeNames.at(i).toList().at(0).toString());
            m_testTypeIDs.append(typeNames.at(i).toList().at(1).toInt());
        }
         sqlFinished();
    });
    waitForSql();
}


void testInfoEditor::on_testTypeBox_currentTextChanged(const QString &arg1)
{

    QString sql=QString("SELECT sampleType from sample_type where testTypeID=(select id from test_type where testType='%1');").arg(arg1);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());sqlFinished();
            return;
        }
        QList<QVariant> typeNames=msg.result().toList();
        ui->sampleTypeBox->clear();
        for(int i=1;i<typeNames.count();i++){
            ui->sampleTypeBox->addItem(typeNames.at(i).toList().at(0).toString());
        }
        sqlFinished();
    });
    waitForSql();
}


void testInfoEditor::on_testItemAddBtn_clicked()
{
    int index=ui->comboBox->currentIndex();
    switch (index) {
    case 0:
    {
        ImplementingStandardSelectDlg dlg;

        connect(&dlg,&ImplementingStandardSelectDlg::doSql,[this](const QString &sql, DealFuc f, int p, const QJsonArray &values){
            doSql(sql,f,p,values);
        });
        connect(&dlg,&ImplementingStandardSelectDlg::selectDone,this,[this](const QStringList items,const QList<int>&IDs,const QString&standardName,int limitStandardID){
//            ui->testItemEdit->clear();
            ui->testItemEdit->setText(ui->testItemEdit->toPlainText()+"、"+items.join("、"));
//            m_monitoringParameters=items;
//            m_monitoringParameterIDs=IDs;
            m_limitStandardID=limitStandardID;
            ui->standardNameEidt->setText(standardName);
        });
        dlg.init();
        dlg.exec();
    }
    break;
    case 2:
    {
//        QStringList items=QInputDialog::getText(this,"","请输入检测项目，以“、”隔开")
//                                .remove(' ').replace("，",",").replace("’","'").replace("（","(").replace("）",")").split("、");
//        if(!items.count()){
//            return;
//        }
//        m_monitoringParameterIDs.clear();
//        for(int i=0;i<items.count();i++){
//            auto item=items.at(i);
//            if(item.isEmpty()) continue;
//            QString sql;
//            QEventLoop loop;
//            connect(this, &testInfoEditor::doSqlFinished, &loop, &QEventLoop::quit);
//            sql=QString("select A.id from (select id, parameterName, testFieldID from detection_parameters where testFieldID=%2) as A "
//                                  "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID "
//                                  "where parameterName='%1' or alias='%1';").arg(item).arg(m_testFieldIDs.at(ui->testFiledBox->currentIndex()));
////            sql=QString("SELECT id from detection_parameters where testFieldID =%2 and (parameterName='%1' or alias='%1' or abbreviation='%1') ")
////                      .arg(item).arg(m_testFieldIDs.at(ui->testFiledBox->currentIndex()));

//            bool error=false;
//            m_monitoringParameterIDs.clear();
//            // 发送异步的数据库查询请求
//            doSql(sql,[this,item,i,items,&error](const QSqlReturnMsg&msg){
//                if(msg.error()){
//                    QMessageBox::information(this,"查找输入项目时出错：",msg.result().toString());
//                    error=true;
//                    emit doSqlFinished();
//                    return;
//                }
//                QList<QVariant>r=msg.result().toList();
//                if(r.count()==1){
//                    QMessageBox::information(this,"项目错误",item+"不能被识别。");
//                    error=true;
//                    emit doSqlFinished();
//                    return;
//                }
//                m_monitoringParameterIDs.append(r.at(1).toList().at(0).toInt());
//                if(i==items.count()-1){//最后一个项目查询完成，如果没有出错，执行下面操作
//                    ui->testItemEdit->clear();
//                    ui->testItemEdit->append(items.join("、"));
//                }
//                emit doSqlFinished();
//            });
//            // 等待异步查询完成
//            loop.exec();
//            if(error) return;
//        }
//        m_monitoringParameters=items;
    }
        break;
    case 1:
    {

        emit doSql(QString("select parameterName,alias,abbreviation from detection_parameters where testFieldID=%1);").arg(m_testFieldIDs.at(ui->testFiledBox->currentIndex())),[&](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                return;
            }
        });
    }
        break;
    default:
        break;
    }
}


void testInfoEditor::on_sampleSiteSeclectBtn_clicked()
{
    int index=ui->siteSelectBox->currentIndex();
    switch(index){
    case 0:
    {
//        QString sites=QInputDialog::getText(this,"","请输入点位名称，多个点位以“、”隔开");
//        if(sites.isEmpty()) return;
//        m_samplingSites.append(sites);
//        ui->samplePosEdit->clear();
//        ui->samplePosEdit->setText(m_samplingSites);
    }
    break;
    case 1:
    {

    }
    break;

    }
}


void testInfoEditor::on_cancelBtn_clicked()
{
    reject();
}

