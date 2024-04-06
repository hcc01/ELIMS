#include "methodselectdlg.h"
#include "dbmater.h"
#include "ui_methodselectdlg.h"
#include<QMessageBox>
#include<QJsonArray>
MethodSelectDlg::MethodSelectDlg(TabWidgetBase *tabWiget) :
    QDialog(tabWiget),
    SqlBaseClass(tabWiget),
    ui(new Ui::MethodSelectDlg),
    m_methodLoad(false)
{
    ui->setupUi(this);
    ui->OkBtn->setDisabled(true);
    ui->tableView->setHeader({"检测类型","检测项目","检测方法","CMA资质","是否分包","分包原因"});
    m_methodBox=new ComboBoxDelegate(ui->tableView,{});
//    ui->tableView->setItemDelegateForColumn(2,m_methodBox);这个在重新加载方法后设置，不然没数据，会出现原方法点击后消失的问题。
    //当用户改变方法时，检查其它项目是否也存在这种方法并确认是否同时改变
    connect(m_methodBox,&ComboBoxDelegate::selectChanged,this,[this](const QString&text,const QModelIndex& index){
        QString preText=ui->tableView->value(index.row(),index.column()).toString();
        qDebug()<<"index changed:"<<text<<preText;
        if(preText==text) return;
        qDebug()<<QString("%1 -> %2").arg(preText).arg(text);
        int n=0;
        for(int i=0;i<ui->tableView->rowCount();i++){
            if(ui->tableView->value(i,2).toString()==preText&&m_methodBox->boxItems(i,2).contains(text)) {
                n++;
            }
            if(n==2) break;//有其它项目存在同样的方法，提示是否应用到其它项目
        }
        if(n==2){
            int a=QMessageBox::question(nullptr,"","是否应用到其它项目？");
            if(a==QMessageBox::Yes){
                qDebug()<<"yes!";
                int n=ui->tableView->rowCount();
                for(int i=0;i<n;i++){
                    if(ui->tableView->value(i,2).toString()==preText&&m_methodBox->boxItems(i,2).contains(text)) {
                        ui->tableView->setData(i,2,text);
                        m_methodBox->setBoxItemsIndex(i,2,text);
                    }
                }
            }
            else{
                qDebug()<<"No!";
            }
            qDebug()<<"finished!";
        }
    });
    ComboBoxDelegate* subpackageEditor=new ComboBoxDelegate(ui->tableView,{"否","是"});
    ui->tableView->setItemDelegateForColumn(4,subpackageEditor);
    ui->tableView->setEditableColumn(2);
    ui->tableView->setEditableColumn(4);
    ui->tableView->setEditableColumn(5);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

MethodSelectDlg::~MethodSelectDlg()
{
    delete ui;
    delete m_methodBox;
//    m_methods.clear();
//    m_specialMehtods.clear();
//    m_MethodMores.clear();

}

void MethodSelectDlg::setTestInfo(QList<TestInfo *> testInfo)
{
    m_testInfo=testInfo;
    m_methodLoad=false;

}

void MethodSelectDlg::showMethods(const QList<QList<QVariant> > &table)
{
    ui->tableView->clear();
    for(auto line:table){
        if(line.count()!=6){
            QMessageBox::information(nullptr,"","显示方法时错误：参数不匹配");
            return;
        }
        ui->tableView->append(line);
    }
}

bool MethodSelectDlg::saveMethod(int taskSheetID)
{
    if(!taskSheetID){
        QMessageBox::information(nullptr,"error","无效的任务单ID！");
        return false;
    }
    QString sql;
    QJsonArray values;
    QHash<int,QHash<QString,int>>groupOrders;//保存组名的序号
    int nowOrder=0;
    int nowType=0;

    qDebug()<<"正在保存方法：";
    for(int i=0;i<ui->tableView->rowCount();i++){
        int testTypeID=m_typeIDs.value(ui->tableView->value(i,0).toString().split("/").first());//样品类型可能有多个合在一起
//        int parameterID=m_parameterIDs.value(testTypeID).value(ui->tableView->value(i,"检测项目").toString());
        int parameterID=DB.getParameterID(testTypeID,ui->tableView->value(i,"检测项目").toString());
        if(!parameterID){
            return false;
        }

        QString methodName=ui->tableView->value(i,"检测方法").toString();
        int methodID=m_methodIDs.value(methodName);

        //如果已经存在方法，则更新，没有，则插入
        sql+="INSERT INTO type_methods (taskSheetID, testTypeID, parameterID, testMethodID, subpackage, subpackageDesc, CMA, sampleTypes) "
          "VALUES (?,?,?,?,?,?,?,?) ON DUPLICATE KEY "
          "UPDATE taskSheetID = VALUES(taskSheetID),testTypeID=Values(testTypeID), parameterID=Values(parameterID), testMethodID=Values(testMethodID), "
          "subpackage=Values(subpackage), subpackageDesc=Values(subpackageDesc), CMA=Values(CMA);";
        values.append(taskSheetID);
        values.append(testTypeID);
        values.append(parameterID);
        values.append(methodID?methodID:QJsonValue());
        values.append(ui->tableView->value(i,"是否分包").toString()=="是"?1:0);
        values.append(ui->tableView->value(i,"分包原因").toString());
        values.append(ui->tableView->value(i,"CMA资质").toString()=="是"?1:0);
        values.append(ui->tableView->value(i,0).toString());
    }
    bool error=false;
    doSql(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"保存方法时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,values);
    waitForSql("正在保存方法");
    if(error) {
        return false;
    }
    //送样样品，不分组
    if(m_testInfo.at(0)->delieveryTest) return true;
    //确认样品分组：
    for(int i=0;i<ui->tableView->rowCount();i++){
        int testTypeID=m_typeIDs.value(ui->tableView->value(i,0).toString().split("/").first());//样品类型可能有多个合在一起
        int parameterID=m_parameterIDs.value(testTypeID).value(ui->tableView->value(i,"检测项目").toString());
        QString methodName=ui->tableView->value(i,"检测方法").toString();
        int methodID=m_methodIDs.value(methodName);
        int testMod=m_methodGroupInfo.value(methodID).first;
        if(testMod) {
            m_sampleGroups[testTypeID][parameterID]=0;//现场测试的项目，序号为0
            continue;
        }
        QString groupName=m_methodGroupInfo.value(methodID).second;
        if(testTypeID!=nowType){//下一个类型
            nowType=testTypeID;
            nowOrder=1;
        }
        if(groupName.isEmpty()){
            groupName=QString::number(methodID);//没有分组的，同一方法自动一组
        }
        if(groupOrders.value(testTypeID).contains(groupName)){//判断下这个类型是否已经存在这个组名分组
            m_sampleGroups[testTypeID][parameterID]=groupOrders.value(testTypeID).value(groupName);
        }
        else{
            m_sampleGroups[testTypeID][parameterID]=nowOrder;//不存在，继续编号
            groupOrders[testTypeID][groupName]=nowOrder;  //保存当前组名的编号
            nowOrder++;
        }
    }
    //开始保存样品分组
    sql="";
    values={};
    for (auto it = m_sampleGroups.constBegin(); it != m_sampleGroups.constEnd(); ++it) {
        int testTypeID = it.key();
        const QHash<int, int> &innerHash = it.value();

        // 遍历内层的 QHash
        for (auto innerIt = innerHash.constBegin(); innerIt != innerHash.constEnd(); ++innerIt) {
            int parameterID = innerIt.key();
            int sampleOrder = innerIt.value();
            sql+="update task_parameters set sampleGroup=? where taskSheetID=? and parameterID=? and testTypeID=?;";
            values.append(sampleOrder);
            values.append(taskSheetID);
            values.append(parameterID);
            values.append(testTypeID);
        }
    }
    doSql(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"保存方法时出错：",msg.errorMsg());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,values);
    if(error){
        return false;
    }
    qDebug()<<"方法保存完成。";
    return true;
}

void MethodSelectDlg::showMethod(int taskSheetID)
{
    if(!taskSheetID) return;
    //显示已经选择的方法：
    QHash<int,QPair<QList<int>,QStringList>> allInfo;//<检测类型：项目IDs，样品类型>
    QHash<int,QHash<int,QString>>parameterNames;//
    for(auto info:m_testInfo){
        QList<int>parameters=info->parametersIDs;
        for(int i=0;i<parameters.count();i++){
            int p=parameters.at(i);
            if(!allInfo[info->testTypeID].first.contains(p)) {
                allInfo[info->testTypeID].first.append(p);
                parameterNames[info->testTypeID][p]=info->monitoringParameters.at(i);
            }
        }
        if(!allInfo[info->testTypeID].second.contains(info->sampleType)) allInfo[info->testTypeID].second.append(info->sampleType);
    }
    QString sql;
    sql="select A.testTypeID, A.parameterID, CONCAT(C.methodName,' ', IFNULL(C.methodNumber,'')),CASE when A.CMA=1 THEN '是' ELSE '否' END, "
          " CASE when A.subpackage=1 THEN '是' ELSE '否' END, A.subpackageDesc "
          "from type_methods as A "
          "left join detection_parameters as B on A.parameterID=B.id "
          "left join test_methods as C on A.testMethodID=C.id "
          "where A.taskSheetID=? order by A.id;";
    ui->tableView->clear();
    doSql(sql,[this, &allInfo, &parameterNames](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"加载方法时出错：",msg.errorMsg());
            sqlFinished();
            return;
        }
        qDebug()<<"allInfo"<<allInfo<<"parameterNames"<<parameterNames;
        QList<QVariant>r=msg.result().toList();
        for(int i=1;i<r.count();i++){
            auto row=r.at(i).toList();
            int testTypeID=row.first().toInt();
            int parameterID=row.at(1).toInt();
            QPair<QList<int>,QStringList>parameter;

//            if(!allInfo.contains(testTypeID)) continue;//上次没有确认的方法；
            if(allInfo.value(testTypeID).first.contains(parameterID)){//因为方法表可能因为修改导致多作的方法，不进行显示
                row[0]=allInfo.value(testTypeID).second.join("/");
                row[1]=parameterNames.value(testTypeID).value(parameterID);
                ui->tableView->append(row);
                addMethod(testTypeID,parameterID,row.at(2).toString());
                allInfo[testTypeID].first.removeOne(parameterID);//这个项目检查完成，移出列表

                if(allInfo[testTypeID].first.isEmpty()) allInfo.remove(testTypeID);
            }
        }
        sqlFinished();

    },0,{taskSheetID});
    waitForSql();
    if(allInfo.count()){
        QMessageBox::information(nullptr,"","部分项目没有确认方法，请重新加载确认方法。");
    }
}

