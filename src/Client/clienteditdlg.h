#ifndef CLIENTEDITDLG_H
#define CLIENTEDITDLG_H

#include <QDialog>
#include"tabwigetbase.h"
namespace Ui {
class ClientEditDlg;
}

class ClientEditDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ClientEditDlg(TabWidgetBase *parent );
    ~ClientEditDlg();
    void load(const QString&clientName,const QString&clientAddr,const QString&contact,const QString&phone,int id);
private slots:
    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::ClientEditDlg *ui;
    TabWidgetBase* tab;
    bool m_editMod;
    int m_clientID;
};

#endif // CLIENTEDITDLG_H
