#include "staticdatamanager.h"
#include "ui_staticdatamanager.h"
#include"../Client/QExcel.h"
#include<QFileDialog>
#include<QMessageBox>
#include<QSqlQuery>
#include<QSqlError>
#include"cdatabasemanage.h"
#include<QInputDialog>
#include"../Client/global.h"
#include<QLayout>
StaticDataManager::StaticDataManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StaticDataManager)
{
    ui->setupUi(this);
    statusBar = new QStatusBar(this);
    this->layout()->addWidget(statusBar);
}

StaticDataManager::~StaticDataManager()
{
    delete ui;
}

bool StaticDataManager::checkParameter(int &parameterID, const QString &parameter, int areaID)
{
    parameterID=0;
    QSqlQuery query(DB.database());
    bool ret=false;
    query.prepare("select A.id from (select id, parameterName, testFieldID from detection_parameters where testFieldID=:ID) as A "
                  "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID "
                  "where parameterName=:value or alias=:value;");
    query.bindValue(":value",parameter);
    query.bindValue(":ID",areaID);
    if(!query.exec()){
        QMessageBox::information(this,"select id from detection_parameters error",query.lastError().text());
        DB.database().rollback();
        return ret;
    }
    if(!query.next()){//项目参数表中不没有该参数，自动添加
        //取消自动添加，应当进行确认修改后添加
//        query.prepare("INSERT INTO detection_parameters (testFieldID, parameterName) VALUES(?,?);");
//        query.addBindValue(areaID);
//        query.addBindValue(parameter);
//        if(!query.exec()){
//            QMessageBox::information(this,"INSERT INTO detection_parameters",query.lastError().text());
//            DB.database().rollback();
//            return ret;
//        }
//        query.exec("SELECT LAST_INSERT_ID()");
//        if(!query.next()) {
//            QMessageBox::information(this,"error","无法获取上次插入ID");
//            DB.database().rollback();
//            return ret;
//        }
//        parameterID=query.value(0).toInt();
        ret=false;
        //                    newParameters.append(coverages.split("、").at(0)+": "+parameter+";");
    }
    else{
        parameterID=query.value(0).toInt();
        ret=true;
    }
    return ret;
}

