#include "staticdatamanager.h"
#include "ui_staticdatamanager.h"
#include"../Client/QExcel.h"
#include<QFileDialog>
#include<QMessageBox>
#include<QSqlQuery>
#include<QSqlError>
#include"cdatabasemanage.h"
#include<QInputDialog>
StaticDataManager::StaticDataManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StaticDataManager)
{
    ui->setupUi(this);
}

StaticDataManager::~StaticDataManager()
{
    delete ui;
}

void StaticDataManager::on_improtLimitStandardBtn_clicked()
{

    QString fileName=QFileDialog::getOpenFileName(this,"","./","Excel文件(*.xls *.xlsx)");
    if(fileName.isEmpty()) return;
    QAxObject*book=EXCEL.Open(fileName);
    if(!book){
        QMessageBox::information(this,"error","无法打开文件："+fileName);
        return;
    }
    fileName=fileName.split("/").last();
    fileName=fileName.left(fileName.indexOf("."));
    int n=fileName.indexOf("@");
    if(n<0){
        QMessageBox::information(this,"error","文件名格式错误，确认文件名<标准名称@标准号>");
        return;
    }
    QString standardName=fileName.left(n);
    QString standardNum=fileName.mid(n+1);
    int sheetCount=EXCEL.sheetCount(book);

    QAxObject* sheet;
    for(int i=1;i<=sheetCount;i++){
        sheet=EXCEL.selectSheet(i,book);
        QString str=EXCEL.sheetName(sheet);
        int n=str.indexOf("@");
        if(n<0){
            QMessageBox::information(this,"error","表格名称格式错误，确认文件名<表格名称@检测领域>");
            return;
        }
        QString tableName=str.left(n);
        QString testType=str.mid(n+1);
        QString sql=QString("select id from test_field where testField='%1'").arg(testType);
        QSqlQuery query(DB.database());
        if(!query.exec(sql)){
            QMessageBox::information(this,"error",query.lastError().text());
            return;
        }
        if(!query.next()){
            QMessageBox::information(this,"error","不存在的检测类型："+testType);
            return;
        }
        int testFieldID=query.value(0).toInt();
        int c=7;
        QString classNum=EXCEL.cellValue(1,c,sheet).toString();
        if(classNum.isEmpty()){
            QMessageBox::information(this,"error","表格格式不对，G1为空值，请确认。");
            return;
        }
        while(!classNum.isEmpty()){
            //插入限值表
            sql=QString("INSERT INTO implementing_standards(standardName, standardNum, tableName, testFieldID, classNum) VALUES('%1','%2','%3',%4,'%5')")
                              .arg(standardName).arg(standardNum).arg(tableName).arg(testFieldID).arg(classNum);

            if(!query.exec(sql)){
                QMessageBox::information(this,"error",query.lastError().text());
                return;
            }
            int r=2;
            QString parameter=EXCEL.cellValue(r,3,sheet).toString();
            if(DB.database().transaction()){
                while(!parameter.isEmpty()){
                    if(!query.exec(QString("SELECT id, parameter_name, alias, uniqueMark from detection_parameters where (parameter_name='%1' or alias='%1') and testFieldID=%2;").arg(parameter).arg(testFieldID))){
                        QMessageBox::information(this,"error","未知的检测参数："+parameter+query.lastError().text());
                        DB.database().rollback();
                        return;
                    }
                    if(!query.size()){
                        QMessageBox::information(this,"error","未知的检测参数："+parameter);
                        DB.database().rollback();
                        return;
                    }
                    int parameterID;
                    if(query.size()>1){
                        QString str;
                        for(int i=0;i<query.size();i++){
                            query.next();
                            str.append(QString("%1:%2/%3(%4);").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString()).arg(query.value(3).toString()));
                        }
                        int c=QInputDialog::getInt(this,str,"");
                        query.seek(c);
                        parameterID=query.value(0).toInt();
                        qDebug()<<"select is "<<query.value(1).toString()<<query.value(2).toString();
                    }
                    else{
                        query.next();
                        parameterID=query.value(0).toInt();
                    }
                    QString unit=EXCEL.cellValue(r,5,sheet).toString();
                    QString flag=EXCEL.cellValue(r,6,sheet).toString();
                    QString trade=EXCEL.cellValue(r,4,sheet).toString();
                    double higher=-1,lower=-1;
                    //确认上限和下限
                    if(flag=="[]"){
                        QString str=EXCEL.cellValue(r,c,sheet).toString();
                        lower=str.split("~").first().toDouble();
                        higher=str.split("~").last().toDouble();
                    }
                    else if(flag=="<="){
                        higher=EXCEL.cellValue(r,c,sheet).toDouble();
                    }
                    else if(flag==">="){
                        lower=EXCEL.cellValue(r,c,sheet).toDouble();
                    }
                    else{//不属于上下限的限值，即为文本类描述，记录的unit列中
                        unit=EXCEL.cellValue(r,c,sheet).toString();
                    }
                    sql="INSERT INTO standard_limits (standardID, parameterName, parameterID , trade, higher, lower, unit) "
                          "VALUES((SELECT MAX(id) FROM implementing_standards), :parameterName, :parameterID , :trade, :higher, :lower, :unit)";
                    query.prepare(sql);
                    query.bindValue(":parameterName",parameter);
                    query.bindValue(":parameterID",parameterID);
                    query.bindValue(":trade",trade);
                    query.bindValue(":higher",higher);
                    query.bindValue(":lower",lower);
                    query.bindValue(":unit",unit);
                    if(!query.exec()){
                        QMessageBox::information(this,"error",query.lastError().text());
                        DB.database().rollback();
                        return;
                    }

                    r++;
                    parameter=EXCEL.cellValue(r,3,sheet).toString();
                }
            }
            else{
                qDebug()<<"启动事务失败";
                return;
            }
            DB.database().commit();
            c++;
            classNum=EXCEL.cellValue(1,c,sheet).toString();

        }
    }
    EXCEL.CloseBook(book);
}


