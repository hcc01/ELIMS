#include "processmanager.h"
#include<QMenu>
#include<QDebug>
ProcessManager::ProcessManager(QWidget *parent) :
    QListWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);//比较重要 只有这样设置 才能使用信号SIGNAL(customContextMenuRequested(QPoint))
    connect( this, &QListWidget::customContextMenuRequested, this,[&](const QPoint& pos){
        QModelIndex index =this->indexAt(pos);
       // qDebug()<<"index"<<index<<model()->data(index,Qt::UserRole).toJsonObject();
        QMenu menu;
        //添加菜单项，指定图标、名称、响应函数
        ProcessNoticeCMD cmd=model()->data(index,Qt::UserRole).toJsonObject();
       // QJsonArray actions=cmd.actions();
       // int n=actions.size();
//        for(int i=0;i<n;i++){
//            menu.addAction( actions.at(i).toArray().at(1).toString(),this,[&](){
//               // OnApply(index);
//            });
//        }

        //在鼠标位置显示
        menu.exec(QCursor::pos());
    });
}

ProcessManager::~ProcessManager()
{
}

void ProcessManager::addTodo(const ProcessNoticeCMD& cmd)
{
    addItem(QString("你有待%1的流程：%2于%3发起的【%4】，请及时处理。").arg(cmd.nodeName()).arg(cmd.createName()).arg(cmd.createTime()).arg(cmd.ProcessName()));
    QModelIndex index=this->indexFromItem(item(this->count()-1));
    this->model()->setData(index,cmd.data(),Qt::UserRole);
    //qDebug()<<"index"<<index<<model()->data(index,Qt::UserRole).toJsonObject();

}

void ProcessManager::todo(const ProcessNoticeCMD &cmd)
{

}
