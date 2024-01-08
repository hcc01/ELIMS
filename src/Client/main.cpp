#include "mainwindow.h"

#include <QApplication>
#include<QIcon>
#include<QFile>
#include<QSystemTrayIcon>
#include<QMenu>
#include<QSystemSemaphore>
#include<QSharedMemory>
#include<QDir>
#include<QLockFile>
bool isAnotherInstanceRunning()
{
    const QString mutexName = "MyApplicationMutex";
    const QString sharedMemoryKey = "MyApplicationSharedMemory";

    // 创建一个命名的互斥锁
    QSystemSemaphore semaphore(mutexName, 1, QSystemSemaphore::Open);

    // 尝试获取互斥锁
    if (semaphore.acquire() == QSystemSemaphore::AlreadyExists) {
        // 互斥锁已存在，表示另一个实例正在运行
        return true;
    }

    // 创建一个共享内存区域
    QSharedMemory sharedMemory(sharedMemoryKey);

    // 尝试创建共享内存
    if (!sharedMemory.create(1)) {
        // 共享内存创建失败，表示另一个实例正在运行
        return true;
    }

    // 释放互斥锁
    semaphore.release();

    return false;
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 本测试程序id取名为SingleAppTest
    QString path = QDir::temp().absoluteFilePath("SingleAppTest.lock");
    // path = C:/Users/yu/AppData/Local/Temp/SingleAppTest.lock
    QLockFile lockFile(path);

    bool isLock = lockFile.isLocked();
    // bool QLockFile::tryLock(int timeout = 0)
    // tryLock尝试创建锁定文件。此函数如果获得锁，则返回true; 否则返回false。
    // 如果另一个进程（或另一个线程）已经创建了锁文件，则此函数将最多等待timeout毫秒
    if (!lockFile.tryLock(100))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The application is already running.\n"
                       "Allowed to run only one instance of the application.");
        msgBox.exec();
        return 1;
    }
     a.setWindowIcon(QIcon("./logo.ico"));
    QFile qssFile("./style.qss");
    qssFile.open(QFile::ReadOnly);
    QString qssStyle;
    if (qssFile.isOpen()) {
        qssStyle = QLatin1String(qssFile.readAll());
        a.setStyleSheet(qssStyle);
    }
    MainWindow w;
    w.showMaximized();
//    w.show();
    QObject::connect(&w, &MainWindow::changeSkin, [&,qssStyle](int skin) {
        switch (skin) {
        case 0:
            a.setStyleSheet(qssStyle);
            break;
        case 1:
            a.setStyleSheet("");
        default:
            break;
        }
    });

    // 创建系统托盘图标
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(QIcon("./logo.ico"));

    // 创建托盘菜单
    QMenu* trayMenu = new QMenu();
    QAction* quitAction = trayMenu->addAction("退出");

    // 将托盘菜单设置给托盘图标
    trayIcon->setContextMenu(trayMenu);

    // 显示托盘图标
    trayIcon->show();

    QObject::connect(quitAction, &QAction::triggered, &a, &QApplication::quit);
    QObject::connect(trayIcon,&QSystemTrayIcon::activated,&w,&MainWindow::show);
    a.setQuitOnLastWindowClosed(false);
    QObject::connect(&w,&MainWindow::notice,[trayIcon](const QString&msg){
        trayIcon->showMessage("Notification", msg, QSystemTrayIcon::Information, 0);
    });
    return a.exec();
}
