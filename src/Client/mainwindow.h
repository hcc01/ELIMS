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
signals:
    void Connected();
    void loginResult(int result);
    void changeSkin(int);
private:
    TabWidgetBase *getTabWidget(const QString& widgetText)const;

private slots:
    void sendData(const QJsonObject& json);
    void onNestMsg(netmsg_DataHeader* header);
    void onJsonCMD( const QJsonObject& json);
    void onOpenTab();//导航栏打开按纽的响应函数

    void on_tabWidget_tabCloseRequested(int index);

    void on_btEmployeeManage_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_clicked();

    void on_btModInit_clicked();

    void on_actionInitMod_triggered();

    void onSkinChanged();

    void on_actionVersion_triggered();

private:
    Ui::MainWindow *ui;    
    CClient _clientSocket;
    QMap<QString,int*> _tabWidgetMap;
    bool isLogined;
    QDialog _waitDlg;
    CUser* m_user;

};
#endif // MAINWINDOW_H
