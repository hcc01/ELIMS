#include "mainwindow.h"

#include <QApplication>
#include<QSystemTrayIcon>
#include<QMenu>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.setQuitOnLastWindowClosed(false);
    // 创建系统托盘图标
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(QIcon("./logo.ico"));

    // 创建托盘菜单
    QMenu* trayMenu = new QMenu();
    QAction* quitAction = trayMenu->addAction("退出服务");

    // 将托盘菜单设置给托盘图标
    trayIcon->setContextMenu(trayMenu);

    // 显示托盘图标
    trayIcon->show();
    QObject::connect(trayIcon,&QSystemTrayIcon::activated,&w,&MainWindow::show);
    QObject::connect(quitAction, &QAction::triggered, &a, &QApplication::quit);
    return a.exec();
}
