#ifndef RMADDUI_H
#define RMADDUI_H

#include <QDialog>

namespace Ui {
class RMAddUI;
}

class RMAddUI : public QDialog
{
    Q_OBJECT

public:
    explicit RMAddUI(QWidget *parent = nullptr);
    ~RMAddUI();
signals:
    void doQuery(const QString&sql);
private slots:
    void on_btAdd_clicked();

    void on_btFinished_clicked();

private:
    Ui::RMAddUI *ui;
};

#endif // RMADDUI_H
