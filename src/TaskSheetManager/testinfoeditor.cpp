#include "testinfoeditor.h"
#include "ui_testinfoeditor.h"
#include<QMessageBox>
#include"implementingstandardselectdlg.h"
#include<QInputDialog>
testInfoEditor::testInfoEditor(TestInfo *info, int inspectedEentityID, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::testInfoEditor),
    m_info(info),
    m_inspectedEentityID(inspectedEentityID)
{
    ui->setupUi(this);
    ui->testItemEdit->setPlaceholderText("多个检测参数以“、”隔开");
}

testInfoEditor::~testInfoEditor()
{
    delete ui;
}

void testInfoEditor::init()
{
    emit doSql("SELECT testField, id from test_field where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant> fieldNames=msg.result().toList();
        ui->testFiledBox->clear();
        m_testFieldIDs.clear();
        for(int i=1;i<fieldNames.count();i++){
            m_testFieldIDs.append(fieldNames.at(i).toList().at(1).toInt());
            ui->testFiledBox->addItem(fieldNames.at(i).toList().at(0).toString());

        }
    });
}

void testInfoEditor::on_testInofOkBtn_clicked()
{
        QStringList items=ui->testItemEdit->toPlainText().remove(' ').replace("，",",").replace("’","'").replace("（","(").replace("）",")").split("、");
        if(!items.count()){
            return;
        }
        m_monitoringParameterIDs.clear();
        for(int i=0;i<items.count();i++){
            auto item=items.at(i);
            if(item.isEmpty()) continue;
            QString sql;
            QEventLoop loop;
            connect(this, &testInfoEditor::doSqlFinished, &loop, &QEventLoop::quit);
            sql=QString("select A.id from (select id, parameterName, testFieldID from detection_parameters where testFieldID=%2) as A "
                                  "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID "
                                  "where parameterName='%1' or alias='%1';").arg(item).arg(m_testFieldIDs.at(ui->testFiledBox->currentIndex()));
//            sql=QString("SELECT id from detection_parameters where testFieldID =%2 and (parameterName='%1' or alias='%1' or abbreviation='%1') ")
//                      .arg(item).arg(m_testFieldIDs.at(ui->testFiledBox->currentIndex()));

            bool error=false;
            // 发送异步的数据库查询请求
            doSql(sql,[this,item,i,items,&error](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(this,"查找输入项目时出错：",msg.result().toString());
                    error=true;
                    emit doSqlFinished();
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                if(r.count()==1){
                    QMessageBox::information(this,"项目错误",item+"不能被识别。");
                    error=true;
                    emit doSqlFinished();
                    return;
                }
                m_monitoringParameterIDs.append(r.at(1).toList().at(0).toInt());
                if(i==items.count()-1){//最后一个项目查询完成，如果没有出错，执行下面操作
//                    ui->testItemEdit->clear();
//                    ui->testItemEdit->append(items.join("、"));
                    qDebug()<<"m_monitoringParameterIDs"<<m_monitoringParameterIDs;

                }
                emit doSqlFinished();
            });
            // 等待异步查询完成
            loop.exec();
            if(error) return;
        }
        qDebug()<<m_monitoringParameterIDs;
    m_monitoringParameters=items;

    if(m_monitoringParameterIDs.isEmpty()){
        QMessageBox::information(this,"error","请选择检测项目！");
        return;
    }

    if(m_samplingSites.count()&&ui->samplingPosCountBox->value()!=m_samplingSites.count()){
        QMessageBox::information(this,"error","点位数量不对，请确认");
        return;
    }
    TestInfo* info=m_info;

    info->sampleType=ui->sampleTypeBox->currentText();
    info->testFieldID=m_testFieldIDs.at(ui->testFiledBox->currentIndex());
    info->testTypeID=m_testTypeIDs.at(ui->testTypeBox->currentIndex());
    info->monitoringParameters=m_monitoringParameters;
    info->parametersIDs=m_monitoringParameterIDs;
    info->limitStandard=ui->standardNameEidt->text();
    info->limitStandardID=m_limitStandardID;
    info->remark=ui->remarkEdit->text();
    info->samplingFrequency=ui->samplingFrequencyBox->value();
    info->samplingSites=m_samplingSites;
    info->samplingSiteCount=ui->samplingPosCountBox->value();
    info->samplingPeriod=ui->samplingPeriodCountBox->value();
//    m_testInfo.append(info);
//    ui->testInfoTableView->append(info->infoList());
    accept();
}


void testInfoEditor::on_testFiledBox_currentIndexChanged(int index)
{
//    if(!m_testFieldIDs.count()) return;
    QString sql=QString("SELECT testType, id from test_type where testFieldID=%1;").arg(m_testFieldIDs.at(index));
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant> typeNames=msg.result().toList();
        ui->testTypeBox->clear();
        m_testTypeIDs.clear();
        for(int i=1;i<typeNames.count();i++){
            ui->testTypeBox->addItem(typeNames.at(i).toList().at(0).toString());
            m_testTypeIDs.append(typeNames.at(i).toList().at(1).toInt());
        }
    });
}


void testInfoEditor::on_testTypeBox_currentTextChanged(const QString &arg1)
{
    QString sql=QString("SELECT sampleType from sample_type where testTypeID=(select id from test_type where testType='%1');").arg(arg1);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant> typeNames=msg.result().toList();
        ui->sampleTypeBox->clear();
        for(int i=1;i<typeNames.count();i++){
            ui->sampleTypeBox->addItem(typeNames.at(i).toList().at(0).toString());
        }
    });
}


void testInfoEditor::on_testItemAddBtn_clicked()
{
    int index=ui->comboBox->currentIndex();
    switch (index) {
    case 0:
    {
        ImplementingStandardSelectDlg dlg;

        connect(&dlg,&ImplementingStandardSelectDlg::doSql,this,&testInfoEditor::doSql);
        connect(&dlg,&ImplementingStandardSelectDlg::selectDone,this,[this](const QStringList items,const QList<int>&IDs,const QString&standardName,int limitStandardID){
            ui->testItemEdit->clear();
            ui->testItemEdit->append(items.join("、"));
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
        QString sites=QInputDialog::getText(this,"","请输入点位名称，多个点位以“、”隔开");
        m_samplingSites.append(sites.split("、"));
        ui->samplePosEdit->clear();
        ui->samplePosEdit->setText(m_samplingSites.join("、"));
    }
    break;
    case 1:
    {

    }
    break;

    }
}

