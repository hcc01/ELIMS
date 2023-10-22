#ifndef QDOUBLEEDIT_H
#define QDOUBLEEDIT_H

#include <QLineEdit>

class QDoubleEdit:public QLineEdit
{
public:
    QDoubleEdit(QWidget* parent=nullptr);
    double doubleValue();
};

#endif // QDOUBLEEDIT_H
