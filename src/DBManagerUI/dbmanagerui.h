#ifndef DBMANAGERUI_H
#define DBMANAGERUI_H

#include <QWidget>
#include"DBManagerUI_global.h"
#include"../Client/tabwigetbase.h"
#include"../Client/ctablemodel.h"
#include"../Client/qjsoncmd.h"
namespace Ui {
class  DBManagerUI;
}

class DBMANAGERUI_EXPORT DBManagerUI : public TabWidgetBase
{
    Q_OBJECT

public:
    enum cmdType{
        GET_TABLE,
        CHANGE_TABLE,
        DO_SQL
    };
    explicit DBManagerUI(QWidget *parent = nullptr);
    ~DBManagerUI();

    void showEvent(QShowEvent *event)override;
    static QString tabText(){
        return "数据库管理";
    }
//    void onSqlReturn(const QSqlReturnMsg& jsCmd);
//    void dealProcess(const ProcessNoticeCMD& cmd) override{};

    void initMod()override;
    void showTable(const QSqlReturnMsg&msg);

signals:
    void sqlFinished();
private slots:
    void onPageChanged(const QString&sql,int p);
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_lineEdit_returnPressed();

    void on_bt_PageUP_clicked();

    void on_bt_PageDown_clicked();

    void on_comboBox_editTextChanged(const QString &arg1);

    void on_exportToExcelBtn_clicked();

    void on_RefreshBtn_clicked();

private:
    void initCMD();
private:
    Ui::DBManagerUI *ui;
    CTableModel _model;
    QSqlCmd _sqlCmd;
    int _totalPage;
};

#endif // DBMANAGERUI_H
