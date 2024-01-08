#ifndef CLINETEDITOR_H
#define CLINETEDITOR_H


#include <QWidget>

namespace Ui {
class ClinetEditor;
}

class ClinetEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ClinetEditor(QWidget *parent = nullptr);
    ~ClinetEditor();

private:
    Ui::ClinetEditor *ui;
};

#endif // CLINETEDITOR_H
