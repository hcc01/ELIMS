#ifndef TESTTYPEEDITOR_H
#define TESTTYPEEDITOR_H

#include <QMainWindow>
#include"../Client/qjsoncmd.h"
#include"../Client/tabwigetbase.h"
namespace Ui {
class TestTypeEditor;
}

class TestTypeEditor : public QMainWindow,public SqlBaseClass
{
    Q_OBJECT

public:
    explicit TestTypeEditor(TabWidgetBase*parent);
    ~TestTypeEditor();
    void init();

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
