#ifndef METHODSELECTDLG_H
#define METHODSELECTDLG_H

#include <QDialog>
#include<QStyledItemDelegate>
#include<QComboBox>
#include<QTableView>
#include"../Client/qjsoncmd.h"
namespace Ui {
class MethodSelectDlg;
}
class ComboBoxDelegate : public QStyledItemDelegate {
public:
    ComboBoxDelegate(QTableView* view, const QStringList&items={}) : QStyledItemDelegate(view),m_items(items) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QComboBox* editor = new QComboBox(parent);
        QPair<int,int> p=QPair(index.row(),index.column());
        qDebug()<<p;
        qDebug()<<m_CellItems;
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

    void setCellItems(int row,int column,QStringList items){//让不同单元格有不同的选择器
        m_CellItems.insert(QPair(row,column),items);
    }
private:
   QStringList m_items;
   QHash<QPair<int,int>,QStringList>m_CellItems;
   QTableView* m_view;
};

class MethodSelectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MethodSelectDlg(int taskID, QWidget *parent = nullptr);
    ~MethodSelectDlg();
    void showMethods(const QList<QList<QVariant>>&table);
signals:
    void doSql(const QString& sql,DealFuc f,int p=0,const QJsonArray& bindValuse={});
    void doSqlFinished();
    void methodSelected(const QList<QList<QVariant>>&);
private slots:
    void on_pushButton_clicked();

    void on_OkBtn_clicked();

private:
    Ui::MethodSelectDlg *ui;
    int m_taskID;
    ComboBoxDelegate* m_methodBox;
    QList<int>m_parameterIDs;
    QList<int> m_testTypeIDs;
    bool m_saving;
    QList<QList<QVariant>> m_methodTable;
};

#endif // METHODSELECTDLG_H
