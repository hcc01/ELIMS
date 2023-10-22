#include "sqlpagecontroleui.h"
#include"ui_pageControler.h"
#include<QDebug>
SqlPageControleUI::SqlPageControleUI(QWidget *parent):
    QWidget(parent),
    ui(new Ui::SqlPageControleUI),
    m_currentPage(1)
{
    ui->setupUi(this);
    this->setVisible(false);

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


void SqlPageControleUI::on_btNext_clicked()
{
    if(!_totalPage) return;
    if(m_currentPage+1 >_totalPage) return;
    m_currentPage++;
    emit pageChanged(m_sql,m_currentPage);
}

void SqlPageControleUI::on_btPre_clicked()
{
    if(!_totalPage) return;
    if(m_currentPage<=1) return;
    m_currentPage--;
    emit pageChanged(m_sql,m_currentPage);
}

void SqlPageControleUI::on_btGo_clicked()
{
    if(ui->lineEdit->text().isEmpty()) return;
    int p=ui->lineEdit->text().toUInt();
    if(p>_totalPage) return;
    m_currentPage=p;
    emit pageChanged(m_sql,m_currentPage);
}
