#include "loginui.h"
#include "ui_loginui.h"
#include"QMessageBox"
#include"QCryptographicHash"
#include"../../depends/MessageHeader.h"
#include<QFile>
#include<QDebug>
LoginUI::LoginUI(QString company, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginUI)
{
    ui->setupUi(this);
//    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setWindowTitle("请登录:");
    ui->comLabel->setText(company);
    QFile file("./users");
    if(!file.open(QFile::ReadOnly)) return;
    QString user=file.readAll();
    ui->comboBox_ID->addItems(user.split("\n"));
    file.close();
}

LoginUI::~LoginUI()
{
    delete ui;
}

void LoginUI::onLoginResult(int result)
{
    switch (result) {
    case LOGIN_SUCCESSED:
    {
        accept();
    }
        break;
    case LOGIN_FAIL:
    {
        QMessageBox::warning(this, "error","错误的用户名或密码！");
    }
        break;
    case DB_ERROR:
        QMessageBox::warning(this, "error","数据库错误！");
        break;
    }
}

void LoginUI::reject()
{
    exit(0);
}

void LoginUI::on_btLogin_clicked()
{
    if(ui->comboBox_ID->currentText().isEmpty()||ui->lineEdit_Password->text().isEmpty()){
        QMessageBox::warning(this,"","请输入工号和密码。");
        return;
    }
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(ui->lineEdit_Password->text().toLocal8Bit());
    emit login(ui->comboBox_ID->currentText(),QString(hash.result().toHex()));
    QFile file("./users");
    if(!file.open(QFile::ReadWrite)) return;
    QStringList users;
    if(ui->comboBox_ID->currentText()!=ui->comboBox_ID->itemText(ui->comboBox_ID->currentIndex())){
        ui->comboBox_ID->addItem(ui->comboBox_ID->currentText());
    }
    for(int i=0;i<ui->comboBox_ID->count();i++){
        users.append(ui->comboBox_ID->itemText(i));
    }
    file.write(users.join("\n").toUtf8());
    file.close();
}

void LoginUI::on_btExit_clicked()
{
    exit(0);
}

void LoginUI::on_comboBox_ID_currentIndexChanged(int index)
{

}


void LoginUI::on_comboBox_ID_currentTextChanged(const QString &arg1)
{
//    qDebug()<<ui->comboBox_ID->currentText();
}


void LoginUI::on_comboBox_ID_editTextChanged(const QString &arg1)
{
//    qDebug()<<ui->comboBox_ID->currentText();
}

