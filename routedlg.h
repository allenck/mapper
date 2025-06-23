#ifndef ROUTEDLG_H
#define ROUTEDLG_H

#include <QDialog>
#include "data.h"
#include "configuration.h"
#include "sql.h"
#include"ccombobox.h"


namespace Ui {
    class RouteDlg;
}

class MainWindow;
class RouteDlg : public QDialog
{
    Q_OBJECT

public:
    RouteDlg(QWidget *parent = 0);
    ~RouteDlg();
    //void configuration(Configuration* cfg);
    void setRouteNbr(qint32 rt);
    qint32 routeNbr();
    void setSegmentId(qint32 segmentid);
    qint32 SegmentId();
    //enum TypeOfChange{Add,Delete,Update};
    void setSegmentData(RouteData value);
    void setSegmentData(SegmentData *sd);


public slots:
    void SegmentChanged(qint32 segmentId);
    void routeChanged(RouteData rd);
    void setAddMode(bool value);
    void OnNewCity();
    void fillCompanies();

signals:
    void SegmentChangedEvent(qint32 changedSegment, qint32 newSegment);
    //void routeChangedEvent(TypeOfChange typeOfChange, qint32 segmentId, qint32 tractionType);
    //void routeChangedEvent( qint32 routeNbr, QString cbRouteName, qint32 routeSegment, qint32 tractionType, qint32 companyKey, QString dateEnd, TypeOfChange typeOfChange);
    void routeChangedEvent(RouteChangedEventArgs args);
//    void setStartDate(QDate  date);
//    void setEndDate(QDate  date);

private:
    Ui::RouteDlg *ui;
    Configuration* config;
    qint32 _routeNbr = -1;
    QString _alphaRoute;
    QList<SegmentData*> _segmentDataList;
    QList<CompanyData*> _companyList;
    QMap<int,TractionTypeInfo> _tractionList;
    QList<QString> _routeNamesList;
    SQL* sql;
    qint32 _segmentId;
    bool bNewRouteNbr;
//        string error = "";
    bool bRouteChanged;
    //public event routeChangedEventHandler routeChanged;
    //public EventHandler segmentChanged;
    SegmentData* sd = nullptr;
    SegmentData* oldSd = nullptr;
    SegmentInfo si;
    RouteData _rd;
    //bool bIgnoreDirection = false;
    //public event segmentChangedEventHandler SegmentChanged;
    bool formNotLoaded;
    //int normalEnter, normalLeave, reverseEnter, reverseLeave;
    //SegmentData _sd;
    bool bSegmentChanging;
    bool bRouteChanging;
    bool bAddMode;
    //int _normalEnter, _normalLeave, _reverseEnter, _reverseLeave;
    QString strNoRoute;
    void fillTractionTypes();
    void fillSegmentsComboBox();
    void checkUpdate(QString str);
    void setDefaultTurnInfo();
    void checkTurnInfo();
    void setCompany(qint32 companyKey);
    void checkDirection(QString routeDirection);
    void displayDates(QString str);
    MainWindow* myParent;
    QDate minDate;
    QDate maxDate;
    void CalculateDates();

private slots:
    void Form_Load();
//    void txtRouteNbr_Leave();
//    void txtRouteName_Leave();
    void cbSegments_SelectedIndexChanged(int i);
    void btnClose_click();
    void gbNormalEnter_Leave();
    void gbNormalLeave_Leave();
    void gbReverseEnter_Leave();
    void gbReverseLeave_Leave();
    void btnDelete_Click();
    void btnUpdateTurn_Click();
    void btnAdd_Click();
    void rbNFromLeft_CheckedChanged(bool bChecked);
    void rbNFromBack_CheckedChanged(bool bChecked);
    void rbNFromRight_CheckedChanged(bool bChecked);
    void rbNToLeft_CheckedChanged(bool bChecked);
    void rbNAhead_CheckedChanged(bool bChecked);
    void rbNToRight_CheckedChanged(bool bChecked);
    void rbRFromLeft_CheckedChanged(bool bChecked);
    void rbRFromBack_CheckedChanged(bool bChecked);
    void rbRFromRight_CheckedChanged(bool bChecked);
    void rbRToLeft_CheckedChanged(bool bChecked);
    void rbRAhead_CheckedChanged(bool bChecked);
    void rbRToRight_CheckedChanged(bool bChecked);
    void cbCompany_SelectedIndexChanged(int i);
    void dateStart_Leave();
    void dateEnd_Leave();
    void cbOneWay_checkedChanged(bool);

};

#endif // ROUTEDLG_H
