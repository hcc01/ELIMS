#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cclient.h"
#include<QDialog>
#include<QMap>
#include"tabwigetbase.h"
#include "processmanager.h"
#include"cuser.h"
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
protected:
    void closeEvent(QCloseEvent *event) override ;
signals:
    void Connected();
    void loginResult(int result);
    void changeSkin(int);
    void notice(const QString&);//任务栏通知消息
//private:
public:
    TabWidgetBase *getTabWidget(const QString& widgetText)const;//模块的操作窗口
    TabWidgetBase *getModule(const QString& widgetText);//主要用于流程处理等非交互操作的地方，如果有窗口，直接返回操作窗口。
    void loadUser();
private slots:
    void sendData(const QJsonObject& json);
    void onNestMsg(netmsg_DataHeader* header);
    void onJsonCMD( const QJsonObject& json);
    void onOpenTab();//导航栏打开按纽的响应函数

    void on_tabWidget_tabCloseRequested(int index);

    void on_btEmployeeManage_clicked();

//    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_clicked();

    void on_btModInit_clicked();

    void on_actionInitMod_triggered();

    void onSkinChanged();

    void on_actionVersion_triggered();

    void on_exitAct_triggered();

    void on_updateTypeAct_triggered();

    void on_updateParameterAct_triggered();

private:
    Ui::MainWindow *ui;    
    CClient _clientSocket;
    QMap<QString,int*> _tabWidgetMap;
    bool isLogined;
    QDialog _waitDlg;
    CUser* m_user;
    QHash<QString,TabWidgetBase*>m_modules;//模块集，对于非交互界面的模块操作，创建的模块放在这里。
    QString m_company;

};
#endif // MAINWINDOW_H
