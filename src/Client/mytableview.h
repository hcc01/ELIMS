#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QTableView>
#include"mymodel.h"
#include "qcombobox.h"
#include "qstyleditemdelegate.h"
#include<QAction>
#include<QMenu>
using ActionFuc = std::function<void()>;
class ComboBoxDelegate : public QStyledItemDelegate {
public:
    ComboBoxDelegate(QTableView* view, const QStringList&items={}) : QStyledItemDelegate(view),m_items(items) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QComboBox* editor = new QComboBox(parent);
        QPair<int,int> p=QPair<int, int>(index.row(),index.column());

        if(m_CellItems.contains(p)){
            editor->addItems(m_CellItems.value(p));
        }
        else{
            editor->addItems(m_items);
        }

        editor->setCurrentIndex(0);
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QString text = index.model()->data(index, Qt::EditRole).toString();
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(text);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }

    void setCellItems(int row,int column,QStringList items){//让不同单元格有不同的选择器.BUG: 当VIEW清空后，选择器还在！！！需要一个清空动作。
        m_CellItems.insert(QPair<int, int>(row,column),items);
    }
    void clearCellItems(){m_CellItems.clear();}
    QHash<QPair<int,int>,QStringList>cellItems()const {return m_CellItems;}
private:
    QStringList m_items;
    QHash<QPair<int,int>,QStringList>m_CellItems;

};

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
    void deleteRow(int row);
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
