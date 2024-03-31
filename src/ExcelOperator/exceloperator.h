#ifndef EXCELOPERATOR_H
#define EXCELOPERATOR_H
//#include "xlsxglobal.h"
#include "ExcelOperator_global.h"
#include"xlsxdocument.h"
#include"../Client/QExcel.h"

QXLSX_USE_NAMESPACE
class EXCELOPERATOR_EXPORT ExcelOperator
{
public:
    ExcelOperator();
    bool openExcel(const QString&file);
    bool newExcel(const QString&file);
    bool save();
    bool saveAs(const QString&file);
    void close();
    void show();
    Document* document()const{return m_xlsx;}
    CellRange usedRange()const;
    CellRange find(const QVariant &what);
    void copyAll(int fromRowStart,int fromColumnStart,int fromRowEnd,int fromColumnEnd,int ToRow,int ToColumn);
    QVariant cellValue(int row,int col)const{return m_xlsx->read(row,col);}
    QVariant cellValue(const QString&range)const{return m_xlsx->read(range);}
    QString LastError()const{return m_error;}
    void setValue(const CellRange&range,const QVariant&value,const Format&format=Format());
    void setValue(const QVariant&value,int row,int col){m_xlsx->write(row,col,value);}
    double rowHeight(int row)const{return m_xlsx->rowHeight(row);}
    double columnWidth(int col)const{return m_xlsx->columnWidth(col);}

private:
    Document* m_xlsx;
    QString m_fileName;
    QString m_error;
};

#endif // EXCELOPERATOR_H
