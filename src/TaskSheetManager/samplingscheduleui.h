#ifndef SAMPLINGSCHEDULEUI_H
#define SAMPLINGSCHEDULEUI_H

#include "TaskSheetManager_global.h"
#include <QWidget>
#include"tabwigetbase.h"

#include"doscheduledlg.h"
namespace Ui {
class SamplingScheduleUI;
}

class TASKSHEETMANAGER_EXPORT SamplingScheduleUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit SamplingScheduleUI(QWidget *parent = nullptr);
    ~SamplingScheduleUI();
    void initMod()override;
    void initCMD()override;
    void doSchedule(QString startDate,QString endDate,QString leader,QString samplers,QString remark);
    void updateScheduledView();
    void updateToScheduledView();
private slots:
    void on_taskToScheduledView_doubleClicked(const QModelIndex &index);

    void on_taskScheduledView_doubleClicked(const QModelIndex &index);

    void on_refleshBtn_clicked();

    void on_myTaskBtn_clicked(bool checked);

    void on_myScheduleBtn_clicked(bool checked);

    void on_allBtn_clicked(bool checked);

private:
    Ui::SamplingScheduleUI *ui;
    QStringList m_allSamplers;
    DoScheduleDlg* m_doScheduledlg;
    bool m_reschedule;

};

#endif // SAMPLINGSCHEDULEUI_H
