#include "QExcel.h"
#include<QDebug>
#include<QFile>
#include<QtSql/QSqlQuery>
#include<QMessageBox>
#include<QList>
#include<QtSql/QSqlError>
#include<QVariantList>
#include<QDir>
#include<QException>
#include<QBuffer>
#include<QMessageBox>
#include <QtMath>
void QExcel::openExcel()
{

    //    _ExcelApp = new QAxObject();
    //    _ExcelApp->setControl("Excel.Application");//连接Excel控件
    _ExcelApp=new QAxObject("Excel.Application",0);
    if(_ExcelApp){
        qDebug()<<"create excel success.";
    }
    else{
        qDebug()<<"unable to create excel.";
        return;
    }
    excelOpend=true;
    _ExcelApp->dynamicCall("SetVisible(bool)", true);//false不显示窗体
    _ExcelApp->setProperty("DisplayAlerts", false);//不显示任何警告信息。
    _WorkBooks=_ExcelApp->querySubObject("WorkBooks");
    //    QAxObject *workbook = _WorkBooks->querySubObject("Open(const QString&)", "E:/工作/南平排放限值.xlsx");
    //    qDebug()<<workbook->dynamicCall("Name()");
    //    qDebug()<<_ExcelApp->querySubObject("ActiveWorkbook")->property("Name");
}

bool QExcel::isOpen(const QString&fileName) const
{
    if(!_WorkBooks) return false;
    int c=_WorkBooks->dynamicCall("Count()").toInt();
    if(!c) return false;
    if(!fileName.isEmpty()){
        for(int i=1;i<=c;i++){
            QAxObject* book=_WorkBooks->querySubObject("Item(int)",i);
            QString bookName=book->property("Name").toString();
            delete book;
            if(bookName==fileName) return true;
        }
        return false;
    }
    return true;
}

void QExcel::Quit()
{
    _ExcelApp->dynamicCall("Quit(void)");
    delete _ExcelApp;
    _ExcelApp=nullptr;
    excelOpend=false;
    qDebug()<<"quit exel";
}

void QExcel::setScreenUpdating(bool update)
{
    _ExcelApp->setProperty("ScreenUpdating",update);
}

void QExcel::SetVisible(bool v)
{
    _ExcelApp->dynamicCall("SetVisible(bool)", v);
}

void QExcel::DisplayAlerts(bool v)
{
    _ExcelApp->setProperty("DisplayAlerts", v);
}

QAxObject *QExcel::newBook()
{
    if(!excelOpend) openExcel();
    if(!excelOpend){
        QMessageBox::information(nullptr,"error","无法打开EXCEL应用。");
        return nullptr;
    }
    _ExcelApp->setProperty("Visible", true);
    if(!_WorkBooks) _WorkBooks=_ExcelApp->querySubObject("WorkBooks");
    _WorkBooks->dynamicCall("Add");
    return _ExcelApp->querySubObject("ActiveWorkBook");
}

QAxObject* QExcel::Open(const QString &path, const QVariant &UpdateLinks, bool readOnly, const QString& password)
{
    if(!excelOpend) openExcel();
    if(!excelOpend){
        QMessageBox::information(nullptr,"error","无法打开EXCEL应用。");
        return nullptr;
    }
    _ExcelApp->setProperty("Visible", true);
    QDir dir(path);
    QFile file(path);
    if(!_WorkBooks) _WorkBooks=_ExcelApp->querySubObject("WorkBooks");
    if (file.exists())
    {
        return _WorkBooks->querySubObject("Open(const QString &,const QVariant &, const QVariant& , const QVariant&,const QVariant&,const QVariant&)", dir.absolutePath(),UpdateLinks,readOnly,QVariant(),QVariant(),password);

    }
    else {
        _lastError="文件不存在："+dir.absolutePath();
                                            return nullptr;
    }
}

void QExcel::Close()
{
    if(!_WorkBooks) return;
    _WorkBooks->dynamicCall("Close()");
    delete _WorkBooks;
    _WorkBooks=nullptr;
}

