#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QWidget>
#include"qjsoncmd.h"
#include<QListWidget>


class ProcessManager : public QListWidget
{
    Q_OBJECT

public:
    explicit ProcessManager(QWidget *parent = nullptr);
    ~ProcessManager();
    void addTodo(const ProcessNoticeCMD &cmd);
    void todo(const ProcessNoticeCMD& cmd);


private:
};

#endif // PROCESSMANAGER_H
