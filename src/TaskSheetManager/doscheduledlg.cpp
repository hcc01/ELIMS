#include "doscheduledlg.h"
#include "ui_doscheduledlg.h"
#include<QCalendarWidget>
#include<QMessageBox>
#include<QDebug>
#include"itemsselectdlg.h"
DoScheduleDlg::DoScheduleDlg(QStringList allSamplers, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoScheduleDlg),
    m_allSamplers(allSamplers),
    m_endDateEdited(false)
{
    ui->setupUi(this);
    ui->dateEdit->setCalendarPopup(true);
    ui->dateEdit2->setCalendarPopup(true);
    ui->dateEdit->setDate(QDate::currentDate().addDays(1));
    QTextCharFormat format = ui->dateEdit->calendarWidget()->weekdayTextFormat(Qt::Saturday);
    format.setForeground(QBrush(QColor::fromRgb(150,0,0), Qt::SolidPattern));
    ui->dateEdit->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateEdit->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);
    ui->comboBox->addItems(m_allSamplers);
    connect(ui->dateEdit,&QDateEdit::editingFinished,[this](){
        if(!m_endDateEdited) ui->dateEdit2->setDate(ui->dateEdit->date());
    });
    connect(ui->dateEdit2,&QDateEdit::editingFinished,[this](){
        m_endDateEdited=true;
    });


}

DoScheduleDlg::~DoScheduleDlg()
{
    delete ui;
}

void DoScheduleDlg::on_samplerEdit_customContextMenuRequested(const QPoint &pos)
{
    QMessageBox::information(nullptr,"","hello!");
}


void DoScheduleDlg::on_selectBtn_clicked()
{
    QStringList samplers=itemsSelectDlg::getSelectedItems(m_allSamplers);
    ui->samplerEdit->setText(samplers.join("、"));
}


void DoScheduleDlg::on_btnOk_clicked()
{
    QDate startDate=ui->dateEdit->date();
    QDate endDate=ui->dateEdit2->date();
    if(startDate>endDate){
        QMessageBox::information(nullptr,"error","起始时间错误！");
        return;
    }
    QString leader=ui->comboBox->currentText();
    QString samplers=ui->samplerEdit->toPlainText();
    if(samplers.isEmpty()){
        QMessageBox::information(nullptr,"error","请选择采样人员。");
        return;
    }
    QString remark=ui->remarkEdit->toPlainText();
    emit doSchedule(startDate.toString("yyyy-MM-dd"),endDate.toString("yyyy-MM-dd"),leader,samplers,remark);
    accept();
}


void DoScheduleDlg::on_btnCancel_clicked()
{
     reject();
}

