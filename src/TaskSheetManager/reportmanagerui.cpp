
#include "reportmanagerui.h"
#include "ui_reportmanagerui.h"

ReportManagerUI::ReportManagerUI(QWidget *parent) :
    TabWidgetBase(parent),
    ui(new Ui::ReportManagerUI)
{
    ui->setupUi(this);
}

ReportManagerUI::~ReportManagerUI()
{
    delete ui;
}
