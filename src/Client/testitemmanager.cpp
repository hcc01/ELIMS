#include "testitemmanager.h"
#include "ui_testitemmanager.h"
#include<QMessageBox>
#include<QCompleter>
#include"itemsselectdlg.h"
#include<QThread>
TestItemManager::TestItemManager(bool addMode,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestItemManager),
    m_newItem(addMode),
    m_itemEditChanged(false)
{
    ui->setupUi(this);
}

TestItemManager::~TestItemManager()
{
    delete ui;
}

void TestItemManager::init()
{
    emit doSql("select testField from test_field where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant>types=msg.result().toList();
        ui->fieldBox->clear();
        for(int i=1;i<types.count();i++){
            ui->fieldBox->addItem(types.at(i).toList().at(0).toString());
        }
    });


}

void TestItemManager::reset()
{
    ui->testNameEdit->setText("");
    ui->testAliasEdit->setText("");
    ui->shortNameEdit->setText("");
    ui->uniqueMarkEdit->setText("");
    ui->textBrowser->clear();
//    ui->listWidget->clear();
//    ui->checkBox->setChecked(false);
}

void TestItemManager::on_OKBtn_clicked()
{
    if(m_newItem){
        QString sql;
        QString name=ui->testNameEdit->text();
        QString alias=ui->testAliasEdit->text();
        QString shortName=ui->shortNameEdit->text();
        QString uniqueMark=ui->uniqueMarkEdit->text();
        QString subParameter=m_subItemIds.join(",");

        if(ui->checkBox->isChecked()){
            for(int i=0;i<ui->listWidget->count();i++){
                sql=QString("INSERT INTO detection_parameters(testFieldID, parameter_name,alias,abbreviation,uniqueMark,subparameter) VALUES((select id from test_field where testField='%6'),'%1','%2','%3','%4','%5');")
                          .arg(name).arg(alias).arg(shortName).arg(uniqueMark).arg(subParameter).arg(ui->listWidget->item(i)->text());
                emit doSql(sql,[&](const QSqlReturnMsg&msg){
                    if(msg.error()){
                        QMessageBox::information(this,"error",msg.result().toString());
                        return;
                    }
                });
            }
        }
        sql=QString("INSERT INTO detection_parameters(testFieldID, parameter_name,alias,abbreviation,uniqueMark,subparameter) VALUES((select id from test_field where testField='%6'),'%1','%2','%3','%4','%5');")
                  .arg(name).arg(alias).arg(shortName).arg(uniqueMark).arg(subParameter).arg(ui->fieldBox->currentText());
        emit doSql(sql,[&](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                return;
            }
            this->reset();
        });
    }
}


void TestItemManager::on_fieldBox_currentTextChanged(const QString &arg1)
{
    emit doSql(QString("select parameter_name,alias,abbreviation from detection_parameters where testFieldID=(select id from test_field where testField='%1');").arg(arg1),[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant>types=msg.result().toList();
        QStringList items;
        for(int i=1;i<types.count();i++){
            items.append(types.at(i).toStringList());
        }
        QCompleter *completer = new QCompleter(items, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
        ui->testNameEdit->setCompleter(completer);
    });
}


void TestItemManager::on_testNameEdit_editingFinished()
{
//    if(!m_itemEditChanged) return;
//    m_itemEditChanged=false;
//    QCompleter *completer=ui->testNameEdit->completer();
//    if(completer->currentIndex().isValid()){//存在的项目，修改模式
//        QString sql=QString("select parameter_name,alias,abbreviation,uniqueMark,subparameter from detection_parameters where parameter_name='%1' or alias='%1' or abbreviation='%1';").arg(ui->testNameEdit->text());
//        emit doSql(sql,[&](const QSqlReturnMsg&msg){
//            if(msg.error()){
//                QMessageBox::information(this,"error",msg.result().toString());
//                return;
//            }
//            QVector<QVariant>list=msg.result().toList();
//            QStringList items=list.at(1).toStringList();
//            ui->testNameEdit->setText(items.at(0));
//            ui->testAliasEdit->setText(items.at(1));
//            ui->shortNameEdit->setText(items.at(2));
//            ui->uniqueMarkEdit->setText(items.at(3));
//            ui->textBrowser->setText(items.at(4));
////            m_newItem=false;
//        });
//    }
//    else{
//        m_newItem=true;
//    }
}


void TestItemManager::on_checkBox_clicked()
{
//    QStringList fields;
//    for(int i=0;i<ui->fieldBox->count();i++) fields.append(ui->fieldBox->itemText(i));
//    if(ui->checkBox->isChecked()){
//        ui->listWidget->clear();
//        ui->listWidget->addItems(itemsSelectDlg::getSelectedItems(fields));
//    }
}


void TestItemManager::on_selectItemBtn_clicked()
{
    qDebug()<<"testItemDlg threadID:"<< QThread::currentThreadId();
    emit doSql(QString("select CONCAT_WS('/',parameter_name,alias,abbreviation) ,id from detection_parameters where testFieldID=(select id from test_field where testField='%1'); ").arg(ui->fieldBox->currentText()),[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QVector<QVariant>v=msg.result().toList();
        QStringList list;
        QList<int> idlist;
        for(int i=1;i<v.count();i++){
            list.append(v.at(i).toList().at(0).toString());
            idlist.append(v.at(i).toList().at(1).toInt());

        }
        QStringList items;
        foreach(auto x,itemsSelectDlg::getSelectedItemsID(list,idlist,items)){
            m_subItemIds.append(QString::number(x));
        }
        ui->textBrowser->append(items.join("、"));

    });
}


void TestItemManager::on_testNameEdit_textChanged(const QString &arg1)
{
    m_itemEditChanged=true;
}


void TestItemManager::on_selectFieldBtn_clicked()
{
    QStringList fields;
    for(int i=0;i<ui->fieldBox->count();i++) fields.append(ui->fieldBox->itemText(i));

        ui->listWidget->clear();
        ui->listWidget->addItems(itemsSelectDlg::getSelectedItems(fields));

}


void TestItemManager::on_clearItemBtn_clicked()
{
        m_subItemIds={};
        ui->textBrowser->clear();
}

