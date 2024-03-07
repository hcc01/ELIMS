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
#include "mytableview.h"
#include "qlabel.h"
#include <QWidget>
#include<QJsonObject>
#include<QMessageBox>
#include<QMutex>
#include<QWaitCondition>
//流程处理函数
//typedef void (*DealFuc) (const QSqlReturnMsg& );qjsoncmd.h中统一定义
class FlowWidget:public QWidget{
     Q_OBJECT
signals:
     void pushProcess(const QFlowInfo&flowInfo,bool passed);
};

#include<QDialog>
class WaitDlg:public QDialog{
    Q_OBJECT
public:
    explicit WaitDlg(QWidget *parent = nullptr);
    void setMsg(const QString&msg){m_msg->setText(msg);}
    void wait(){m_execFlag++;qDebug()<<"msg:"<< m_msg->text() <<"waitCount:"<<m_execFlag;exec();}
    void end(){if(!m_execFlag) return;m_execFlag--;if(!m_execFlag) accept();}
    void reject()override{m_execFlag=0;QDialog::reject();}//强制取消等待，用于出错的情况

private:
    QLabel* m_msg;
    int m_execFlag;
};

class TabWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidgetBase(QWidget *parent = nullptr);
    enum FlowOperateFlag{ VIEWINFO,AGREE,REJECT};
    virtual ~TabWidgetBase();
    virtual void onSqlReturn(const QSqlReturnMsg& jsCmd);
    virtual void dealProcess(const QFlowInfo&, int operateFlag);//处理流程事件
    virtual bool pushProcess(QFlowInfo flowInfo, bool passed, const QString &comments);//推进流程
    virtual void showFlowInfo(const QSqlReturnMsg& flowIDsQueryMsg);//显示审批记录，暂时保留，已经被showFlowRecord替代
    virtual void initMod();//新增模块时初始化操作，建表等。
    virtual void initCMD(){}//初次调用模块窗口时需要进行的初始化操作。
    virtual FlowWidget* flowWidget(const QFlowInfo &flowInfo){return nullptr;}//用于流程审批的窗口，各模块自定义
    void doSqlQuery(const QString&sql,DealFuc f=nullptr,int page=0, const QJsonArray&bindValue={});
    int submitFlow(const QFlowInfo& flowInfo, QList<int> operatorIDs,const QString& identityValue,int operatorCount=1, const QString&tableName="all_flows",
                   const QString&identityColumn="identityColumn", const QString&flowIDColumn="flowID");
    QWidget* showFlowRecord(const QString& identityValue, const QString &tableName="all_flows", const QString&identityColumn="identityColumn", const QString&flowIDColumn="flowID");
    void setUser(CUser* user){m_user=user;}
    void setTabName(const QString&name){m_tabName=name;}
    CUser* user()const{return m_user;}
    QString tabName()const{return m_tabName;}
    void waitForSql(const QString&msg=QStringLiteral("数据处理中……"));
    void notifySqlError(const QString&tytle,const QString&errorMsg){//用于查找出错时快速返回并自动发送查询完成信号：return notifySqlError();
        QMessageBox::information(nullptr,tytle,errorMsg);
//        emit sqlFinished();
        sqlEnd();
    }
    void sqlEnd();
    void sqlFinished(){sqlEnd();}
    bool connectDB(CMD Transaction);//连接数据库时，使用专用的连接进行操作，可以开启事务
    void releaseDB(CMD TransactionType);//断开数据库，请注意及时断开

private:
signals:
    void sendData(const QJsonObject&);
//    void sqlFinished();
    void processOk(bool);
private:
    QString m_tabName;
    //保存流程数据的函数地址，在需要处理流程时，在服务器中保存编号，客户端根据编号对应处理函数。
    QMap<int,DealFuc> m_fucMap;
    int flag;
    CUser* m_user;
    WaitDlg m_dlg;
    MyTableView *m_view;
};

class SqlBaseClass
{
public:
    SqlBaseClass(TabWidgetBase* tab);
    void doSql(const QString&sql,DealFuc f=nullptr,int p=0,const QJsonArray&values={});
    CUser* user(){return m_tabWiget->user();}
    TabWidgetBase* tabWiget(){return m_tabWiget;}
    void setTabWidget(TabWidgetBase* tab){m_tabWiget=tab;}
    void waitForSql(const QString&msg=QStringLiteral("数据处理中……"));
    void sqlFinished();
private:
    TabWidgetBase* m_tabWiget;

};

#endif // TABWIGETBASE_H
