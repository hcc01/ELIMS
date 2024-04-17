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
    explicit employeeEditor(TabWidgetBase* tabWiget,bool addMode=true,QWidget *parent = nullptr);
    ~employeeEditor();
    void load(const QString&name,const QString&phone,const QString&EducationDegree,const QString&Title,const QString&posName,int position);
private slots:

    void on_positionSelectBtn_clicked();

    void on_finishBtn_clicked();

private:
    Ui::employeeEditor *ui;
    bool m_addMode;
    int m_position;
};

#endif // EMPLOYEEEDITOR_H
