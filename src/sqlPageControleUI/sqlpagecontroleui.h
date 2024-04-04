#ifndef SQLPAGECONTROLEUI_H
#define SQLPAGECONTROLEUI_H

#include "sqlPageControleUI_global.h"
#include<QWidget>
#include"../Client/qjsoncmd.h"
#include<QTableView>
#include<QValidator>
#include"../Client/tabwigetbase.h"
namespace  Ui{
    class SqlPageControleUI;
}

class SQLPAGECONTROLEUI_EXPORT SqlPageControleUI:public QWidget
{
    Q_OBJECT
public:
    explicit SqlPageControleUI(QWidget* tab=nullptr);
    void setTabWidget(TabWidgetBase* tab){m_sqlClass=tab;};
    void setTotalPage(int totalPage);
    void setCurrentPage(int page);
    void startSql(TabWidgetBase *tab, const QString& sql, int p, QJsonArray v={}, DealFuc f=nullptr,int itemsPerPage=0);//这是新的使用方式
    void setSql(const QString& sql,int p){m_sql=sql;m_currentPage=p;}//这个是之前的使用方式，保留，避免修改
    void updatePage();
    void dealSqlReturn(const QSqlReturnMsg &msg);
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
    QJsonArray m_values;
    int m_currentPage;
    QIntValidator _vali;
    DealFuc m_dealFuc;//查询处理函数
    TabWidgetBase* m_sqlClass;
};

#endif // SQLPAGECONTROLEUI_H
