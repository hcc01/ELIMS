#ifndef TODOUI_H
#define TODOUI_H

#include <QWidget>
#include"tabwigetbase.h"
namespace Ui {
class ToDoUI;
}
class MainWindow;
class ToDoUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit ToDoUI(QWidget *parent = nullptr);
    ~ToDoUI();
    void initCMD()override;
    void removeTodo(int row);
    void agreeFlow();
    void loadUser(CUser* user,MainWindow* main);
    bool pushProcess(QFlowInfo flowInfo, bool passed,const QString& comments);
signals:
    void dealFLow(const QFlowInfo&, int operateFlag);//发出审核结果信号，由各自的模块继续流程处理。
private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::ToDoUI *ui;
    QList<QFlowInfo>m_flowInfos;
    QList<int>m_flowIDs;
    MainWindow* m_main;
};

#endif // TODOUI_H
