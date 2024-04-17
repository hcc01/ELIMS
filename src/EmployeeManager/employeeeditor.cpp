#include "employeeeditor.h"
#include "ui_employeeeditor.h"
#include<QCryptographicHash>
#include"../Client/cuser.h"
employeeEditor::employeeEditor(TabWidgetBase *tabWiget, bool addMode, QWidget *parent) :
    QDialog(parent),
    SqlBaseClass(tabWiget),
    ui(new Ui::employeeEditor),
    m_addMode(addMode),
    m_position(0)
{
    ui->setupUi(this);
    ui->listWidget->addItems(CUser::allPositions().values());
    if(!m_addMode){
        ui->nameEdit->setEnabled(false);
    }
}

employeeEditor::~employeeEditor()
{
    delete ui;
}

void employeeEditor::load(const QString&name, const QString&phone, const QString&EducationDegree, const QString &Title, const QString &posName, int position)
{
    ui->nameEdit->setText(name);
    ui->phoneEdit->setText(phone);
    ui->EducationEdit->setText(EducationDegree);
    ui->tytleEdit->setText(Title);
    ui->textBrowser->setText(posName);
    m_position=position;
}

void employeeEditor::on_positionSelectBtn_clicked()
{
    int n=0;
    QString s;
    m_position=0;
    auto positions=CUser::allPositions();
    for(auto x:ui->listWidget->selectedItems()){
        n++;
        if(n>1) s.append(" 兼\n");
        s.append(QString("“%1”").arg(x->text()));
        m_position|=positions.key(x->text());
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText(s);
}


void employeeEditor::on_finishBtn_clicked()
{
    QString name=ui->nameEdit->text();
    QString phone=ui->phoneEdit->text();
    QString education=ui->EducationEdit->text();
    QString tytle=ui->tytleEdit->text();
    if(name.isEmpty()||ui->textBrowser->toPlainText().isEmpty()){
        QMessageBox::information(nullptr,"error","姓名和岗位不能为空。");
        return;
    }
    QString sql;
    QString p;
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData("123");
    p=QString(hash.result().toHex());
    QJsonArray values;
    if(m_addMode){
        sql="INSERT INTO sys_employee_login(name,password) VALUES(?,?);";

        values.append(name);values.append(p);
    }
    else sql="";
    if(m_addMode){
        sql+="INSERT INTO users(name,phone,EducationDegree,Title,state,position) VALUES(?,?,?,?,?,?);";

        values.append(name);
        values.append(phone);
        values.append(education);
        values.append(tytle);
        values.append(ui->comboBox->currentIndex());
        values.append(m_position);
    }
    else{
        sql+="update users set phone=?,EducationDegree=?,Title=?,state=?,position=?,State=? where name=?";

        values.append(phone);
        values.append(education);
        values.append(tytle);
        values.append(ui->comboBox->currentIndex());
        values.append(m_position);
        values.append(ui->comboBox->currentIndex());
        values.append(name);
    }
    bool error=false;
    doSql(sql,[this, &error](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"写入数据库错误",msg.result().toString());
            error=true;
            sqlFinished();
            return;
        }
        sqlFinished();
    },0,values);
    waitForSql();
    if(error) return;
    accept();


}

