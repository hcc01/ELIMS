#include "employeemanageui.h"
#include "ui_employeemanageui.h"
#include"QDebug"
EmployeeManageUI::EmployeeManageUI( QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::EmployeeManageUI)
{
    ui->setupUi(this);
    qDebug()<<"EmployeeManageUI()";
}

EmployeeManageUI::~EmployeeManageUI()
{
    qDebug()<<"~EmployeeManageUI()";
    delete ui;
}

void EmployeeManageUI::onSqlReturn(const QSqlReturnMsg &jsCmd)
{

}

void EmployeeManageUI::initCMD()
{
    qDebug()<<"EmployeeManageUI::initCMD()";
}

bool EmployeeManageUI::initMod()
{

}
