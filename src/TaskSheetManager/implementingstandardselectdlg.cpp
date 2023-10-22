#include "implementingstandardselectdlg.h"
#include "ui_implementingstandardselectdlg.h"
#include<QMessageBox>
ImplementingStandardSelectDlg::ImplementingStandardSelectDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImplementingStandardSelectDlg)
{
    ui->setupUi(this);
}

ImplementingStandardSelectDlg::~ImplementingStandardSelectDlg()
{
    delete ui;
}

void ImplementingStandardSelectDlg::init()
{
    emit doSql("select CONCAT(standardName, '(', standardNum, ')') from implementing_standards where deleted=0 GROUP BY standardNum,standardName",[this](const QSqlReturnMsg&msg){
                   if(msg.error()) {
                       QMessageBox::information(this,"error",msg.result().toString());
                       return;
                   }
                   QList<QVariant> r=msg.result().toList();
                   qDebug()<<r;
                   ui->standardNameBox->clear();
                   QStringList standards;
                   for(int i=1;i<r.count();i++){
                       standards.append(r.at(i).toList().at(0).toString());
                   }
                   ui->standardNameBox->init(standards);

                   qDebug()<<"standards"<<standards;
    });
}



void ImplementingStandardSelectDlg::on_OkBtn_clicked()
{
    m_selectParameterIDs.clear();
    m_selectParameters.clear();
    for(auto item:ui->listWidget->selectedItems()){
        m_selectParameters.append(item->text());
        m_selectParameterIDs.append(m_parameterIDs.at(ui->listWidget->row(item)));
    }
    if(m_selectParameters.count()){
        int limitID=m_standardIDs.at(ui->classNumBox->currentIndex());
        emit selectDone(m_selectParameters, m_selectParameterIDs,QString("%1 %2 %3").arg(ui->standardNameBox->currentText()).arg(ui->tableNameBox->currentText()).arg(ui->classNumBox->currentText()), limitID);
    }
    accept();

}


void ImplementingStandardSelectDlg::on_standardNameBox_currentIndexChanged(int index)
{
    QString standard=ui->standardNameBox->currentText();
    if(standard.isEmpty()) {
        qDebug()<<"standard.isEmpty()";
        return;
    }
    standard=standard.split("(").first();
    emit doSql(QString("SELECT tableName from implementing_standards where standardName='%1' and deleted=0 GROUP BY tableName;").arg(standard),
               [this](const QSqlReturnMsg&msg){
                   if(msg.error()) {
                       QMessageBox::information(this,"error",msg.result().toString());
                       return;
                   }
                   QList<QVariant> r=msg.result().toList();
                   ui->tableNameBox->clear();
                   for(int i=1;i<r.count();i++){
                       ui->tableNameBox->addItem(r.at(i).toList().at(0).toString());

                   }
               });

}


void ImplementingStandardSelectDlg::on_tableNameBox_currentTextChanged(const QString &arg1)
{
    if(arg1.isEmpty()) return;
    QString sql=QString("SELECT id, classNum from implementing_standards where standardName='%1' and tableName='%2' and deleted=0;")
                      .arg(ui->standardNameBox->currentText().split("(").first()).arg(arg1);
    qDebug()<<sql;

    emit doSql(sql, [this](const QSqlReturnMsg&msg){
                   if(msg.error()) {
                       QMessageBox::information(this,"error",msg.result().toString());
                       return;
                   }
                   QList<QVariant> r=msg.result().toList();
                   ui->classNumBox->clear();
                   m_standardIDs.clear();
                   for(int i=1;i<r.count();i++){
                       ui->classNumBox->addItem(r.at(i).toList().at(1).toString());
                       m_standardIDs.append(r.at(i).toList().at(0).toInt());
                   }
                   emit doSql(QString("SELECT parameterID, parameterName from standard_limits where standardID=%1")
                                  .arg(m_standardIDs.at(ui->classNumBox->currentIndex())),
                              [this](const QSqlReturnMsg&msg){
                                  if(msg.error()) {
                                      QMessageBox::information(this,"error",msg.result().toString());
                                      return;
                                  }
                                  QList<QVariant> r=msg.result().toList();
                                  ui->listWidget->clear();
                                  m_parameterIDs.clear();
                                  for(int i=1;i<r.count();i++){
                                      ui->listWidget->addItem(r.at(i).toList().at(1).toString());
                                      m_parameterIDs.append(r.at(i).toList().at(0).toInt());
                                  }
                              });
               });
}

