#ifndef SQLPAGECONTROLEUI_H
#define SQLPAGECONTROLEUI_H

#include "sqlPageControleUI_global.h"
#include<QWidget>
#include"../Client/qjsoncmd.h"

#include<QValidator>
namespace  Ui{
    class SqlPageControleUI;
}

class SQLPAGECONTROLEUI_EXPORT SqlPageControleUI:public QWidget
{
    Q_OBJECT
public:
    explicit SqlPageControleUI(QWidget* parent=nullptr);
    void setTotalPage(int totalPage);
    void setCurrentPage(int page);
    void setCmd(const QSqlCmd& cmd){_sqlCmd=cmd;}
signals:
    void pageChanged(const QJsonObject&);
private slots:

    void on_btNext_clicked();

    void on_btPre_clicked();

    void on_btGo_clicked();

private:
    Ui::SqlPageControleUI* ui;
    int _totalPage;
    QSqlCmd _sqlCmd;
    QIntValidator _vali;
};

#endif // SQLPAGECONTROLEUI_H
