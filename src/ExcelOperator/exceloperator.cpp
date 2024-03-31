#include "exceloperator.h"
#include<QFile>

ExcelOperator::ExcelOperator():
    m_xlsx(nullptr)
{
}

bool ExcelOperator::openExcel(const QString &fileName)
{
//    QFile file(fileName);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "Error opening file:" << file.errorString();
//        return false;
//    }
//    m_fileName=fileName;
    m_xlsx=new Document(fileName);
    if (!m_xlsx->load()){
        qDebug() << "Error opening file:!m_xlsx->load()";
        m_error="无法加载文件。";
        delete m_xlsx;
        m_xlsx=nullptr;
        return false;
    }
    return true;
}

bool ExcelOperator::newExcel(const QString &fileName)
{
    m_xlsx=new Document;
    if (!m_xlsx->saveAs(fileName)) {
        qDebug() << "[ReadExcel] failed to save excel file.";
        return false;
    }
    m_fileName=fileName;
    return true;
}

bool ExcelOperator::save()
{
    return m_xlsx->save();
}

bool ExcelOperator::saveAs(const QString &file)
{
//    if (QFile::exists(file)) {
//        // 如果文件存在，则删除它
//        QFile::remove(file);
//        qDebug() << "Existing file removed successfully.";
//    } else {
//        qDebug() << "File does not exist.";
//    }
    m_fileName=file;
    return m_xlsx->saveAs(file);
}

void ExcelOperator::close()
{
    delete m_xlsx;
    m_xlsx=nullptr;
}

void ExcelOperator::show()
{
    EXCEL.Open(m_fileName);
}

CellRange ExcelOperator::usedRange() const
{
    QXlsx::Worksheet *sheet = m_xlsx->currentWorksheet();
    return sheet->dimension();
}

CellRange ExcelOperator::find(const QVariant &what)
{
    QXlsx::Worksheet *sheet = m_xlsx->currentWorksheet();

    QXlsx::CellRange usedRange = sheet->dimension();

    // 获取范围的起始行和列
    int startRow = usedRange.firstRow();
    int startColumn = usedRange.firstColumn();

    // 获取范围的结束行和列
    int endRow = usedRange.lastRow();
    int endColumn = usedRange.lastColumn();
    for(int row=startRow;row<=endRow;row++){
        for(int col=startColumn;col<=endColumn;col++){
            if(m_xlsx->read(row,col)==what){
                if(m_xlsx->currentWorksheet()->isMergeCell(row,col))
                    return m_xlsx->currentWorksheet()->mergeRange(row,col);
                return CellRange(row,col,row,col);
            }
        }
    }
    return CellRange();
}

void ExcelOperator::copyAll(int fromRowStart, int fromColumnStart, int fromRowEnd, int fromColumnEnd, int ToRow, int ToColumn)
{
    QXlsx::Worksheet *sheet = m_xlsx->currentWorksheet();
    for(int row=0;row<=(fromRowEnd-fromColumnStart);row++){
        for(int column=0;column<=(fromColumnEnd-fromColumnStart);column++){
            int fromR=fromRowStart+row;
            int fromC=fromColumnStart+column;
            int nowR=ToRow+row;
            int nowC=ToColumn+column;
//            m_xlsx->setColumnFormat(nowC,nowC,m_xlsx->columnFormat(fromC));
            //先看是不是合并单元格
            QXlsx::Format format;
            Cell* cell=m_xlsx->cellAt(fromR,fromC);
            if(!cell){
                qDebug()<<"cell is null"<<fromR<<fromC;
            }
            else format=cell->format();
            if(sheet->isMergeCell(fromR,fromC)){
                if(sheet->isFirstMergeCell(fromR,fromC)) {
                    CellRange range=sheet->mergeRange(fromR,fromC);

                    int columns=range.lastColumn()-range.firstColumn();
                    int rows=range.lastRow()-range.firstRow();
                    sheet->mergeCells(CellRange(nowR,nowC,nowR+rows,nowC+columns),format);
                }

            }
            m_xlsx->write(nowR,nowC,m_xlsx->read(fromR,fromC),format);
            m_xlsx->setRowHeight(nowR,m_xlsx->rowHeight(fromR));
            m_xlsx->setColumnWidth(nowC,m_xlsx->columnWidth(fromC));
        }
    }
}

void ExcelOperator::setValue(const CellRange &range, const QVariant &value, const Format &format)
{
    for(int row=range.firstRow();row<=range.lastRow();row++){
        for(int col=range.firstColumn();col<=range.lastColumn();col++){
            m_xlsx->write(row,col,value,format);
        }
    }
}