void StaticDataManager::on_improtLimitStandardBtn_clicked()//导入执行标准
{

    QString fileName=QFileDialog::getOpenFileName(this,"","./","Excel文件(*.xls *.xlsx)");
    if(fileName.isEmpty()) return;
    QAxObject*book=EXCEL.Open(fileName);
    if(!book){
        QMessageBox::information(this,"error","无法打开文件："+fileName);
        return;
    }
    fileName=fileName.split("/").last();
    fileName=fileName.left(fileName.indexOf("."));
    int n=fileName.indexOf("@");
    if(n<0){
        QMessageBox::information(this,"error","文件名格式错误，确认文件名<标准名称@标准号>");
        return;
    }
    QString standardName=fileName.left(n);
    QString standardNum=fileName.mid(n+1);
    int sheetCount=EXCEL.sheetCount(book);

    QAxObject* sheet;
    QSqlQuery query(DB.database());
    if(!query.exec(QString("select id from implementing_standards where standardName='%1';").arg(standardName))){
        qDebug()<<"检查执行标准时错误："<<query.lastError().text();
            return;
    }
    if(query.next()){
        QMessageBox::StandardButton bt=QMessageBox::question(nullptr,"","此标准已经存在，是否覆盖导入？");
        if(bt==QMessageBox::Yes){
            DB.database().transaction();
            query.previous();
            QSqlQuery query2(DB.database());

            QString sql;
            while(query.next()){
                int id=query.value("id").toInt();
                 sql+=QString("delete from standard_limits where standardID=%1;").arg(id);

            }
            sql+=QString("delete from implementing_standards where standardName='%1';").arg(standardName);
            if(!query2.exec(sql)){
                 QMessageBox::information(this,"error",query.lastError().text());
                 DB.database().rollback();
                 return;
            }
            DB.database().commit();
        }
        else return;
    }

    for(int i=1;i<=sheetCount;i++){
        sheet=EXCEL.selectSheet(i,book);
        QString str=EXCEL.sheetName(sheet);
        int n=str.indexOf("@");
        if(n<0){
            QMessageBox::information(this,"error","表格名称格式错误，确认文件名<表格名称@检测类型>");
            return;
        }
        QString tableName=str.left(n);
        QString testType=str.mid(n+1);
        QString sql=QString("select id,testFieldID from test_type where testType='%1'").arg(testType);

        if(!query.exec(sql)){
            QMessageBox::information(this,"查询检测类型时出错",query.lastError().text());
            return;
        }
        if(!query.next()){
            QMessageBox::information(this,"error","不存在的检测类型："+testType);
            return;
        }
        int testTypeID=query.value(0).toInt();
        int testFieldID=query.value(1).toInt();
        int c=7;
        QString classNum=EXCEL.cellValue(1,c,sheet).toString();
        if(classNum.isEmpty()){
            QMessageBox::information(this,"error","表格格式不对，G1为空值，请确认。");
            return;
        }
        sql="";
        QString newParameters="";
        if(DB.database().transaction()){
        while(!classNum.isEmpty()){
            //插入限值表
            sql=QString("INSERT INTO implementing_standards(standardName, standardNum, tableName, testTypeID, classNum) VALUES(?,?,?,?,?)");
            query.prepare(sql);
            query.addBindValue(standardName);
            query.addBindValue(standardNum);
            query.addBindValue(tableName);
            query.addBindValue(testTypeID);
            query.addBindValue(classNum);
            if(!query.exec()){
                QMessageBox::information(this,"插入表格时错误：","sql:"+query.lastQuery()+"\nError:"+query.lastError().text());
                DB.database().rollback();
                return;
            }
            int r=2;
            QString parameter=EXCEL.cellValue(r,3,sheet).toString();
            toStdParameterName(parameter);

                while(!parameter.isEmpty()){
                    query.prepare("select A.id from (select id, parameterName, testFieldID from detection_parameters where testFieldID=?) as A "
                                  "left join (select alias ,parameterID from detection_parameter_alias) as B on A.id=B.parameterID "
                                  "where parameterName=? or alias=?;");
                    query.addBindValue(testFieldID);
                    query.addBindValue(parameter);
                    query.addBindValue(parameter);
                    if(!query.exec()){
                        QMessageBox::information(this,"error","未知的检测参数："+parameter+query.lastError().text());
                        DB.database().rollback();
                        return;
                    }
                    int parameterID;
                    if(!query.next()){                        
                        query.prepare("INSERT INTO detection_parameters (testFieldID, parameterName) VALUES(?,?);");
                        query.addBindValue(testFieldID);
                        query.addBindValue(parameter);
                        if(!query.exec()){
                            QMessageBox::information(this,"INSERT INTO detection_parameters",query.lastError().text());
                            DB.database().rollback();
                            return;
                        }
                        query.exec("SELECT LAST_INSERT_ID()");
                        if(!query.next()) {
                            QMessageBox::information(this,"error","无法获取上次插入ID");
                            DB.database().rollback();
                            return;
                        }
                        parameterID=query.value(0).toInt();
                        newParameters+=parameter+"、";
                    }
                    else parameterID=query.value(0).toInt();

                    QString unit=EXCEL.cellValue(r,5,sheet).toString();
                    QString flag=EXCEL.cellValue(r,6,sheet).toString();
                    QString trade=EXCEL.cellValue(r,4,sheet).toString();
                    double higher=-1,lower=-1;
                    //确认上限和下限
                    if(flag=="[]"){
                        QString str=EXCEL.cellValue(r,c,sheet).toString();
                        lower=str.split("~").first().toDouble();
                        higher=str.split("~").last().toDouble();
                    }
                    else if(flag=="<="){
                        higher=EXCEL.cellValue(r,c,sheet).toDouble();
                    }
                    else if(flag==">="){
                        lower=EXCEL.cellValue(r,c,sheet).toDouble();
                    }
                    else{//不属于上下限的限值，即为文本类描述，记录的unit列中
                        unit=EXCEL.cellValue(r,c,sheet).toString();
                    }
                    sql="INSERT INTO standard_limits (standardID, parameterName, parameterID , trade, higher, lower, unit) "
                          "VALUES((SELECT MAX(id) FROM implementing_standards), :parameterName, :parameterID , :trade, :higher, :lower, :unit)";
                    query.prepare(sql);
                    query.bindValue(":parameterName",parameter);
                    query.bindValue(":parameterID",parameterID);
                    query.bindValue(":trade",trade);
                    query.bindValue(":higher",higher);
                    query.bindValue(":lower",lower);
                    query.bindValue(":unit",unit);
                    if(!query.exec()){
                        QMessageBox::information(this,"插入限值时错误：","sql:"+query.lastQuery()+"\nError:"+query.lastError().text());
                        DB.database().rollback();
                        return;
                    }

                    r++;
                    parameter=EXCEL.cellValue(r,3,sheet).toString();
                    toStdParameterName(parameter);
                }

                c++;
                classNum=EXCEL.cellValue(1,c,sheet).toString();
            }


        }
        else{
            qDebug()<<"启动事务失败";
            return;
        }
        DB.database().commit();
        if(!newParameters.isEmpty()){
            QFile file("./newParameters.txt");
            if(!file.open(QFile::ReadWrite)) qDebug()<<"无法创建文件。";
            else file.write(QString(testType+"\n"+newParameters).toLocal8Bit());
            QMessageBox::information(nullptr,"","操作完成，有新检测参数添加，请确认是否需要完善。");
        }
        else QMessageBox::information(nullptr,"","操作完成。");
    }
    EXCEL.CloseBook(book);
}


