#ifndef TESTINFOEDITOR_H
#define TESTINFOEDITOR_H

#include <QDialog>
#include"../Client/qjsoncmd.h"
namespace Ui {
class testInfoEditor;
}
struct SamplingSige{
    QString name;
    int id;
};

struct TestInfo{
    QString sampleType;//样品类型
    int testFieldID;//检测领域
    int testTypeID;//检测类型
    int samplingSiteCount=1;//点位数量
    int samplingFrequency;//采样频次
    int samplingPeriod;//采样天数
    QStringList samplingSites;//采样点位
    QStringList monitoringParameters;//检测参数
    QList<int>parametersIDs;//保存参数ID
    QString limitStandard;//执行标准
    int limitStandardID;//限值ID
    QString remark;//备注
    QList<QVariant> infoList(){
        return {sampleType,samplingSites.join("、"),monitoringParameters.join("、"),QString("%1点*%2次*%3天").arg(samplingSiteCount).arg(samplingFrequency).arg(samplingPeriod),limitStandard,remark};
    }
};

class testInfoEditor : public QDialog
{
    Q_OBJECT

public:
    explicit testInfoEditor(TestInfo* info, int inspectedEentityID, QWidget *parent = nullptr);
    ~testInfoEditor();
    void init();
signals:
    void doSql(const QString& sql,DealFuc f,int p=0,const QJsonArray& bindValuse={});
    void doSqlFinished();
private slots:
    void on_testInofOkBtn_clicked();

    void on_testFiledBox_currentIndexChanged(int index);

    void on_testTypeBox_currentTextChanged(const QString &arg1);

    void on_testItemAddBtn_clicked();

    void on_sampleSiteSeclectBtn_clicked();

private:
    Ui::testInfoEditor *ui;
    TestInfo* m_info;
    QList<int> m_testFieldIDs;
    QList<int> m_testTypeIDs;
    QStringList m_monitoringParameters;//检测项目
    QStringList m_samplingSites;
    QList<int> m_monitoringParameterIDs;//对应ＩＤ
    int m_limitStandardID;//限值ID
    int m_inspectedEentityID;//受检单位ID，用于查找历史检测信息
};

#endif // TESTINFOEDITOR_H
