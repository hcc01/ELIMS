#ifndef CUSER_H
#define CUSER_H
#include<QObject>
#include<QMap>
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
        SalesSupervisor=1<<20,//业务主管
        SystemAdin=1<<21,//系统管理员

    };
    void reset(const QString&name,int position){m_name=name;m_position=position;}
    QString name()const{return m_name;}
    int position()const{return m_position;}
    QString phone()const{return m_phone;}
    void setPhone(const QString&phone){m_phone=phone;}
    int id()const{return m_id;}
    void setID(int id){m_id=id;}
    static QMap<int,QString>allPositions(){
        return{
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
        {1<<19,"业务代表"},
            {1<<20,"业务主管"},
            {1<<21,"系统管理员"}

        };
    }
signals:
private:
    QString m_name;
    QString m_password;
    QString m_phone;
    int _authority;
    int m_position;
    int m_id;
};

#endif // CUSER_H
