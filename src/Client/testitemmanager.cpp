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
    ui->delBtn->hide();
}

TestItemManager::~TestItemManager()
{
    delete ui;
}

void TestItemManager::init()
{
    emit doSql("select testField, id from test_field where deleted=0;",[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QList<QVariant>types=msg.result().toList();
        ui->fieldBox->clear();
        for(int i=1;i<types.count();i++){
            ui->fieldBox->addItem(types.at(i).toList().at(0).toString());
            m_testFieldIDs.append(types.at(i).toList().at(1).toInt());
        }
    });


}

void TestItemManager::reset()
{
    ui->testNameEdit->setText("");
    ui->testAliasEdit->setText("");
//    ui->shortNameEdit->setText("");
//    ui->uniqueMarkEdit->setText("");
    ui->textBrowser->clear();
//    ui->listWidget->clear();
//    ui->checkBox->setChecked(false);

    m_subItemIds.clear();
    ui->subParameterCheck->setChecked(false);
    if(!m_newItem) setAddMode(true);
}

void TestItemManager::setAddMode(bool addMode)
{
    m_newItem=addMode;
    if(m_newItem){
        setWindowTitle("添加检测项目：");
        ui->delBtn->hide();
        ui->OKBtn->setText("确认添加");
        ui->testNameEdit->setCompleter(m_completer);
    }
    else{
        setWindowTitle("修改检测项目："+m_IDtoItems.value(m_currentID));
        ui->delBtn->show();
        ui->OKBtn->setText("确认修改");
        ui->testNameEdit->setCompleter(nullptr);
    }
}

void TestItemManager::on_OKBtn_clicked()
{
    QString name=ui->testNameEdit->text();
    if(name.isEmpty()) return;
    if(1){
        QString sql;

        QString alias=ui->testAliasEdit->text();
//        QString shortName=ui->shortNameEdit->text();
//        QString uniqueMark=ui->uniqueMarkEdit->text();
        QString subParameter=m_subItemIds.join(",");

//        if(ui->copytoCheck->isChecked()&&m_newItem){
//            for(int i=0;i<ui->listWidget->count();i++){
//                sql=QString("INSERT INTO detection_parameters(testFieldID, parameter_name,alias,abbreviation,uniqueMark,subparameter) VALUES((select id from test_field where testField='%6'),'%1','%2','%3','%4','%5');")
//                          .arg(name).arg(alias).arg(subParameter).arg(ui->listWidget->item(i)->text());
//                emit doSql(sql,[&](const QSqlReturnMsg&msg){
//                    if(msg.error()){
//                        QMessageBox::information(this,"error",msg.result().toString());
//                        return;
//                    }
//                    this->reset();
//                });
//            }
//        }
        QJsonArray values;
        if(m_newItem)
        {
            sql="INSERT INTO detection_parameters(testFieldID, parameterName,additive ) values(?,?,?);";

            values={m_testFieldIDs.at(ui->fieldBox->currentIndex()),name,ui->subParameterCheck->isChecked()};
            sql+="SET @id = LAST_INSERT_ID();";
        }
        else{
            sql="update detection_parameters set parameterName=?, additive=? where id=?;";
            values={name,ui->subParameterCheck->isChecked(),m_currentID};
        }
        //插入别名
//        if(!ui->testAliasEdit->text().isEmpty()){
        QStringList aliases;
        if(!ui->testAliasEdit->text().isEmpty()) aliases=ui->testAliasEdit->text().split("、");
            if(!m_newItem){
                sql+="delete from detection_parameter_alias where parameterID=?;";//先删除原来的别名
                values.append(m_currentID);
            }
            for(QString alias:aliases){
                if(!m_newItem) {

                    sql+="INSERT INTO detection_parameter_alias(parameterID, alias) values(?,?);";
                    values.append(m_currentID);
                    values.append(alias);
                }
                else{
                    sql+="INSERT INTO detection_parameter_alias(parameterID, alias) values(@id,?);";
                    values.append(alias);
                }
            }

//        }
        //插入子项目
        if(!m_newItem){
            sql+="delete from detection_subparameters where parameterID=?;";
            values.append(m_currentID);
        }
        if(ui->subParameterCheck->isChecked()){

            for(QString subID:m_subItemIds){
                if(!m_newItem) {
                    sql+="INSERT INTO detection_subparameters(parameterID, subName,subparameterID ) values(?,?,?);";
                    values.append(m_currentID);
                }
                else sql+="INSERT INTO detection_subparameters(parameterID, subName,subparameterID ) values(@id,?,?);";
                if(!ui->subItemGroupEdit->text().isEmpty()) values.append(ui->subItemGroupEdit->text());
                else values.append("1");//默认子参数的类别用1标识
                values.append(subID.toInt());
            }
        }
        emit doSql(sql,[&](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(this,"error",msg.result().toString());
                return;
            }
            this->reset();

        },0,values);
    }

}


