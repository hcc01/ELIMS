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

private:
    Ui::ReportManagerUI *ui;
};

#endif // REPORTMANAGERUI_H
