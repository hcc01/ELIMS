#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cclient.h"
#include<QDialog>
#include<QMap>
#include"tabwigetbase.h"
#include "processmanager.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    void doTabwidgetMapping();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void DoConnect();
    void DoLogin();
signals:
    void Connected();
    void loginResult(int result);

private slots:
    void sendData(const QJsonObject& json);
    void onNestMsg(netmsg_DataHeader* header);
    void onJsonCMD( const QJsonObject& json);
    void onOpenTab();//导航栏的按纽统一使用这个槽，对应的TAB窗体必须有静态的tabText函数用于返回按纽名称（同时也是页的名称），并且需要使用tabFactory注册
    TabWidgetBase *getTabWidget(const QString& widgetText)const;
    void on_tabWidget_tabCloseRequested(int index);

    void on_btEmployeeManage_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_clicked();

    void on_btModInit_clicked();

private:
    Ui::MainWindow *ui;    
    CClient _clientSocket;
    QMap<QString,int*> _tabWidgetMap;
    bool isLogined;
    QDialog _waitDlg;

};
#endif // MAINWINDOW_H
