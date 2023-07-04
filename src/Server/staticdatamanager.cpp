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

}

