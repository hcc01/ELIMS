#ifndef IMPLEMENTINGSTANDARDSELECTDLG_H
#define IMPLEMENTINGSTANDARDSELECTDLG_H

#include <QDialog>
#include"../Client/qjsoncmd.h"
namespace Ui {
class ImplementingStandardSelectDlg;
}

class ImplementingStandardSelectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ImplementingStandardSelectDlg(QWidget *parent = nullptr);
    ~ImplementingStandardSelectDlg();
    void init();

signals:
    void doSql(const QString&sql,DealFuc f,int p=0);
    void selectDone(const QStringList&selectParameters,const QList<int>&selectParameterIDs,const QString&standardName,int limitStandardID);
private slots:
    void on_OkBtn_clicked();

    void on_standardNameBox_currentIndexChanged(int index);

    void on_tableNameBox_currentTextChanged(const QString &arg1);

private:
    Ui::ImplementingStandardSelectDlg *ui;
    QList<int>m_standardIDs;
    QList<int>m_selectParameterIDs;
    QList<int>m_parameterIDs;
    QStringList m_selectParameters;
};

#endif // IMPLEMENTINGSTANDARDSELECTDLG_H
