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
private slots:
    void on_sampleReceiveBtn_clicked();

private:
    Ui::SampleCirculationUI *ui;
};

#endif // SAMPLECIRCULATIONUI_H
