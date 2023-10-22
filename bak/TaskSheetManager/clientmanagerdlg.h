#ifndef CLIENTMANAGERDLG_H
#define CLIENTMANAGERDLG_H

#include <QDialog>
#include"../Client/qjsoncmd.h"
using DealFuc = std::function<void(const QSqlReturnMsg&)>;
namespace Ui {
class ClientManagerDlg;
}
struct ClientInfo{
    int ID;
    QString clientAddr;
};

class ClientManagerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ClientManagerDlg(bool editMod, QMap<QString,ClientInfo> *clients,QWidget *parent = nullptr);
    ~ClientManagerDlg();
signals:
    void doSql(const QString&sql,DealFuc f,int p=0);
private slots:
    void on_OKButton_clicked();

    void on_addContactsBtn_clicked();

    void on_clientNameBox_currentIndexChanged(int index);

private:
    Ui::ClientManagerDlg *ui;
    QMap<QString ,ClientInfo> *m_clients;
    bool m_editMod;
};

#endif // CLIENTMANAGERDLG_H
