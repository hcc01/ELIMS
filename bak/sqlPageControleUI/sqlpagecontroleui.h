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
    void setSql(const QString& sql,int p){m_sql=sql;m_currentPage=p;}
signals:
    void pageChanged(const QString&,int);
private slots:

    void on_btNext_clicked();

    void on_btPre_clicked();

    void on_btGo_clicked();

private:
    Ui::SqlPageControleUI* ui;
    int _totalPage;
    QString m_sql;
    int m_currentPage;
    QIntValidator _vali;
};

#endif // SQLPAGECONTROLEUI_H
