#ifndef DBMATER_H
#define DBMATER_H
#define DB DBMater::Instance()
#include <QObject>
#include "QtSql/qsqlerror.h"
#include "QtSql/qsqlquery.h"
class DBMater : public QObject
{
    Q_OBJECT
public:
    static DBMater& Instance(){
        static DBMater dm;
        return dm;
    }
    QSqlDatabase database(){return m_db;}
private:
    explicit DBMater(QObject *parent = nullptr);

signals:
private:
    QSqlDatabase m_db;
};

#endif // DBMATER_H
