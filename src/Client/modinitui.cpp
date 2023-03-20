#include "modinitui.h"
#include "ui_modinitui.h"

ModInitUI::ModInitUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModInitUI)
{
    ui->setupUi(this);
}

ModInitUI::~ModInitUI()
{
    delete ui;
}

void ModInitUI::setTabsText(const QStringList &strList)
{
    ui->comboBox->addItems(strList);
}

void ModInitUI::on_buttonBox_accepted()
{
    if(ui->checkBox->isChecked()) {
        for(int i=0;i<ui->comboBox->count();i++) emit tabsToInit(ui->comboBox->itemText(i));
    }
    else emit tabsToInit(ui->comboBox->currentText());
}
