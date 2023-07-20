#ifndef METHODSELECTDLG_H
#define METHODSELECTDLG_H

#include <QDialog>
#include<QStyledItemDelegate>
#include<QComboBox>
#include"../Client/qjsoncmd.h"
namespace Ui {
class MethodSelectDlg;
}
class ComboBoxDelegate : public QStyledItemDelegate {
public:
    ComboBoxDelegate(const QStringList&items={}, QObject* parent = nullptr) : QStyledItemDelegate(parent),m_items(items) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QComboBox* editor = new QComboBox(parent);
        QPoint p=QPoint(index.row(),index.column());
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

    void setCellItems(int r,int c,QStringList items){//让不同单元格有不同的选择器
        m_CellItems.insert(QPoint(r,c),items);
    }
private:
   QStringList m_items;
    QHash<QPoint,QStringList>m_CellItems;
};

class MethodSelectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MethodSelectDlg(int taskID, QWidget *parent = nullptr);
    ~MethodSelectDlg();
signals:
    void doSql(const QString& sql,DealFuc f,int p=0,const QJsonArray& bindValuse={});
private slots:
    void on_pushButton_clicked();

private:
    Ui::MethodSelectDlg *ui;
    int m_taskID;
    ComboBoxDelegate* m_methodBox;
};

#endif // METHODSELECTDLG_H
