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
    void init(const QVariant &data);
    void setHeader(const QStringList& header);    //设置列名，初始化表格。
    void addContextAction(const QString&action, ActionFuc f);//添加右键命令
    void append(const QList<QVariant> &);//添加一行数据
    QModelIndex getIndex(int row, int column);
    QList<QList<QVariant>> data()const;//获取全部数据
    QVariant value(int row,int column)const;
    QVariant value(int row,const QString&head)const;
    QVariant cellFlag(int row,int column)const;
    void clear();//清空
    bool setData(int row,int colunm, const QVariant &value, int role = Qt::EditRole);
    void setBackgroundColor(int row,int colunm,const QColor& color);
    void setCellFlag(int row,int colunm,const QVariant &value);
    int selectedRow()const;//当前选中的行
    void setEditableColumn(int colunm);//设置可编辑的列
    void setMappingCell(int row, int column, int relatedToRow, int relatedTocolumn, QHash<QString, QVariant> relatedData);//设置单元格的数据关联到另一单元格
signals:
    void info(const QList<QVariant>&);//发送行数据（这个是用于任务单显示监测信息）
    void rowChanged(int row);
private:
    MyModel* m_model;
    QAction* m_removeAction;
    QAction* m_infoAction;
    QMenu *m_contextMenu;

};

#endif // MYTABLEVIEW_H
