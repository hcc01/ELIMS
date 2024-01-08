#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"QMessageBox"
#include"loginui.h"
#include<QLabel>
#include<QDebug>
#include<QHostInfo>
#include"tabfactory.h"
#include"todoui.h"
#include"qjsoncmd.h"
#include"rmmanage.h"
#include"employeemanageui.h"
#include"dbmanagerui.h"
#include"modinitui.h"
#include"tasksheetui.h"
#include"labcapabilitiesmanagerui.h"
#include"standardsmanager.h"
#include"persnaldatamanagerui.h"
#include<QTimer>
#include<QThread>
#include"reportmanagerui.h"
//REGISTER_TAB(RMManageUI);
//REGISTER_TAB(EmployeeManageUI);
//REGISTER_TAB(DBManagerUI);
/* 去掉了原先的REGISTER_TAB宏，这个操作不明朗，后续在增加模块时还要涉及其它地方的不明朗操作。
 * 使用宏来增加模块ADD_MODUEL(模块UI类，打开按钮），按钮放在导航栏中即可，不需要任何设置。注意按钮的文本就是区别模块的标识，不可重复。
 * 模块是一个设计好的以TabWidgetBase为基类的窗口类，在MainWindow的Tab页中显示出来。
 */
#define ADD_MODULE(module,linkButton) \
    TabFactory::Register(linkButton->text(), static_cast<CREATE_FUNC>([](QWidget *parent) -> void * { return new module(parent); })); \
    connect(linkButton, &QPushButton::clicked, this, &MainWindow::onOpenTab);
