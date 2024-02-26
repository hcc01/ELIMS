
#ifndef QEXCEL_H
#define QEXCEL_H
#include<QDebug>
#include<QAxObject>
#include<QVariant>
#define EXCEL QExcel::instance()
#include<QtSql/QSqlDatabase>
class Range;
class WorkSheet;
class WorkBook;
class  QExcel:public QObject
{

public:
    static QExcel& instance(int processID=0){
        static QExcel excel(processID);
        return  excel;
    }
    static QString columnIndexToName(int columnIndex);
    void openExcel();
    inline bool isOpen(const QString &fileName="")const;
    void Quit();//在退出程序之前调用，结束EXCEL
    QString LastError(){
        return _lastError;
    }
    //屏幕刷新
    void setScreenUpdating(bool update);
    void setEnableEvents(bool);
    void SetVisible(bool v);
    void DisplayAlerts(bool v);
    //app operate fuc
    QAxObject *newBook();
    QAxObject *Open(const QString& path, const QVariant &UpdateLinks=QVariant(), bool readOnly=false, const QString &password="");
    WorkBook *OpenBook(const QString& path, const QVariant &UpdateLinks=QVariant(), bool readOnly=false, const QString &password="");
    void Close();
    bool CloseBook(QAxObject *book);
    void showExcel(bool show) const;//为防止意外错误，show完建议删除book，不再操作。

    //book operate fuc
    QAxObject * getBook(const QString& bookName) const;

    //sheet operate fuc
    int sheetCount(QAxObject *book) const;
    int rowCount(QAxObject* sheet) const;
    int columnCount(QAxObject* sheet) const;
    int pages(QAxObject* sheet) const;
    QString sheetName(QAxObject* sheet) const;
    QAxObject * ActiveSheet(QAxObject *book=nullptr) const;
    QAxObject *selectSheet(const QString& sheetName, QAxObject *book)const;
    QAxObject * selectSheet(int i, QAxObject * book)const;
    QAxObject *selectRow(int row, QAxObject * sheet)const;
    QAxObject *usedRange(QAxObject * sheet)const;
    //range operate fuc
    QVariant cellValue(int row, int column, QAxObject *sheet) const;
    void writeCell(int row,int column,QAxObject *sheet,const QVariant& var) const;
    void writeRange(const QVariant&,const QString& rangeAdds="",QAxObject *sheet=nullptr)const;
    void writeRow(int row, QAxObject *sheet, const QList<QVariant> &var) const;
    QAxObject *getSelectRange(QAxObject* sheet=nullptr)const;
    void writeCurrent(const QVariant& var) const;//在选定的单元格中写入数据
    QVariant readAll(QAxObject *sheet) const;
    QVariant readAera(QAxObject *sheet,int startRow,int startColumn,int endRow,int endColumn) const;
    void setCellValue(int row, int column, const QVariant& value, QAxObject *sheet) const;
    void writeArea(const QList<QVariantList> &data, QAxObject *sheet, int startColumn=1, int startRow=1);
    void writeArea(const QVariant &data, QAxObject *sheet, int startColumn=1, int startRow=1);
    QAxObject *find(QAxObject *ranges,const QString&str, const QVariant&After=QVariant(), const QVariant&LookIn=QVariant(), const QVariant&LookAt=QVariant(), const QVariant&SearchOrder=QVariant(), const QVariant&SearchDirection=QVariant(), const QVariant&MatchCase=QVariant())const;
    bool replace(QAxObject *ranges,const QString&What, const QString&Replacement,const QVariant&LookAt=QVariant(), const QVariant&SearchOrder=QVariant(), const QVariant&SearchDirection=QVariant(), const QVariant&MatchCase=QVariant())const;
    void insertPic(QAxObject*sheet, const QString &picPath, int column, int row, int left=0, int top=0,int width=80,int height=40);
    void insertPic(QAxObject*sheet, QAxObject*range, const QString &picPath, int left=0, int top=0, int width=80, int height=40);

public:
    //以下进行数据库相关操作
    //将表格转到数据库，抬头需要注明数据类型，以|分割;第一列不能为空,example: name|varchar(16)。
//    bool toDbTable(QAxObject *sheet, const QSqlDatabase& db, QString tableName="",  int startColumn=0, int endColumn=0, bool isNewTable=true);//将当前一个表格复制到指定数据库
//    bool toDB(QAxObject*book,const QSqlDatabase& db, QString dbName="");//将当前全部表格复制到指定数据库
//    bool toDB(QString excelFile,const QSqlDatabase& db, QString dbName="");//将当前全部表格复制到指定数据库
public slots:
    void onException(int code,  QString source,   QString desc,   QString help);

private:
    inline  QString areaName(int row1,int row2,int col1,int col2) const;
    inline QString convertToColName(int colunm) const;
    inline QString to26AlphabetString(int data) const;
    inline void castListListVariant2Variant(QVariant &var, const QList<QList<QVariant> > &res)const;
private:
    QExcel(int processID=0, QObject* parent=nullptr);
    ~QExcel();
    QAxObject *_ExcelApp;
    QAxObject *_WorkBooks;
    QString _lastError;

