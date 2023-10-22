#ifndef TASKSHEETUI_H
#define TASKSHEETUI_H

#include <QWidget>
#include "TaskSheetManager_global.h"
#include"../Client/tabwigetbase.h"
namespace Ui {
class TaskSheetUI;
}

class TASKSHEETMANAGER_EXPORT TaskSheetUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit TaskSheetUI(QWidget *parent = nullptr);
    enum TaskStatus{

    };

    ~TaskSheetUI();
    //基类纯虚函数的实现start
//    virtual void onSqlReturn(const QSqlReturnMsg& jsCmd);//处理数据库操作返回的信息
    virtual void dealProcess(const ProcessNoticeCMD&);//处理流程事件
    virtual void initMod();//新增模块时初始化操作，建表等。
    void initCMD() override;
    //end

private:

private slots:
    void on_newSheetBtn_clicked();

private:
    Ui::TaskSheetUI *ui;
};

#endif // TASKSHEETUI_H