void StaticDataManager::on_improtTestMethod_clicked()//导入检测方法
{
    QString newParameters;
    QString fileName=QFileDialog::getOpenFileName(this,"选择文件","","EXCEL文件(*.xlsx *.xls)");
    if(fileName.isEmpty()) return;
    QAxObject*book=EXCEL.Open(fileName);
    if(!book){
        QMessageBox::information(this,"error","无法打开文件："+fileName);
        return;
    }
    QAxObject*sheet=EXCEL.selectSheet(1,book);
    int r=2;
    QString methodName=EXCEL.cellValue(r,1,sheet).toString().simplified();//列A是方法名称，去掉多余空格
    QString methodNum,unit,sampleGroup,sampleMedium,preservatives,storageCondition,stability,samplingFlow,samplingDuration,samplingRequirements,
        blankControl,curveCalibration,parallelControl,spikeControl,inspector,price;
    QString parameters,MDLs,coverages,labParameters;
    int coverage=0,testingMode,minAmount,mediumPrepare,series;
    QSqlQuery query(DB.database());
    bool rollBack=false;
    if(DB.database().transaction()){//启动事务，准备插入数据
        while(!methodName.isEmpty()){
            statusBar->showMessage(QString("正在处理第%1行……").arg(r));
            // 处理事件循环，刷新界面
            QApplication::processEvents();
            methodNum=EXCEL.cellValue(r,2,sheet).toString().simplified();//列B是标准编号
            methodNum.replace("—","-");
            if (methodNum.isEmpty()) {
                methodNum = "";
            }
            unit=EXCEL.cellValue(r,7,sheet).toString();//列G是数据单位
            sampleGroup=EXCEL.cellValue(r,10,sheet).toString();//列J是样品分组
            sampleMedium=EXCEL.cellValue(r,11,sheet).toString();//列K是采样介质
            series=EXCEL.cellValue(r,12,sheet).toInt();//列L，串联情况
            preservatives=EXCEL.cellValue(r,14,sheet).toString();//列N是固定剂
            storageCondition=EXCEL.cellValue(r,15,sheet).toString();//列O是保存条件
            stability=EXCEL.cellValue(r,16,sheet).toString();//列P是时效性
            samplingFlow=EXCEL.cellValue(r,17,sheet).toString();//列Q采样流量
            samplingDuration=EXCEL.cellValue(r,18,sheet).toString();//列R采样时间
            samplingRequirements=EXCEL.cellValue(r,19,sheet).toString();//列S采样要求
            blankControl=EXCEL.cellValue(r,20,sheet).toString();//列T空白要求
            curveCalibration=EXCEL.cellValue(r,21,sheet).toString();//列U曲线校准要求
            parallelControl=EXCEL.cellValue(r,22,sheet).toString();//列V平行要求
            spikeControl=EXCEL.cellValue(r,23,sheet).toString();//列W加标要求
            coverages=EXCEL.cellValue(r,3,sheet).toString();//列C适用范围
            testingMode=EXCEL.cellValue(r,8,sheet).toInt();//列H测试地点
            minAmount=EXCEL.cellValue(r,9,sheet).toInt();//列I测试最小用量
            mediumPrepare=EXCEL.cellValue(r,13,sheet).toInt();//列M介质是否需要准备
            parameters=EXCEL.cellValue(r,4,sheet).toString();//列D测试参数
            toStdParameterName(parameters);
            MDLs=EXCEL.cellValue(r,6,sheet).toString();//列F检出限
            labParameters=EXCEL.cellValue(r,5,sheet).toString();//列E实验室资质项目
            toStdParameterName(labParameters);
            QString labMDLs=EXCEL.cellValue(r,26,sheet).toString();//列实验室检出限
            inspector=EXCEL.cellValue(r,28,sheet).toString();//AB检测负责人
            price=EXCEL.cellValue(r,29,sheet).toString();//AC单价
            QString labPriority,priorityType;
            int typePriority=0;
            labPriority=EXCEL.cellValue(r,24,sheet).toString();//列X实验室优先级
            priorityType=EXCEL.cellValue(r,25,sheet).toString();//类型优先级
            QString sql;
            coverage=0;
            QStringList extendCoverages;
            int extendCoverage=0;
            int n=coverages.indexOf("[");
            if(n>=0) {
                extendCoverages=coverages.mid(n+1,coverages.length()-n-2).split("、");
                coverages=coverages.left(n);
            }
            for(QString x:extendCoverages){//确认扩展适用范围
                query.prepare("select bitNum from test_type where testType=?");
                query.bindValue(0,x);
                if(!query.exec()){
                    QMessageBox::information(this,"select bitNum from test_type error",query.lastError().text());
                    DB.database().rollback();
                    return;
                }
                if(!query.next()) {
                    QMessageBox::information(this,"确认扩展适用范围时出错：","没有查询到类型："+x);
                    qDebug()<<extendCoverages;
                    DB.database().rollback();
                    return;
                }
                extendCoverage|=query.value(0).toInt();
            }
            foreach(QString x,coverages.split("、")){//确认适用范围
                query.prepare("select bitNum from test_type where testType=?");
                query.bindValue(0,x);
                       if(!query.exec()){
                           QMessageBox::information(this,"select bitNum from test_type error",query.lastError().text());
                           DB.database().rollback();
                           return;
                       }
                       if(!query.next()) {
                           QMessageBox::information(this,"确认适用范围时出错：","没有查询到类型："+x);
                           DB.database().rollback();
                           return;
                       }
                    coverage|=query.value(0).toInt();
            }
            if(!priorityType.isEmpty()){
                foreach(QString x,priorityType.split("、")){//确认优先类型
                       query.prepare("select bitNum from test_type where testType=?");
                       query.bindValue(0,x);
                       if(!query.exec()){
                    QMessageBox::information(this,"select bitNum from test_type error",query.lastError().text());
                    DB.database().rollback();
                    return;
                       }
                       if(!query.next()) {
                    QMessageBox::information(this,"确认优先类型时出错：","没有查询到类型："+x);
                    DB.database().rollback();
                    return;
                       }
                       typePriority|=query.value(0).toInt();
                }
            }
            //开始解析检测参数
            QStringList parameterList;
            int parameterCount;//记录一般指标的数量，用以后面处理项目时识别综合指标ID
            QStringList classParameters;//综合指标
            QHash<QString, QStringList> classParameterSubs;//综合指标的子参数
            QHash<QString,QString>classParameterSubIDs;//综合指标的子参数ID
            QHash<QString,int>parameterIDs;//参数ID映射
            for(auto p:parameters.split("、")){//综合指标保存格式：苯系物@苯|甲苯|乙苯|二甲苯|异丙苯|苯乙烯
                int n=p.indexOf("@");
                if(n>=0){
                   classParameters.append(p.left(n));
                   parameterList.append(p.mid(n+1).split("|"));
                   classParameterSubs[p.left(n)]=p.mid(n+1).split("|");
                }
                else{
                    parameterList.append(p);
                }
            }
            QStringList MDLlist=MDLs.split("、");
            if(MDLlist.count()){//综合指标不设检出限
                if(MDLlist.count()!=1&&MDLlist.count()!=parameterList.count()){
                           QMessageBox::information(this,"error",QString("检出限数量与参数数量不匹配,at line %1").arg(r));
                       DB.database().rollback();
                           qDebug()<<parameterList;
                       return;
                }
            }
            parameterCount=parameterList.count();
            parameterList.append(classParameters);//将综合指标合并入检测指标。
            //开始解析实验室资质参数
            QStringList labParameterList;
            QStringList classlabParameterList;//综合指标
            if(!labParameters.isEmpty()) for(auto p:labParameters.split("、")){//综合指标保存格式：苯系物@苯|甲苯|乙苯|二甲苯|异丙苯|苯乙烯
                int n=p.indexOf("@");
                    if(n>=0){
                           classlabParameterList.append(p.left(n));
                           labParameterList.append(p.mid(n+1).split("|"));
                }
                    else{
                           labParameterList.append(p);
                    }
            }
            QStringList labMDLlist;
            if(!labMDLs.isEmpty()) labMDLlist=labMDLs.split("、");
            if(labMDLlist.count()){//综合指标不设检出限
                if(labMDLlist.count()!=1&&labMDLlist.count()!=labParameterList.count()){
                           QMessageBox::information(this,"error",QString("实验室检出限数量与参数数量不匹配,at line %1").arg(r));
                       DB.database().rollback();
                       return;
                }
            }
            labParameterList.append(classlabParameterList);//将综合指标合并入检测指标。
            QStringList labParameterList2=labParameterList;
            QString parameter;
            int parameterID;
            int areaID;
            //获取领域ID，用于下面自动添加检测参数
            query.prepare("select testFieldID from test_type where testType=?");
            query.bindValue(0,coverages.split("、").at(0));
           if(!query.exec()){
               QMessageBox::information(this,"select testFieldID from test_type error",query.lastError().text());
               DB.database().rollback();
               return;
           }
           if(!query.next()) {
               QMessageBox::information(this,"error",QString("没有查询到类型：%1,at line %2").arg(coverages.split("、").at(0)).arg(r));
               DB.database().rollback();
               return;
           }
            areaID=query.value(0).toInt();

            //确认是否已经存在的方法，如果存在，则更新方法
            query.prepare("select id from test_methods where methodName=? and methodNumber=? and coverage=?");
            query.addBindValue(methodName);
            query.addBindValue(methodNum);
            query.addBindValue(coverage);
            if(!query.exec()){
                   QMessageBox::information(this,"检查法时错误：",query.lastError().text());
                   DB.database().rollback();
                   return;
            }
            int methodID;
            if(query.next()){//存在的方法
               methodID=query.value(0).toInt();
//               query.prepare("update test_methods set sampleGroup=:,sampleMedium=:,preservatives=:,storageCondition=:,stability=:,samplingFlow=:,samplingDuration=:,samplingRequirements=:,"
//                             "blankControl=:,curveCalibration=:,parallelControl=:,spikeControl=:, testFieldID=:, extendCoverage=:,testingMode=:,minAmount=:,mediumPrepare=: where id=:id;");
               query.prepare("UPDATE test_methods SET sampleGroup=:sampleGroup, sampleMedium=:sampleMedium, seriesConnection=:seriesConnection, preservatives=:preservatives, storageCondition=:storageCondition, stability=:stability, samplingFlow=:samplingFlow, samplingDuration=:samplingDuration, samplingRequirements=:samplingRequirements, "
                             "blankControl=:blankControl, curveCalibration=:curveCalibration, parallelControl=:parallelControl, spikeControl=:spikeControl, testFieldID=:testFieldID, extendCoverage=:extendCoverage, testingMode=:testingMode, minAmount=:minAmount, mediumPrepare=:mediumPrepare WHERE id=:id;");
               query.bindValue(":testFieldID",areaID);
               query.bindValue(":sampleGroup",sampleGroup);
               query.bindValue(":sampleMedium",sampleMedium);
               query.bindValue(":seriesConnection",series);
               query.bindValue(":preservatives",preservatives);
               query.bindValue(":storageCondition",storageCondition);
               query.bindValue(":stability",stability);
               query.bindValue(":samplingFlow",samplingFlow);
               query.bindValue(":samplingDuration",samplingDuration);
               query.bindValue(":samplingRequirements",samplingRequirements);
               query.bindValue(":blankControl",blankControl);
               query.bindValue(":curveCalibration",curveCalibration);
               query.bindValue(":parallelControl",parallelControl);
               query.bindValue(":spikeControl",spikeControl);
               query.bindValue(":extendCoverage",extendCoverage);
               query.bindValue(":testingMode",testingMode);
               query.bindValue(":minAmount",minAmount);
               query.bindValue(":mediumPrepare",mediumPrepare);
               query.bindValue(":id",methodID);
               if(!query.exec()){
                   QMessageBox::information(this,"更新检测方法时错误：",query.lastError().text());

                   DB.database().rollback();
                    return;
               }
               //接下来更新检测参数表
               for (int i=0;i< parameterList.count();i++) {
                    parameter=parameterList.at(i);
                    //检查参数是否存在
                    if(!checkParameter(parameterID,parameter,areaID)){
                        rollBack=true;
                        newParameters.append(coverages.split("、").at(0)+": "+parameter+";");
                        continue;//这里不用return,要把每个项目都检查过去，有问题一次改好。
                    }
                    parameterIDs[parameter]=parameterID;
                    if(!parameterID){
                        QMessageBox::information(this,"checkParameter error","无法获取ID:"+parameter);
                        DB.database().rollback();
                        return;
                    }
                    //判断是否综合指标，是则需要计算子参数ID。
                    if(i>=parameterCount){
                        QStringList ids;
                        for(auto sub:classParameterSubs.value(parameter)){
                            ids.append(QString::number(parameterIDs[sub]));
                        }
                        classParameterSubIDs[parameter]=ids.join(",");//将子参数列表储存好
                    }
                    //先检测下方法参数是否存在，存在则更新
                    query.prepare("select id from method_parameters where methodID=? and parameterID=?;");
                    query.addBindValue(methodID);
                    query.addBindValue(parameterID);
                    if(!query.exec()){
                        QMessageBox::information(this,"检查方法参数时错误：",query.lastError().text());
                        DB.database().rollback();
                        return;
                    }
                    if(query.next()){//已经存在的检测参数，更新
                        int id=query.value(0).toInt();
                        query.prepare("update method_parameters set subParameterIDs=:subParameterIDs, MDL=:MDL, unit=:unit, CMA=:CMA, LabMDL=:LabMDL, non_stdMethod=:non_stdMethod, labPriority=:labPriority, typePriority=:typePriority "
                                      "where id=:id;");

                        int CMA=0;
                        int n=labParameterList.indexOf(parameter);
                        if(n>=0) {
                            CMA=1;
                            labParameterList2.removeOne(parameter);
                        }
                        double mdl;
                        if(MDLlist.count()>1&&MDLlist.count()>i)
                            mdl=MDLlist.at(i).toDouble();
                        else if(MDLlist.count())
                            mdl=MDLlist.at(0).toDouble();
                        else
                            mdl=0;
                        query.bindValue(":subParameterIDs",classParameterSubIDs[parameter]);
                        query.bindValue(":MDL",mdl);
                        query.bindValue(":unit",unit);
                        query.bindValue(":CMA",CMA);
                        if(labMDLlist.count()>1&&n<labMDLlist.count())
                            mdl=labMDLlist.at(n).toDouble();
                        else if(labMDLlist.count())
                            mdl=labMDLlist.at(0).toDouble();
                        else
                            mdl=0;
                        query.bindValue(":LabMDL",mdl);
                        query.bindValue(":non_stdMethod",false);
                        query.bindValue(":labPriority",labPriority);
                        query.bindValue(":typePriority",typePriority);
                        query.bindValue(":id",id);
                        if(!query.exec()){
                            QMessageBox::information(this,"更新方法参数时错误：",query.lastError().text());
                            qDebug()<<query.lastQuery();
                            DB.database().rollback();
                            return;
                        }
                    }
                    else{
                        //添加方法参数
                        query.prepare("INSERT INTO method_parameters ( methodID, parameterID, MDL, unit, CMA,LabMDL,labPriority, typePriority,subParameterIDs) VALUES(?,?,?,?,?,?,?,?,?);");
                        query.bindValue(0,methodID);
                        query.bindValue(1,parameterID);
                        if(MDLlist.count()>1&&MDLlist.count()>i)
                            query.bindValue(2,MDLlist.at(i).toDouble());
                        else if(MDLlist.count())
                            query.bindValue(2,MDLlist.at(0).toDouble());
                        else
                            query.bindValue(2,-1.0);
                        query.bindValue(3,unit);
                        bool CMA=false;
                        int n=labParameterList.indexOf(parameter);
                        if(n>=0) {
                            CMA=true;
                            labParameterList2.removeOne(parameter);
                        }
                        query.bindValue(4,CMA);
                        double mdl;
                        if(labMDLlist.count()>1&&n<labMDLlist.count())
                            mdl=labMDLlist.at(n).toDouble();
                        else if(labMDLlist.count())
                            mdl=labMDLlist.at(0).toDouble();
                        else
                            mdl=0;
                        query.bindValue(5,mdl);
                        query.bindValue(6,labPriority);
                        query.bindValue(7,typePriority);
                        query.bindValue(8,classParameterSubIDs.value(parameter));
                        if(!query.exec()){
                            QMessageBox::information(this,"INSERT INTO method_parameters error",query.lastError().text());
                            qDebug()<<query.lastError().text();
                            qDebug()<<query.lastQuery();
                            qDebug()<<parameterID<<parameter;
                            DB.database().rollback();
                            return;
                        }
                    }

                }

                for(auto parameter:labParameterList2){//添加非标参数（如果有）
                    if(!checkParameter(parameterID,parameter,areaID)){
                        rollBack=true;
                        newParameters.append(coverages.split("、").at(0)+": "+parameter+";");
                        continue;
                    }
                    if(!parameterID){
                        QMessageBox::information(this,"checkParameter error","无法获取ID:"+parameter);
                        DB.database().rollback();
                        return;
                    }
                    //检查是否存在该参数信息
                    query.prepare("select id from method_parameters where methodID=? and parameterID=?;");
                    query.addBindValue(methodID);
                    query.addBindValue(parameterID);
                    if(!query.exec()){
                        QMessageBox::information(this,"检查方法参数时错误：",query.lastError().text());
                        DB.database().rollback();
                        return;
                    }
                    if(query.next()){//已经存在的检测参数，更新
                        int id=query.value(0).toInt();
                        query.prepare("update method_parameters set MDL=:MDL, unit=:unit, CMA=:CMA, LabMDL=:LabMDL, non_stdMethod=:non_stdMethod, labPriority=:labPriority, typePriority=:typePriority "
                                      "where id=:id;");

                        bool CMA=true;
                        int n=labParameterList.indexOf(parameter);
                        double mdl;
                        mdl=0;
                        query.bindValue(":MDL",mdl);
                        query.bindValue(":unit",unit);
                        query.bindValue(":CMA",CMA);
                        if(labMDLlist.count()>1&&n<labMDLlist.count())
                            mdl=labMDLlist.at(n).toDouble();
                        else if(labMDLlist.count())
                            mdl=labMDLlist.at(0).toDouble();
                        else
                            mdl=0;
                        query.bindValue(":LabMDL",mdl);
                        query.bindValue(":non_stdMethod",true);
                        query.bindValue(":labPriority",labPriority);
                        query.bindValue(":typePriority",typePriority);
                        query.bindValue(":id",id);
                        if(!query.exec()){
                            QMessageBox::information(this,"更新方法参数时错误：",query.lastError().text());
                            DB.database().rollback();
                            return;
                        }
                    }
                    else{
                        query.prepare("INSERT INTO method_parameters ( methodID, parameterID, MDL, unit, CMA,LabMDL,labPriority, typePriority) VALUES(?,?,?,?,?,?,?,?);");
                        query.bindValue(0,methodID);
                        query.bindValue(1,parameterID);
                        query.bindValue(2,0);
                        query.bindValue(3,unit);
                        int n=labParameterList.indexOf(parameter);
                        query.bindValue(4,true);
                        double mdl;
                        if(labMDLlist.count()>1&&n<labMDLlist.count())
                            mdl=labMDLlist.at(n).toDouble();
                        else if(labMDLlist.count())
                            mdl=labMDLlist.at(0).toDouble();
                        else
                            mdl=0;
                        query.bindValue(5,mdl);
                        query.bindValue(6,labPriority);
                        query.bindValue(7,typePriority);
                        if(!query.exec()){
                            QMessageBox::information(this,"INSERT INTO method_parameters error",query.lastError().text());
                            qDebug()<<query.lastError().text();
                            qDebug()<<query.lastQuery();
                            qDebug()<<parameterID<<parameter;
                            DB.database().rollback();
                            return;
                        }
                    }
                }
            }
            else{
                //插入新方法
                sql="INSERT INTO test_methods (methodName,methodNumber,sampleGroup,sampleMedium,seriesConnection,preservatives,storageCondition,stability,samplingFlow,samplingDuration,samplingRequirements,"
                      "blankControl,curveCalibration,parallelControl,spikeControl, testFieldID, coverage,testingMode,minAmount,mediumPrepare,extendCoverage) "
                      "VALUES(:methodName,:methodNum,:sampleGroup,:sampleMedium,:seriesConnection,:preservatives,:storageCondition,:stability,:samplingFlow,:samplingDuration,:samplingRequirements,"
                      ":blankControl,:curveCalibration,:parallelControl,:spikeControl, :testFieldID, :coverage,:testingMode,:minAmount,:mediumPrepare,:extendCoverage)";
                query.prepare(sql);
                query.bindValue(":methodName",methodName);
                query.bindValue(":methodNum",methodNum);
                query.bindValue(":testFieldID",areaID);
                query.bindValue(":sampleGroup",sampleGroup);
                query.bindValue(":sampleMedium",sampleMedium);                
                query.bindValue(":seriesConnection",series);
                query.bindValue(":preservatives",preservatives);
                query.bindValue(":storageCondition",storageCondition);
                query.bindValue(":stability",stability);
                query.bindValue(":samplingFlow",samplingFlow);
                query.bindValue(":samplingDuration",samplingDuration);
                query.bindValue(":samplingRequirements",samplingRequirements);
                query.bindValue(":blankControl",blankControl);
                query.bindValue(":curveCalibration",curveCalibration);
                query.bindValue(":parallelControl",parallelControl);
                query.bindValue(":spikeControl",spikeControl);
                query.bindValue(":coverage",coverage);
                query.bindValue(":testingMode",testingMode);
                query.bindValue(":minAmount",minAmount);
                query.bindValue(":mediumPrepare",mediumPrepare);
                query.bindValue(":extendCoverage",extendCoverage);
                if(!query.exec()){
                    QMessageBox::information(this,"INSERT INTO test_methods error",query.lastError().text());
                    qDebug()<<query.lastQuery();
                    qDebug()<<query.lastError().text();
                    DB.database().rollback();
                    return;
                }
                query.exec("SELECT LAST_INSERT_ID()");
                   if(!query.next()) {
                       QMessageBox::information(this,"error","无法获取上次插入ID");
                       DB.database().rollback();
                       return;
                   }
                methodID=query.value(0).toInt();
                //插入方法参数表
                for (int i=0;i< parameterList.count();i++) {
                    parameter=parameterList.at(i);
                    if(!checkParameter(parameterID,parameter,areaID)){
                        rollBack=true;
                        newParameters.append(coverages.split("、").at(0)+": "+parameter+";");
                        continue;
                    }
                    if(!parameterID){
                        QMessageBox::information(this,"checkParameter error","无法获取ID:"+parameter);
                        DB.database().rollback();
                        return;
                    }
                    //判断是否综合指标，是则需要计算子参数ID。
                    if(i>=parameterCount){
                        QStringList ids;
                        for(auto sub:classParameterSubs.value(parameter)){
                            ids.append(QString::number(parameterIDs[sub]));
                        }
                        classParameterSubIDs[parameter]=ids.join(",");//将子参数列表储存好
                    }
                    //开始保存方法参数
                    query.prepare("INSERT INTO method_parameters ( methodID, parameterID, MDL, unit, CMA,LabMDL,labPriority, typePriority, subParameterIDs) VALUES(?,?,?,?,?,?,?,?,?);");
                    query.bindValue(0,methodID);
                    query.bindValue(1,parameterID);
                    if(MDLlist.count()>1&&MDLlist.count()>i)
                        query.bindValue(2,MDLlist.at(i).toDouble());
                    else if(MDLlist.count())
                        query.bindValue(2,MDLlist.at(0).toDouble());
                    else
                        query.bindValue(2,-1.0);
                    query.bindValue(3,unit);
                    bool CMA=false;
                    int n=labParameterList.indexOf(parameter);
                    if(n>=0) {
                        CMA=true;
                        labParameterList2.removeOne(parameter);
                    }
                    query.bindValue(4,CMA);
                    double mdl;
                    if(labMDLlist.count()>1&&n<labMDLlist.count())
                        mdl=labMDLlist.at(n).toDouble();
                    else if(labMDLlist.count())
                        mdl=labMDLlist.at(0).toDouble();
                    else
                        mdl=0;
                    query.bindValue(5,mdl);
                    query.bindValue(6,labPriority);
                    query.bindValue(7,typePriority);
                    query.bindValue(8,classParameterSubIDs.value(parameter));
                    if(!query.exec()){
                        QMessageBox::information(this,"INSERT INTO method_parameters error",query.lastError().text());
                        qDebug()<<query.lastError().text();
                        qDebug()<<query.lastQuery();
                        qDebug()<<parameterID<<parameter;
                        DB.database().rollback();
                        return;
                    }
                }
                for(auto parameter:labParameterList2){//添加非标参数（如果有）
                    if(!checkParameter(parameterID,parameter,areaID)){
                        newParameters.append(coverages.split("、").at(0)+": "+parameter+";");
                        rollBack=true;
                        continue;
                    }
                    if(!parameterID){
                        QMessageBox::information(this,"checkParameter error","无法获取ID:"+parameter);
                        DB.database().rollback();
                        return;
                    }
                    query.prepare("INSERT INTO method_parameters ( methodID, parameterID, MDL, unit, CMA,LabMDL,labPriority, typePriority) VALUES(?,?,?,?,?,?,?,?);");
                    query.bindValue(0,methodID);
                    query.bindValue(1,parameterID);
                    query.bindValue(2,0);
                    query.bindValue(3,unit);
                    int n=labParameterList.indexOf(parameter);
                    query.bindValue(4,true);
                    double mdl;
                    if(labMDLlist.count()>1&&n<labMDLlist.count())
                        mdl=labMDLlist.at(n).toDouble();
                    else if(labMDLlist.count())
                        mdl=labMDLlist.at(0).toDouble();
                    else
                        mdl=0;
                    query.bindValue(5,mdl);
                    query.bindValue(6,labPriority);
                    query.bindValue(7,typePriority);
                    if(!query.exec()){
                        QMessageBox::information(this,"INSERT INTO method_parameters error",query.lastError().text());
                        qDebug()<<query.lastError().text();
                        qDebug()<<query.lastQuery();
                        qDebug()<<parameterID<<parameter;
                        DB.database().rollback();
                        return;
                    }
                }
            }
            r++;
            methodName=EXCEL.cellValue(r,1,sheet).toString().simplified();
        }
    }
    else{
        qDebug()<<"启动事务失败";
        return;
    }
    if(!rollBack) DB.database().commit();
    else DB.database().rollback();
    if(!newParameters.isEmpty()){
        QFile file("./newParameters.txt");
        if(!file.open(QFile::ReadWrite)) qDebug()<<"无法创建文件。";
        else file.write(newParameters.toLocal8Bit());
        QMessageBox::information(nullptr,"","操作错误，有未知的检测参数，请打开./newParameters.txt文件确认。");
        file.close();
    }
    else QMessageBox::information(nullptr,"","操作完成。");
    statusBar->clearMessage();
    EXCEL.CloseBook(book);
}


