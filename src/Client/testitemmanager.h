#ifndef TESTITEMMANAGER_H
#define TESTITEMMANAGER_H
#include<QCompleter>
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
private:
    void setAddMode(bool addMode);
signals:
    void doSql(const QString&sql,DealFuc f,int p=0,const QJsonArray& bindValuse={},int ipp=0);
private slots:
    void on_OKBtn_clicked();

    void on_fieldBox_currentTextChanged(const QString &arg1);

    void on_testNameEdit_editingFinished();

    void on_checkBox_clicked();

    void on_selectItemBtn_clicked();

    void on_testNameEdit_textChanged(const QString &arg1);

    void on_selectFieldBtn_clicked();

    void on_clearItemBtn_clicked();

    void on_subParameterCheck_clicked();

    void on_delBtn_clicked();

private:
    Ui::TestItemManager *ui;
    QList<int> m_testFieldIDs;
    QStringList m_subItemIds;
    bool m_newItem;
    bool m_itemEditChanged;
    QStringList m_items;
    QMap<QString,int>m_itemIDs;
    QMap<int,QString>m_IDtoItems;
    int m_currentID;
    QCompleter* m_completer;
};

#endif // TESTITEMMANAGER_H
