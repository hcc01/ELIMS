#ifndef EMPLOYEEMANAGEUI_H
#define EMPLOYEEMANAGEUI_H


#include "EmployeeManager_global.h"
#include <QWidget>
#include"../Client/tabwigetbase.h"
#include"../Client/qjsoncmd.h"

#include <QFlags>
namespace Ui {
class EmployeeManageUI;
}



class EMPLOYEEMANAGER_EXPORT EmployeeManageUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit EmployeeManageUI(QWidget *parent = nullptr);
    ~EmployeeManageUI();

    void initCMD()override;
//    void dealProcess(const ProcessNoticeCMD&)override{};//处理流程事件
    void initMod() override;
private:
    Ui::EmployeeManageUI *ui;

};

#endif // EMPLOYEEMANAGEUI_H