void StaticDataManager::on_improtParameterBtn_clicked()
{
    QString fileName=QFileDialog::getOpenFileName(this,"选择文件","","EXCEL文件(*.xlsx *.xls)");
    if(fileName.isEmpty()) return;
    QAxObject*book=EXCEL.Open(fileName);
    if(!book){
        QMessageBox::information(this,"error","无法打开文件："+fileName);
        return;
    }
    QAxObject*sheet=EXCEL.selectSheet(1,book);
    int r=2;
    int id=EXCEL.cellValue(r,1,sheet).toInt();
    QSqlQuery query(DB.database());
    if(DB.database().transaction()){
        while(id){
            int testFiledID=EXCEL.cellValue(r,2,sheet).toInt();
            QString parameterName=EXCEL.cellValue(r,3,sheet).toString();
            toStdParameterName(parameterName);
            QString aliasStr=EXCEL.cellValue(r,10,sheet).toString();
            QStringList alias;
            if(!aliasStr.isEmpty())  alias=aliasStr.split("、");
            query.prepare("Select id from detection_parameters where id=?");
            query.bindValue(0,id);
            if(!query.exec()){
                QMessageBox::information(this,"查询ID时错误",query.lastError().text());
                DB.database().rollback();
                return;
            }
            if(!query.next()){
                //新项目
                query.prepare("insert into detection_parameters (id, testFieldID, parameterName) values(?,?,?)");
                query.addBindValue(id);
                query.addBindValue(testFiledID);
                query.addBindValue(parameterName);
                if(!query.exec()){
                    QMessageBox::information(this,"插入参数时错误",parameterName+"; Error:"+query.lastError().text());
                    DB.database().rollback();
                    return;
                }
            }
            else{
                //已经存在的项目(针对旧表的更新）
                query.prepare("update detection_parameters set parameterName=? where id=?");
                query.addBindValue(parameterName);
                query.addBindValue(id);
                if(!query.exec()){
                    QMessageBox::information(this,"修改参数时错误",query.lastError().text());
                    DB.database().rollback();
                    return;
                }
            }
            //插入别名表
            for(QString a:alias){
                query.prepare("insert into detection_parameter_alias (parameterID, alias) values(?,?)");
                query.addBindValue(id);
                query.addBindValue(a);
                if(!query.exec()){
                    QMessageBox::information(this,"插入别名时错误","sql:"+query.lastQuery()+" Error:"+query.lastError().text());
                    DB.database().rollback();
                    return;
                }
            }
            r++;
            id=EXCEL.cellValue(r,1,sheet).toInt();
        }
    }
    else{
        qDebug()<<"启动事务失败";
        return;
    }
    DB.database().commit();
    QMessageBox::information(this,"","操作成功");
}

