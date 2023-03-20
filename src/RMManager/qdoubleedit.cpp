#include "qdoubleedit.h"
#include"QDoubleValidator"
QDoubleEdit::QDoubleEdit(QWidget *parent):
    QLineEdit(parent)
{
    QDoubleValidator* vd=new QDoubleValidator;

    vd->setRange(0,1000000);
    vd->setDecimals(6);
    vd->setNotation(QDoubleValidator::StandardNotation);
    this->setValidator(vd);
}

double QDoubleEdit::doubleValue()
{
    return this->text().toDouble();
}
