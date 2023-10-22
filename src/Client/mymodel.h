#ifndef MYMODEL_H
#define MYMODEL_H

#include <QAbstractTableModel>
#include<QColor>
class MyModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MyModel(const QStringList& header={""},QObject *parent = nullptr);
    void setHeader(const QStringList& header);
    bool setRawData(QList<QVariant> data);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(int row,int colunm, int role = Qt::DisplayRole) const ;
    QVariant data(int row,const QString&head)const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QList<QList<QVariant>> getData()const;
    bool setData(int row,int colunm, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void append(const QList<QVariant> &);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void removeAll();
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setEditableColumn(int colunm);//设置可编辑的列
    void setReatedData(const QModelIndex&mapToCell, const QModelIndex& mapSourseCell, const QHash<QString, QVariant> &mapData);
    void setRelatedData(int row, int column, int sourseRow, int sourseColumn, QHash<QString, QVariant> relatedData);//用于数据关联，主要用于可选数据（检测方法选择时），当前的单元格数据与指定的单元格数据相关，相关性用HASH对应。
signals:
private:
    QList<QList<QVariant>> m_data;
    QStringList m_header;
    QList<int>m_editableColumns;
    QHash<QModelIndex,QHash<QString,QVariant>> m_relatedData;
    QHash<QModelIndex,QModelIndex>m_relateions;//用于数据关联，主要用于可选数据（检测方法选择时），当前的单元格数据与指定的单元格数据相关，相关性用HASH对应。
    QHash<QModelIndex,QColor>m_backgroundColors;
    QHash<QModelIndex,QVariant>m_flags;//用于保存一些标记数据
};

#endif // MYMODEL_H
