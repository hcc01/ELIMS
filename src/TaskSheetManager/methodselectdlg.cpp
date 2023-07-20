#include "methodselectdlg.h"
#include "ui_methodselectdlg.h"
#include<QMessageBox>
MethodSelectDlg::MethodSelectDlg(int taskID, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MethodSelectDlg),
    m_taskID(taskID)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"样品类型","检测项目","检测方法","CMA资质","是否分包","分包原因"});
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

void MethodSelectDlg::on_pushButton_clicked()//加载方法
{
    QString sql;
    sql=QString("select testTypeID, testType, bitNum, parameterID, parameterName from (select testTypeID, parameterID, parameterName "
                  "from task_methods where taskSheetID ='%1' group by testTypeID, parameterID, parameterName) as A JOIN test_type on A.testTypeID= test_type.id ;").arg(m_taskID);

    doSql(sql,[this](const QSqlReturnMsg&msg){//查找本任务单的检测类型和检测参数，合并相同的类型
        if(msg.error()){
            QMessageBox::information(nullptr,"error",msg.result().toString());
            return;
        }
        QList<QVariant> r=msg.result().toList();
        qDebug()<<r;
        for(int i=1;i<r.count();i++){
            QVariantList row=r.at(i).toList();
            int testTypeID=row.at(0).toInt();
            int typeBit=row.at(2).toInt();
            int parameterID=row.at(3).toInt();
            QString parameter=row.at(4).toString();
            QString testType=row.at(1).toString();
            QString sql=QString("select methodID, methodName, methodNumber, CMA from (select methodID,CMA from method_parameters where parameterID='%1') as A "
                                  "join (select * from test_methods where coverage&%2 ) as B on A.methodID= B.id;").arg(parameterID).arg(typeBit);
            doSql(sql,[this,i,testType,parameter](const QSqlReturnMsg&msg){
                if(msg.error()){
                    QMessageBox::information(nullptr,"select method error",msg.result().toString());
                    return;
                }
                QList<QVariant> r=msg.result().toList();
                qDebug()<<r;
                QStringList methods;
                QHash<QString,QVariant> maps;
                for(int j=1;j<r.count();j++){
                    QList<QVariant>row=r.at(j).toList();
                    QString method=QString("%1 %2 @%3").arg(row.at(2).toString()).arg(row.at(1).toString()).arg(row.at(0).toInt());
                    QString cma=row.at(3).toInt()?"否":"是";
                    methods.append(method);
                    maps.insert(method,cma);
                }
                m_methodBox->setCellItems(i-1,2,methods);
                ui->tableView->append({testType,parameter,"","","",""});
                ui->tableView->setMappingCell(i-1,3,i-1,2,maps);
                ui->tableView->setData(i-1,2,methods.at(0));

            });
            //ui->tableView->append({row.at(1),row.at(4),})
        }
    });
}

