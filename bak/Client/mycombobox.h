
#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H


#include <QComboBox>
#include<QCompleter>

class MyComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit MyComboBox(QWidget *parent = nullptr);
    void init(const QStringList& items);
signals:

private:
};

#endif // MYCOMBOBOX_H
