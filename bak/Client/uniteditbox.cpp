#include "uniteditbox.h"
#include<QCompleter>
UnitEditBox::UnitEditBox(QWidget *parent)
    : QComboBox{parent}
{
    QStringList items={"mg/kg","μg/g","μg/kg","pg/kg","ng/kg","mg/L","μg/L","μg/mL","ng/L","mg/m3","μg/m3","pg/m3","ng/m3",
"CFU/L","CFU/ml","MPN/L","MPN/100mL","m3/h","m3/d","kg/h","kg/d","μS/cm","mS/m"};

    addItems(items);
    QCompleter *completer = new QCompleter(items, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    setCompleter(completer);
    setEditable(true);
    connect(this,&QComboBox::currentTextChanged,this,[&](const QString&s){
        if(s.contains("u")){
            QString str=s;
            str.replace("u","μ");
                setCurrentText(str);
        }
    });
}