void MainWindow::doTabwidgetMapping()
{

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      isLogined(false),
    m_user(nullptr)
{
    ui->setupUi(this);
    ADD_MODULE(ToDoUI,ui->btnToDo);
    ADD_MODULE(RMManageUI,ui->btRMManage);
    ADD_MODULE(EmployeeManageUI,ui->btEmployeeManage);
    ADD_MODULE(DBManagerUI,ui->btDBManage);
    ADD_MODULE(TaskSheetUI,ui->btTaskSheet);
    ADD_MODULE(LabCapabilitiesManagerUI,ui->btLabCapability);
    ADD_MODULE(StandardsManager,ui->standardsManagerBtn);
    ADD_MODULE(PersnalDataManagerUI,ui->btPersonalInfo);
    ADD_MODULE(ReportManagerUI,ui->reportManagerBtn);
    _waitDlg.setWindowFlag(Qt::FramelessWindowHint);
    QLabel* label=new QLabel("请等待……",&_waitDlg);
    _waitDlg.show();
    connect(&_clientSocket,&CClient::onConnectError,this,[&](const char* error){
        DoConnect();
        if(isLogined) DoLogin();//已经登录过了，断线后重新登录。（否则就是在登录界面等待登录，不要再重走这个步骤）；目前这个设置不完善，待优化
    });
    connect(&_clientSocket,&CClient::netMsg,this,&MainWindow::onNestMsg,Qt::BlockingQueuedConnection);//第五个参数很重要，否则当服务器连续发送消息时不能及时处理，只重复处理到最后一条。
//    connect(&_clientSocket,&CClient::sendingData,this,[&](const QString&data){
//        qDebug()<<"sendingData"<<data;
//        _waitDlg.exec();
//    });
    DoConnect();
    DoLogin();
    isLogined=true;
//    netMsg_Init msg;
//    _clientSocket.SendData(&msg);
    QList<QAction*> skinActions = ui->skinMenu->actions();
    for(auto a:skinActions){
        if(a->isChecked()) qDebug() << "Action text: " << a->text();
        connect(a,&QAction::triggered,this,&MainWindow::onSkinChanged);
    }
    //根据权限过滤模块
    if(m_user->name()!="admin") for(int i=0;i<ui->toolBox->count();i++){
        for(auto w:ui->toolBox->widget(i)->findChildren<QWidget*>()){
            w->setVisible(false);
        }
    }
//    ui->btnToDo->show();
    ui->btPersonalInfo->show();
    if(m_user->position()&(CUser::ReportWriter|CUser::LabManager|LabSupervisor)){
        ui->btTaskSheet->show();
    }
    if(m_user->position()&(CUser::LabManager|CUser::LabSupervisor)){
        ui->btEmployeeManage->show();
    }
    ui->btnToDo->clicked();
    ToDoUI* todoUI=static_cast<ToDoUI* >( getTabWidget("我的待办"));
    if(todoUI){
        connect(todoUI,&ToDoUI::dealFLow,[this](const QFlowInfo&flowInfo,int operateFlag){

            TabWidgetBase* tab=getModule(flowInfo.tabName());
            qDebug()<<"接收到流程处理信息"<<flowInfo.object();
            qDebug()<<"调用模块"<<tab<<tab->tabName();
            tab->dealProcess(flowInfo,operateFlag);
        });
    }
}

MainWindow::~MainWindow()
{
    qDebug()<<"~MainWindow()";
  //  _clientSocket.Close();
    delete ui;
    delete m_user;
}

void MainWindow::DoConnect()
{

//    QHostInfo info = QHostInfo::fromName("127.0.0.1");
    QHostInfo info = QHostInfo::fromName("mud.tpddns.cn");
    if(_clientSocket.Connect(info.addresses().first().toString().toUtf8(),5555)==SOCKET_ERROR){
        int r=QMessageBox::warning(nullptr,"","无法连接服务器","重新连接","退出");
        switch (r) {
            case 0:
            DoConnect();
            break;
        case 1:
            exit(0);
            break;
        }
    }

    _waitDlg.hide();
}

void MainWindow::DoLogin()
{
    static bool logining=false;
    if(logining) return;
    logining=true;
    LoginUI loginUI;
    connect(&loginUI,&LoginUI::login,this,[&](const QString& id, const QString& password){
        netmsg_Login msgLogin;
        memcpy(msgLogin.userName,id.toUtf8().data(),32);
        memcpy(msgLogin.PassWord,password.toUtf8().data(),33);
        _clientSocket.SendData(&msgLogin);
    });
    connect(this,&MainWindow::loginResult,&loginUI, &LoginUI::onLoginResult);
    loginUI.exec();
    logining=false;
}

void MainWindow::sendData(const QJsonObject &json)
{
    _clientSocket.SendData(json);
}



void MainWindow::onNestMsg(netmsg_DataHeader *header)
{
//    if(_waitDlg.isModal()){
//        qDebug()<<"_waitDlg.isModal()";
//        _waitDlg.accept();
//    }
    switch (header->cmd) {
    case CMD_S2C_HEART:
    {

    }
        break;
    case CMD_LOGIN_RESULT:
    {
        netmsg_LoginR* lr=(netmsg_LoginR*)header;
        emit loginResult(lr->result);
        if(!m_user){
            m_user=new CUser(lr->name,lr->position);
        }
        else{
            m_user->reset(lr->name,lr->position);
        }
        setWindowTitle(QString("%1 -厦门市政南方海洋检测有限公司").arg(lr->name));

    }
        break;
    case CMD_JSON_CMD:
    {
        QJsonObject js=CELLReadStream(header).getJsonData();
        onJsonCMD(js);

    }
        break;
    default:
        break;
    }
}

void MainWindow::onJsonCMD(const QJsonObject &json)
{
    int cmd=json.value("cmd").toInt();
  //  qDebug()<<"cmd="<<json;
    switch (cmd) {
    case JC_DO_SQL:
    {
        QSqlReturnMsg jsCmd(json);
        QString tabText=jsCmd.tytle();
        TabWidgetBase*w= getModule(tabText);
        if(w) w->onSqlReturn(jsCmd);//返回信息的处理交由各窗口处理。
        else {
            if(jsCmd.error()){
                QMessageBox::warning(this,"",jsCmd.result().toString());
            }
            else QMessageBox::information(this,"","操作成功");
        }
//        bool r=json.value("result").toBool();
//        switch(json.value("sql_type").toInt()){
//            case SQL_ADD_RM:{
//                if(r) QMessageBox::information(this,"","添加标准物质信息成功!");
//                else QMessageBox::information(this,"添加失败",json.value("sql_info").toString());
//            }
//            break;
//            case SQL_QUERY_RM:
//           {
//                bool r=json.value("result").toBool();
//               if(!r){
//                    QMessageBox::information(this,"查询失败",json.value("sql_info").toString());
//                    return;
//                }
//                RMManageUI* rmUI=((RMManageUI*)getTabWidget(RMManageUI::tabText()));
//               if(!rmUI){
//                    qDebug()<<"error on find rmUI";
//                    return;
//               }
//              rmUI ->setData( json.value("sql_info").toArray());
//            }
//                break;
//            case SQL_GET_TABLES:
//            {

//            }
//            break;
//        }


    }
        break;
    case JC_WORKFLOW:
    {

    }
        break;
//    case JC_NOTICE:
//    {
//        NoticeCMD cmd(json);
//        int type=cmd.type();
//        switch(type){
//        case NT_WORKFLOW://流程待办
//        {

//            ProcessNoticeCMD cmd(json);
//            ui->listWidget->addTodo(cmd);
//        }
//            break;
//        }
//    }
//        break;
//    case CMD_LOGOUT:
//    {
//        int r=QMessageBox::warning(this,"", json.value("data").toString(),"重新登录","退出");
//        if(r==0){
//            DoLogin();
//        }
//        else exit(0);
//    }
//        break;
//    case JC_QUERY_RM:
//    {
//        bool r=json.value("result").toBool();
//        if(!r){
//            QMessageBox::information(this,"查询失败",json.value("reason").toString());
//            return;
//        }
//        RMManageUI* rmUI=((RMManageUI*)getTabWidget(RMManageUI::tabText()));
//        if(!rmUI){
//            qDebug()<<"error on find rmUI";
//            return;
//        }
//        rmUI ->setData( json.value("data").toArray());
//    }
        break;
    default:
        break;
    }
}

void MainWindow::onOpenTab()
{
    QPushButton* bt=static_cast<QPushButton*>(sender()) ;
    if(!bt) return;
    QString text=bt->text();
    for(int i=0;i<ui->tabWidget->count();i++){//检查相应的窗口是否已经被打开
        if(ui->tabWidget->tabText(i)==text){
            ui->tabWidget->setCurrentIndex(i);
            return;
        }
    }

    TabWidgetBase *tab;
    tab=m_modules.value(text);//如果模块里面有创建过，直接移到界面
    m_modules[text]=nullptr;
    if(!tab) tab=static_cast<TabWidgetBase *>(TabFactory::CreateObject(text));
    if(!tab) {
        qDebug()<<"无法创建窗体："<<text;
        return;
    }
    tab->setUser(m_user);
    tab->setTabName(text);
    connect(tab,&TabWidgetBase::sendData,this,[=](const QJsonObject&sqlCmd){
        QJsonObject j=sqlCmd;
        j["tytle"]=text;//标识下处理窗口
        sendData(j);
    });
    ui->tabWidget->addTab(tab,text);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    tab->initCMD();//用于向服务器发送初始命令，记住：要在加到tabWidget里面后才能执行，否则会出错（因为依赖于getTabWidget来指向这个窗体）
}

TabWidgetBase *MainWindow::getTabWidget(const QString &widgetText) const
{
    for(int i=0;i<ui->tabWidget->count();i++){
        if(ui->tabWidget->tabText(i)==widgetText) return static_cast<TabWidgetBase *>( ui->tabWidget->widget(i));
    }
    return nullptr;
}

TabWidgetBase *MainWindow::getModule(const QString &widgetText)
{
    TabWidgetBase *tab;

    tab=getTabWidget(widgetText);//先确认下有没交互的模块窗口
    if(tab) return tab;
    tab= m_modules.value(widgetText);
    if(tab) return tab;
    //都没有，创建一个
    tab=static_cast<TabWidgetBase *>(TabFactory::CreateObject(widgetText,this));
    if(!tab) {
        qDebug()<<"无法创建窗体："<<widgetText;
        return nullptr;
    }
    tab->setUser(m_user);
    tab->setTabName(widgetText);
    connect(tab,&TabWidgetBase::sendData,this,[=](const QJsonObject&sqlCmd){
        QJsonObject j=sqlCmd;
        j["tytle"]=widgetText;//标识下处理窗口
        sendData(j);
    });
    m_modules[widgetText]=tab;//加在模块列表里面
    return tab;
}


void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if(index<1) return;
    QWidget* w=ui->tabWidget->widget(index);
    delete w;
   // ui->tabWidget->removeTab(index);
}

