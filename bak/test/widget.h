#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
struct Test{
    int n;
    char str[10];
    QString str2;
    int m;
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    long long jiecheng(int n);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
