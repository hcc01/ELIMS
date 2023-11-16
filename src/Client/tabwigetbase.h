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
#include "qlabel.h"
#include <QWidget>
#include<QJsonObject>
#include<QMessageBox>
#include<QMutex>
#include<QWaitCondition>
//流程处理函数
//typedef void (*DealFuc) (const QSqlReturnMsg& );qjsoncmd.h中统一定义
#include<QDialog>
class WaitDlg:public QDialog{
    Q_OBJECT
public:
    explicit WaitDlg(QWidget *parent = nullptr);
    void setMsg(const QString&msg){m_msg->setText(msg);}
private:
    QLabel* m_msg;
};

class TabWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidgetBase(QWidget *parent = nullptr);
    enum FlowOperateFlag{ VIEWINFO,AGREE,REJECT};
    virtual ~TabWidgetBase(){}
    virtual void onSqlReturn(const QSqlReturnMsg& jsCmd);
    virtual void dealProcess(const QFlowInfo&, int operateFlag);//处理流程事件
    virtual void initMod();//新增模块时初始化操作，建表等。
    virtual void initCMD(){}//初次调用模块窗口时需要进行的初始化操作。
    void doSqlQuery(const QString&sql,DealFuc f=nullptr,int page=0, const QJsonArray&bindValue={});
    int submitFlow(const QFlowInfo& flowInfo, QList<int> operatorIDs,int operatorCount=1 );
    void setUser(CUser* user){m_user=user;}
    void setTabName(const QString&name){m_tabName=name;}
    CUser* user()const{return m_user;}
    QString tabName()const{return m_tabName;}
    void waitForSql(const QString&msg=QStringLiteral("数据处理中……"));
private:
signals:
    void sendData(const QJsonObject&);
    void sqlFinished();
private:
    QString m_tabName;
    //保存流程数据的函数地址，在需要处理流程时，在服务器中保存编号，客户端根据编号对应处理函数。
    QMap<int,DealFuc> m_fucMap;
    CUser* m_user;
    WaitDlg m_dlg;

};

class SqlBaseClass
{
public:
    SqlBaseClass(TabWidgetBase* tab);
    void doSql(const QString&sql,DealFuc f=nullptr,int p=0,const QJsonArray&values={});
    CUser* user(){return m_tabWiget->user();}
    TabWidgetBase* tabWiget(){return m_tabWiget;}
    void sqlFinished();
    void waitForSql(const QString&msg=QStringLiteral("数据处理中……"));
private:
    TabWidgetBase* m_tabWiget;

};

#endif // TABWIGETBASE_H
