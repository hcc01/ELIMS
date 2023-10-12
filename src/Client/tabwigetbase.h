#ifndef TABWIGETBASE_H
#define TABWIGETBASE_H
/*这个是模块的基类。模块的添加使用DLL创建UI界面，在主窗口的导航栏添加按钮打开模块窗口。
 *添加了另一个基类：SqlBaseClass。用于模块中各个窗口的数据库操作指令。
 *
 *
 *
 *
 *
 *
 */
#include"../Client/qjsoncmd.h"
#include"cuser.h"
#include "qeventloop.h"
#include <QWidget>
#include<QJsonObject>
#include<QMessageBox>
#include<QMutex>
#include<QWaitCondition>
//流程处理函数
//typedef void (*DealFuc) (const QSqlReturnMsg& );qjsoncmd.h中统一定义
class TabWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidgetBase(QWidget *parent = nullptr);
    virtual ~TabWidgetBase(){}
    virtual void onSqlReturn(const QSqlReturnMsg& jsCmd);
    virtual void dealProcess(const ProcessNoticeCMD&);//处理流程事件(没用了）
    virtual void initMod()=0;//新增模块时初始化操作，建表等。
    //virtual void initCMD()=0;//用于窗口建立后给服务器发送初始化命令。设为纯虚是因为不知道为什么子类如果不写这个函数，调用就会奔溃！
    virtual void initCMD(){}//初次调用模块窗口时需要进行的初始化操作。
    void doSqlQuery(const QString&sql,DealFuc f=nullptr,int page=0, const QJsonArray&bindValue={});
    void setUser(CUser* user){m_user=user;}
    CUser* user()const{return m_user;}
private:
signals:
    void sendData(const QJsonObject&);
private:
    //保存流程数据的函数地址，在需要处理流程时，在服务器中保存编号，客户端根据编号对应处理函数。
    QMap<int,DealFuc> m_fucMap;
    CUser* m_user;

};

class SqlBaseClass
{
public:
    SqlBaseClass(TabWidgetBase* tab);
    void doSql(const QString&sql,DealFuc f=nullptr,int p=0,const QJsonArray&values={});
    CUser* user(){return m_tabWiget->user();}
    TabWidgetBase* tabWiget(){return m_tabWiget;}
private:
    TabWidgetBase* m_tabWiget;

};

#endif // TABWIGETBASE_H
