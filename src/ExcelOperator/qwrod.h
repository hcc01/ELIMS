#ifndef QWROD_H
#define QWROD_H

#include "qaxobject.h"
#include <QObject>

class QWrod : public QObject
{
    Q_OBJECT

public:
    static QWrod& instance(){
        static QWrod word;
        return  word;
    }
private:
    explicit QWrod(QObject *parent = nullptr);

signals:
private:
    QAxObject* m_wordApplication; // Word应用程序对象
    QAxObject* m_wordDocuments;   // Word文档对象
    QAxObject* m_wordDocument;    // 当前打开的Word文档对象
};

#endif // QWROD_H
