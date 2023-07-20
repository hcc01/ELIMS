#ifndef TESTITEMMANAGER_H
#define TESTITEMMANAGER_H

#include <QDialog>
#include"qjsoncmd.h"
namespace Ui {
class TestItemManager;
}

class TestItemManager : public QDialog
{
    Q_OBJECT

public:
    explicit TestItemManager(bool addMode=true,QWidget *parent = nullptr);
    ~TestItemManager();
    void init();
    void reset();
signals:
    void doSql(const QString&sql,DealFuc f,int p=0,const QJsonArray& bindValuse={});
private slots:
    void on_OKBtn_clicked();

    void on_fieldBox_currentTextChanged(const QString &arg1);

    void on_testNameEdit_editingFinished();

    void on_checkBox_clicked();

    void on_selectItemBtn_clicked();

    void on_testNameEdit_textChanged(const QString &arg1);

    void on_selectFieldBtn_clicked();

    void on_clearItemBtn_clicked();

private:
    Ui::TestItemManager *ui;
    QStringList m_subItemIds;
    bool m_newItem;
    bool m_itemEditChanged;
};

#endif // TESTITEMMANAGER_H
