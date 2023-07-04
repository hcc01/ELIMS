#ifndef MYMODEL_H
#define MYMODEL_H

#include <QAbstractTableModel>

class MyModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MyModel(const QStringList& header,QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(int row,int colunm, int role = Qt::DisplayRole) const ;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVector<QVector<QVariant>> getData()const;
    bool setData(int row,int colunm, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void append(const QVector<QVariant>&);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void removeAll();
signals:
private:
    QVector<QVector<QVariant>> m_data;
    QStringList m_header;
};

#endif // MYMODEL_H
