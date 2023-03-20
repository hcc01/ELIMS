#ifndef TABWIGETBASE_H
#define TABWIGETBASE_H
#include"../Client/qjsoncmd.h"
#include <QWidget>
#include<QJsonObject>
class TabWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidgetBase(QWidget *parent = nullptr);
    virtual ~TabWidgetBase(){}
    virtual void onSqlReturn(const QSqlReturnMsg& jsCmd)=0;//处理数据库操作返回的信息
    virtual void initCMD()=0;//用于窗口建立后给服务器发送初始化命令。设为纯虚是因为不知道为什么子类如果不写这个函数，调用就会奔溃！
    virtual void dealProcess(const ProcessNoticeCMD&)=0;//处理流程事件
    virtual bool initMod()=0;//新增模块时初始化操作，建表等。
signals:
    void sendData(const QJsonObject&);
};

#endif // TABWIGETBASE_H
