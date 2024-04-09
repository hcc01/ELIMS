#ifndef TESTMANAGER_H
#define TESTMANAGER_H

#include <QWidget>
#include"tabwigetbase.h"
#include"TaskSheetManager_global.h"
namespace Ui {
class TestManager;
}

class  TASKSHEETMANAGER_EXPORT TestManager : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit TestManager(QWidget *parent = nullptr);
    ~TestManager();
    void initCMD();
private slots:
    void on_myTask_clicked();

    void on_onSamplingBtn_clicked();

    void on_UnassignedBtn_clicked();

    void on_AssignedBtn_clicked();

    void on_startTestBtn_clicked();

    void on_submitBtn_clicked();

private:
    Ui::TestManager *ui;
};

#endif // TESTMANAGER_H
