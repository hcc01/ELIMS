#ifndef CONTRACTREVIEWDLG_H
#define CONTRACTREVIEWDLG_H

#include "tasksheeteditor.h"
#include <QDialog>

namespace Ui {
class ContractReviewDlg;
}

class ContractReviewDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ContractReviewDlg(QWidget *parent = nullptr);
    ~ContractReviewDlg();
    void setFlowInfo(const QFlowInfo&flowInfo){m_flowInfo=flowInfo;setWindowTitle(QString("合同评审-%1").arg(flowInfo.value("taskNum").toString()));}
    QFlowInfo flowInfo()const{return m_flowInfo;}
signals:
    void reviewResult(const QString& reviewRecord,const QString& reviewComments,bool passed);//评审结果以信号发送给任务单管理模块进行处理。
    void getTaskInfo();//显示任务单请求
private slots:
    void on_agreeBtn_clicked();

    void on_disagreeBtn_clicked();

    void on_viewInfoBtn_clicked();

private:
    Ui::ContractReviewDlg *ui;
    QFlowInfo m_flowInfo;
};

#endif // CONTRACTREVIEWDLG_H
