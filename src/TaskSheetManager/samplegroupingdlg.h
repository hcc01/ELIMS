#ifndef SAMPLEGROUPINGDLG_H
#define SAMPLEGROUPINGDLG_H

#include "qlistwidget.h"
#include <QDialog>
#include<tabwigetbase.h>
namespace Ui {
class SampleGroupingDlg;
}

class SampleGroupingDlg : public QDialog,SqlBaseClass
{
    Q_OBJECT

public:
    explicit SampleGroupingDlg(TabWidgetBase *parent);
    ~SampleGroupingDlg();
    void init(QString taskNum);
private slots:

    void on_typeView_itemClicked(QListWidgetItem *item);

    void on_printBtn_clicked();

    void on_saveBtn_clicked();

    void on_typeSelectBox_currentIndexChanged(int index);

    void on_siteSelectBox_currentIndexChanged(int index);

    void on_printOkbtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::SampleGroupingDlg *ui;
    QString m_taskNum;
    int m_taskSheetID;
    QHash<QString,int>m_itemIdMap;
    QHash<QString,int>m_typeIdMap;
    QHash<QString,QList<QString>>m_typeItemsMap;
    QHash<QString,QMap<int,QStringList>>m_typeItemGroupMap;
    QHash<QString,QMap<int,QString>>m_sampling;
    QMap<int,int>m_periodMap;
    QHash<int,int>m_frequencyMap;
    QList<int>m_selectSiteIDs;
    QMap<int,int>m_siteIDTypeMap;
    QMap<int,int>m_siteAreaMap;
};

#endif // SAMPLEGROUPINGDLG_H
