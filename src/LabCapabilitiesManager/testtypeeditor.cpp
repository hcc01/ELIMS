#include "testtypeeditor.h"
#include "ui_testtypeeditor.h"
#include<QMessageBox>
#include<QInputDialog>
TestTypeEditor::TestTypeEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestTypeEditor)
{
    ui->setupUi(this);
}

TestTypeEditor::~TestTypeEditor()
{
    delete ui;
}

void TestTypeEditor::init()
{
    emit doSql("SELECT testField from test_field where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant> fieldNames=msg.result().toList();
        ui->fieldBox->clear();
        for(int i=1;i<fieldNames.count();i++){
            ui->fieldBox->addItem(fieldNames.at(i).toList().at(0).toString());
        }
    });
}

void TestTypeEditor::on_addBtn_clicked()//增加样品类型
{
    QString name=ui->sampleTypeBox->currentText();
    if(name.isEmpty()){
        QMessageBox::information(this,"error","请输入样品类型");
        return;
    }
    QString sql=QString("INSERT INTO sample_type(testTypeID, sampleType) values((select id from test_type where testType='%1'), '%2') ;").arg(ui->testTypeBox->currentText()).arg(name);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","添加样品类型成功");
//        on_fieldBox_currentIndexChanged(ui->fieldBox->currentIndex());
    });
}


void TestTypeEditor::on_FieldAddBtn_clicked()
{
    QString filed=ui->fieldBox->currentText();
    if(filed.isEmpty()){
        QMessageBox::information(this,"error","请输入检测领域");
        return;
    }
    QString sql=QString("INSERT INTO test_Field(testField) values('%1') ;").arg(filed);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","添加检测领域成功");
        init();
    });
}


void TestTypeEditor::on_FieldModifyBtn_clicked()
{
    int index=ui->fieldBox->currentIndex();
    QString oldName=ui->fieldBox->itemText(index);
    QString newName=QInputDialog::getText(this,"请输入领域名称：","");
    if(newName.isEmpty()) return;
    QString sql=QString("UPDATE test_field SET testField='%1' where testField='%2';").arg(newName).arg(oldName);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","修改领域成功");
        init();
    });
}


void TestTypeEditor::on_fieldBox_currentIndexChanged(int index)
{
    qDebug()<<"on_fieldBox_currentIndexChanged";    QString fieldName=ui->fieldBox->itemText(index);
    QString sql=QString("SELECT testType from test_type where testFieldID=(select id from test_field where testField='%1');").arg(fieldName);
    qDebug()<<fieldName;
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant> typeNames=msg.result().toList();
        ui->testTypeBox->clear();
        for(int i=1;i<typeNames.count();i++){
            ui->testTypeBox->addItem(typeNames.at(i).toList().at(0).toString());
        }
    });

}


void TestTypeEditor::on_testTypeAddBtn_clicked()
{
    QString name=ui->testTypeBox->currentText();
    if(name.isEmpty()){
        QMessageBox::information(this,"error","请输入检测类型");
        return;
    }
    QString sql=QString("INSERT INTO test_type(testFieldID, testType) values((select id from test_field where testField='%1'), '%2') ;").arg(ui->fieldBox->currentText()).arg(name);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QMessageBox::information(this,"","添加检测类型成功");
        on_testTypeBox_currentIndexChanged(ui->fieldBox->currentIndex());
    });
}


void TestTypeEditor::on_testTypeBox_currentIndexChanged(int index)
{
    QString typeName=ui->testTypeBox->itemText(index);
    QString sql=QString("SELECT sampleType from sample_type where testTypeID=(select id from test_type where testType='%1');").arg(typeName);
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

