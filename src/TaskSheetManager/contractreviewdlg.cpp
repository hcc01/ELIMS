#include "contractreviewdlg.h"
#include "qmessagebox.h"
#include "ui_contractreviewdlg.h"
#include<QComboBox>
#include<QHBoxLayout>
ContractReviewDlg::ContractReviewDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContractReviewDlg)
{
    ui->setupUi(this);
    ui->tableView->setHeader({"评审要素","评审内容","评审记录"});
    ui->tableView->append({"1.检测方法评审","检测方法是否明确、适用、有效","是"});
    ui->tableView->append({"2.检测项目评审","检测项目是否明确	","是"});
    ui->tableView->append({"3.样品要求评审","样品类型、数量、采/抽样、运输、存储、制备、防护、处理处置、是否确定","是"});
    ui->tableView->append({"4.安全环保评审	","样品存储、制备、检测过程中是否存在潜在的危险或危害		","否"});
    ui->tableView->append({"5.结果传递评审","结果提交的方式是否明确","是"});
    ui->tableView->append({"6.分包评审","检测过程中是否需要分包","否"});
    ui->tableView->append({"7.人力资源评审	","检测人员是否具备相应检测技能，是否需要采/抽样、其他特殊资质要求","是"});
    ui->tableView->append({"8.设备设施评审","是否具备开展检测和客户需求所需设备设施","是"});
    ui->tableView->append({"9.资质能力评审","是否具备检验资质	","是"});
    ui->tableView->append({"10.时间评审","检测周期是否满足客户要求","是"});
    ui->tableView->setEditableColumn(2);
    ComboBoxDelegate* editor=new ComboBoxDelegate(ui->tableView,{"否","是"});
    ui->tableView->setItemDelegateForColumn(2,editor);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);



}

ContractReviewDlg::~ContractReviewDlg()
{
    delete ui;
}

void ContractReviewDlg::on_agreeBtn_clicked()//同意。
{
    QString comments=ui->textEdit->toPlainText();
    QStringList reviewRecord;
    QList<QList<QVariant>> table=ui->tableView->data();
    for(auto row:table){
        reviewRecord.append(row.at(2).toString());
    }
    emit reviewResult(reviewRecord.join("、"),comments,true);//发送审核结果
    accept();
}


void ContractReviewDlg::on_disagreeBtn_clicked()
{
     QString comments=ui->textEdit->toPlainText();
    if(comments.isEmpty()){
        QMessageBox::information(nullptr,"","请输入驳回原因。");
        return;
    }
    emit reviewResult("",comments,false);
    reject();
}


void ContractReviewDlg::on_viewInfoBtn_clicked()
{
    emit getTaskInfo();

}

