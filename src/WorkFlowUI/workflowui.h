#ifndef WORKFLOWUI_H
#define WORKFLOWUI_H
//流程界面，用来处理流程操作，为其它模块提供流程接口
#include "WorkFlowUI_global.h"
#include"../Client/tabwigetbase.h"
#include"../Client/ctablemodel.h"
#include"../Client/qjsoncmd.h"

class WORKFLOWUI_EXPORT WorkFlowUI:public TabWidgetBase
{
    Q_OBJECT
public:
    explicit WorkFlowUI(QWidget* parent=nullptr);
    static QString tabText(){return "流程管理";}
    bool initMod() override;
    //bool dealProcess(int instanceID,int nodeID,bool pass,QString remark);
    //以下函数对其它模块提供流程操作接口
    //流程的定义
    void createProceesInfo(const QString&processName,const QString&processDesc ,const QString&operateTab);//创建一个流程信息，当一个模块需要使用流程管理时，进行流程 定义。
    void createProcessNode(const QString &processName, const QString&nodeName,const QString&acceptToNode,const QString&refuseToNode,const QString&posisionName );//定义流程的节点
    //流程的操作
    void newProcess(const QString& processName, const QJsonObject content);//新建流程实例，用户启动一个新流程
    void doProcess(int recordID, bool action, const QString remarks);//执行流程操作，用户进行流程审核
private:

};

#endif // WORKFLOWUI_H