bool QExcel::CloseBook(QAxObject *book)
{
    if(!book){
        _lastError="文件未打开！";
        return false;
    }
    book->dynamicCall("Close()");
    delete  book;
    return true;
}

void QExcel::showExcel(bool show)const
{
    _ExcelApp->dynamicCall("SetVisible(bool)", show);
}

QAxObject *QExcel::getBook(const QString &bookName)const
{
    if(!isOpen(bookName)){
        QMessageBox::critical(nullptr,"error","error: file not open");
        //qDebug()<<"error: file not open";
        return nullptr;
    }
    return _WorkBooks->querySubObject("Item(const QString&)",bookName);
}

int QExcel::sheetCount(QAxObject *book) const
{
    if(!book) return 0;
    QAxObject* shees=book->querySubObject("Worksheets()");
    int n=shees->dynamicCall("Count()").toInt();
    delete  shees;
    return n;
}

int QExcel::rowCount(QAxObject *sheet) const
{
    if(!sheet) return 0;
    QAxObject * usedrange = sheet->querySubObject("UsedRange");
    QAxObject * rows = usedrange->querySubObject("Rows");
    int iRows = rows->property("Count").toInt();
    delete  usedrange;
    return iRows;
}

int QExcel::columnCount(QAxObject *sheet) const
{
    if(!sheet) return 0;
    QAxObject * usedrange = sheet->querySubObject("UsedRange");
    QAxObject * columns = usedrange->querySubObject("Columns");
    int iColumns = columns->property("Count").toInt();
    delete  usedrange;
    return iColumns;
}

int QExcel::pages(QAxObject *worksheet) const
{
    QAxObject* pageSetup = worksheet->querySubObject("PageSetup()");
    QAxObject* pages=pageSetup->querySubObject("Pages()");
    int p=pages->dynamicCall("count()").toInt();
    pageSetup->clear();
    pages->clear();
    return p;
}

QString QExcel::sheetName(QAxObject *sheet) const
{
    return sheet->dynamicCall("Name()").toString();
}

QAxObject *QExcel::ActiveSheet(QAxObject *book)const
{
    if(!isOpen()){
        QMessageBox::critical(nullptr,"error","error: file not open");
        // qDebug()<<"error: file not open";
        return nullptr;
    }
    if(!book) {
        book=_ExcelApp->querySubObject("ActiveWorkbook");
        QAxObject* sheet=book->querySubObject("ActiveSheet");
        delete book;
        return sheet;
    }
    return book->querySubObject("ActiveSheet");
}

QAxObject * QExcel::selectSheet(const QString &sheetName,QAxObject * book) const
{
    if(!isOpen()){
        // qDebug()<<"error: file not open";
        QMessageBox::critical(nullptr,"error","error: file not open");
        return nullptr;
    }
    QAxObject * WorkSheets=book->querySubObject("WorkSheets");
    int c=WorkSheets->dynamicCall("Count()").toInt();
    QAxObject* Sheet;
    QString name;
    for(int i=1;i<=c;i++){
        Sheet=WorkSheets->querySubObject("Item(int)",i);
        name=Sheet->property("Name").toString();
        if(name==sheetName) {
            return Sheet;
        }
        delete Sheet;
    }
    QMessageBox::critical(nullptr,"error","表格名称错误。");
    return nullptr;
}

QAxObject * QExcel::selectSheet(int i, QAxObject * book)const
{
    if(!isOpen()){
        qDebug()<<"error: file not open";
        return nullptr;
    }
    QAxObject *WorkSheets=book->querySubObject("WorkSheets");
    int c=WorkSheets->dynamicCall("Count()").toInt();
    if(i>c||i<1){
        QMessageBox::critical(nullptr,"error","错误的索引号，超出表格数量。");
        return nullptr;
    }
    return WorkSheets->querySubObject("Item(int)",i);
}

QAxObject *QExcel::selectRow(int row, QAxObject *sheet) const
{
    return sheet->querySubObject("Rows(const QString&)",QString("%1:%1").arg(row).arg(row));
}


