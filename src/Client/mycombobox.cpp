
#include "mycombobox.h"

MyComboBox::MyComboBox(QWidget *parent)
    : QComboBox{parent}
{

}

void MyComboBox::init(const QStringList &items)
{
    setEditable(true);
    this->clear();
    this->addItems(items);
    QCompleter *completer = new QCompleter(items, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    setCompleter(completer);
}

