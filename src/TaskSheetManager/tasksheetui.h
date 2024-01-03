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
        CREATE, REVIEW, MODIFY,SCHEDULING,SCHEDUL_CONFIRM, WAIT_SAMPLING, SAMPLING,
    };
    const QHash<int,QString> StatusName={{CREATE,"新建"},{REVIEW,"审核"},{MODIFY,"退回修改"},
        {SCHEDULING,"排单"}, {SCHEDUL_CONFIRM,"等待排单确认"}, {WAIT_SAMPLING,"等待采样"},
        {SAMPLING,"采样"}};
    ~TaskSheetUI();
    void submitProcess(int node);//流程推进
    virtual void dealProcess(const QFlowInfo&, int operateFlag)override;//处理流程事件
    virtual void initMod()override;//新增模块时初始化操作，建表等。
    void initCMD() override;
    //end
    bool updateTaskStatus(int taskID, int status);
    TaskSheetEditor* sheetEditorDlg(int openMode=TaskSheetEditor::NewMode);
    void viewTaskSheet(const QString &taskSheetNum);
    void editTaskSheet(const QString &taskSheetNum);
private:

private slots:
    void on_newSheetBtn_clicked();
    void submitReview(int taskSheetID,const QString &taskNum);
    void on_refleshBtn_clicked();
    void doContractReview(const QFlowInfo &flowInfo, const QString&record, const QString&comments, bool passed);
private:
    Ui::TaskSheetUI *ui;
    TaskSheetEditor* m_sheet;
    ContractReviewDlg m_contractReviewDlg;
};

#endif // TASKSHEETUI_H
