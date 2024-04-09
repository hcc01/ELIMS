#include "sqlpagecontroleui.h"
#include"ui_pageControler.h"
#include<QDebug>
SqlPageControleUI::SqlPageControleUI(QWidget *tab):
    QWidget(tab),
    ui(new Ui::SqlPageControleUI),
    m_currentPage(1),
    m_dealFuc(nullptr),
    m_ipp(0)
{
    ui->setupUi(this);
//    this->setVisible(false);
    m_sqlClass=(nullptr);
    ui->lineEdit->setValidator(&_vali);
}

void SqlPageControleUI::setTotalPage(int totalPage)
{
    if(totalPage<0){
        qDebug()<<"SqlPageControleUI::setTotalPage error: n<0";
        return;
    }
    _totalPage=totalPage;
    _vali.setRange(1,totalPage);
    ui->totalPage->setText(QString::number(totalPage));
    if(totalPage==1){
        this->setVisible(false);
        return;
    }
    this->setVisible(true);
}

void SqlPageControleUI::setCurrentPage(int page)
{
    if(page<0||page>_totalPage) return;
    m_currentPage=page;
    ui->currentPage->setText(QString::number(page));
}

void SqlPageControleUI::startSql(TabWidgetBase* tab, const QString &sql, int p, QJsonArray v, DealFuc f, int ipp)
{
    m_sqlClass=tab;
    m_sql=sql;m_currentPage=p;m_values=v;m_dealFuc=f;
    qDebug()<<"数据库查询页面控件收到sql:"<<sql;
    m_ipp=ipp;
    tab->doSqlQuery(sql,[this](const QSqlReturnMsg&msg){
            if(msg.error()){
                QMessageBox::information(nullptr,"error",msg.errorMsg());
                m_sqlClass->sqlFinished();
                return;
            }
            int pages=msg.totalPage();
            setTotalPage(pages);
//            qDebug()<<msg.jsCmd();
            m_dealFuc(msg);
            m_sqlClass->sqlFinished();
        },p,v,ipp);
    tab->waitForSql();
}

void SqlPageControleUI::updatePage()
{
    ui->currentPage->setText(QString::number(m_currentPage));
    ui->totalPage->setText(QString::number(_totalPage));
}

void SqlPageControleUI::dealSqlReturn(const QSqlReturnMsg&msg)
{
                if(msg.error()){
                    QMessageBox::information(nullptr,"error",msg.errorMsg());
                    return;
                }
                int pages=msg.totalPage();
                setTotalPage(pages);
                setCurrentPage(m_currentPage);
                m_dealFuc(msg);

}

void SqlPageControleUI::on_btNext_clicked()
{
    qDebug()<<"on_btNext_clicked";
    if(!m_sqlClass&&m_dealFuc){
        qDebug()<<"error:未设定模块。";
        return;
    }
    if(m_sql.isEmpty()){
        qDebug()<<"errror:未初始化语句。";
        return;
    }
    if(!_totalPage) return;
    if(m_currentPage+1 >_totalPage) return;
    m_currentPage++;

    if(!m_dealFuc){
        emit pageChanged(m_sql,m_currentPage);
    }
    else{
        m_sqlClass->doSqlQuery(m_sql,[this](const QSqlReturnMsg&msg){
                dealSqlReturn(msg);
                },m_currentPage,m_values,m_ipp);
    }
}

void SqlPageControleUI::on_btPre_clicked()
{
    if(!m_sqlClass&&m_dealFuc){
        qDebug()<<"error:未设定模块。";
        return;
    }
    if(m_sql.isEmpty()){
        qDebug()<<"errror:未初始化语句。";
        return;
    }
    if(!_totalPage) return;
    if(m_currentPage<=1) return;
    m_currentPage--;
    if(!m_dealFuc) {
        emit pageChanged(m_sql,m_currentPage);
    }
    else{
        m_sqlClass->doSqlQuery(m_sql,[this](const QSqlReturnMsg&msg){
                dealSqlReturn(msg);
            },m_currentPage,m_values,m_ipp);
    }
}

void SqlPageControleUI::on_btGo_clicked()
{
    if(!m_sqlClass&&m_dealFuc){
        qDebug()<<"error:未设定模块。";
        return;
    }
    if(m_sql.isEmpty()){
        qDebug()<<"errror:未初始化语句。";
        return;
    }
    if(ui->lineEdit->text().isEmpty()) return;
    int p=ui->lineEdit->text().toUInt();
    if(p>_totalPage) return;
    m_currentPage=p;
    if(!m_dealFuc) {
        emit pageChanged(m_sql,m_currentPage);
    }
    else{
        m_sqlClass->doSqlQuery(m_sql,[this](const QSqlReturnMsg&msg){
                dealSqlReturn(msg);
            },m_currentPage,m_values,m_ipp);

    }
}
