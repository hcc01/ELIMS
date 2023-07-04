#ifndef TESTINFOEDITOR_H
#define TESTINFOEDITOR_H

#include <QDialog>
#include"../Client/qjsoncmd.h"
namespace Ui {
class testInfoEditor;
}
struct TestInfo{
    QString sampleType;
    int sampleTypeID;
    int samplingSiteNumber;
    int samplingFrequency;
    int samplingPeriod;
    QString samplingSites;
    QString monitoringParameters;
    QList<int>parametersIDs;
    QString limitStandard;
    int limitStandardID;
    QString remark;
    QList<QVariant> infoList(){
        return {sampleType,samplingSites,monitoringParameters,QString("%1点*%2次*%3天").arg(samplingSiteNumber).arg(samplingFrequency).arg(samplingPeriod),limitStandard,remark};
        }
    };

class testInfoEditor : public QDialog
{
    Q_OBJECT

public:
    explicit testInfoEditor(TestInfo* info, QWidget *parent = nullptr);
    ~testInfoEditor();
    void init();
signals:
    void doSql(const QString& sql,DealFuc f,int p=0);
private slots:
    void on_testInofOkBtn_clicked();

    void on_testFiledBox_currentIndexChanged(int index);

    void on_testTypeBox_currentTextChanged(const QString &arg1);

    void on_testItemAddBtn_clicked();

private:
    Ui::testInfoEditor *ui;
    TestInfo* m_info;
    QList<int> m_testFieldIDs;//记录当前的检测类型

    QStringList m_monitoringParameters;
    QList<int> m_monitoringParameterIDs;
    int m_limitStandardID;
};

#endif // TESTINFOEDITOR_H
