#include "loginui.h"
#include "ui_loginui.h"
#include"QMessageBox"
#include"QCryptographicHash"
#include"../../depends/MessageHeader.h"
LoginUI::LoginUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginUI)
{
    ui->setupUi(this);
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
}

void LoginUI::on_btExit_clicked()
{
    exit(0);
}
