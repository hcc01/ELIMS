#include "qwrod.h"

QWrod::QWrod(QObject *parent)
    : QObject{parent}
    , m_wordApplication(nullptr)
    , m_wordDocuments(nullptr)
    , m_wordDocument(nullptr)
{
    // 创建Word应用程序对象
    m_wordApplication = new QAxObject("Word.Application");
    m_wordDocuments = m_wordApplication->querySubObject("Documents");
}
