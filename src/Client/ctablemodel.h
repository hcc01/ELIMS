#ifndef CTABLEMODEL_H
#define CTABLEMODEL_H
#include<QAbstractTableModel>
#include<QJsonArray>
class CTableModel:public QAbstractTableModel
{
public:
    CTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    void setModelData(const QJsonArray& data){
        beginResetModel();
        _data=data;
        endResetModel();
    }
    QVariant value(int row,int column){
        return _data[row+1].toArray()[column].toVariant();
    }

private:
    QJsonArray _data;
};

#endif // CTABLEMODEL_H
