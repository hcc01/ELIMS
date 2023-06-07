#ifndef TABWIGETBASE_H
#define TABWIGETBASE_H
/*这个是模块的基类。模块的添加使用DLL创建UI界面，在主窗口的导航栏添加按钮打开模块窗口。
 *
 *
 *
 *
 *
 *
 *
 */
#include"../Client/qjsoncmd.h"
#include <QWidget>
#include<QJsonObject>
//流程处理函数
//typedef void (*DealFuc) (const QSqlReturnMsg& );
class TabWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidgetBase(QWidget *parent = nullptr);
    virtual ~TabWidgetBase(){}
    virtual void onSqlReturn(const QSqlReturnMsg& jsCmd);
    virtual void dealProcess(const ProcessNoticeCMD&)=0;//处理流程事件
    virtual bool initMod()=0;//新增模块时初始化操作，建表等。
    //virtual void initCMD()=0;//用于窗口建立后给服务器发送初始化命令。设为纯虚是因为不知道为什么子类如果不写这个函数，调用就会奔溃！
    virtual void initCMD(){}//这个没用了，先保留着。
    void doSqlQuery(const QString&sql,DealFuc f=nullptr,int page=0);
signals:
    void sendData(const QJsonObject&);
private:
    //保存流程数据的函数地址，在需要处理流程时，在服务器中保存编号，客户端根据编号对应处理函数。
    QMap<int,DealFuc> m_fucMap;
};

#endif // TABWIGETBASE_H
