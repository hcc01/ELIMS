#ifndef SAMPLECIRCULATIONUI_H
#define SAMPLECIRCULATIONUI_H

#include <QWidget>
#include "TaskSheetManager_global.h"
#include<tabwigetbase.h>
namespace Ui {
class SampleCirculationUI;
}

class TASKSHEETMANAGER_EXPORT SampleCirculationUI : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit SampleCirculationUI(QWidget *parent = nullptr);
    ~SampleCirculationUI();
    virtual void initCMD()override;
    void doSamplingReceive();
    void initDeliveryTask();
    void doDeliveryReceive();
private slots:
    void on_sampleReceiveBtn_clicked();

    void on_refleshBtn_clicked();

    void on_samplingBtn_clicked();

    void on_deliveryBtn_clicked();

    void on_samplingReceiveBtn_clicked();

    void on_nextBtn_clicked();

    void on_printLabelBtn_clicked();

private:
    Ui::SampleCirculationUI *ui;
    int m_taskSheetID=0;
};

#endif // SAMPLECIRCULATIONUI_H
