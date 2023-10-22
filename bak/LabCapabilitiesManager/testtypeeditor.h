#ifndef TESTTYPEEDITOR_H
#define TESTTYPEEDITOR_H

#include <QMainWindow>
#include"../Client/qjsoncmd.h"
namespace Ui {
class TestTypeEditor;
}

class TestTypeEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestTypeEditor(QWidget *parent = nullptr);
    ~TestTypeEditor();
    void init();
signals:
    void doSql(const QString&,DealFuc f=nullptr,int p=0,const QJsonArray&values={});
private slots:
    void on_addBtn_clicked();

    void on_FieldAddBtn_clicked();

    void on_FieldModifyBtn_clicked();

    void on_fieldBox_currentIndexChanged(int index);

    void on_testTypeAddBtn_clicked();

    void on_testTypeBox_currentIndexChanged(int index);

    void on_testTypeModifyBtn_clicked();

private:
    Ui::TestTypeEditor *ui;
};

#endif // TESTTYPEEDITOR_H
