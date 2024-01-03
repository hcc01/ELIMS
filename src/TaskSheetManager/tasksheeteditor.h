#ifndef TASKSHEETEDITOR_H
#define TASKSHEETEDITOR_H
#include"testinfoeditor.h"
#include <QMainWindow>
#include"../Client/tabwigetbase.h"
#include"clientmanagerdlg.h"
#include"methodselectdlg.h"
using DealFuc = std::function<void(const QSqlReturnMsg&)>;
namespace Ui {
class TaskSheetEditor;
}

class TaskSheetEditor : public QMainWindow,public SqlBaseClass
{
    Q_OBJECT

public:
    explicit TaskSheetEditor( TabWidgetBase* tabWiget,int openMode=NewMode);
    ~TaskSheetEditor();
    void init();
    void doSave();
    bool saveMethod();
    void load(const QString& taskNum);
    enum OpenMode{NewMode,EditMode,ReviewMode,ViewMode};
    void setOpenMode(int mode);
    void reset();
    void updateTestInfoView();
private:
signals:
    void submitReview(int, const QString&);
private slots:
    void on_inspectedComBox_currentIndexChanged(int index);

    void on_clientBox_currentIndexChanged(int index);

    void on_clientContactsBox_currentIndexChanged(int index);

    void on_inspectedContactsBox_currentIndexChanged(int index);

//    void on_testFiledBox_currentIndexChanged(int index);

//    void on_testFiledBox_currentTextChanged(const QString &arg1);

//    void on_testTypeBox_currentTextChanged(const QString &arg1);

    void on_testItemAddBtn_clicked();

    void on_testInofOkBtn_clicked();

    void on_methodSelectBtn_clicked();

    void on_checkBox_stateChanged(int arg1);

    void on_saveBtn_clicked();

    void on_exportBtn_clicked();

    void on_clientContactsBox_currentTextChanged(const QString &arg1);

    void on_submitBtn_clicked();

    void on_contractEdit_editingFinished();

    void on_salesRepresentativeBox_currentTextChanged(const QString &arg1);

    void on_reportPurposEdit_currentTextChanged(const QString &arg1);

    void on_clientBox_currentTextChanged(const QString &arg1);

    void on_clientAddrEdit_editingFinished();

    void on_clientContactsPhoneEdit_editingFinished();

    void on_inspectedComBox_currentTextChanged(const QString &arg1);

    void on_inspectedAddrEdit_editingFinished();

    void on_inspectedContactsBox_currentTextChanged(const QString &arg1);

    void on_inspectedPhoneEidt_editingFinished();

    void on_projectNameEdit_editingFinished();

    void on_projectAddrEdit_editingFinished();

    void on_reportPeriodBox_textChanged(const QString &arg1);

    void on_reportCopiesBox_textChanged(const QString &arg1);

    void on_sampleDisposalBox_currentTextChanged(const QString &arg1);

    void on_methodSourseBox_currentTextChanged(const QString &arg1);

    void on_subpackageBox_currentTextChanged(const QString &arg1);

    void on_otherRequirementsEdit_editingFinished();

private:
    Ui::TaskSheetEditor *ui;
    QStatusBar* statusBar;
    QMap<QString,ClientInfo>m_clients;//保存客户列表，用以通过客户名索引客户ID和地址，省得一直查询数据库。
    QMap<QString,QString>m_contacts;//保存联系人列表，用以通过联系人名，索引联系电话。
    MethodSelectDlg *m_MethodDlg;
    testInfoEditor m_infoEditor;
    bool m_userOpereate;
    bool m_isSaving;
//    bool m_bSaved;//是否已保存到数据库（保存后如果有修改，则需要修改数据库）
    bool m_bTasksheetModified;//任务基本信息被修改，只需要更新任务单表。
    bool m_bTestInfoModified;//检测信息更改，则需要删除方法评审和和检测信息表，重新处理。
    bool m_bMethodModified;//检测方法被修改，只需要更新方法
    int m_status;//保存下任务单状态，避免不必要的查询。
    //以下是需要保存的数据
    int m_inspectedEentityID;//受检单位ID
    QString m_taskNum;//任务单编号 ，在载入任务单或保存任务单时自动获取
    int m_taskSheetID;//任务单ID，在载入或保存时生成
    QList<TestInfo*>m_testInfo;//保存任务单的监测信息
//    QList<TestInfo*>m_deletedTestInfo;//被删除的监测信息，在保存时从数据库中删除相关数据(直接全部数据删除重新录，没有使用这个办法）
    int m_mode;//打开模式（新单、修改模式、查看模式）
    QHash<QString,int>m_testTypeIDs;//类型映射表
    QHash<int,QHash<QString,int>>m_parameterIDs;//所有类型的参数ID表;
    QString m_createor;//记录录单员(暂时没用了）
};

#endif // TASKSHEETEDITOR_H
