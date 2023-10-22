#include "implementingstandardeditor.h"
#include "ui_implementingstandardeditor.h"
#include<QMessageBox>

ImplementingStandardEditor::ImplementingStandardEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImplementingStandardEditor)
{
    ui->setupUi(this);
    ui->standardEditWidget->hide();
    ui->tableEditWidget->hide();
    ui->classEditWidget->hide();
    ui->itemEditWidget->hide();
}

ImplementingStandardEditor::~ImplementingStandardEditor()
{
    delete ui;
}

void ImplementingStandardEditor::init()
{
    emit doSql("select CONCAT_WS('@',standardName,standardNum) from implementing_standards where deleted=0 GROUP BY standardNum, standardName; ",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QStringList items;
        QVector<QVariant> v=msg.result().toList();
        for(int i=1;i<v.count();i++){
            items.append(v.at(i).toList().at(0).toString());
        }
        ui->standardSelectBox->init(items);
    });
    emit doSql("select testType,id from test_type where deleted=0",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant> v=msg.result().toList();
        for(int i=1;i<v.count();i++){
            m_testTypes.append(v.at(i).toList().at(0).toString());
            m_testTypeIDs.append(v.at(i).toList().at(1).toInt());
        }
        ui->testTypeBox->clear();
        ui->testTypeBox->addItems(m_testTypes);
    });
}

void ImplementingStandardEditor::on_standardEidtBtn_clicked()
{
    ui->standardEditWidget->show();
}


void ImplementingStandardEditor::on_addTestItemBtn_clicked()
{
    ui->itemEditWidget->show();
//    TestItemSelectDlg dlg;
//    dlg.exec();
}


void ImplementingStandardEditor::on_tableEditBtn_clicked()
{
    ui->tableEditWidget->show();
}


void ImplementingStandardEditor::on_classEditBtn_clicked()
{
    ui->classEditWidget->show();
}


void ImplementingStandardEditor::on_standardEditOK_clicked()
{
    QString name=ui->standerNameEdit->text();
    QString num=ui->standardNumEdit->text();
    if(name.isEmpty() || num.isEmpty()){
        return;
    }
    m_standardName=name;
    m_standardNum=num;
    ui->standardSelectBox->setCurrentText(num+"@"+name);
    ui->tableSelectBox->setCurrentText("");
    ui->classSelectBox->setCurrentText("");
    ui->standardEditWidget->hide();
    ui->tableEditWidget->show();
}


void ImplementingStandardEditor::on_classEditOK_clicked()
{
    if(ui->classNameEdit->text().isEmpty()) return;
    m_limitClass=ui->classNameEdit->text();
    QString sql=QString("INSERT INTO implementing_standards(standardName,standardNum,tableName,classNum,testTypeID) VALUES('%1','%2','%3','%4',%5);")
                      .arg(m_standardName).arg(m_standardNum).arg(m_tableName).arg(m_limitClass).arg(m_testTypeID);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }

    });
}


void ImplementingStandardEditor::on_tableEditOk_clicked()
{
    if(ui->tableNameEdit->text().isEmpty()) return;
    m_tableName=ui->tableNameEdit->text();
    m_testType=ui->testTypeBox->currentText();
    m_testTypeID=m_testTypeIDs.at(ui->testTypeBox->currentIndex());
    ui->tableSelectBox->setCurrentText(m_tableName+"@"+m_testType);
    ui->classEditWidget->show();
}


void ImplementingStandardEditor::on_testTypeBox_currentIndexChanged(int index)
{
    m_testTypeID=m_testTypeIDs.at(index);
}


void ImplementingStandardEditor::on_standardSelectBox_currentIndexChanged(int index)
{
    if(ui->standardSelectBox->currentText().isEmpty()) return;
    QStringList l=ui->standardSelectBox->currentText().split("@");
    m_standardName=l.at(0);
    m_standardNum=l.at(1);
    QString sql=QString("select CONCAT_WS('@',tableName,testTypeID) from implementing_standards where standardNum='%1' and deleted=0 group by tableName,testTypeID;").arg(m_standardNum);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QStringList items;
        QVector<QVariant> v=msg.result().toList();
        for(int i=1;i<v.count();i++){
            items.append(v.at(i).toList().at(0).toString());
        }
        ui->tableSelectBox->addItems(items);
    });
}


void ImplementingStandardEditor::on_tableSelectBox_currentIndexChanged(int index)
{
    if(ui->tableSelectBox->currentText().isEmpty()) return;
    QStringList l=ui->tableSelectBox->currentText().split("@");
    m_tableName=l.at(0);
    m_testTypeID=l.at(1).toInt();
    QString sql=QString("select classNum from implementing_standards where standardNum='%1' and tableName='%2';").arg(m_standardNum).arg(m_tableName);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QStringList items;
        QVector<QVariant> v=msg.result().toList();
        for(int i=1;i<v.count();i++){
            items.append(v.at(i).toList().at(0).toString());
        }
        ui->classSelectBox->clear();
        ui->classSelectBox->addItems(items);
    });
}


void ImplementingStandardEditor::on_classSelectBox_currentIndexChanged(int index)
{
    m_limitClass=ui->classSelectBox->currentText();
    QString sql=QString("select classNum from implementing_standards where standardNum='%1' and tableName='%2' and classNum='%3';").arg(m_standardNum).arg(m_tableName).arg(m_limitClass);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QStringList items;
        QVector<QVariant> v=msg.result().toList();
        m_limitClassID=v.at(1).toList().at(0).toInt();
    });
}

