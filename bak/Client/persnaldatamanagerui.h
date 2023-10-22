#ifndef PERSNALDATAMANAGERUI_H
#define PERSNALDATAMANAGERUI_H

#include <QWidget>
#include "tabwigetbase.h"
namespace Ui {
class PersnalDataManagerUI;
}

class PersnalDataManagerUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit PersnalDataManagerUI(QWidget *parent = nullptr);
    ~PersnalDataManagerUI();
    void initMod()override;
    void initCMD()override;

private slots:
    void on_informationOkbtn_clicked();

    void on_passwordOkbtn_clicked();

private:
    Ui::PersnalDataManagerUI *ui;
};

#endif // PERSNALDATAMANAGERUI_H
