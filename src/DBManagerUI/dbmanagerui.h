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

    static QString tabText(){
        return "数据库管理";
    }
    void onSqlReturn(const QSqlReturnMsg& jsCmd);
    void dealProcess(const ProcessNoticeCMD& cmd) override{};
    bool initMod()override;
private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_lineEdit_returnPressed();

    void on_bt_PageUP_clicked();

    void on_bt_PageDown_clicked();

    void on_comboBox_editTextChanged(const QString &arg1);

private:
    void initCMD();
private:
    Ui::DBManagerUI *ui;
    CTableModel _model;
};

#endif // DBMANAGERUI_H