void MainWindow::on_btEmployeeManage_clicked()
{

}

//void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
//{
//    ProcessNoticeCMD cmd=ui->listWidget->model()->data(ui->listWidget->currentIndex(),Qt::UserRole).toJsonObject();
//    TabWidgetBase* tab=getTabWidget(cmd.tabText());
//    if(!tab){
//        qDebug()<<"error to find widget"<<cmd.tabText();
//        return;
//    }
//    tab->dealProcess(cmd);
//}

void MainWindow::on_pushButton_clicked()
{
    qDebug()<<TabFactory::tabClasses();
}

void MainWindow::on_btModInit_clicked()
{
    ModInitUI m;
    m.setTabsText(TabFactory::tabClasses());
    connect(&m,&ModInitUI::tabsToInit,this,[&](const QString& tabText){
        TabWidgetBase *tab=static_cast<TabWidgetBase *>(TabFactory::CreateObject(tabText));
        if(!tab) {
            qDebug()<<"无法创建窗体："<<tabText;
            return;
        }
//        connect(tab,&TabWidgetBase::sendData,this,&MainWindow::sendData);
        connect(tab,&TabWidgetBase::sendData,this,[&](const QJsonObject&sqlCmd){
            QJsonObject j=sqlCmd;
            j["tytle"]=tabText;//标识下处理窗口
            sendData(j);
        });
        tab->initMod();
//        delete tab;
        tab->hide();
    });
    m.exec();
}

void MainWindow::on_actionInitMod_triggered()
{
    TabWidgetBase*tab=(TabWidgetBase*)ui->tabWidget->currentWidget();
    tab->initMod();
}


void MainWindow::onSkinChanged()
{
    QAction *action = qobject_cast<QAction*>(sender());
    qDebug()<<action->text();
    if(!action->isChecked()) {
        action->setChecked(true);
        return;
    }
    QList<QAction*> skinActions = ui->skinMenu->actions();
    for(auto a:skinActions){
        if(a->isChecked()&&a!=action) a->setChecked(false);
    }
    emit changeSkin(skinActions.indexOf(action));
    qDebug()<<skinActions.indexOf(action);
}


void MainWindow::on_actionVersion_triggered()
{
    QMessageBox::information(nullptr,"","版本号：测试版v0.110");
}


void MainWindow::on_exitAct_triggered()
{
    exit(0);
}

