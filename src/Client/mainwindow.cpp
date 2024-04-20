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
#include"samplingscheduleui.h"
#include"samplecirculationui.h"
#include"workhoursatistics.h"
#include"dbmater.h"
#include<QSettings>
#include"testmanager.h"
#include"businessmanagerui.h"
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
    ADD_MODULE(SamplingScheduleUI,ui->btSamplingSchedule);
    ADD_MODULE(SampleCirculationUI,ui->samplecirculationBtn);
    ADD_MODULE(WorkHourSatistics,ui->workHourStatisticsBtn);
    ADD_MODULE(TestManager,ui->testManagerBtn);
    ADD_MODULE(BusinessManagerUI,ui->saleManagerBtn);

    _waitDlg.setWindowFlag(Qt::FramelessWindowHint);
    QLabel* label=new QLabel("请等待……",&_waitDlg);
    _waitDlg.show();
    _clientSocket=new CClient;
    connect(_clientSocket,&CClient::onConnectError,this,[&](const char* error){
        DoConnect();
        if(isLogined) DoLogin();//已经登录过了，断线后重新登录。（否则就是在登录界面等待登录，不要再重走这个步骤）；目前这个设置不完善，待优化
    });
    connect(_clientSocket,&CClient::netMsg,this,&MainWindow::onNestMsg,Qt::BlockingQueuedConnection);//第五个参数很重要，否则当服务器连续发送消息时不能及时处理，只重复处理到最后一条。
//    connect(&_clientSocket,&CClient::sendingData,this,[&](const QString&data){
//        qDebug()<<"sendingData"<<data;
//        _waitDlg.exec();
//    });
    DoConnect();
    DoLogin();
    isLogined=true;
