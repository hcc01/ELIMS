#include "clineteditor.h"
#include "ui_clineteditor.h"

ClinetEditor::ClinetEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClinetEditor)
{
    ui->setupUi(this);
}

ClinetEditor::~ClinetEditor()
{
    delete ui;
}
