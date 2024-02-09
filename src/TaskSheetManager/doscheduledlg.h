#ifndef DOSCHEDULEDLG_H
#define DOSCHEDULEDLG_H

#include <QDialog>

namespace Ui {
class DoScheduleDlg;
}

class DoScheduleDlg : public QDialog
{
    Q_OBJECT

public:
    explicit DoScheduleDlg(QStringList allSamplers,QWidget *parent = nullptr);
    ~DoScheduleDlg();
signals:
    void doSchedule(QString startDate, QString endDate, QString leader, QString samplers, QString remark);
private slots:
    void on_samplerEdit_customContextMenuRequested(const QPoint &pos);

    void on_selectBtn_clicked();

    void on_btnOk_clicked();

    void on_btnCancel_clicked();

private:
    Ui::DoScheduleDlg *ui;
    QStringList m_allSamplers;
    bool m_endDateEdited;
};

#endif // DOSCHEDULEDLG_H