void MethodSelectDlg::reset()
{
    m_methodLoad=false;
    m_methodBox->clearCellItems();
    ui->OkBtn->setDisabled(true);
}

QList<QList<QVariant> > MethodSelectDlg::methodTable() const
{
    return ui->tableView->data();
}

void MethodSelectDlg::on_loadMethodBtn_clicked()//加载方法
{
    if(m_methodLoad) return;
    ui->OkBtn->setDisabled(false);
    m_methodLoad=true;
    ui->tableView->clear();//先清空视图，因为可能涉及项目改变
    ui->tableView->setItemDelegateForColumn(2,m_methodBox);
    m_methodBox->clearCellItems();//一定要清空方法数据
    QString sql;
    //根据检测信息，查找各参数的检测方法
    QHash <int,int> typeBits;//检测类型-位标识映射
    int testTypeID;
    QHash<int,QHash<int,int>> allTypeParameters;//合并类型后的参数表【检测类型，【参数ID，VIEW的行号】】，记录VIEW的行号是因为当样品不同时，需要在原样品类型后面增加显示新的样品类型（相同检测类型会有多个不同的样品类型）
    QHash<QString,int>methodScore;//方法得分，用于智能方法选择
    QHash<QString,int>methodAppearedTimes;//多参数同时分析的方法,记录本次涉及的项目数量，用于计算方法得分。
    int row=0;
    for(auto info:m_testInfo){
        testTypeID=info->testTypeID;//当前处理的检测类型
        m_typeIDs[info->sampleType]=testTypeID;//保存下类型对应的id，用于后面从VIEW中识别样品类型ID
        int typeBit=-1;

        if(!typeBits.contains(testTypeID)){//新的检测类型，选择方法时需要确认适用范围，需要检索检测类型的位标识

            sql=QString("select bitNum from test_type where id=%1").arg(testTypeID);
            doSql(sql,[this,&typeBit](const QSqlReturnMsg&msg){//查找本任务单的检测类型和检测参数，合并相同的类型
                if(msg.error()){
                    QMessageBox::information(nullptr,"获取类型位标识时出错：",msg.result().toString());
                    sqlFinished();
                    return;
                }
                QList<QVariant>r=msg.result().toList();
                if(r.count()!=2){
                    QMessageBox::information(nullptr,"获取类型位标识时出错：","无效的查询结果");
                    sqlFinished();
                    return;
                }
                typeBit=r.at(1).toList().at(0).toInt();
                emit sqlFinished();
            });
            waitForSql();
            qDebug()<<"typeBit"<<typeBit;
            if(typeBit==-1) return;
            typeBits[testTypeID]=typeBit;
        }
        else{
            typeBit=typeBits.value(testTypeID);
        }//位标识确认完成
        int testFieldID=info->testFieldID;


        for(int i=0;i<info->parametersIDs.count();i++){//开始对每个项目选择方法(先保存的VIEW中)
            int parameterID=info->parametersIDs.at(i);//当前检测参数的ID
            QString sampleType=info->sampleType;
            QString parameter=info->monitoringParameters.at(i);
            m_parameterIDs[testTypeID][parameter]=parameterID;//保存参数ID，用于后面从VIEW中识别参数ID
            if(allTypeParameters.value(testTypeID).contains(parameterID)){//当前类型的检测参数已经处理过了
//                row++;//到底加不加？
                int r=allTypeParameters.value(testTypeID).value(parameterID);
                QString type=ui->tableView->value(r,0).toString();
                QString thisType=info->sampleType;
                if(!type.contains(thisType))  ui->tableView->setData(r,0,QString("%1/%2").arg(type).arg(thisType));
                continue;//已经有相同类型相同项目，在样品类型列中增加新的样品类型
            }

            allTypeParameters[testTypeID][parameterID]=row;//记录下参数的行号，下次如果有相同类型的参数，使用这行方法

            //开始查找合适的方法
//            sql="select A.methodID, methodName, methodNumber, CMA, non_stdMethod,labPriority,typePriority ,coverage, extendCoverage,B.testingMode, B.sampleGroup from (select * from method_parameters where parameterID=?) as A "
//                  "join (select * from test_methods where (coverage&? or extendCoverage&?) and testFieldID=? ) as B on A.methodID= B.id;";
            sql="select A.methodID, B.methodName,B.methodNumber,A.CMA, A.non_stdMethod,A.labPriority,A.typePriority, B.coverage,B.extendCoverage,B.testingMode,  B.sampleGroup "
                  "from method_parameters as A "
                  "left join test_methods as B on A.methodID=B.id "
                  "where A.parameterID=? and (B.coverage&? or B.extendCoverage&?) and B.testFieldID=?";
            doSql(sql,[this,row,sampleType,parameter,typeBit,&methodAppearedTimes,&methodScore,testTypeID,parameterID](const QSqlReturnMsg&msg){//每个项目可能有多个方法，进行方法选择
                if(msg.error()){
                    QMessageBox::information(nullptr,"select method error",msg.result().toString());
                    emit sqlFinished();
                    return;
                }
                QList<QVariant> r=msg.result().toList();
                QStringList methods;
                QHash<QString,QVariant> maps;//用来记录方法的资质情况
                QMap<QString,int>methodToID;

                for(int j=1;j<r.count();j++){//处理每个方法的得分
                    QList<QVariant>row=r.at(j).toList();
                    QString method=QString("%2 %1").arg(row.at(2).toString()).arg(row.at(1).toString());
                    QString cma=row.at(3).toInt()?"是":"否";
                    int methodID=row.at(0).toInt();
                    if(!m_methodGroupInfo.contains(methodID)){
                        m_methodGroupInfo[methodID]={row.at(9).toInt(),row.at(10).toString()};//这两个地方为了后续保存方法时进行样品分组
                    }
                    methods.append(method);
                    methodToID[method]=(methodID);
                    m_methodIDs[method]=(methodID);//记录方法ID表，用于后面VIEW中识别方法ID

                    maps.insert(method,cma);
                    methodAppearedTimes[method]+=1;
                    methodScore[method]=0;
//                    if(getMethod(testTypeID,parameterID)){
                        if(method==getMethod(testTypeID,parameterID)){
                            methodScore[method]+=1000;//用户确认的方法，设置为最优先
                        }

//                    }
                    if(typeBit&row.at(7).toInt()) methodScore[method]+=20;//类型匹配，加20分
                    else if(!(typeBit&row.at(8).toInt())) methodScore[method]-=20;//类型不适用，-100分
                    int labPriority=row.at(5).toInt();
                    if(labPriority) methodScore[method]+=30*labPriority;//实验室选用优先级，1级30分。
                    if(typeBit&row.at(6).toInt()) methodScore[method]+=50;//优先适配的类型，加50分（也就是类型优先高于1级选用优先，低于2级选用优先）
                    if(cma=="否") methodScore[method]-=100;
                }
                m_methodBox->setCellItems(row,2,methods);//将所有可选的方法加入选择框
                ui->tableView->append({sampleType,parameter,"","","",""});//视图增加一行
                ui->tableView->setMappingCell(row,3,row,2,maps);//设置关联单元格
                qDebug()<<row-1<<maps;
                ui->tableView->setCellFlag(row,2,QVariant::fromValue(methodToID));//记录方法ID映射，用以查找方法信息
//                if(methods.count()) {//多个方法的情况
//                    if(m_methodTable.count()>row&&m_methodTable.at(row-1).at(0)==sampleType&&m_methodTable.at(row-1).at(1)==parameter&&!m_methodTable.at(row-1).at(2).toString().isEmpty()){//如果之前有选择方法，使用之前的方法
//                        ui->tableView->setData(row-1,2,m_methodTable.at(row-1).at(2));
//                    }
//                    //                    else ui->tableView->setData(i-1,2,methods.at(0));//智能方法选择，这里先留空，后面确认最佳方法
//                }
                //                if(methods.count()>1) {
                //                    ui->tableView->setBackgroundColor(i-1,2,qRgba(250,250,100,50));
                //                    ui->tableView->repaint();
                //                }
                emit sqlFinished();
            },0,{parameterID,typeBit,typeBit,testFieldID});
            waitForSql();

            row++;//下一行
        }

    }
    //所有方法处理完成，现在要在视图上选出最优方法
    //按得分大小重新排序
    auto cellItems=m_methodBox->cellItems();//每行的方法列表
    if(cellItems.count()){
        for(auto it=cellItems.begin();it!=cellItems.end();it++){
            QStringList items=it.value();
            if(!items.count()){
                continue;
            }
            std::sort(items.begin(),items.end(),[&methodScore,&methodAppearedTimes](const QString&a,const QString&b){
                int t=methodAppearedTimes.value(a)-methodAppearedTimes.value(b)>0?1:0;
                return methodScore.value(a)+t*40>methodScore.value(b);
            });
            m_methodBox->setCellItems(it.key().first,it.key().second,items);
            ui->tableView->setData(it.key().first,it.key().second,items.first());//使用第一个方法
        }
    }
}