void StaticDataManager::on_parameterStdBtn_clicked()
{
    //代码有问题
//    ui->status->setText("正在处理……");
//    QString sql="select id, parameterName from detection_parameters;";
//    QSqlQuery query(DB.database());
//    if(!query.exec(sql)){
//        QMessageBox::information(nullptr,"error:",query.lastError().text());
//        return;
//    }
//    int id;QString name;
//    int n=0;
//    sql="";
//    while(query.next()){
//        id=query.value("id").toInt();
//        name=query.value(1).toString();
//        QString t_name=name;
//        toStdParameterName(t_name);
//        if(t_name!=name){
//            n++;
//            ui->status->setText("正在修改"+name);
//            sql+=QString("update detection_parameters set parameterName='%1' where id=%2;").arg(t_name).arg(id);
//        }

//    }
//    if(!query.exec(sql)){
//        QMessageBox::information(nullptr,"error:",query.lastError().text());
//        return;
//    }
//    QMessageBox::information(nullptr,"",QString("操作完成，共修改%1个数据。").arg(n));
//    ui->status->clear();
}


void StaticDataManager::on_methodDDbtn_clicked()//检测方法去重：因为methodNumber为空时导致唯一约束失效，现手动删除重复。
{
    QSqlQuery query(DB.database());
//    DB.database().transaction();
//    if(!query.exec("select GROUP_CONCAT(  id SEPARATOR ','),methodName from test_methods where methodNumber is null group by methodName")){
//        QMessageBox::information(nullptr,"查询方法出错：",query.lastError().text());
//        DB.database().rollback();
//        return;
//    }
//    QStringList allIds;
//    while (query.next()){
//        allIds.append(query.value(0).toString());
//    }
//    for(auto ids:allIds){
//        QStringList IDs=ids.split(",");
//        for(int i=0;i<IDs.count();i++){
//            int id=IDs.at(i).toInt();
//            query.exec(QString("delete from method_parameters where methodID=%1").arg(id));
//            query.exec(QString("delete from test_methods where id=%1").arg(id));
//        }
//    }
//    DB.database().commit();
    if(!query.exec("select monitoringInfoID, taskSheetID, testTypeID, parameterID, parameterName, reportNum from task_methods as A left join test_task_info as B on A.taskSheetID=B.id where B.deleted=0")){
        QMessageBox::information(nullptr,"查询方法出错：",query.lastError().text());
        return;
    }
    while(query.next()){
        QSqlQuery q(DB.database());
        q.prepare("insert into task_parameters (taskSheetID,monitoringInfoID,testTypeID,parameterID,parameterName,reportNum) values(?,?,?,?,?,?)");
        q.addBindValue(query.value("taskSheetID"));
        q.addBindValue(query.value("monitoringInfoID"));
        q.addBindValue(query.value("testTypeID"));
        q.addBindValue(query.value("parameterID"));
        q.addBindValue(query.value("parameterName"));
        q.addBindValue(query.value("reportNum"));
        if(!q.exec()){
            QMessageBox::information(nullptr,"拷贝参数时出错：",query.lastError().text());
            return;
        }
    }
    QMessageBox::information(nullptr,"","操作完成");
}

