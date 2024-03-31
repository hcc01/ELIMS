#ifndef LOGINUI_H
#define LOGINUI_H

#include <QDialog>

namespace Ui {
class LoginUI;
}

class LoginUI : public QDialog
{
    Q_OBJECT

public:
    explicit LoginUI(QString company="",QWidget *parent = nullptr);
    ~LoginUI();
    void onLoginResult(int result);
    void reject()override;
signals:
    void login(const QString&,const QString&);
private slots:
    void on_btLogin_clicked();

    void on_btExit_clicked();

    void on_comboBox_ID_currentIndexChanged(int index);

    void on_comboBox_ID_currentTextChanged(const QString &arg1);

    void on_comboBox_ID_editTextChanged(const QString &arg1);

private:
    Ui::LoginUI *ui;
};

#endif // LOGINUI_H
