#ifndef RMMANAGE_H
#define RMMANAGE_H

#include <QWidget>
#include "RMManager_global.h"
#include"../Client/ctablemodel.h"
#include"../Client/tabwigetbase.h"
#include"rmapplycheckui.h"
namespace Ui {
class RMManageUI;
}

class RMMANAGER_EXPORT RMManageUI : public TabWidgetBase
{
    Q_OBJECT
private:
    void init();
    void initCMD(){};
public:
    enum OperateType{
        QUERY_RM,
        ADD_RM,
        RM_APPLY,
        GET_CONTENT,
        NORETURN
    };
    explicit RMManageUI(QWidget *parent = nullptr);
    ~RMManageUI();
    static QString tabText(){
        return "标准物质管理";
    }
    bool initMod() override;//模块初始化，当新增该模块时，需要建表等操作。
    void doAddRM(const QJsonObject& dbINfo);
    void setData(const QJsonArray &data);
    void onSqlReturn(const QSqlReturnMsg& jsCmd) override;
    void dealProcess(const ProcessNoticeCMD& cmd) override;
    void test();
signals:
private slots:
    void on_pushButton_clicked();

    void on_btQuery_clicked();

    void on_btAddRM_clicked();
    void OnApply(const QModelIndex& index);//领用

    void on_btDoQuery_clicked();


private:
    Ui::RMManageUI *ui;
    RMApplyCheckUI _rmCheck;
    CTableModel _model;
    bool _isQuerying;
};

#endif // RMMANAGE_H
