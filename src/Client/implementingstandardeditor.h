#ifndef IMPLEMENTINGSTANDARDEDITOR_H
#define IMPLEMENTINGSTANDARDEDITOR_H

#include <QDialog>
#include"qjsoncmd.h"
namespace Ui {
class ImplementingStandardEditor;
}

class ImplementingStandardEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ImplementingStandardEditor(QWidget *parent = nullptr);
    ~ImplementingStandardEditor();
    void init();
signals:
    void doSql(const QString&sql,DealFuc f,int p=0,const QJsonArray& bindValuse={},int ipp=0);
private slots:
    void on_standardEidtBtn_clicked();

    void on_addTestItemBtn_clicked();

    void on_tableEditBtn_clicked();

    void on_classEditBtn_clicked();

    void on_standardEditOK_clicked();

    void on_classEditOK_clicked();

    void on_tableEditOk_clicked();

    void on_testTypeBox_currentIndexChanged(int index);

    void on_standardSelectBox_currentIndexChanged(int index);

    void on_tableSelectBox_currentIndexChanged(int index);

    void on_classSelectBox_currentIndexChanged(int index);

private:
    Ui::ImplementingStandardEditor *ui;
    QString m_standardName;
    QString m_standardNum;
    QString m_tableName;
    QString m_testType;
    QString m_limitClass;
    QStringList m_testTypes;
    QList<int> m_testTypeIDs;
    int m_testTypeID;
    int m_limitClassID;

};

#endif // IMPLEMENTINGSTANDARDEDITOR_H