QVariant QExcel::cellValue(int row, int column, QAxObject * sheet) const
{
    QVariant var;
    if(!isOpen()){
        qDebug()<<"error: file not open";
        return var;
    }
    QAxObject* range=sheet->querySubObject("Cells(int,int)",row, column);
    var=range->dynamicCall("Value2()");
    delete range;
    return var;
}

void QExcel::writeCell(int row, int column, QAxObject *sheet, const QVariant &var) const
{
    if(!sheet) return ;
    sheet->querySubObject("Cells(int,int)",row,column)->setProperty("Value2",var);
}

void QExcel::writeRange( const QVariant &v,const QString &rangeAdds, QAxObject *sheet) const
{
    if(!isOpen()) return;
    QAxObject* range;
    bool delFlag=false;
    if(!sheet){
        sheet=ActiveSheet();
        delFlag=true;
    }
    if(rangeAdds.isEmpty()) range=getSelectRange();
    else range=sheet->querySubObject("Range(const QString&)",rangeAdds);
    if(!range) return;
    range->setProperty("Value",v);
    if(delFlag) delete sheet;
    delete range;
}

void QExcel::writeRow(int row, QAxObject *sheet, const QList<QVariant> &var) const
{
    for(int i=1;i<=var.count();i++) {
        writeCell(row,i,sheet,var.at(i-1));
    }
}

QAxObject *QExcel::getSelectRange(QAxObject *sheet) const
{
    if(!isOpen()) return nullptr;
    if(!sheet) {
        sheet=ActiveSheet();
        QAxObject *range=sheet->querySubObject("Select");
        delete sheet;
        return range;
    }
    return sheet->querySubObject("Select");
}

void QExcel::writeCurrent(const QVariant &var) const
{
    QAxObject *range=getSelectRange();
    if(!range) return;
    range->setProperty("Value2",var);
    delete range;
}

QVariant QExcel::readAll( QAxObject * sheet) const
{
    QVariant var;
    if(!isOpen()){
        qDebug()<<"error: file not open";
        return var;
    }
    QAxObject *usedRange = sheet->querySubObject("UsedRange");
    if(nullptr == usedRange || usedRange->isNull())
    {
        return var;
    }
    var = usedRange->dynamicCall("Value");
    delete usedRange;
    return var;
}

QVariant QExcel::readAera(QAxObject *sheet, int startRow, int startColumn, int endRow, int endColumn) const
{
    QAxObject* range=sheet->querySubObject("Range(const QString&)",areaName(startRow,endRow,startColumn,endColumn));
    QVariant var=range->dynamicCall("Value2()");
    delete range;
    return  var;
}

void QExcel::setCellValue(int row, int column, const QVariant &value,QAxObject* sheet)const
{
    sheet->querySubObject("Cells(int,int)",row, column)->dynamicCall("SetValue(const QVariant&)",value);
}

void QExcel::writeArea(const QList<QVariantList> &data, QAxObject* sheet,int startColumn, int startRow)
{
    if(data.size()<1) return;
    QVariantList v;
    int row,column;
    row=data.size();
    column=data.at(0).size();
    QVariant var;
    castListListVariant2Variant(var,data);
    QString str=QString("%1%2:%3%4")
                      .arg(convertToColName(startColumn)).arg(startRow).arg(convertToColName(startColumn+column-1))
                      .arg((startRow+row-1));
    QAxObject* range=sheet->querySubObject("Range(const QString&)",str);
    if(nullptr == range || range->isNull())
    {
        qDebug()<<"error range area:"<<str;
        return ;
    }
    if(!range->setProperty("Value", var)){
        qDebug()<<"error on write range:"<<str;
    }
}