void TestItemManager::on_fieldBox_currentTextChanged(const QString &arg1)
{
    QString sql=QString("select parameterName, alias, id from (select * from detection_parameters where testFieldID=(select id from test_field where testField='%1')) as A "
                          "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID;").arg(arg1);
    emit doSql(sql,[&](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(this,"error",msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        m_items.clear();
        m_itemIDs.clear();
        m_IDtoItems.clear();
        for(int i=1;i<r.count();i++){
            QStringList l=r.at(i).toStringList();
            if(!m_items.contains(l.at(0))) {
                m_items.append(l.at(0));
                m_itemIDs[l.at(0)]=l.at(2).toInt();
                m_IDtoItems[l.at(2).toInt()]+=l.at(0)+"/";
            }
            if(!l.at(1).isEmpty()){
                 m_itemIDs[l.at(1)]=l.at(2).toInt();
                m_items.append(l.at(1));
                 m_IDtoItems[l.at(2).toInt()]+=l.at(1)+"/";
            }
        }
        m_completer = new QCompleter(m_items, this);
        m_completer->setCaseSensitivity(Qt::CaseInsensitive);
        m_completer->setFilterMode(Qt::MatchContains);
        ui->testNameEdit->setCompleter(m_completer);
    });
}


void TestItemManager::on_testNameEdit_editingFinished()
{
   if(!m_newItem) return;//已经是修改模式
    QString itemName=ui->testNameEdit->text();
    if(itemName.isEmpty()) return;
    int id=m_itemIDs.value(itemName);
    if(!id){
        setAddMode(true);
        return;
    }
    m_currentID=id;
    QString sql;
    sql=QString("select parameterName from detection_parameters where id=%1").arg(id);
    doSql(sql,[this,sql](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查找参数时错误:","sql:"+sql+" Error:"+msg.result().toString());
            return;
        }
        ui->testNameEdit->setText(msg.result().toList().at(1).toList().at(0).toString());
    });
    sql=QString("select alias from detection_parameter_alias where parameterID=%1").arg(id);
    doSql(sql,[this,sql](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查找别名时错误:","sql:"+sql+" Error:"+msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        ui->testAliasEdit->clear();
        QStringList aliases;
        for(int i=1;i<r.count();i++){
            aliases.append(r.at(i).toList().at(0).toString());
        }
        ui->testAliasEdit->setText(aliases.join("、"));
    });
    sql=QString("select subName,subparameterID from detection_subparameters where parameterID=%1").arg(id);
    emit doSql(sql,[this,sql](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"查找子参数时错误:","sql:"+sql+" Error:"+msg.result().toString());
            return;
        }
        QList<QVariant>r=msg.result().toList();
        QString subName="";
        QStringList subs;
        QMap<QString,QStringList>subItems;
        QMap<QString,QStringList>subItemIDs;
        for(int i=1;i<r.count();i++){
            QString s=r.at(i).toList().at(0).toString();
            int id=r.at(i).toList().at(1).toInt();
            if(!subs.contains(s)) subs.append(s);
            subItemIDs[s].append(QString::number(id));
            subItems[s].append(m_IDtoItems.value(id));
        }
        if(subs.count()){
            if(subs.count()>1) subName=itemsSelectDlg::getSelectedItem(subs);
            if(subName.isEmpty()) subName=subs.at(0);
            ui->subParameterCheck->setChecked(true);
            ui->subItemGroupEdit->setText(subName);
            ui->textBrowser->setText(subItems.value(subName).join("、"));
            m_subItemIds=subItemIDs.value(subName);
        }
    });
    setAddMode(false);
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
    QStringList items;
    foreach(auto x,itemsSelectDlg::getSelectedItemsID(m_IDtoItems.values(),m_IDtoItems.keys(),items)){
        m_subItemIds.append(QString::number(x));
    }
    if(!items.count()) return;
    ui->textBrowser->append(items.join("、"));
    ui->subParameterCheck->setChecked(true);
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
        qDebug()<<m_subItemIds;
        m_subItemIds.clear();
        qDebug()<<m_subItemIds;
        ui->textBrowser->clear();

        ui->subParameterCheck->setChecked(false);
}


void TestItemManager::on_subParameterCheck_clicked()
{
        return;
}


void TestItemManager::on_delBtn_clicked()
{
        setAddMode(true);
        ui->testNameEdit->clear();
}

