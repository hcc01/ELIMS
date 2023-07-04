#ifndef TASKSHEETEDITOR_H
#define TASKSHEETEDITOR_H
#include"testinfoeditor.h"
#include <QMainWindow>
#include"../Client/qjsoncmd.h"
#include"clientmanagerdlg.h"
using DealFuc = std::function<void(const QSqlReturnMsg&)>;
namespace Ui {
class TaskSheetEditor;
}



class TaskSheetEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TaskSheetEditor( QWidget *parent = nullptr);
    ~TaskSheetEditor();
    void init();
signals:
    void doSql(const QString& sql,DealFuc f,int p=0);
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

private:
    Ui::TaskSheetEditor *ui;
    QMap<QString,ClientInfo>m_clients;//保存客户列表
    QMap<QString,QString>m_contacts;//保存联系人列表
    QList<TestInfo*>m_testInfo;
};

#endif // TASKSHEETEDITOR_H