void QExcel::writeArea(const QVariant &data, QAxObject *sheet, int startColumn, int startRow)
{
    QVariantList vl=data.toList();
    int row=vl.count();
    int column=vl.at(0).toList().count();
    if(!row||!column) return;
    QString str=QString("%1%2:%3%4")
                      .arg(convertToColName(startColumn)).arg(startRow).arg(convertToColName(startColumn+column-1))
                      .arg((startRow+row-1));
    QAxObject* range=sheet->querySubObject("Range(const QString&)",str);
    if(nullptr == range || range->isNull())
    {
        qDebug()<<"error range area:"<<str;
        return ;
    }
    if(!range->setProperty("Value", data)){
        qDebug()<<"error on write range:"<<str;
    }
}

QAxObject *QExcel::find(QAxObject *ranges,  const QString& str,const QVariant &After, const QVariant &LookIn, const QVariant &LookAt, const QVariant &SearchOrder, const QVariant &SearchDirection, const QVariant &MatchCase) const
{
    return ranges->querySubObject("Find(const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&, const XlSearchDirection&, const QVariant&)",
                                  str, After, LookIn, LookAt,SearchOrder, SearchDirection,MatchCase);
}

bool QExcel::replace(QAxObject *ranges, const QString &What, const QString &Replacement, const QVariant &LookAt, const QVariant &SearchOrder, const QVariant &SearchDirection, const QVariant &MatchCase) const
{
    return ranges->querySubObject("Replace(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)",
                                  What, Replacement, LookAt,SearchOrder, SearchDirection,MatchCase);
}

void QExcel::insertPic(QAxObject*sheet,const QString&picPath,int column,int row,int left,int top,int width,int height)
{
    QAxObject* shapes = sheet->querySubObject("Shapes");
    QAxObject*range=sheet->querySubObject("Cells(int,int)",row,column);
    QAxObject* shape = shapes->querySubObject("AddPicture(const QString&, bool, bool, double, double, double, double)", picPath, false, true, range->property("Left").toInt()+left, range->property("Top").toInt()-top, width, height);
    if(!shape) {
     _lastError = "无法载入图片：" + picPath;
        qDebug()<<_lastError;
     return;
    }
    ////	shape->setProperty("Top", range->property("Top").toInt()-top); // 图片的上边缘对齐单元格的上边缘
    ////	shape->setProperty("Left", range->property("Left").toInt()+left); // 图片的左边缘对齐单元格的左边缘


    // 释放资源
    shape->clear();
    shapes->clear();
    range->clear();
}

void QExcel::insertPic(QAxObject *sheet, QAxObject *range, const QString &picPath, int left, int top, int width, int height)
{
    insertPic(sheet,picPath,range->dynamicCall("column()").toInt(),range->dynamicCall("row()").toInt(),left,top,width,height);
}

//bool QExcel::toDbTable(QAxObject *sheet, const QSqlDatabase &db, QString tableName, int startColumn, int endColumn, bool isNewTable)
//{
//    if(!sheet){
//        _lastError = " WorkSheet not selected.";
//        return false;
//    }
//    qDebug()<<"处理表格："<<sheet->property("Name").toString();
//                                     if(tableName.isEmpty()) tableName=sheet->property("Name").toString();
//    if(tableName.isEmpty()){
//        _lastError = "table name is incorrect.";
//        return false;
//    }
//    QSqlQuery query(db);
//    QString sql;
//    //获取数据
//    qDebug()<<"读取EXCEL数据...";
//    int columns=-1;

//    //***************处理表格范围********************************
//    QAxObject * usedrange = sheet->querySubObject("UsedRange");
//    QAxObject * Range = usedrange->querySubObject("Columns");
//    if(!endColumn) endColumn=Range->property("Count").toInt();
//    if(!startColumn) startColumn=Range->property("Column").toInt();
//    delete Range;
//    Range=usedrange->querySubObject("Rows(int)",1);
//    QVariantList v=Range->dynamicCall("Value2()").toList();
//    bool flag=false;
//    for(int i=0;i<endColumn;i++){

//        QString str=v[0].toList()[i].toString();
//        qDebug()<<str;
//        if(str.indexOf("|")>=0) {
//            startColumn=i+1;
//            flag=true;
//            break;
//        }

