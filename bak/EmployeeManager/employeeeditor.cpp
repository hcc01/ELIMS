#include "employeeeditor.h"
#include "ui_employeeeditor.h"
#include<QCryptographicHash>
employeeEditor::employeeEditor(QMap<int, QString> positions, TabWidgetBase *tabWiget, QWidget *parent) :
    QDialog(parent),
    SqlBaseClass(tabWiget),
    ui(new Ui::employeeEditor),
    m_positions(positions),
    m_position(0)
{
    ui->setupUi(this);
    ui->listWidget->addItems(positions.values());
}

employeeEditor::~employeeEditor()
{
    delete ui;
}

void employeeEditor::on_positionSelectBtn_clicked()
{
    int n=0;
    QString s;
    m_position=0;
    for(auto x:ui->listWidget->selectedItems()){
        n++;
        if(n>1) s.append(" 兼\n");
        s.append(QString("“%1”").arg(x->text()));
        m_position|=m_positions.key(x->text());
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
    sql="INSERT INTO sys_employee_login(name,password) VALUES(?,?);";

    values.append(name);values.append(p);
    sql+="INSERT INTO users(name,phone,EducationDegree,Title,state,position) VALUES(?,?,?,?,?,?);";

    values.append(name);
    values.append(phone);
    values.append(education);
    values.append(tytle);
    values.append(ui->comboBox->currentIndex());
    values.append(m_position);
    doSql(sql,[this](const QSqlReturnMsg&msg){
        if(msg.error()){
            QMessageBox::information(nullptr,"写入数据库错误",msg.result().toString());
            return;
        }
        this->accept();
    },0,values);

}

