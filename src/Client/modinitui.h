#ifndef MODINITUI_H
#define MODINITUI_H

#include <QDialog>

namespace Ui {
class ModInitUI;
}

class ModInitUI : public QDialog
{
    Q_OBJECT

public:
    explicit ModInitUI(QWidget *parent = nullptr);
    ~ModInitUI();
    void setTabsText(const QStringList& strList);
signals:
    void tabsToInit(const QString& );
private slots:
    void on_buttonBox_accepted();

private:
    Ui::ModInitUI *ui;
};

#endif // MODINITUI_H