//    }
//    if(!flag){
//        _lastError="数据错误，表头需要后接|注明数据类型。";
//        return false;
//    }

//    Range=usedrange->querySubObject("Columns(int)",startColumn);
//    int nRow=0;
//    v=Range->dynamicCall("Value2()").toList();
//    if(v.count()<=1){
//        _lastError=("空表。");
//        return false;
//    }
//    for(int i=1;i<v.count();i++){//第一列非空位置。（取excel用过的且非空的数据，防止大量空数据出现。）
//        if(!v[i].toList()[0].isValid()){
//            nRow=i;
//            break;
//        }
//    }
//    if(!nRow){
//        nRow=usedrange->querySubObject("End(int)",-4121)->property("Row").toInt();
//    }
//    QString str;
//    Range=sheet->querySubObject(("Cells(int,int)"),Range->property("Row").toInt(),startColumn);
//    str=Range->dynamicCall("Address(bool,bool)",false,false).toString();
//    delete Range;
//    str.append(":");
//    Range=sheet->querySubObject(("Cells(int,int)"),nRow,endColumn);
//    str.append(Range->dynamicCall("Address(bool,bool)",false,false).toString());
//    delete Range;
//    Range=nullptr;
//    delete usedrange;
//    usedrange=nullptr;
//    Range=sheet->querySubObject("Range(const QString&)",str);
//    //获取数据
//    QVariant value=Range->dynamicCall("Value2()");
//    QList<QVariant> Rows=value.toList();
//    QList<QVariant> head=Rows[0].toList();
//    delete Range;
//    Range=nullptr;
//    //***************表格数据处理完成***************

//    //对数据进行检查,抬头不能为空
//    for(int i=0;i<head.size();i++){
//        if(!head[i].isValid()){
//            columns=i;
//            break;
//        }
//    }
//    if(!columns){
//        _lastError=("EXCEL表格格式有误");
//        return false;
//    }
//    if(columns==-1)columns=head.count();
//    //创建表
//    qDebug()<<"creating new table...";
//    if(isNewTable){
//        sql=QString("drop table '%1'").arg(tableName);
//        query.exec(sql);
//        sql=QString("create table '%1'(").arg(tableName);
//        for(int i=0;i<columns;i++){
//            auto k=head[i];
//            QString str=k.toString();
//            int p=str.indexOf("|");
//            if(p<0){
//                _lastError="数据错误，表头需要后接|注明数据类型。";
//                return false;
//            }
//            sql.append(QString("%1 %2").arg(str.left(p)).arg(str.mid(p+1)));
//            if(i<columns-1){
//                sql.append(",");
//            }
//            else sql.append(");");
//        }
//        if(!query.exec(sql)){
//            _lastError="sql error:"+sql+"\n"+query.lastError().text();
//            return false;
//        }
//        qDebug()<<"table create finish.";
//    }
//    //写入数据
//    query.exec(QString("alter table %1 convert to character set utf8;").arg(tableName));
//    qDebug()<<"start to write table...";
//    QSqlDatabase::database().transaction();//大量写入数据，要使用事务操作，避免频繁读写硬盘。
//    QString s1="(",s2="values(";
//    for(int i=0;i<columns;i++){
//        auto k=head[i];
//        QString str=k.toString();
//        str=str.left(str.indexOf("|"));
//        s1.append(str);
//        s2.append("?");
//        if(i<columns-1){
//            s1.append(",");
//            s2.append(",");
//        }
//        else {
//            s1.append(")");
//            s2.append(")");
//        }
//    }
//    sql=QString("insert into  %1 %2 %3").arg(tableName).arg(s1).arg(s2);
//    query.prepare(sql);
//    QList<QVariantList> binder;
//    QVariantList row1=Rows[1].toList();
//    for(int j=0;j<columns;j++){
//        QVariantList v;
//        v.append(row1[j]);
//        binder.append(v);
//    }
//    for(int i=2;i<Rows.count();i++){
//        QVariantList row=Rows[i].toList();
//        for(int j=0;j<columns;j++){
//            binder[j].append(row[j]);
//        }
//    }
//    qDebug()<<"binding value...";
//    for(int i=0;i<columns;i++){
//        query.addBindValue(binder[i]);
//    }
//    qDebug()<<"execBatch...";
//    if(!query.execBatch()){
//        _lastError= "写入错误:"+query.lastError().text();
//                                       return false;
//    }
//    QSqlDatabase::database().commit();
//    qDebug()<<"write data success.";
//    _lastError="";
//    return true;

