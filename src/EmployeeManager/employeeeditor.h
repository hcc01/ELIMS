#ifndef EMPLOYEEEDITOR_H
#define EMPLOYEEEDITOR_H

#include <QDialog>
#include"tabwigetbase.h"
namespace Ui {
class employeeEditor;
}

class employeeEditor : public QDialog,public SqlBaseClass
{
    Q_OBJECT

public:
    explicit employeeEditor(QMap<int,QString>positions,TabWidgetBase* tabWiget,QWidget *parent = nullptr);
    ~employeeEditor();

private slots:
    void on_positionSelectBtn_clicked();

    void on_finishBtn_clicked();

private:
    Ui::employeeEditor *ui;
    QMap<int,QString>m_positions;
    int m_position;
};

#endif // EMPLOYEEEDITOR_H
