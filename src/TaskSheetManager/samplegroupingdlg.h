#ifndef SAMPLEGROUPINGDLG_H
#define SAMPLEGROUPINGDLG_H

#include "qlistwidget.h"
#include <QDialog>
#include<tabwigetbase.h>
#define SAMPLING_COLOR QColor(50,200,100)
namespace Ui {
class SampleGroupingDlg;
}

class SampleGroupingDlg : public QDialog,SqlBaseClass
{
    Q_OBJECT

public:
    explicit SampleGroupingDlg(TabWidgetBase *parent);
    ~SampleGroupingDlg();
    void init(int taskSheetID, const QStringList&samplers);
    void showGroup()const;
    QStringList parameterNames(QList<int>parameterIDs)const;
    //处理样品编号，样品编号由日期、任务单序号、采样类型、点位号、（样品组号）、（串联号）、（频次号）组成
    QString dateNum(const QDate&date);//日期编号
    QString clientNum(const QDate&date, int taskSheetID);//采样单顺序信号
    void initNum(const QDate&dateStart,int days);
    QString testTypeNum(int testTypeID, int start=0);//采样类型
    QString testTypeNum(QString testType, int start=0, bool resetNum=false);//采样类型+点位号
private slots:

//    void on_typeView_itemClicked(QListWidgetItem *item);

    void on_printBtn_clicked();

    void on_saveBtn_clicked();

    void on_printOkbtn_clicked();

    void on_cancelBtn_clicked();

    void on_addSamplerBtn_clicked();

    void on_importLabelBtn_clicked();

    void on_radioGenerate_clicked();

    void on_radioPrint_clicked();

    void on_typeView_currentRowChanged(int currentRow);

private:
    Ui::SampleGroupingDlg *ui;
    QString m_taskNum;
    int m_taskSheetID;
    QString m_clientID;//保存样品编号的企业代码
    QHash<QString,int>m_itemIdMap;//保存检测参数的ID，用于操作数据库【参数名称，参数ID】
//    QHash<QString,int>m_typeIdMap;//保存检测类型的ID，用于操作数据库【检测类型，检测ID】
    QHash<QString,QList<QString>>m_typeItemsMap;//每个类型的测试项目，分组只和类型有关，和点位无关【检测类型，{检测项目}】
//    QHash<QString,QMap<int,QStringList>>m_typeItemGroupMap;//保存每个类型的分组情况【检测类型，【分组序号，{检测参数}】】//使用m_groups替代
    QHash<int,int>m_seriesConnection;//样品序号的记录串联数
    QMap<int,QPair<int,int>>m_periodMap;//点位的周期<总周期，剩余周期>
    QHash<int,int>m_frequencyMap;//点位的频次
    QList<int>m_selectSiteIDs;
    QMap<int,int>m_siteAreaMap;
    QHash<int,int>m_siteIDTypeID;//保存点位的类型ID【点位ID，类型ID】
    QMap<int,QPair<QString,QString>>m_allSides;//保存需要采样的点位ID和[名称:样品类型]，用于标签
    QMap<int,QPair<QString,QString>>m_SidesInfo;//各点位ID和[名称:样品类型]，用于打印标签时显示
    QList<int>m_samplingSideID;//拟采样的点位ID
    QStringList m_samplers;//安排的采样人员
    QHash<int ,int >m_typeUsedOrder;//记录每个类型的已经用完的点位序号，当有新点位需要编号时继续往下编。
    QMap<int,int>m_siteUsedOrderMap;//记录点位序号，如果已经使用。


    QMap<int,QMap<int,QList<int>>>m_groups;//分组情况<类型ID：<样品序号：参数列表>>
    QMap<int,int>m_typeOrder;//<类型ID：序号>记录各类型已经使用的序号
    bool m_groupChanged;
    QStringList m_clientNum;//保存全周期的采样客户序号编码
};

#endif // SAMPLEGROUPINGDLG_H
