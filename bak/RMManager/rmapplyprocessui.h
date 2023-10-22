#ifndef RMAPPLYPROCESSUI_H
#define RMAPPLYPROCESSUI_H

#include <QWidget>

namespace Ui {
class RMApplyProcessUI;
}

class RMApplyProcessUI : public QWidget
{
    Q_OBJECT

public:
    explicit RMApplyProcessUI(QWidget *parent = nullptr);
    ~RMApplyProcessUI();

private:
    Ui::RMApplyProcessUI *ui;
};

#endif // RMAPPLYPROCESSUI_H
