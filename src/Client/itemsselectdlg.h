#ifndef ITEMSSELECTDLG_H
#define ITEMSSELECTDLG_H

#include <QDialog>
#include<QListWidget>
namespace Ui {
class itemsSelectDlg;
}

class FilteredListWidget : public QListWidget {
public:
    FilteredListWidget(QWidget *parent = nullptr) : QListWidget(parent) {setSelectionMode(QAbstractItemView::MultiSelection);}

    void setItems(const QStringList &items,const QList<int>idList) {
        // 设置列表项，并保存原始列表项
//        QListWidget::addItems(items);
        for(int i=0;i<items.count();i++){
            QListWidgetItem *item = new QListWidgetItem(this);
            item->setText(items.at(i));
            if(idList.count()) item->setData(Qt::UserRole,idList.at(i));
            addItem(item);
            if(idList.count()) allItems.append({item->text(),idList.at(i)});
        }
//        this->items = items;
//        allItems = findItems("", Qt::MatchWildcard);
    }

    void setFilterText(const QString &text) {
        // 设置过滤文本，并更新列表显示
        filter_text = text;
        update();
    }

    QStringList getFilteredItems() const {
        // 返回过滤后的列表项
        QStringList filtered_items;
//        for (const QString &item : items) {
//            if (item.contains(filter_text, Qt::CaseInsensitive)) {
//                filtered_items.append(item);
//            }
//        }
        return filtered_items;
    }

private:
    QList<QPair<QString,int>> allItems;
    QString filter_text;

    void update() {
        // 根据当前过滤文本更新列表显示
        clear();
//        QStringList filtered_items;
//        for (const QString &item : items) {
//            if (item.contains(filter_text, Qt::CaseInsensitive)) {
//                filtered_items.append(item);
//            }
//        }
//        QListWidget::addItems(filtered_items);
        QList<QListWidgetItem*> filtered_items;
        for(auto item:allItems){
            if (item.first.contains(filter_text, Qt::CaseInsensitive)) {
                QListWidgetItem *x = new QListWidgetItem();
                x->setText(item.first);
                x->setData(Qt::UserRole,item.second);
                addItem(x);
            }
        }

    }
};
class itemsSelectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit itemsSelectDlg(const QStringList&list,const QList<int>&IDlist={},QWidget *parent = nullptr);
    ~itemsSelectDlg();
    static QStringList getSelectedItems(const QStringList& list,const QString&tytle="");
    static QList<int> getSelectedItemsID(const QStringList& fromList, const QList<int> &IDList, QStringList &selectedList, const QString &tytle="");
    static QString getSelectedItem(const QStringList& fromList);
private slots:
    void on_pushButton_clicked();

private:
    Ui::itemsSelectDlg *ui;
    FilteredListWidget* m_listWidget;
    QStringList m_selectedItems;
    QList<int> m_selectedIDs;
};

#endif // ITEMSSELECTDLG_H
