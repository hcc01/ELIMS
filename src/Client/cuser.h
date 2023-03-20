#ifndef CUSER_H
#define CUSER_H
#include<QObject>

class CUser : public QObject
{
    Q_OBJECT
public:
    explicit CUser(QObject *parent = nullptr);

signals:
private:
    QString _name;
    QString _password;
    int _authority;
};

#endif // CUSER_H
