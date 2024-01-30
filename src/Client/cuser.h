#ifndef CUSER_H
#define CUSER_H
#include<QObject>

class CUser
{
public:
    explicit CUser(QString name,int position);
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
    void reset(const QString&name,int position){m_name=name;m_position=position;}
    QString name()const{return m_name;}
    int position()const{return m_position;}
    QString phone()const{return m_phone;}
    void setPhone(const QString&phone){m_phone=phone;}
signals:
private:
    QString m_name;
    QString m_password;
    QString m_phone;
    int _authority;
    int m_position;
};

#endif // CUSER_H
