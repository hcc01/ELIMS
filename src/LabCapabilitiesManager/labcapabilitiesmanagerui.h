#ifndef LABCAPABILITIESMANAGERUI_H
#define LABCAPABILITIESMANAGERUI_H

#include <QWidget>
#include "LabCapabilitiesManager_global.h"
#include"../Client/tabwigetbase.h"
#include"testtypeeditor.h"
namespace Ui {
class LabCapabilitiesManagerUI;
}

class LABCAPABILITIESMANAGER_EXPORT LabCapabilitiesManagerUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit LabCapabilitiesManagerUI(QWidget *parent = nullptr);
    ~LabCapabilitiesManagerUI();
    virtual void dealProcess(const ProcessNoticeCMD&);//处理流程事件
    virtual void initMod()override;//新增模块时初始化操作，建表等。

private slots:
    void on_testTypeEditBtn_clicked();

private:
    Ui::LabCapabilitiesManagerUI *ui;
    TestTypeEditor m_testTypeEdt;
};

#endif // LABCAPABILITIESMANAGERUI_H
