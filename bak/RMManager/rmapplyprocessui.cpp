#include "rmapplyprocessui.h"
#include "ui_rmapplyprocessui.h"

RMApplyProcessUI::RMApplyProcessUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RMApplyProcessUI)
{
    ui->setupUi(this);
}

RMApplyProcessUI::~RMApplyProcessUI()
{
    delete ui;
}