//    netMsg_Init msg;
//    _clientSocket.SendData(&msg);
    //处理皮肤
    QSettings set("settings",QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    QString style=set.value("style").toString();
    QList<QAction*> skinActions = ui->skinMenu->actions();
    for(auto a:skinActions){
        if(a->text()==style){
            a->setChecked(true);
        }
        else a->setChecked(false);
        connect(a,&QAction::triggered,this,&MainWindow::onSkinChanged);
    }
    //根据权限过滤模块
    qDebug()<<m_user->name()<<m_user->phone()<<m_user->position();
    if(m_user->name()!="admin") for(int i=0;i<ui->toolBox->count();i++){
        for(auto w:ui->toolBox->widget(i)->findChildren<QWidget*>()){
            w->setVisible(false);
        }
    }
//    ui->btnToDo->show();
    ui->btPersonalInfo->show();
    //报告编制和实验室主管
    if(m_user->position()&(CUser::ReportWriter|CUser::LabManager|CUser::LabSupervisor)){
        ui->btTaskSheet->show();
        ui->reportManagerBtn->show();
    }
    //实验室主管
    if(m_user->position()&(CUser::LabManager|CUser::LabSupervisor)){
        ui->btEmployeeManage->show();
        ui->workHourStatisticsBtn->show();
        ui->testManagerBtn->show();
    }
    //采样和采样组长
    if(m_user->position()&(CUser::Sampler|CUser::SamplerLeader)){
        ui->btSamplingSchedule->show();
    }
    //样品管理员
    if(m_user->position()&(CUser::SampleAdministrator)){
        ui->samplecirculationBtn->show();
    }
    //分析人员
    if(m_user->position()&(CUser::InorganicAnalyst|CUser::PhysicalChemicalAnalyst|CUser::OrganicAnalyst)){
        ui->testManagerBtn->show();
    }
    ui->btnToDo->clicked();
    ToDoUI* todoUI=static_cast<ToDoUI* >( getTabWidget("我的待办"));
    if(todoUI){
//        connect(todoUI,&ToDoUI::dealFLow,[this](const QFlowInfo&flowInfo,int operateFlag){

//            TabWidgetBase* tab=getModule(flowInfo.tabName());
//            qDebug()<<"接收到流程处理信息"<<flowInfo.object();
//            qDebug()<<"调用模块"<<tab<<tab->tabName();
//            tab->dealProcess(flowInfo,operateFlag);
//        });

        loadUser();//登录返回人员名和职位，其它人员信息在这里载入
        qDebug()<<m_user->name()<<m_user->phone();
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

    QSettings set("settings",QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    QHostInfo info = QHostInfo::fromName("127.0.0.1");
    if(set.value("company").isValid()){
        m_company=set.value("company").toString();
    }
    else{
        set.setValue("company","实验室信息管理系统");
        m_company="实验室信息管理系统";
    }
    if(set.value("server/ip").isValid()){
        info= QHostInfo::fromName(set.value("server/ip").toString());
    }
    else{
        info= QHostInfo::fromName("127.0.0.1");
        set.setValue("server/ip","127.0.0.1");
    }
    int port;
    if(set.value("server/port").isValid()){
        port=set.value("server/port").toInt();
    }
    else{
        port= 6666;
        set.setValue("server/port",6666);
    }
    if(_clientSocket->Connect(info.addresses().first().toString().toUtf8(),port)==SOCKET_ERROR){
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
    LoginUI loginUI(m_company);
    connect(&loginUI,&LoginUI::login,this,[&](const QString& id, const QString& password){
        netmsg_Login msgLogin;
        memcpy(msgLogin.userName,id.toUtf8().data(),32);
        memcpy(msgLogin.PassWord,password.toUtf8().data(),33);
        _clientSocket->SendData(&msgLogin);
    });
    connect(this,&MainWindow::loginResult,&loginUI, &LoginUI::onLoginResult);

    loginUI.exec();
    logining=false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("选择操作");
    msgBox.setText("您想要做什么？");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, "退出程序");
    msgBox.setButtonText(QMessageBox::No, "最小化到托盘");

    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        exit(0);
    }
    else{
        this->hide();
    }
}

TabWidgetBase *MainWindow::newTab(const QString &text)
{
    TabWidgetBase *tab=nullptr;
    tab=static_cast<TabWidgetBase *>(TabFactory::CreateObject(text));
    if(tab) {
        tab->setUser(m_user);
        tab->setTabName(text);
        connect(tab,&TabWidgetBase::sendData,this,[this, tab](const QJsonObject&json){
                QJsonObject j=json;
                j["tytle"]=tab->tabName();//标识下处理窗口
                sendData(j);
            },Qt::QueuedConnection);
    }
    return tab;
}

void MainWindow::sendData(const QJsonObject &json)
{
    if(!_clientSocket->SendData(json)){
        QMessageBox::information(nullptr,"error","无法发送数据！");
    }
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
        if(m_user->name()=="admin") m_user->reset(m_user->name(),2147483647);
        setWindowTitle(QString("%1 -&2").arg(lr->name).arg(m_company));

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
        qDebug()<<"jsCmd"<<json;
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
    if(tab){
        m_modules[text]=nullptr;
        ui->tabWidget->addTab(tab,text);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        tab->initCMD();//用于向服务器发送初始命令，记住：要在加到tabWidget里面后才能执行，否则会出错（因为依赖于getTabWidget来指向这个窗体）
        return;
    }
    tab=newTab(text);
    if(!tab) {
        qDebug()<<"无法创建窗体："<<text;
        return;
    }

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
    tab=newTab(widgetText);
    if(!tab) {
        qDebug()<<"getModule无法创建窗体："<<widgetText;
        return nullptr;
    }

    m_modules[widgetText]=tab;//加在模块列表里面
    return tab;
}

void MainWindow::loadUser()//在我在待办模块中操作
{
    ToDoUI* todoUI=static_cast<ToDoUI* >( getTabWidget("我的待办"));
    if(!todoUI) {
        qDebug()<<"error:todoUI is 0";
         return;
    }
    todoUI->loadUser(m_user,this);
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
        TabWidgetBase *tab=getModule(tabText);
        tab->initMod();
//        delete tab;
//        tab->hide();
    });
    m.exec();
}

void MainWindow::on_actionInitMod_triggered()
{
    if((!(m_user->position()&CUser::SystemAdin))&&m_user->name()!="admin") return;
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
    emit changeSkin(action->text());
}


void MainWindow::on_actionVersion_triggered()
{
    QMessageBox::information(nullptr,"","版本号：测试版V0.5.6");
}


void MainWindow::on_exitAct_triggered()
{
    exit(0);
}


void MainWindow::on_updateTypeAct_triggered()
{
    ToDoUI* todoUI=static_cast<ToDoUI* >( getTabWidget("我的待办"));
    todoUI->updateTypes();
}


void MainWindow::on_updateParameterAct_triggered()
{
    ToDoUI* todoUI=static_cast<ToDoUI* >( getTabWidget("我的待办"));
    todoUI->updateParameters();
}

