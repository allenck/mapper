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

class mainWindow;
class RouteDlg : public QDialog
{
    Q_OBJECT

public:
    RouteDlg(Configuration *cfg, QWidget *parent = 0);
    ~RouteDlg();
    //void configuration(Configuration* cfg);
    void setRouteNbr(qint32 rt);
    qint32 routeNbr();
    void setSegmentId(qint32 segmentid);
    qint32 SegmentId();
    //enum TypeOfChange{Add,Delete,Update};
    void setRouteData(RouteData value);
    void fillCompanies();


public slots:
    void SegmentChanged(qint32 segmentId);
    void routeChanged(RouteData rd);
    void setAddMode(bool value);
    void OnNewCity();

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
    qint32 _routeNbr;
    QString _alphaRoute;
    QList<RouteData> _segmentInfoList;
    QList<CompanyData> _companyList;
    QList<tractionTypeInfo> _tractionList;
    QList<QString> _routeNamesList;
    SQL* sql;
    qint32 _SegmentId;
    bool bNewRouteNbr;
//        string error = "";
    bool bRouteChanged;
    //public event routeChangedEventHandler routeChanged;
    //public EventHandler segmentChanged;
    SegmentInfo si;
    //bool bIgnoreDirection = false;
    //public event segmentChangedEventHandler SegmentChanged;
    bool formNotLoaded;
    int normalEnter, normalLeave, reverseEnter, reverseLeave;
    RouteData _rd;
    bool bSegmentChanging;
    bool bRouteChanging;
    bool bAddMode;
    int _normalEnter, _normalLeave, _reverseEnter, _reverseLeave;
    QString strNoRoute;
    void fillTractionTypes();
    void fillComboBox();
    void checkUpdate(QString str);
    void setDefaultTurnInfo();
    void checkTurnInfo();
    void setCompany(qint32 companyKey);
    void checkDirection(QString routeDirection);
    void displayDates(QString str);
    mainWindow* myParent;
    QDate minDate;
    QDate maxDate;
    void CalculateDates();

private slots:
    void Form_Load();
    void txtRouteNbr_Leave();
    void txtRouteName_Leave();
    void cbSegments_SelectedIndexChanged(int i);
    void btnOK_click();
    void groupBox2_Leave();
    void groupBox3_Leave();
    void groupBox4_Leave();
    void groupBox5_Leave();
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

};

#endif // ROUTEDLG_H
