#ifndef BUSINESSMANAGERUI_H
#define BUSINESSMANAGERUI_H

#include <QWidget>
#include"tabwigetbase.h"
namespace Ui {
class BusinessManagerUI;
}

class BusinessManagerUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit BusinessManagerUI(QWidget *parent = nullptr);
    ~BusinessManagerUI();
    void initMod()override;
    void initCMD()override;

private slots:
    void on_clientView_doubleClicked(const QModelIndex &index);

    void on_addClientBtn_clicked();

    void on_refleshBtn_clicked();

    void on_EditSaleManBtn_clicked();

    void on_clientEditBtn_clicked();

private:
    Ui::BusinessManagerUI *ui;
    QStringList m_sales;
};

#endif // BUSINESSMANAGERUI_H
