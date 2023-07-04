#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QTableView>
#include"mymodel.h"
#include<QAction>
#include<QMenu>
using ActionFuc = std::function<void()>;
class MyTableView : public QTableView
{
    Q_OBJECT
public:
    explicit MyTableView(QWidget *parent = nullptr);
    void setHeader(const QStringList& header);    
    void addContextAction(const QString&action, ActionFuc f);
    void append(const QVector<QVariant>&);
    QVector<QVector<QVariant>> data()const;
    void clear();
    int selectedRow()const;
signals:
    void info(const QVector<QVariant>&);
private:
    MyModel* m_model;
    QAction* m_removeAction;
    QAction* m_infoAction;
    QMenu *m_contextMenu;

};

#endif // MYTABLEVIEW_H
