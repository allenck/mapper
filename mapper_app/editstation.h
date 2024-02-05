#ifndef EDITSTATION_H
#define EDITSTATION_H

#include <QDialog>
#include "configuration.h"
#include "data.h"
#include "sql.h"
#include "editcomments.h"

namespace Ui {
    class editStation;
}

class EditStation : public QDialog
{
    Q_OBJECT

public:
    explicit EditStation(StationInfo sti, QWidget *parent = 0);
    ~EditStation();
    qint32 segmentId();
//    void setSegmentId(qint32 value);
    qint32 StationId();
    void setStationId(qint32 value);
    QString StationName();
    void setStationName(QString value);
//    LatLng Point();
//    void setPoint(LatLng value);
//    int Index ();
//    void setIndex(qint32 value);
    //void setConfiguration (Configuration * cfg);
    qint32 infoKey ();
    bool WasStationDeleted();
    QDate StartDate();
    void setStartDate(QDate value);
    QDate EndDate();
    void setEndDate(QDate value);
    LatLng latLng();
    void setLatLng(LatLng value);
    void setMarkerType(QString);

private:
    Ui::editStation *ui;
    qint32 _segmentId;
    qint32 _stationKey;
    QString _stationName;
    qint32 _infoKey;
    Configuration *config;
    bool bDirty;
    LatLng _latLng;
    SQL* sql;
    //SegmentInfo si;
    //void setRadioButtons();
    QString markerType;
    void setStationId(StationInfo sti);
    void closeEvent(QCloseEvent*);
    StationInfo _sti;
    bool _bStationDeleted = false;

private slots:
    void txtLatLng_Leave();
    void btnOK_Click();
    void btnEditText_Click();
    void btnDelete_Click();
    void txtStationName_Leave();
    void dateTimePicker2_ValueChanged();
    void txtStationName_edited(QString);
    void On_cbIcons_selectionChanged(int);
};

#endif // EDITSTATION_H
