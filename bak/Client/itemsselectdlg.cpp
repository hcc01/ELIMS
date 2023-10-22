/*项目选择窗口
 * 用于选择检测项目，
 * 需要提供检测项目名称列表和对应的ID列表，返回所选择的ID
 * 使用方法：Qlist<int> list=itemsSelectDlg::getSelectedItemsID(const QStringList &fromList, const QList<int> &IDList, QStringList& selectedList)
 */
#include "itemsselectdlg.h"
#include "ui_itemsselectdlg.h"
#include<QVBoxLayout>
#include<QLineEdit>
itemsSelectDlg::itemsSelectDlg(const QStringList&items, const QList<int> &IDlist, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::itemsSelectDlg)
{
    ui->setupUi(this);
    m_listWidget=new FilteredListWidget(this);

    // 设置列表项
    QLineEdit*line_edit =new QLineEdit(this);
    m_listWidget->setItems(items,IDlist);


    // 创建布局管理器，并将控件添加到布局中
    QVBoxLayout *layout=new QVBoxLayout(this);
    layout->addWidget(line_edit);
    layout->addWidget(m_listWidget);
    layout->addWidget(ui->pushButton);
    // 创建主窗口，并将布局设置为主窗口的中央部件
    setLayout(layout);


    // 在 QLineEdit 的 textChanged 信号触发时更新过滤文本，并更新 FilteredListWidget 的显示
    connect(line_edit, &QLineEdit::textChanged, [&](const QString &text){
        m_listWidget->setFilterText(text);
    });
}

itemsSelectDlg::~itemsSelectDlg()
{
    delete ui;
}

QStringList itemsSelectDlg::getSelectedItems(const QStringList &list)
{
    itemsSelectDlg dlg(list);
    dlg.exec();
    return dlg.m_selectedItems;
}

QList<int> itemsSelectDlg::getSelectedItemsID(const QStringList &fromList, const QList<int> &IDList, QStringList& selectedList)
{
    itemsSelectDlg dlg(fromList,IDList);
    dlg.exec();
    selectedList= dlg.m_selectedItems;
    return dlg.m_selectedIDs;
}

QString itemsSelectDlg::getSelectedItem(const QStringList &fromList)
{
    itemsSelectDlg dlg(fromList);
    dlg.m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    dlg.exec();
    if(dlg.m_selectedItems.count()) return dlg.m_selectedItems.at(0);
    return "";
}

void itemsSelectDlg::on_pushButton_clicked()
{
    foreach(QListWidgetItem* x,m_listWidget->selectedItems()){
        m_selectedItems.append(x->text());
        m_selectedIDs.append(x->data(Qt::UserRole).toInt());
    }
    accept();
}

