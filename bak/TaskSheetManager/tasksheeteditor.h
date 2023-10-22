#ifndef TASKSHEETEDITOR_H
#define TASKSHEETEDITOR_H
#include"testinfoeditor.h"
#include <QMainWindow>
#include"../Client/tabwigetbase.h"
#include"clientmanagerdlg.h"

using DealFuc = std::function<void(const QSqlReturnMsg&)>;
namespace Ui {
class TaskSheetEditor;
}



class TaskSheetEditor : public QMainWindow,public SqlBaseClass
{
    Q_OBJECT

public:
    explicit TaskSheetEditor( TabWidgetBase* tabWiget,QWidget *parent = nullptr);
    ~TaskSheetEditor();
    void init();
    void doSave();
    void load(const QString& taskNum);
signals:
//    void doSql(const QString& sql,DealFuc f,int p=0,const QJsonArray& bindValuse={});
private slots:
    void on_inspectedComBox_currentIndexChanged(int index);

    void on_clientBox_currentIndexChanged(int index);

    void on_clientContactsBox_currentIndexChanged(int index);

    void on_inspectedContactsBox_currentIndexChanged(int index);

    void on_testFiledBox_currentIndexChanged(int index);

    void on_testFiledBox_currentTextChanged(const QString &arg1);

    void on_testTypeBox_currentTextChanged(const QString &arg1);

    void on_testItemAddBtn_clicked();

    void on_testInofOkBtn_clicked();

    void on_methodSelectBtn_clicked();

    void on_checkBox_stateChanged(int arg1);

    void on_saveBtn_clicked();

    void on_exportBtn_clicked();

    void on_clientContactsBox_currentTextChanged(const QString &arg1);

    void on_commitBtn_clicked();

private:
    Ui::TaskSheetEditor *ui;
    QMap<QString,ClientInfo>m_clients;//保存客户列表，用以通过客户名索引客户ID和地址，省得一直查询数据库。
    QMap<QString,QString>m_contacts;//保存联系人列表，用以通过联系人名，索引联系电话。问题是：联系人名可能重复？
    bool m_isSaving;
    bool m_bSaved;//是否已保存到数据库（保存后如果有修改，则需要修改数据库）
    int m_taskSheetID;//如果有保存，则保存下数据库中的任务单ID，用以更改。
    bool m_bTasksheetModified;//任务基本信息被修改，只需要更新任务单表。
    bool m_bTestInfoModified;//检测信息更改，则需要删除方法评审和和检测信息表，重新处理。
    int m_status;//保存下任务单状态，避免不必要的查询。
    //以下是需要保存的数据
    int m_inspectedEentityID;//受检单位ID
    QString m_taskNum;//保存任务单号，在做方法选择时需要提供任务单号
    int m_taskID;
    QList<TestInfo*>m_testInfo;//监测信息列表，保存到数据库时和显示在窗口时使用
    bool editable;
    QVector<QVector<QVariant>> m_mthds;//方法表，与方法界面的表头匹配

};

#endif // TASKSHEETEDITOR_H
