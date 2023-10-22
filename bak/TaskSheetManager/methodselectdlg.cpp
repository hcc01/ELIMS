#include "methodselectdlg.h"
#include "ui_methodselectdlg.h"
#include<QMessageBox>
#include<QJsonArray>
MethodSelectDlg::MethodSelectDlg(int taskID, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MethodSelectDlg),
    m_taskID(taskID),
    m_saving(false)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"检测类型","检测项目","检测方法","CMA资质","是否分包","分包原因"});
    m_methodBox=new ComboBoxDelegate({},this);
    ui->tableView->setItemDelegateForColumn(2,m_methodBox);
    ComboBoxDelegate* subpackageEditor=new ComboBoxDelegate({"否","是"},this);
    ui->tableView->setItemDelegateForColumn(4,subpackageEditor);
    ui->tableView->setEditableColumn(2);
    ui->tableView->setEditableColumn(4);
    ui->tableView->setEditableColumn(5);
}

MethodSelectDlg::~MethodSelectDlg()
{
    delete ui;
}

void MethodSelectDlg::showMethods(const QVector<QVector<QVariant> > &table)
{
    for(auto line:table){
        if(line.count()!=6){
            QMessageBox::information(nullptr,"","显示方法时错误：参数不匹配");
            return;
        }
        ui->tableView->append(line);
    }
    m_methodTable=table;
}

void MethodSelectDlg::on_pushButton_clicked()//加载方法
{
    ui->tableView->clear();
    QString sql;
    sql=QString("select testTypeID, testType, bitNum, parameterID, parameterName ,testFieldID from (select testTypeID, parameterID, parameterName "
                  "from task_methods where taskSheetID ='%1' group by testTypeID, parameterID, parameterName) as A JOIN test_type on A.testTypeID= test_type.id ;").arg(m_taskID);

    doSql(sql,[this](const QSqlReturnMsg&msg){//查找本任务单的检测类型和检测参数，合并相同的类型
        if(msg.error()){
            QMessageBox::information(nullptr,"error",msg.result().toString());
            return;
        }
        QList<QVariant> r=msg.result().toList();
        qDebug()<<r;
        m_parameterIDs.clear();
        m_testTypeIDs.clear();
        for(int i=1;i<r.count();i++){
            QVariantList row=r.at(i).toList();
            int testTypeID=row.at(0).toInt();
            int typeBit=row.at(2).toInt();
            int parameterID=row.at(3).toInt();
            int testFieldID=row.at(5).toInt();

            m_parameterIDs.append(parameterID);
            m_testTypeIDs.append(testTypeID);
            QString parameter=row.at(4).toString();
            QString testType=row.at(1).toString();

            QString sql=QString("select A.id, methodName, methodNumber, CMA from (select id,methodID,CMA from method_parameters where parameterID='%1') as A "
                                  "join (select * from test_methods where coverage&%2 and testFieldID=%3 ) as B on A.methodID= B.id;")
                              .arg(parameterID).arg(typeBit).arg(testFieldID);
            doSql(sql,[this,i,testType,parameter](const QSqlReturnMsg&msg){//进行方法选择
                if(msg.error()){
                    QMessageBox::information(nullptr,"select method error",msg.result().toString());
                    return;
                }
                QList<QVariant> r=msg.result().toList();
                qDebug()<<r;
                QStringList methods;
                QHash<QString,QVariant> maps;
                QMap<QString,int>methodToID;
                for(int j=1;j<r.count();j++){
                    QList<QVariant>row=r.at(j).toList();
                    QString method=QString("%1 %2").arg(row.at(2).toString()).arg(row.at(1).toString());
                    QString cma=row.at(3).toInt()?"否":"是";
                    methods.append(method);
                    methodToID[method]=(row.at(0).toInt());
                    maps.insert(method,cma);
                }
                m_methodBox->setCellItems(i-1,2,methods);
                ui->tableView->append({testType,parameter,"","","",""});
                ui->tableView->setMappingCell(i-1,3,i-1,2,maps);
                ui->tableView->setCellFlag(i-1,2,QVariant::fromValue(methodToID));
                if(methods.count()) {
                    if(m_methodTable.count()>i&&m_methodTable.at(i-1).at(0)==testType&&m_methodTable.at(i-1).at(1)==parameter&&!m_methodTable.at(i-1).at(2).toString().isEmpty()){//如果之前有选择方法，使用之前的方法
                        ui->tableView->setData(i-1,2,m_methodTable.at(i-1).at(2));
                    }
                    else ui->tableView->setData(i-1,2,methods.at(0));
                }
                if(methods.count()>1) {
                    ui->tableView->setBackgroundColor(i-1,2,qRgba(250,250,100,50));
                    ui->tableView->repaint();
                }

            });
//            ui->tableView->viewport()->update();
            //ui->tableView->append({row.at(1),row.at(4),})
        }
    });
}


void MethodSelectDlg::on_OkBtn_clicked()
{
    if(m_saving) return;
    m_saving=true;
    auto table=ui->tableView->data();
    QString sql;
    QJsonArray values;
    sql="update task_methods set testMethodID=?, testMethodName=?, subpackage=?, subpackageDesc=? where taskSheetID=? and testTypeID=? and parameterID=?;";
    for(int i=0;i<table.count();i++){
        QEventLoop loop;
        connect(this, &MethodSelectDlg::doSqlFinished, &loop, &QEventLoop::quit);
        bool error=false;
        QString methodNmae=ui->tableView->value(i,"检测方法").toString();
                                 if(methodNmae.isEmpty()) continue;//没有方法，跳过
                                 QMap<QString,int>methodToId=ui->tableView->cellFlag(i,2).value< QMap<QString,int>>();
                                 if(!methodToId.contains(methodNmae)){
            qDebug()<<"!methodToId.contains(methodNmae)"<<methodToId<<methodNmae;
                                 }
                                 int methodID=methodToId.value(methodNmae);


                            values={methodID,methodNmae,(ui->tableView->value(i,"是否分包").toString()=="是"?1:0),ui->tableView->value(i,"分包原因").toString(), m_taskID,m_testTypeIDs.at(i),m_parameterIDs.at(i)};
        doSql(sql,[this,&error,methodNmae,methodID,methodToId](const QSqlReturnMsg&msg){
                if(msg.error()){
                                        QMessageBox::information(nullptr,QString("更新方法%1(%2)时错误：").arg(methodNmae).arg(methodID),msg.result().toString());
                    error=true;
                                        qDebug() << methodToId ;
                    emit doSqlFinished();
                    m_saving = false;
                    return;
                }
                emit doSqlFinished();
            },0,values);
        loop.exec();
        if(error) {
            m_saving=false;
            return;
        }
    }
    m_saving=false;
    qDebug()<<"emit methodSelected(table);";
    emit methodSelected(table);
    accept();
}

