#include "standardsmanager.h"
#include "ui_standardsmanager.h"
#include<QMessageBox>
#include"QExcel.h"
#include<QFileDialog>
StandardsManager::StandardsManager(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::StandardsManager),
    m_testItemEditor(new TestItemManager(this))
{
    ui->setupUi(this);
    connect(m_testItemEditor,&TestItemManager::doSql,this,&TabWidgetBase::doSqlQuery);
    m_standardEditor=new ImplementingStandardEditor(this);
    connect(m_standardEditor,&ImplementingStandardEditor::doSql,this,&TabWidgetBase::doSqlQuery);
}

StandardsManager::~StandardsManager()
{
    delete ui;
}

void StandardsManager::initMod()
{

}

void StandardsManager::on_editTestItemBtn_clicked()
{
    m_testItemEditor->show();
    m_testItemEditor->init();
}


void StandardsManager::on_standardEditBtn_clicked()
{
    m_standardEditor->show();
    m_standardEditor->init();
}


void StandardsManager::on_impotrStandardBtn_clicked()
{
//    QString fileName=QFileDialog::getOpenFileName(this,"","./","Excel文件(*.xls,*.xlsx");
//    if(fileName.isEmpty()) return;
//    QAxObject*book=EXCEL.Open(fileName);
//    if(!book){
//        QMessageBox::information(this,"error","无法打开文件："+fileName);
//        return;
//    }
//    fileName=fileName.split("/").last();
//    fileName=fileName.left(fileName.indexOf("."));
//    int n=fileName.indexOf("@");
//    if(n<0){
//        QMessageBox::information(this,"error","文件名格式错误，确认文件名<标准名称@标准号>");
//        return;
//    }
//    QString standardName=fileName.left(n);
//    QString standardNum=fileName.mid(n+1);
//    int sheetCount=EXCEL.sheetCount(book);

//    QAxObject* sheet;
//    for(int i=1;i<=sheetCount;i++){
//        sheet=EXCEL.selectSheet(i,book);
//        QString str=EXCEL.sheetName(sheet);
//        int n=str.indexOf("@");
//        if(n<0){
//            QMessageBox::information(this,"error","表格名称格式错误，确认文件名<表格名称@检测类型>");
//            return;
//        }
//        QString tableName=str.left(n);
//        QString testType=str.mid(n+1);
//        int r=1,c=7;
//        QString classNum=EXCEL.cellValue(r,c,sheet).toString();
//        if(classNum.isEmpty()){
//            QMessageBox::information(this,"error","表格格式不对，G1为空值，请确认。");
//            return;
//        }
//        while(!classNum.isEmpty()){
//            //插入限值表
//            QString sql=QString("INSERT INTO implementing_standards(standardName, standardNum, tableName, testTypeID, classNum) VALUES('%1','%2','%3',(select id from test_type where testType='%4'),'%5')")
//                              .arg(standardName).arg(standardNum).arg(tableName);
//            doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
//                if(msg.error()){
//                    QMessageBox::information(this,"error",msg.result().toString());
//                    return;
//                }
//                for(int r)
//            });

//            c++;
//            classNum=EXCEL.cellValue(r,c,sheet).toString();

//        }
//    }

}

