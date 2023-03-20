#include "rmapplycheckui.h"
#include "ui_rmapplycheckui.h"

RMApplyCheckUI::RMApplyCheckUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RMApplyCheckUI)
{
    ui->setupUi(this);
}

RMApplyCheckUI::~RMApplyCheckUI()
{
    delete ui;
}

void RMApplyCheckUI::append(const QString &str)
{
    ui->textBrowser->append(str);
}
