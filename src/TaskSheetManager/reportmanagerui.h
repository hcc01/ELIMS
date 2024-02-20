#ifndef REPORTMANAGERUI_H
#define REPORTMANAGERUI_H
#include "TaskSheetManager_global.h"
#include <QWidget>
#include"tabwigetbase.h"
namespace Ui {
class ReportManagerUI;
}

class TASKSHEETMANAGER_EXPORT ReportManagerUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit ReportManagerUI(QWidget *parent = nullptr);
    ~ReportManagerUI();
    void initCMD()override;
    FlowWidget* flowWidget(const QFlowInfo&flowInfo)override;

private slots:
    void on_submitBtn_clicked();

    void on_reportEditBtn_clicked();

    void on_refleshBtn_clicked();

    void on_reviewRecordBtn_clicked();

private:
    Ui::ReportManagerUI *ui;
};

#endif // REPORTMANAGERUI_H
