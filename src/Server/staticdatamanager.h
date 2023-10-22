#ifndef STATICDATAMANAGER_H
#define STATICDATAMANAGER_H

#include <QDialog>

namespace Ui {
class StaticDataManager;
}

class StaticDataManager : public QDialog
{
    Q_OBJECT

public:
    explicit StaticDataManager(QWidget *parent = nullptr);
    ~StaticDataManager();
    bool checkParameter(int& parameterID, const QString&parameter,int areaID);//检测参数ID，如果不存在，插入检测参数。返回是否新参数
private slots:
    void on_improtLimitStandardBtn_clicked();

    void on_improtTestMethod_clicked();

    void on_improtParameterBtn_clicked();

    void on_parameterStdBtn_clicked();

private:
    Ui::StaticDataManager *ui;
};

#endif // STATICDATAMANAGER_H