void MethodSelectDlg::on_OkBtn_clicked()
{
    /*
    //保存方法到QHash<int,QHash<int,MethodMore>>m_methods【类型ID，【参数ID，方法信息】】
    auto table=ui->tableView->data();
    m_methods.clear();
    for(int i=0;i<table.count();i++){
        int testTypeID=m_typeIDs.value(ui->tableView->value(i,0).toString().split("/").first());//样品类型可能有多个合在一起
        int parameterID=m_parameterIDs.value(testTypeID).value(ui->tableView->value(i,"检测项目").toString());

        QString methodName=ui->tableView->value(i,"检测方法").toString();
                             qDebug()<<QString("正在保存%1-%2-%3。").arg(ui->tableView->value(i,0).toString().split("/").first()).arg(ui->tableView->value(i,"检测项目").toString()).arg(methodName)<<testTypeID<<parameterID;
        int methodID=m_methodIDs.value(methodName);
        MethodMore *mm=new MethodMore;
        mm->methodID=methodID;
        mm->testMethodName=methodName;
        mm->subpackage=ui->tableView->value(i,"是否分包").toString()=="是"?1:0;
        mm->subpackageDesc=ui->tableView->value(i,"分包原因").toString();
        mm->CMA=ui->tableView->value(i,"CMA资质").toString()=="是"?1:0;
        if(!methodName.isEmpty()){
            mm->testMod=m_MethodMores.value(methodID)->testMod;
            mm->sampleGroup=m_MethodMores.value(methodID)->sampleGroup;
        }
        else{
            mm->testMod=-1;//没有方法
        }
        addMethod(testTypeID,parameterID,mm);
    }
    if(!m_methodLoad) this->hide();//没有加载方法，就没有改变方法。
    else accept();//accept会通知任方法有变更，保存时会保存方法。
    */
    if(!m_methodLoad) {
        this->hide();
        return;
    }

    accept();
}


void MethodSelectDlg::on_cancelBtn_clicked()
{
    reject();
}


void MethodSelectDlg::on_clearBtn_clicked()
{
    ui->tableView->clear();
    m_methods.clear();
}