    bool excelOpend;
    //    QAxObject *_WorkBook;
    //    QAxObject *_WorkSheets;
    //    QAxObject *_WorkSheet;
    //    bool _isNewFile;

};

class Range:public QObject
{

public:
    enum HorizontalAlignment {
        AlignLeft,
        AlignCenter,
        AlignRight
    };

    enum VerticalAlignment {
        AlignTop,
        AlignMiddle,
        AlignBottom
    };
    Range(QAxObject*range,QObject* parent=nullptr);
    virtual ~Range();
    int row()const{return m_range->dynamicCall("row()").toInt();}
    int column()const{return m_range->dynamicCall("column()").toInt();}
    QVariant value() const { return m_range->property("Value"); }
    void setValue(const QVariant& value) { m_range->setProperty("Value", value); }
    void setFont(const QFont& font);
    void setFillColor(const QColor& color);
    void setHorizontalAlignment(HorizontalAlignment alignment);
    void setVerticalAlignment(VerticalAlignment alignment);
    void setWrapText(bool wrap);
    void setMergeCells(bool merge);
    void setBorder(int lineStyle, int weight, int colorIndex);
    void copy();
    void paste(const QVariant&PasteType=QVariant());
    void clear();
    void insert();
    void Delete();
    double width()const{if(m_range) return m_range->property("Width").toDouble();return 0;};
    double Height()const{if(m_range) return m_range->property("Height").toDouble();return 0;};;

    Range* find(const QString& searchText, bool matchCase = false, bool matchWholeWord = false);
    bool replace(const QString&What, const QString&Replacement,const QVariant&LookAt=QVariant(), const QVariant&SearchOrder=QVariant(), const QVariant&SearchDirection=QVariant(), const QVariant&MatchCase=QVariant());
    Range*entireRow()
    {
        QAxObject *entireRowRange = m_range->querySubObject("entireRow()");
        return new Range(entireRowRange,this->parent());
    }
private:
    QAxObject* m_range;
};
class WorkSheet:public QObject
{
public:
    WorkSheet(QAxObject*s,QObject* parent=nullptr);
    Range* usedRange();
    Range* range(const QString& rangeStr);
    Range *selectRange(int startRow,int startColumn,int endRow,int endColumn);
    Range *selectRow(int row);
    void setValue(const QVariant& value, int startRow, int startColumn, int endRow=0, int endColumn=0);
    void insertRow(int row);
    void deleteRow(int row);
    void insertColumn(int column);
    void deleteColumn(int column);
    void insertPic(const QString&path,int row,int column);
private:
    QAxObject* m_sheet;
};

class WorkBook:public QObject
{
public:
    WorkBook(QAxObject* book,QObject*parent=nullptr);
    virtual ~WorkBook();
    bool open(const QString& path, const QVariant &UpdateLinks=QVariant(), bool readOnly=false, const QString &password="");
    WorkSheet *sheet(int i);
    bool save();
    void saveAs(const QString& path);
    void close();
    WorkSheet* addSheet(const QString& name);
    void deleteSheet(int index);
    int sheetCount();
private:
    QAxObject* m_book;
};

#endif // QEXCEL_H
