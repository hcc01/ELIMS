#include "ctablemodel.h"

CTableModel::CTableModel(QObject *parent):
    QAbstractTableModel(parent)
{

}

int CTableModel::rowCount(const QModelIndex &parent) const
{
    return _data.size()-1;
}

int CTableModel::columnCount(const QModelIndex &parent) const
{
    return _data.at(0).toArray().size();
}

QVariant CTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (role == Qt::DisplayRole) {

        return _data.at(index.row()+1).toArray().at(index.column());
    }
    return QVariant();
}

QVariant CTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole&&orientation==Qt::Horizontal) return _data.at(0).toArray().at(section);
    return QAbstractTableModel::headerData( section,  orientation,  role);
}

