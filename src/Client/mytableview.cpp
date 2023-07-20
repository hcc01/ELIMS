#include "mytableview.h"

MyTableView::MyTableView(QWidget *parent)
    : QTableView{parent}
{
    setSelectionBehavior(QAbstractItemView::SelectRows);

}

void MyTableView::setHeader(const QStringList &header)
{
    m_model=new MyModel(header,this);
    setModel(m_model);
//    m_removeAction = new QAction(tr("删除"), this);
//    connect(m_removeAction,&QAction::triggered,this,[&](){
//        m_model->removeRows(selectedIndexes().first().row(),1);
//    });
//    m_infoAction = new QAction(tr("详情"), this);
//    connect(m_infoAction,&QAction::triggered,this,[&](){
//        emit info(data().at(selectedIndexes().first().row()));
//    });

    m_contextMenu=new QMenu(this);

//    m_contextMenu->addAction(m_removeAction);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MyTableView::customContextMenuRequested, this, [this](const QPoint &pos) {
            m_contextMenu->exec(mapToGlobal(pos));
        });
//    m_contextMenu->addAction(m_infoAction);
}

void MyTableView::addContextAction(const QString &action, ActionFuc f)
{
    QAction* a=new QAction(action, this);
    m_contextMenu->addAction(a);
    connect(a,&QAction::triggered,f);
}

void MyTableView::append(const QVector<QVariant> &data)
{
    m_model->append(data);
    resizeRowToContents(m_model->rowCount()-1);
}

QVector<QVector<QVariant> > MyTableView::data() const
{
    return m_model->getData();
}

void MyTableView::clear()
{

    m_model->removeAll();
}

bool MyTableView::setData(int row, int colunm, const QVariant &value, int role)
{
    return m_model->setData(row,colunm,value,role);
}

int MyTableView::selectedRow() const
{
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
