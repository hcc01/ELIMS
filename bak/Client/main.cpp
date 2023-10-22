#include "mainwindow.h"

#include <QApplication>
#include<QIcon>
#include<QFile>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    a.setWindowIcon(QIcon(":/icon/logo.png"));
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
    return a.exec();
}
