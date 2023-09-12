#ifndef SEGMENTDLG_H
#define SEGMENTDLG_H

#include <QDialog>
#include "data.h"
#include "sql.h"
#include "configuration.h"
#include "ccombobox.h"

namespace Ui {
    class SegmentDlg;
}

class SegmentDlg : public QDialog
{
    Q_OBJECT

public:
    SegmentDlg(Configuration *cfg, QWidget *parent = 0);
    ~SegmentDlg();
    QT_DEPRECATED void setConfiguration(Configuration * value);
    void setPt(int value);
    void setSegmentId(qint32 value);
    qint32 SegmentId();
    qint32 newSegmentId();
    QString street();
    QString routeName();
    qint32 tractionType();
    void setRouteData(RouteData value);
    bool oneWay();
    int tracks();

private:
    Ui::SegmentDlg *ui;
    QWidget *myParent;
    qint32 _pt;
    qint32 _SegmentId;
    qint32 _routeNbr;
    QString _alphaRoute;
    SQL* sql;
    qint32 _newSegmentId;
    QString streetName;
    QString _routeName;
    bool bOriginalChanged;
    bool bNewChanged;
    bool bRouteChanged;
    QList<CompanyData*> _companyList;
    QMap<int, TractionTypeInfo> _tractionTypeList;
    QList<QString> _routeNameList;
    QList<QString> _routeTypeList;
    SegmentInfo sd;
    LatLng pi;
    Bearing bearing;
    Configuration *config;
    RouteData _rd;
    qint32 normalEnter, normalLeave, reverseEnter, reverseLeave;
    QString strNoRoute;
    bool bNewRouteNbr;
    bool bSplitting;
    QMap<int, TractionTypeInfo> tractionTypeList;
    void fillCompanies();
    void fillTractionTypes();
    void checkUpdate();
    QString getColor(qint32 tractionType);
    QStringList _locations;

private slots:
    void rbUseOriginal_CheckedChanged();
    void txtRouteNbr_Leave();
    void rbUseNew_CheckedChanged();
    void txtRouteName_TextChanged(QString text);
    void rbNoAdd_CheckedChanged();
    void cbRouteName_Leave();
    void txtNewName_TextChanged(QString text);
    void txtNewName_Leave();
    void txtOriginalName_Leave();
    void txtOriginalName_TextChanged(QString);
    void btnCancel_click();
    void btnOK_Click();
    void chkNewOneWay_Changed(bool bChecked);
    void cbCompany_currentIndexChanged(int);

signals:
    void routeChangedEvent( RouteChangedEventArgs args);
    void companySelectionChanged(int companyKey);
};

#endif // SEGMENTDLG_H