//}

//bool QExcel::toDB(QAxObject *book, const QSqlDatabase &db, QString dbName)
//{
//    if(!book){
//        _lastError="book is null";
//        return false;
//    }
//    QAxObject *sheets=book->querySubObject("Sheets()");
//    int n=sheets->dynamicCall("Count()").toInt();
//    if(!n){
//        _lastError="book is empty";
//        return false;
//    }
//    if(dbName.isEmpty()){
//        dbName=book->dynamicCall("Name()").toString();
//        dbName=dbName.left(dbName.indexOf("."));
//    }
//    QAxObject *sheet;
//    for(int i=1;i<=n;i++){
//        sheet=sheets->querySubObject("Item(int)",i);
//        if(!toDbTable(sheet,db)){
//            return false;
//        }
//    }
//    return true;
//}

//bool QExcel::toDB(QString excelFile, const QSqlDatabase &db, QString dbName)
//{
//    qDebug()<<excelFile;
//    QAxObject* book=_WorkBooks->querySubObject("Open(QString&)",excelFile);
//    if(!book){
//        qDebug()<<"无法打开EXCEL文件："<<excelFile;
//        return false;
//    }
//    QAxObject *sheets=book->querySubObject("Sheets()");
//    int n=sheets->dynamicCall("Count()").toInt();
//    if(!n){
//        _lastError="book is empty";
//        return false;
//    }
//    if(dbName.isEmpty()){
//        dbName=book->dynamicCall("Name()").toString();
//        dbName=dbName.left(dbName.indexOf("."));
//    }
//    QAxObject *sheet;
//    for(int i=1;i<=n;i++){
//        sheet=sheets->querySubObject("Item(int)",i);
//        if(!toDbTable(sheet,db)){
//            return false;
//        }
//    }
//    return true;
//}

void QExcel::onException(int code, QString source, QString desc, QString help)
{

    Q_UNUSED(code)
    Q_UNUSED(source)
    _lastError = "Exception: " + desc + "\nHelp: " + help;
    qDebug() << _lastError;

}

QString QExcel::areaName(int row1, int row2, int col1, int col2) const
{
    return QString("%1%2:%3%4").arg(convertToColName(col1)).arg(row1).arg(convertToColName(col2)).arg(row2);
}

QString QExcel::convertToColName(int colunm) const
{
    QString res;
    Q_ASSERT(colunm>0 && colunm<65535);
    int tempData = colunm / 26;
    if(tempData > 0)
    {
        int mode = colunm % 26;
        res=convertToColName(mode);
        res+=convertToColName(tempData);
    }
    else
    {
        res=(to26AlphabetString(colunm));
    }
    return res;
}
///
/// \brief 数字转换为26字母
///
/// 1->A 26->Z
/// \param data
/// \return
///
QString QExcel::to26AlphabetString(int data)const
{
    QChar ch =QChar (data + 0x40);//A对应0x41
    return QString(ch);
}

void QExcel::castListListVariant2Variant(QVariant &var, const QList<QList<QVariant> > &res) const
{
    QVariant temp = QVariant(QVariantList());
    QVariantList record;
    int listSize = res.size();
    for (int i = 0; i < listSize;++i)
    {
        temp = res.at(i);
        record << temp;
    }
    temp = record;
    var = temp;
}

QExcel::QExcel(int processID, QObject *parent)
    :_ExcelApp(nullptr),_WorkBooks(nullptr),
    excelOpend(false)
{
    openExcel();
}

QExcel::~QExcel()
{

}
