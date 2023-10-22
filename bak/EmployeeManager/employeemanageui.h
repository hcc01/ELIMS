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

// 定义实验室人员岗位的枚举
enum LabPosition {
    None = 0,
    Sampler = 1 << 0,  // 采样员
    SamplerLeader = 1 << 1,  // 采样组长
    OrganicAnalyst = 1 << 2,  // 有机分析员
    OrganicLeader = 1 << 3,  // 有机组长
    InorganicAnalyst = 1 << 4,  // 无机分析员
    InorganicLeader = 1 << 5,  // 无机组长
    PhysicalChemicalAnalyst = 1 << 6,  // 理化分析员
    PhysicalChemicalLeader = 1 << 7,  // 理化组长
    QualitySpecialist = 1 << 8,  // 质量专员
    QualityManager = 1 << 9,  // 质量负责人
    SampleAdministrator = 1 << 10,  // 样品管理员
    EquipmentAdministrator = 1 << 11,  // 设备管理员
    ConsumableAdministrator = 1 << 12,  // 耗材管理员
    ReportWriter = 1 << 13,  // 报告编制员
    ReportReviewer = 1 << 14,  // 报告审核员
    AuthorizedSignatory = 1 << 15,  // 授权签字人
    TechnicalManager = 1 << 16,  // 技术负责人
    LabSupervisor = 1 << 17,  // 实验室主管
    LabManager = 1 << 18,  // 实验室经理
    salesRepresentative =1<<19,//业务代表

};

// 声明 QFlags 类型
Q_DECLARE_FLAGS(LabPositions, LabPosition)
class EMPLOYEEMANAGER_EXPORT EmployeeManageUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit EmployeeManageUI(QWidget *parent = nullptr);
    ~EmployeeManageUI();

    void initCMD()override;
    void dealProcess(const ProcessNoticeCMD&)override{};//处理流程事件
    void initMod() override;
private:
    Ui::EmployeeManageUI *ui;
    QMap<int,QString>m_positions= {
        {1 << 0, "采样员"},
        {1 << 1,  "采样组长"},
        {1 << 2,  "有机分析员"},
        {1 << 3,  "有机组长"},
        {1 << 4,  "无机分析员"},
        {1 << 5,  "无机组长"},
        {1 << 6,  "理化分析员"},
        {1 << 7,  "理化组长"},
        {1 << 8,  "质量专员"},
        {1 << 9,  "质量负责人"},
        {1 << 10,  "样品管理员"},
        {1 << 11,  "设备管理员"},
        {1 << 12,  "耗材管理员"},
        {1 << 13,  "报告编制员"},
        {1 << 14,  "报告审核员"},
        {1 << 15,  "授权签字人"},
        {1 << 16,  "技术负责人"},
        {1 << 17,  "实验室主管"},
        {1 << 18,  "实验室经理"},
        {1<<19,"业务代表"}
    };
};

#endif // EMPLOYEEMANAGEUI_H