void StaticDataManager::on_improtTestMethod_clicked()
{

    QString fileName=QFileDialog::getOpenFileName(this,"选择文件","","EXCEL文件(*.xlsx *.xls)");
    if(fileName.isEmpty()) return;
    QAxObject*book=EXCEL.Open(fileName);
    if(!book){
        QMessageBox::information(this,"error","无法打开文件："+fileName);
        return;
    }
    QAxObject*sheet=EXCEL.selectSheet(1,book);
    int r=2;
    QString methodName=EXCEL.cellValue(r,1,sheet).toString();
    QString methodNum,unit,sampleGroup,sampleMedium,preservatives,storageCondition,stability,samplingFlow,samplingDuration,samplingRequirements,
        blankControl,curveCalibration,parallelControl,spikeControl;
    QString parameters,MDLs,coverages;
    int coverage=0,testingMode,minAmount,mediumPrepare;
    QSqlQuery query(DB.database());
    if(DB.database().transaction()){
        while(!methodName.isEmpty()){
            methodNum=EXCEL.cellValue(r,2,sheet).toString();
            unit=EXCEL.cellValue(r,6,sheet).toString();
            sampleGroup=EXCEL.cellValue(r,9,sheet).toString();
            sampleMedium=EXCEL.cellValue(r,10,sheet).toString();
            preservatives=EXCEL.cellValue(r,12,sheet).toString();
            storageCondition=EXCEL.cellValue(r,13,sheet).toString();
            stability=EXCEL.cellValue(r,14,sheet).toString();
            samplingFlow=EXCEL.cellValue(r,15,sheet).toString();
            samplingDuration=EXCEL.cellValue(r,16,sheet).toString();
            samplingRequirements=EXCEL.cellValue(r,17,sheet).toString();
            blankControl=EXCEL.cellValue(r,18,sheet).toString();
            curveCalibration=EXCEL.cellValue(r,19,sheet).toString();
            parallelControl=EXCEL.cellValue(r,20,sheet).toString();
            spikeControl=EXCEL.cellValue(r,21,sheet).toString();
            coverages=EXCEL.cellValue(r,3,sheet).toString();
            testingMode=EXCEL.cellValue(r,7,sheet).toInt();
            minAmount=EXCEL.cellValue(r,8,sheet).toInt();
            mediumPrepare=EXCEL.cellValue(r,11,sheet).toInt();
            parameters=EXCEL.cellValue(r,4,sheet).toString();
            MDLs=EXCEL.cellValue(r,5,sheet).toString();
            QString sql;
            coverage=0;
            foreach(QString x,coverages.split("、")){
                query.prepare("select bitNum from test_type where testType=?");
                query.bindValue(0,x);
                       if(!query.exec()){
                           QMessageBox::information(this,"select bitNum from test_type error",query.lastError().text());
                           DB.database().rollback();
                           return;
                       }
                       if(!query.next()) {
                           QMessageBox::information(this,"error","没有查询到类型："+x);
                           DB.database().rollback();
                           return;
                       }
                    coverage|=query.value(0).toInt();
            }

            //先检查下项目库是否有方法的检测参数，如果没有，自动添加
            int areaID;
            query.prepare("select testFieldID from test_type where testType=?");
            query.bindValue(0,coverages.split("、").at(0));
               if(!query.exec()){
                   QMessageBox::information(this,"select testFieldID from test_type error",query.lastError().text());
                   DB.database().rollback();
                   return;
               }
               if(!query.next()) {
                   QMessageBox::information(this,"error","没有查询到类型："+coverages.split("、").at(0));
                   DB.database().rollback();
                   return;
               }
            areaID=query.value(0).toInt();

            sql="INSERT INTO test_methods (methodName,methodNumber,sampleGroup,sampleMedium,preservatives,storageCondition,stability,samplingFlow,samplingDuration,samplingRequirements,"
                  "blankControl,curveCalibration,parallelControl,spikeControl, testFieldID, coverage,testingMode,minAmount,mediumPrepare) "
                  "VALUES(:methodName,:methodNum,:sampleGroup,:sampleMedium,:preservatives,:storageCondition,:stability,:samplingFlow,:samplingDuration,:samplingRequirements,"
                  ":blankControl,:curveCalibration,:parallelControl,:spikeControl, :testFieldID, :coverage,:testingMode,:minAmount,:mediumPrepare)";
            query.prepare(sql);
            query.bindValue(":methodName",methodName);
            query.bindValue(":methodNum",methodNum);
            query.bindValue(":testFieldID",areaID);
            query.bindValue(":sampleGroup",sampleGroup);
            query.bindValue(":sampleMedium",sampleMedium);
            query.bindValue(":preservatives",preservatives);
            query.bindValue(":storageCondition",storageCondition);
            query.bindValue(":stability",stability);
            query.bindValue(":samplingFlow",samplingFlow);
            query.bindValue(":samplingDuration",samplingDuration);
            query.bindValue(":samplingRequirements",samplingRequirements);
            query.bindValue(":blankControl",blankControl);
            query.bindValue(":curveCalibration",curveCalibration);
            query.bindValue(":parallelControl",parallelControl);
            query.bindValue(":spikeControl",spikeControl);
            query.bindValue(":coverage",coverage);
            query.bindValue(":testingMode",testingMode);
            query.bindValue(":minAmount",minAmount);
            query.bindValue(":mediumPrepare",mediumPrepare);
            if(!query.exec()){
                QMessageBox::information(this,"INSERT INTO test_methods error",query.lastError().text());
                DB.database().rollback();
                return;
            }
            query.exec("SELECT LAST_INSERT_ID()");
               if(!query.next()) {
                   QMessageBox::information(this,"error","无法获取上次插入ID");
                   DB.database().rollback();
                   return;
               }
            int methodID=query.value(0).toInt();
            int parameterID;
            QStringList parameterList=parameters.split("、");
            QStringList MDLlist=MDLs.split("、");
            if(MDLlist.count()){
                if(MDLlist.count()!=1&&MDLlist.count()!=parameterList.count()){
                       QMessageBox::information(this,"error","检出限数量与参数数量不匹配");
                       DB.database().rollback();
                       return;
                }
            }
            QString parameter;
            for (int i=0;i< parameterList.count();i++) {
                parameter=parameterList.at(i);
                query.prepare("select id from detection_parameters where testFieldID=:ID and (parameter_name=:value or alias=:value or abbreviation=:value);");
                query.bindValue(":value",parameter);
                query.bindValue(":ID",areaID);
                if(!query.exec()){
                    QMessageBox::information(this,"select id from detection_parameters error",query.lastError().text());
                    DB.database().rollback();
                    return;
                }
                if(!query.next()){//项目参数表中不没有该参数，自动添加
                    query.prepare("INSERT INTO detection_parameters (testFieldID, parameter_name) VALUES(?,?);");
                    query.addBindValue(areaID);
                    query.addBindValue(parameter);
                    if(!query.exec()){
                               QMessageBox::information(this,"INSERT INTO detection_parameters",query.lastError().text());
                               DB.database().rollback();
                               return;
                    }
                    query.exec("SELECT LAST_INSERT_ID()");
                    if(!query.next()) {
                               QMessageBox::information(this,"error","无法获取上次插入ID");
                               DB.database().rollback();
                               return;
                    }
                    parameterID=query.value(0).toInt();
                }
                else{
                    parameterID=query.value(0).toInt();
                }
                //开始保存方法参数
                query.prepare("INSERT INTO method_parameters ( methodID, parameterID, MDL, unit) VALUES(?,?,?,?);");
                query.bindValue(0,methodID);
                query.bindValue(1,parameterID);
                if(MDLlist.count()>1)
                    query.bindValue(2,MDLlist.at(i).toDouble());
                else if(MDLlist.count())
                     query.bindValue(2,MDLlist.at(0).toDouble());
                else
                     query.bindValue(2,-1.0);
                query.bindValue(3,unit);
                if(!query.exec()){
                    QMessageBox::information(this,"INSERT INTO method_parameters error",query.lastError().text());
                     qDebug()<<query.lastError().text();
                     qDebug()<<query.lastQuery();
                     qDebug()<<parameterID<<parameter;
                    DB.database().rollback();
                    return;
                }
            }


            r++;
            methodName=EXCEL.cellValue(r,1,sheet).toString();
        }
    }
    else{
        qDebug()<<"启动事务失败";
        return;
    }
    DB.database().commit();
    QMessageBox::information(this,"","操作成功");

    EXCEL.CloseBook(book);
}

