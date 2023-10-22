#ifndef STANDARDSMANAGER_H
#define STANDARDSMANAGER_H

#include "tabwigetbase.h"

#include"testitemmanager.h"
#include"implementingstandardeditor.h"
namespace Ui {
class StandardsManager;
}

class StandardsManager : public TabWidgetBase
{
    Q_OBJECT

public:
    explicit StandardsManager(QWidget *parent = nullptr);
    ~StandardsManager();
    void initMod()override;

private slots:
    void on_editTestItemBtn_clicked();

    void on_standardEditBtn_clicked();

    void on_impotrStandardBtn_clicked();

private:
    Ui::StandardsManager *ui;
    TestItemManager* m_testItemEditor;
    ImplementingStandardEditor *m_standardEditor;
};

#endif // STANDARDSMANAGER_H
