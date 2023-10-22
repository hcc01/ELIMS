#include "mytableview.h"
#include<QMessageBox>
MyTableView::MyTableView(QWidget *parent)
    : QTableView{parent}
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    m_model=new MyModel({""},this);
    setModel(m_model);
    m_contextMenu=new QMenu(this);
    //    m_removeAction = new QAction(tr("删除"), this);
    //    connect(m_removeAction,&QAction::triggered,this,[&](){
    //        m_model->removeRows(selectedIndexes().first().row(),1);
    //    });
    //    m_infoAction = new QAction(tr("详情"), this);
    //    connect(m_infoAction,&QAction::triggered,this,[&](){
    //        emit info(data().at(selectedIndexes().first().row()));
    //    });
    //    m_contextMenu->addAction(m_removeAction);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MyTableView::customContextMenuRequested, this, [this](const QPoint &pos) {
        m_contextMenu->exec(mapToGlobal(pos));
    });
    //    m_contextMenu->addAction(m_infoAction);
}

void MyTableView::init(const QVariant &data)
{
    QList<QVariant> rows=data.toList();
    if(!rows.count()){
        QMessageBox::information(nullptr,"error","初始化表头错误，无效的表格数据。");
        return;
    }
    setHeader(rows.at(0).toStringList());
    rows.removeFirst();
    if(!m_model->setRawData(rows)){
        QMessageBox::information(nullptr,"error","初始化表格错误，无效的表格数据。");
        return;
    }
}

void MyTableView::setHeader(const QStringList &header)
{
    m_model->setHeader(header);
}

void MyTableView::addContextAction(const QString &action, ActionFuc f)
{
    QAction* a=new QAction(action, this);
    m_contextMenu->addAction(a);
    connect(a,&QAction::triggered,f);
}

void MyTableView::append(const QList<QVariant> &data)
{
    m_model->append(data);
    resizeRowToContents(m_model->rowCount()-1);
}

QModelIndex MyTableView::getIndex(int row, int column)
{
    return m_model->index(row,column);
}

QList<QList<QVariant> > MyTableView::data() const
{
    return m_model->getData();
}

QVariant MyTableView::value(int row, int column) const
{
    return m_model->data(row,column);
}

QVariant MyTableView::value(int row, const QString &head) const
{
    return m_model->data(row,head);
}

QVariant MyTableView::cellFlag(int row, int column) const
{
    return m_model->data(row,column,Qt::UserRole);
}



void MyTableView::clear()
{

    m_model->removeAll();
}

bool MyTableView::setData(int row, int colunm, const QVariant &value, int role)
{
    emit rowChanged(row);
    return m_model->setData(row,colunm,value,role);
}

void MyTableView::setBackgroundColor(int row, int colunm, const QColor &color)
{
    m_model->setData(m_model->index(row,colunm),QBrush(color),Qt::BackgroundRole);
}

void MyTableView::setCellFlag(int row, int colunm, const QVariant &value)
{
    m_model->setData(m_model->index(row,colunm),value,Qt::UserRole);
}

int MyTableView::selectedRow() const
{
    if(!selectedIndexes().count()) return -1;
    return selectedIndexes().first().row();
}

void MyTableView::setEditableColumn(int colunm)
{
    m_model->setEditableColumn(colunm);
}

void MyTableView::setMappingCell(int row, int column, int relatedToRow, int relatedTocolumn, QHash<QString, QVariant> relatedData)
{
    m_model->setRelatedData(row, column, relatedToRow,  relatedTocolumn, relatedData);
}
