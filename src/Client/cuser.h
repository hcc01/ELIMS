#ifndef CUSER_H
#define CUSER_H
#include<QObject>

class CUser
{
public:
    explicit CUser(QString name,int position);
    enum Position {
        Position_Sampler = 1 << 0,   // 采样员
        Position_Sampler_Supervisor = 1 << 1,   // 采样主管
        Position_Sample_Manager = 1 << 2,   // 样品管理员
        Position_Equipment_Manager = 1 << 3,   // 设备管理员
        Position_Analyst = 1 << 4,   // 分析员
        Position_Analysis_Supervisor = 1 << 5,   // 分析主管
        Position_Report_Editor = 1 << 6,   // 报告编制员
        Position_Report_Auditor = 1 << 7,   // 报告审核员
        Position_Authorized_Signer = 1 << 8,   // 授权签字人
        Position_Quality_Manager = 1 << 9,   // 质量负责人
        Position_Technical_Manager = 1 << 10,   // 技术负责人
        Position_Lab_Manager = 1 << 11,   // 实验室负责人
        Position_Manager = 1 << 12,   // 经理
        Position_Standard_Substance_Manager = 1 << 13,   // 标准物质管理员
        Position_Supervisor = 1 << 14,   // 监督员
        Position_Internal_Auditor = 1 << 15,   // 内审员
    };
    void reset(const QString&name,int position){m_name=name;m_position=position;}
    QString name()const{return m_name;}
    int position()const{return m_position;}
signals:
private:
    QString m_name;
    QString m_password;
    int _authority;
    int m_position;
};

#endif // CUSER_H
