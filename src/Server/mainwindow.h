#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cserer.h"
#include"cfileserver.h"
#include "qsqltablemodel.h"
#include "qtableview.h"
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

    void on_staticDataBtn_clicked();

    void on_viewLogBtn_clicked();

    void on_lineEdit_returnPressed();

private:
    Ui::MainWindow *ui;
    CServer _server;
    CFileServer _fileServer;
    QTableView* m_logView;
    QSqlTableModel *m_model;
};
#endif // MAINWINDOW_H
