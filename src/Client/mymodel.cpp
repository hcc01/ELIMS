#include "mymodel.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qdebug.h"

MyModel::MyModel(const QStringList &header, QObject *parent)
    : QAbstractTableModel{parent},
      m_header(header)
{

}

void MyModel::setHeader(const QStringList &header)
{
    beginResetModel();
    m_data.clear();
    m_header=header;
    endResetModel();
}

bool MyModel::setRawData(QList<QVariant> data)
{
    m_data.clear();
    for (const QVariant& variant : data) {
        // 检查当前QVariant是否可以转换为QList<QVariant>
        if (variant.canConvert<QList<QVariant>>()) {
            // 将QVariant转换为QList<QVariant>并添加到vector中
            QList<QVariant> innerVector = variant.value<QList<QVariant>>();
            m_data.append(innerVector);
        }
        else return false;
    }
    return true;
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
    else if (role == Qt::BackgroundRole) {// 检查该索引是否应该具有特定的背景色
        // 如果是，则返回相应的QBrush对象
        // 例如，使用一个名为m_backgroundColors的成员变量来保存单元格的背景色信息
        if (m_backgroundColors.contains(index))
        {
            QColor color = m_backgroundColors.value(index);
            return QBrush(color);
        }
    }
    else if(role==Qt::UserRole){
        return m_flags[index];
    }

    return QVariant();
}

QVariant MyModel::data(int row, int colunm, int role) const
{
    QModelIndex index = createIndex(row, colunm);
    return data(index,role);
}

QVariant MyModel::data(int row, const QString &head) const
{
    int c=m_header.indexOf(head);
    if(c<0) return QVariant();
    return data(row,c);
}

bool MyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() ) return false;
    if (role == Qt::EditRole) {
        m_data[index.row()][index.column()] = value;
        emit dataChanged(index, index, { role });
        if(m_relatedData.contains(index)){
            QModelIndex i=m_relateions.value(index);
            setData(i,m_relatedData.value(index).value(value.toString()),role);
        }
        return true;
    }
    else if(role==Qt::BackgroundRole){
        // 设置单元格背景色
        if (value.canConvert<QColor>()) {
            m_backgroundColors[index] = value.value<QColor>();
            emit dataChanged(index, index, { role, Qt::BackgroundRole });
        }
    }
    else if(role==Qt::UserRole){
        m_flags[index]=value;
    }

    return QAbstractTableModel::setData(index,value,role);
}

void MyModel::removeBackgroundColor()
{
    m_backgroundColors.clear();
}

QList<QList<QVariant> > MyModel::getData() const
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

void MyModel::append(const QList<QVariant> &list)
{
    int row = rowCount(); // 获取新行的行号
    beginInsertRows(QModelIndex(), row, row); // 发出信号通知视图有新行将要插入

    QList<QVariant> rowData(list.begin(), list.end());
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
           m_data.removeAt(row);

       }
       endRemoveRows();

       return true;

}

void MyModel::removeAll()
{
    if(!m_data.size()) return;
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
