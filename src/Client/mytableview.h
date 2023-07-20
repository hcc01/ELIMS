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
    void setHeader(const QStringList& header);    //设置列名，初始化表格。
    void addContextAction(const QString&action, ActionFuc f);//添加右键命令
    void append(const QVector<QVariant>&);//添加一行数据
    QVector<QVector<QVariant>> data()const;//获取全部数据
    void clear();//清空
    bool setData(int row,int colunm, const QVariant &value, int role = Qt::EditRole);
    int selectedRow()const;//当前选中的行
    void setEditableColumn(int colunm);//设置可编辑的列
    void setMappingCell(int row, int column, int relatedToRow, int relatedTocolumn, QHash<QString, QVariant> relatedData);//设置单元格的数据关联到另一单元格
signals:
    void info(const QVector<QVariant>&);//发送行数据（这个是用于任务单显示监测信息）
private:
    MyModel* m_model;
    QAction* m_removeAction;
    QAction* m_infoAction;
    QMenu *m_contextMenu;

};

#endif // MYTABLEVIEW_H
