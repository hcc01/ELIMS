#ifndef EMPLOYEEMANAGEUI_H
#define EMPLOYEEMANAGEUI_H

#include "EmployeeManager_global.h"
#include <QWidget>
#include"../Client/tabwigetbase.h"
#include"../Client/qjsoncmd.h"
namespace Ui {
class EmployeeManageUI;
}

class EMPLOYEEMANAGER_EXPORT EmployeeManageUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit EmployeeManageUI(QWidget *parent = nullptr);
    ~EmployeeManageUI();
    static QString tabText(){
        return "人员管理";
    }
    void onSqlReturn(const QSqlReturnMsg& jsCmd)override;
    void initCMD()override;
    void dealProcess(const ProcessNoticeCMD&)override{};//处理流程事件
    bool initMod() override;
private:
    Ui::EmployeeManageUI *ui;
};

#endif // EMPLOYEEMANAGEUI_H
