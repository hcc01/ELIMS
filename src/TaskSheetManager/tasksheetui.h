#ifndef TASKSHEETUI_H
#define TASKSHEETUI_H

#include <QWidget>
#include "TaskSheetManager_global.h"
#include"../Client/tabwigetbase.h"
#include "contractreviewdlg.h"
#include "tasksheeteditor.h"

namespace Ui {
class TaskSheetUI;
}

class TASKSHEETMANAGER_EXPORT TaskSheetUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit TaskSheetUI(QWidget *parent = nullptr);
    enum TaskStatus{
        CREATE, REVIEW, MODIFY,SCHEDULING,SCHEDUL_CONFIRM, WAIT_SAMPLING, SAMPLING,TESTING,REPORT_COMPILATION,REPORT_MODIFY,
        REPORT_REVIEW1,REPORT_REVIEW2,REPORT_REVIEW3,REPORT_ARCHIVING,FINISHED,SAMPLE_CIRCULATION,
    };
    static QString getStatusName(int status){
        QHash<int,QString> StatusName={
            {CREATE,"新建"},{REVIEW,"审核"},{MODIFY,"退回修改"},
             {SCHEDULING,"排单"}, {SCHEDUL_CONFIRM,"等待排单确认"}, {WAIT_SAMPLING,"等待采样"},
              {SAMPLING,"采样"},{TESTING,"分析"},{REPORT_COMPILATION,"报告编制"},{REPORT_REVIEW1,"报告初审"},
               {REPORT_REVIEW2,"报告审核"},{REPORT_REVIEW3,"报告签发"},{REPORT_MODIFY,"报告修改"},{REPORT_ARCHIVING,"报告归档"},
                {FINISHED,"完结"},{SAMPLE_CIRCULATION,"样品流转"}
            };
        return StatusName.value(status);
        }
    ~TaskSheetUI();
//    void submitProcess(int node);//流程推进
    virtual void initMod()override;//新增模块时初始化操作，建表等。
    //流程处理窗口
    virtual FlowWidget *flowWidget(const QFlowInfo &flowInfo) override;//如果要处理流程，需要定义一个流速操作窗口，用于显示流程表格、结果处理(窗口会发送pushProcess信息）等
    void initCMD() override;
    bool updateTaskStatus(int taskID, int status);
    TaskSheetEditor* sheetEditorDlg(int openMode=TaskSheetEditor::NewMode);
    void viewTaskSheet(const QString &taskSheetNum);
    void editTaskSheet(const QString &taskSheetNum);
    void showDeleverySample(int taskSheetID);
private:

private slots:
    void on_newSheetBtn_clicked();
    void submitReview(int taskSheetID, const QString &taskNum, bool DeliveryTest);
    void on_refleshBtn_clicked();
    void doContractReview(const QFlowInfo &flowInfo, const QString&record, const QString&comments, bool passed);
    bool on_tasksheetEditBtn_clicked();

    void on_tasksheetViewBtn_clicked();

    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_deleteBtn_clicked();

    void on_reviewCommentsBtn_clicked();

    void on_submitBtn_clicked();

//    void on_printLabelBtn_clicked();

    void on_filterBox_currentIndexChanged(int index);

    void on_findEdit_returnPressed();

private:
    Ui::TaskSheetUI *ui;
    TaskSheetEditor* m_sheet;
    QHash<QString,int>m_taskIDs;
    ContractReviewDlg m_contractReviewDlg;
    bool manual;
};

#endif // TASKSHEETUI_H
