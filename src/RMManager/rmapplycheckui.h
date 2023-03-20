#ifndef RMAPPLYCHECKUI_H
#define RMAPPLYCHECKUI_H

#include <QDialog>

namespace Ui {
class RMApplyCheckUI;
}

class RMApplyCheckUI : public QDialog
{
    Q_OBJECT

public:
    explicit RMApplyCheckUI(QWidget *parent = nullptr);
    ~RMApplyCheckUI();
    void append(const QString &str);
private:
    Ui::RMApplyCheckUI *ui;
};

#endif // RMAPPLYCHECKUI_H
