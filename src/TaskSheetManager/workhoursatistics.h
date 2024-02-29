#ifndef WORKHOURSATISTICS_H
#define WORKHOURSATISTICS_H

#include "tabwigetbase.h"
#include <QWidget>
#include"TaskSheetManager_global.h"
namespace Ui {
class WorkHourSatistics;
}

class TASKSHEETMANAGER_EXPORT WorkHourSatistics : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit WorkHourSatistics(QWidget *parent = nullptr);
    ~WorkHourSatistics();

private slots:
    void on_OKbtn_clicked();

private:
    Ui::WorkHourSatistics *ui;
};

#endif // WORKHOURSATISTICS_H
