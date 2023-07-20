#include "mymodel.h"

MyModel::MyModel(const QStringList &header, QObject *parent)
    : QAbstractTableModel{parent},
      m_header(header)
{

}


int MyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

int MyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_header.size();
}

QVariant MyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_data[index.row()][index.column()];
    }

    return QVariant();
}

QVariant MyModel::data(int row, int colunm, int role) const
{
    QModelIndex index = createIndex(row, colunm);
    return data(index,role);
}

bool MyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()][index.column()] = value;
        emit dataChanged(index, index, { role });
        if(m_relatedData.contains(index)){
            QModelIndex i=m_relateions.value(index);
            setData(i,m_relatedData.value(index).value(value.toString()),role);
        }
        return true;
    }

    return false;
}

QVector<QVector<QVariant> > MyModel::getData() const
{
    return m_data;
}

bool MyModel::setData(int row, int colunm, const QVariant &value, int role)
{
    QModelIndex index = createIndex(row, colunm);
    return setData(index, value,role);
}

QVariant MyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
       return m_header.at(section);
    }
    if (orientation == Qt::Vertical) {
        return section+1;
    }
    return QVariant();
}

void MyModel::append(const QVector<QVariant> &rowData)
{
    int row = rowCount(); // 获取新行的行号
    beginInsertRows(QModelIndex(), row, row); // 发出信号通知视图有新行将要插入
    m_data.append(rowData); // 将新行添加到模型中
    endInsertRows(); // 发出信号通知视图新行已经插入
}

bool MyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
       if (row < 0 || row + count > m_data.size())
           return false;

       beginRemoveRows(QModelIndex(), row, row + count - 1);
       for (int i = 0; i < count; ++i) {
           m_data.remove(row);
       }
       endRemoveRows();

       return true;

}

void MyModel::removeAll()
{
    beginRemoveRows(QModelIndex(), 0,  m_data.size()-1);
    m_data.clear();
    endRemoveRows();
}

Qt::ItemFlags MyModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (m_editableColumns.indexOf(index.column())>=0) {
           flags |= Qt::ItemIsEditable;
    }
    return flags;
}

void MyModel::setEditableColumn(int colunm)
{
    m_editableColumns.append(colunm);
}

void MyModel::setReatedData(const QModelIndex &mapToCell, const QModelIndex &mapSourseCell, const QHash<QString, QVariant> &mapData)
{
    m_relateions.insert(mapSourseCell,mapToCell);
    m_relatedData.insert(mapSourseCell,mapData);
}

void MyModel::setRelatedData(int row, int column, int relatedToRow, int relatedTocolumn, QHash<QString, QVariant> relatedData)
{
     QModelIndex index = createIndex(row, column);
    QModelIndex relatedIndex=createIndex(relatedToRow, relatedTocolumn);
     setReatedData(index,relatedIndex,relatedData);
}
