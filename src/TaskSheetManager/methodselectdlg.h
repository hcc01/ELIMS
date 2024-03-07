#ifndef METHODSELECTDLG_H
#define METHODSELECTDLG_H

#include <QDialog>
#include<QStyledItemDelegate>
#include<QComboBox>
#include<QTableView>
#include "tabwigetbase.h"
#include"testinfoeditor.h"
#include <QSharedPointer>
#include<mytableview.h>
namespace Ui {
class MethodSelectDlg;
}
struct MethodMore{
    int methodID;
    QString testMethodName;
    bool subpackage;
    QString subpackageDesc;
    bool CMA;
    int testMod;//是否现场测试，用于样品分组
    QString sampleGroup;//标准样品组
};
using MethodMorePtr = QSharedPointer<MethodMore>;

class MethodSelectDlg : public QDialog,public SqlBaseClass
{
    Q_OBJECT

public:
    explicit MethodSelectDlg(TabWidgetBase *tabWiget);
    ~MethodSelectDlg();
    void setTestInfo(int taskSheetID,QList<TestInfo*>testInfo);
    void showMethods(const QList<QList<QVariant>>&table);
    void addMethod(int testTypeID, int parameterID, QString method) {
        // 创建 QSharedPointer 对象并存储到 m_methods 中
        m_methods[testTypeID][parameterID] = method;
    }
    void reset();
    QList<QList<QVariant>> methodTable()const;
    QString getMethod(int testTypeID,int parameterID)const{return m_methods.value(testTypeID).value(parameterID);}


private slots:
    void on_loadMethodBtn_clicked();

    void on_OkBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::MethodSelectDlg *ui;
    ComboBoxDelegate* m_methodBox;//方法单元格
    bool m_methodLoad;
    QList<TestInfo*>m_testInfo;//保存任务单的监测信息，这里的TestInfo*是任务单中的变量，不能去删除
    QHash<QString,int>m_typeIDs;//类型-ID映射表
    QHash<int,QHash<QString,int>>m_parameterIDs;//项目-ID映射表
    QHash<QString,int>m_methodIDs;//方法映射表
//    QHash<int,MethodMorePtr>m_MethodMores;//标准方法详情【方法参数表ID，方法详情】，用于根据ID加载方法详情
//    QHash<int,QHash<int,MethodMorePtr>>m_methods;//按类型确认的方法表【类型ID，【参数ID，方法ID】】；使用智能指针，在清空映射时会自动释放资源
    QHash<int,QHash<int,QString>>m_methods;//这个现在用来记录之前选择的方法，在重新加载是，将其列在优先位。
//    QHash<int,QHash<int,MethodMorePtr>>m_specialMehtods;//特别项目的方法，如果有需要（如进口颗粒物的选用的方法与其它不同。);<点位ID，{<参数ID，方法ID>}>
    int m_taskSheetID;
};

#endif // METHODSELECTDLG_H
