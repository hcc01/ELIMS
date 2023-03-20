#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cserer.h"
#include"cfileserver.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btStart_clicked();

    void on_btCheckClients_clicked();

private:
    Ui::MainWindow *ui;
    CServer _server;
    CFileServer _fileServer;
};
#endif // MAINWINDOW_H
